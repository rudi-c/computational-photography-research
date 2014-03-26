#!/usr/bin/python
"""Reads decision trees generated by Weka and evaluate how quickly and how
accurately they can find peaks.
"""

import getopt
import json
import sys

import evaluatetree
import random
import featuresturn     
import featuresfirststep
import rtools
from coarsefine import *
from direction import Direction
from featuresfirststep import first_three_lens_pos
from scene import Scene, load_scenes

seed = 1
backlash_amount = 4

class BenchmarkParameters(object):

    def __init__(self):
        self.left_right_tree = None
        self.first_size_tree = None
        self.action_tree = None
        self.step_size = 1
        self.perfect_classification = None
        self.backlash = False

    def missing_params(self):
        """Returns whether enough parameters have been set for simulation."""
        missing_trees = (self.left_right_tree is None or 
                         self.first_size_tree is None or
                         self.action_tree is None)
        return self.perfect_classification is None and missing_trees

    def initial_pos_range(self, scene):
        """Possible values for the initial lens position."""
        return range(2 * self.step_size, scene.step_count)
        

class Simulator(object):

    def __init__(self, params, scene, initial_pos):
        self.params = params
        self.scene = scene
        self.initial_pos = initial_pos
        self.status = "none"
        self.visited_positions = []
        self.backlash_count = 0
        if params.perfect_classification is None:
            self.perfect_classification = None
        else:
            self.perfect_classification = \
                params.perfect_classification[scene.filename]

    def _clamp_to_scene(self, lens_pos):
        return max(0, min(self.scene.step_count - 1, lens_pos))

    def _walk_left_fine(self, count=1):
        for _ in range(count):
            self.visited_positions.append(
                self._clamp_to_scene(self.last_position() - 1))

    def _walk_left_coarse(self, count=1):
        for _ in range(count):
            self.visited_positions.append(
                self._clamp_to_scene(self.last_position() - 8))

    def _walk_right_fine(self, count=1):
        for _ in range(count):
            self.visited_positions.append(
                self._clamp_to_scene(self.last_position() + 1))

    def _walk_right_coarse(self, count=1):
        for _ in range(count):
            self.visited_positions.append(
                self._clamp_to_scene(self.last_position() + 8))

    def _walk_fine(self, direction, count=1):
        if direction.is_left():
            self._walk_left_fine(count)
        else:
            self._walk_right_fine(count)

    def _walk_coarse(self, direction, count=1):
        if direction.is_left():
            self._walk_left_coarse(count)
        else:
            self._walk_right_coarse(count)

    def _max_among(self, lens_positions):
        return max(lens_positions, 
                   key=(lambda pos : self.scene.fvalues[pos]))

    def _can_keep_moving(self, lens_pos, direction):
        return ((0 < self.last_position() or direction.is_right()) and
                (self.last_position() < self.scene.step_count - 1
                 or direction.is_left()))

    def _do_local_search(self, lens_pos, direction, rev_direction):
        while self._can_keep_moving(lens_pos, direction):
            self._walk_fine(direction, 1)
            if (self.scene.fvalues[self.last_position()] >
                self.scene.fvalues[lens_pos]):
                # We've seen an increase by moving one extra step, so
                # keep moving.
                lens_pos = self.last_position()
            else:
                # We've seen a decrease. Consider moving just one extra step.
                if self._can_keep_moving(lens_pos, direction):
                    self._walk_fine(direction, 1)
                    if (self.scene.fvalues[self.last_position()] >
                        self.scene.fvalues[lens_pos]):
                        # We've seen an increase by moving one extra step, so
                        # keep moving.
                        lens_pos = self.last_position()
                    else:
                        # Backtrack and stop.
                        self._walk_fine(rev_direction, 2)
                        break
                else:
                    # Backtrack and stop.
                    self._walk_fine(rev_direction, 1)
                    break
        return lens_pos

    def _go_to_max(self, lens_positions):
        current_pos = self.last_position()
        maximum_pos = self._max_among(self.visited_positions)

        # Throw off the location of the maximum by a random amount.
        # Except if we're already at the max position, since then we don't
        # have to move.
        if self.params.backlash and current_pos != maximum_pos:
            maximum_pos += random.randint(-backlash_amount, backlash_amount)

        if maximum_pos < current_pos:
            direction = Direction("left")
        elif maximum_pos > current_pos:
            direction = Direction("right")
        elif current_pos < self.visited_positions[-2]:
            direction = Direction("left")
        else:
            direction = Direction("right")
        rev_direction = direction.reverse()

        # Take as many coarse steps as needed to go back to the maximum
        # without going over it.
        distance = abs(current_pos - maximum_pos)
        coarse_steps = distance / 8
        self._walk_coarse(direction, coarse_steps)

        # Keep going in fine steps to see if we can find a higher position.
        start_pos = self.last_position()
        maximum_pos = self._do_local_search(
            start_pos, direction, rev_direction)
        self.backlash_count += 1

        # If we didn't move further, we might want to look in the other
        # direction too.
        if start_pos == maximum_pos:
            maximum_pos = self._do_local_search(
                self.last_position(), rev_direction, direction)
            self.backlash_count += 1

        self.status = "foundmax"

    def _go_back_to_start(self, distance, direction):
        """Go back to where we were initially at the start of the sweep,
        using only coarse steps. It's fine if we don't quite reach it.
        """
        coarse_steps = distance / 8
        self._walk_coarse(direction, coarse_steps)

    def _get_first_direction(self, initial_positions):
        """Direction in which we should start sweeping initially."""
        first, second, third = self.scene.get_focus_values(initial_positions)
        norm_lens_pos = float(self.initial_pos) / (self.scene.step_count - 1)

        evaluator = featuresfirststep.firststep_feature_evaluator(
            first, second, third, norm_lens_pos)
        return Direction(evaluatetree.evaluate_tree(
            self.params.left_right_tree, evaluator))

    def _can_keep_sweeping(self, direction, current_pos):
        """Whether it's possible to take more steps in the given direction."""
        if direction.is_left():
            return 0 < current_pos
        else:
            return current_pos < self.scene.step_count - 1

    def _sweep(self, direction, initial_position):
        """Sweep the lens in one direction and return a
        tuple (success state, positions visited) along the way.
        """
        visited_positions = []
        focus_measures = [ self.scene.fvalues[initial_position] ]
        smallest = min(self.scene.fvalues)

        current_pos = initial_position

        while self._can_keep_sweeping(direction, current_pos):
            # Move the lens forward.
            current_pos = self._clamp_to_scene(current_pos + direction * 8)
            visited_positions.append(current_pos)

            # Measure the focus at the current lens possible. Simulate
            # a bit of noise that could occur in practice due to camera shake,
            # etc.
            focus_measures.append(self.scene.fvalues[current_pos] + 
                random.random() * 0.05 * smallest)

            # Take at least two steps before we allow turning back.
            if len(focus_measures) < 3:
                continue
       
            if self.perfect_classification is None:
                # Obtain the ML classification at the new lens position.
                evaluator = featuresturn.action_feature_evaluator(
                    focus_measures, self.scene.step_count)
                classification = evaluatetree.evaluate_tree(
                    self.params.action_tree, evaluator)
            else:
                key = featuresturn.make_key(str(direction), initial_position, 
                                            current_pos)
                classification = self.perfect_classification[key]

            if classification != "continue":
                assert (classification == "turn_peak" or
                        classification == "backtrack")
                return classification, visited_positions

        # We've reached an edge, but the decision tree still does not want
        # to turn back, so what do we do now?
        # After thinking a lot about it, I think the best thing to do is to
        # introduce a condition manually. It's a bit ad-hoc, but we really need
        # to be able to handle this case robustly, as there are lot of cases
        # (i.e., landscape shots) where peaks will be at the edge.
        ratio_min_to_max = featuresturn.ratio_min_to_max(
            focus_measures=focus_measures)
        if ratio_min_to_max > 0.8:
            return "backtrack", visited_positions
        else:
            return "turn_peak", visited_positions


    def _backtrack(self, current_lens_pos, previous_direction):
        """From the current lens position, go back to the lens position we
        were at before and look on the other side."""

        new_direction = previous_direction.reverse()

        # Go back to where we started.
        distance_from_initial = abs(self.initial_pos - current_lens_pos)

        # Throw off the location of the starting point by a random amount.
        if self.params.backlash:
            distance_from_initial += random.randint(
                -backlash_amount, backlash_amount)

        self._go_back_to_start(distance_from_initial, 
            new_direction)
        initial_position = self.last_position()

        # Sweep again the other way.
        result, positions = self._sweep(new_direction, initial_position)

        # Don't count the initial positions which we've added already.
        self.visited_positions.extend(positions)

        if result == "turn_peak":
            self._go_to_max(positions)
            self.backlash_count += 1
        elif result == "backtrack":
            # If we need to backtrack a second time, we failed.
            self.status = "failed"
        else:
            assert False

    def evaluate(self):
        """For every scene and every lens position, run a simulation and
        store the statistics."""
        initial_positions = first_three_lens_pos(self.initial_pos, 
                                                 self.params.step_size)
        # The first 3 positions (first 2 steps) count as visited.
        self.visited_positions.extend(initial_positions)

        # Decide initial direction in which to look.
        direction = self._get_first_direction(initial_positions)
        if direction.is_left():
            initial_positions.reverse()
            self.backlash_count += 1

        result, positions = self._sweep(direction, initial_positions[-1])

        # Don't count the initial positions which we've added already.
        self.visited_positions.extend(positions)

        if result == "turn_peak":
            self._go_to_max(self.visited_positions)
            self.backlash_count += 1
        elif result == "backtrack":
            self._backtrack(self.last_position(), direction)
            self.backlash_count += 1
        else:
            assert False

    def last_position(self):
        """Last visited lens position - i.e., current position."""
        return self.visited_positions[-1]

    def is_true_positive(self):
        """Whether a peak was found and the peak is close to a real peak."""
        return (self.status == "foundmax" and 
                self.scene.distance_to_closest_peak(self.last_position()) <= 1)

    def is_false_positive(self):
        """Whether a peak was found and the peak not close to a real peak."""
        return (self.status == "foundmax" and 
                self.scene.distance_to_closest_peak(self.last_position()) > 1)

    def is_true_negative(self):
        """Whether we failed to find a peak and we didn't come 
        close to a real peak."""
        return (self.status == "failed" and 
                all(self.scene.distance_to_closest_peak(pos) > 1
                    for pos in self.visited_positions))

    def is_false_negative(self):
        """Whether we failed to find a peak but we did come 
        close to a real peak."""
        return (self.status == "failed" and 
                any(self.scene.distance_to_closest_peak(pos) <= 1
                    for pos in self.visited_positions))

    def get_evaluation(self):
        """Return whether a simulation for this scene starting at the given
        lens position gave a true/false positive/negative.
        """
        if self.is_true_positive():
            return "true positive"
        if self.is_false_positive():
            return "false positive"
        if self.is_true_negative():
            return "true negative"
        if self.is_false_negative():
            return "false negative"


def print_aligned_data_rows(rows):
    """Print rows of data such that each column is aligned."""
    column_lengths = [ len(max(cols, key=len)) for cols in zip(*rows) ]
    for row in rows:
        print "|".join(" " * (length - len(col)) + col
                       for length, col in zip(column_lengths, row))


def benchmark_scene(params, scene, steps_count_list, backlash_count_list):
    """Runs the simulation for a scene for every lens position and returns a
    tuple (# true positive, # false positive, 
           # true negative, # false negative, # steps)
    """
    evaluators = [Simulator(params, scene, initial_pos) 
                  for initial_pos in params.initial_pos_range(scene)]
    for evaluator in evaluators: 
        evaluator.evaluate()
    t_pos = sum(evaluator.is_true_positive() for evaluator in evaluators)
    t_neg = sum(evaluator.is_false_positive() for evaluator in evaluators)
    f_pos = sum(evaluator.is_true_negative() for evaluator in evaluators)
    f_neg = sum(evaluator.is_false_negative() for evaluator in evaluators)
    total_steps = sum(len(evaluator.visited_positions) 
                      for evaluator in evaluators)
    avg_distance = sum(scene.distance_to_closest_peak(initial_pos)
                       for initial_pos in params.initial_pos_range(scene))
    avg_distance = float(avg_distance) / len(params.initial_pos_range(scene))

    steps_count_list.extend(
        [len(evaluator.visited_positions) for evaluator in evaluators])
    backlash_count_list.extend(
        [evaluator.backlash_count for evaluator in evaluators])

    return (t_pos, t_neg, f_pos, f_neg, 
            float(total_steps) / len(evaluators), avg_distance)


def benchmark_scenes(params, scenes):
    """Runs the simulation for every scene and print the number of true/false
    positive/negatives for each scene.
    """
    if len(scenes) > 1:
        data_rows = [( "filename", "t-pos", "f-pos", "t-neg", 
                       "f-neg", "%", "steps", "avgdist" )]
    else:
        data_rows = []

    steps_count_list = []
    backlash_count_list = []
    sum_t_pos = 0
    sum_f_pos = 0
    sum_t_neg = 0
    sum_f_neg = 0
    sum_perct = 0
    sum_steps = 0
    sum_avgds = 0

    for scene in scenes:
        t_pos, f_pos, t_neg, f_neg, steps, avg_distance = benchmark_scene(
            params, scene, steps_count_list, backlash_count_list)
        perct = float(t_pos) / (t_pos + f_pos + t_neg + f_neg) * 100

        sum_t_pos += t_pos
        sum_f_pos += f_pos
        sum_t_neg += t_neg
        sum_f_neg += f_neg
        sum_perct += perct
        sum_steps += steps
        sum_avgds += avg_distance

        data_rows.append((scene.name, 
            "%d" % t_pos, "%d" % f_pos, "%d" % t_neg, "%d" % f_neg,
            "%.1f" % perct, "%d" % steps, "%.1f" % avg_distance))

    # No need to calculate the average with only one scene.
    if len(scenes) > 1:
        data_rows.append(("average",
            "%.1f" % (float(sum_t_pos) / len(scenes)),
            "%.1f" % (float(sum_f_pos) / len(scenes)),
            "%.1f" % (float(sum_t_neg) / len(scenes)),
            "%.1f" % (float(sum_f_neg) / len(scenes)),
            "%.1f" % (float(sum_perct) / len(scenes)),
            "%.1f" % (float(sum_steps) / len(scenes)),
            "%.1f" % (float(sum_avgds) / len(scenes))))

    print_aligned_data_rows(data_rows)

    # This text file can be loaded later to make a histogram of the number
    # of steps taken.
    rows = [",".join(str(step) for step in steps_count_list[i:i+20])
            for i in range(0, len(steps_count_list), 20)]
    f = open("steps.txt", 'w+')
    f.write(",\n".join(rows))
    f.close()
    # And a histograph of the number of backlashes.
    rows = [",".join(str(step) for step in backlash_count_list[i:i+20])
            for i in range(0, len(backlash_count_list), 20)]
    f = open("backlash.txt", 'w+')
    f.write(",\n".join(rows))
    f.close()


def benchmark_specific(params, scenes, specific_scene):
    """Runs the simulation and printing an R script to visualize the simulation
    at every lens position for a specific scene.
    """
    for scene in scenes:
        if scene.filename == specific_scene:
            for initial_pos in params.initial_pos_range(scene):
                evaluator = Simulator(params, scene, initial_pos)
                evaluator.evaluate()

                print_R_script(scene, initial_pos, 
                    evaluator.visited_positions,
                    evaluator.get_evaluation(),
                    evaluator.last_position())


def print_R_script(scene, lens_pos, visited_positions, evaluation, result):

    print "# %s at %d, %s\n" % (scene.filename, lens_pos, evaluation)

    # Some R functions for plotting.
    rtools.print_set_window_division(1, 1)
    print "library(scales)" # for alpha blending

    rtools.print_plot_focus_measures(scene.fvalues, show_grid=True)

    xs = visited_positions
    ys = [ float(i) / max(10, len(visited_positions))
           for i in range(0, len(visited_positions)) ]

    rtools.print_plot_point_pairs(xs, ys, 25, "blue", "blue", True)

    if result >= 0:
        print "segments(%d, 0.0, %d, 1.0)" % (result, result)

    print "\n# Plot me!\n"


def load_classifications(filename):
    try:
        f = open(filename)
        lines = f.readlines()
        f.close()
    except IOError:
        raise Exception("File %s not found." % filename)
    return json.loads("".join(lines))


def print_script_usage():
    print >> sys.stderr, \
        """Script usage : ./benchmark.py 
           --left-right-tree=<decision tree for deciding left vs right>
           --first-size-tree=<decision tree for deciding first coarse vs fine>
           --action-tree=<decision tree for deciding action to take>]
           [-d, --double-step <double step size used>]
           [--backlash <simulate backlash noise>]
           [--specific-scene=<a scene's filename, will print R script]
           [--perfect-file=<use classification from file instead of tree>]"""


def main(argv):
    # Parse script arguments
    try:
        opts, _ = getopt.getopt(argv, "d:uo",
            ["left-right-tree=", "first-size-tree=",
             "action-tree=", "double-step", "backlash",
             "specific-scene=", "perfect-file=",
             "use-only="])
    except getopt.GetoptError:
        print_script_usage()
        sys.exit(2)

    params = BenchmarkParameters()
    specific_scene = None
    use_only_file = None

    for opt, arg in opts:
        if opt in ("-d", "--double-step"):
            params.step_size = 2
        elif opt in ("-uo", "--use-only"):
            use_only_file = arg
        elif opt == "--left-right-tree":
            params.left_right_tree = evaluatetree.read_decision_tree(
                arg, featuresfirststep.all_features_dict())
        elif opt == "--first-size-tree":
            params.first_size_tree = evaluatetree.read_decision_tree(
                arg, featuresfirststep.all_features_dict())
        elif opt == "--action-tree":
            params.action_tree = evaluatetree.read_decision_tree(
                arg, featuresturn.all_features_dict())
        elif opt == "--specific-scene":
            specific_scene = arg
        elif opt == "--perfect-file":
            params.perfect_classification = load_classifications(arg)
        elif opt == "--backlash":
            params.backlash = True
        else:
            print_script_usage()
            sys.exit(2)

    random.seed(seed)

    # Make sure simulator has everything it needs.
    if params.missing_params():
        print_script_usage()
        sys.exit(2)

    scenes = load_scenes(folder="focusraw/",
        excluded_scenes=["cat.txt", "moon.txt", 
                         "projector2.txt", "projector3.txt"])
    if use_only_file:
        scenes = [scene for scene in scenes if scene.filename == use_only_file]

    if specific_scene is None:
        benchmark_scenes(params, scenes)
    else:
        benchmark_specific(params, scenes, specific_scene)


main(sys.argv[1:])