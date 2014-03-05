#!/usr/bin/python
"""Reads decision trees generated by Weka and evaluate how quickly and how
accurately they can find peaks.
"""

import getopt
import json
import sys

import evaluatetree
import featuresturn     
import featuresfirststep
import rtools
from coarsefine import *
from direction import Direction
from featuresfirststep import first_three_lens_pos
from scene import Scene, load_scenes


class BenchmarkParameters(object):

    def __init__(self):
        self.left_right_tree = None
        self.first_size_tree = None
        self.action_tree = None
        self.step_size = 1
        self.perfect_classification = None

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

    def _do_local_search(self, maximum_pos, direction, rev_direction):
        while 0 < self.last_position() < self.scene.step_count - 1:
            self._walk_fine(direction, 1)
            if self.scene.fvalues[self.last_position()] > \
                    self.scene.fvalues[maximum_pos]:
                maximum_pos = self.last_position()
            else:
                # Backtrack and stop.
                self._walk_fine(rev_direction, 1)
                break
        return maximum_pos

    def _go_to_max(self, lens_positions):
        current_pos = self.last_position()
        maximum_pos = self._max_among(lens_positions)

        if maximum_pos < current_pos:
            direction = Direction("left")
        elif maximum_pos > current_pos:
            direction = Direction("right")
        elif current_pos < lens_positions[-2]:
            direction = Direction("left")
        else:
            direction = Direction("right")
        rev_direction = direction.reverse()

        # Find minimum number of coarse and fine steps needed to go back to
        # the maximum point.
        distance = abs(current_pos - maximum_pos)
        coarse_steps = distance / 8
        self._walk_coarse(direction, coarse_steps)

        # If the remainder is 6-7, we will need to take 6-7 fine steps and it
        # would in theory be better to take a coarse step and go back the other
        # way. Fortunately, in practice this is very rare so we ignore this
        # optimization on the number of lens movements needed to reach the max.
        fine_steps = distance % 8
        potential_maxs = []
        for _ in range(fine_steps):
            self._walk_fine(direction, 1)
            potential_maxs.append(self.last_position())

        assert self.last_position() == maximum_pos

        if (len(potential_maxs) > 0 and
            self._max_among(potential_maxs) != maximum_pos):
            # Found something better, go back that way.
            self._walk_fine(rev_direction, 
                abs(maximum_pos - self._max_among(potential_maxs)))
            maximum_pos = self._max_among(potential_maxs)
        else:
            # Keep going to see if we can find a higher position.
            maximum_pos = self._do_local_search(
                maximum_pos, direction, rev_direction)
            # If we didn't take any fine steps at all to get to the
            # max lens position, we should look the other way too.
            if fine_steps == 0:
                maximum_pos = self._do_local_search(
                    maximum_pos, rev_direction, direction)

        self.status = "foundmax"

    def _go_back_to_start(self, distance, direction, initial_positions):
        """Go back to where we were initially at the start of the sweep.
        It's fine if we overshoot it a little bit if it saves some steps.
        """
        coarse_steps = distance / 8
        self._walk_coarse(direction, coarse_steps)

        # An extra distance that coarse steps won't reach exactly
        remainder = distance % 8
        if remainder > 4:
            # Take more fine steps to reach initial position.
            self._walk_fine(direction, 8 - remainder)
        else:
            # Take one big coarse step, which will overshoot a bit.
            self._walk_coarse(direction, 1)
            initial_positions.append(self.last_position())

    def _get_first_direction(self, initial_positions):
        """Direction in which we should start sweeping initially."""
        first, second, third = self.scene.get_focus_values(initial_positions)
        norm_lens_pos = float(self.initial_pos) / (self.scene.step_count - 1)

        evaluator = featuresfirststep.firststep_feature_evaluator(
            first, second, third, norm_lens_pos)
        return Direction(evaluatetree.evaluate_tree(
            self.params.left_right_tree, evaluator))

    def _first_step_is_coarse(self, direction, scene, lens_positions):
        """Return whether the first step is a coarse step by evaluating
        the decision tree for it.
        """
        current_pos = lens_positions[-1]

        # Size of the first step determined by another decision tree.
        if 0 < current_pos < scene.step_count - 1:
            first  = self.scene.fvalues[lens_positions[-3]]
            second = self.scene.fvalues[lens_positions[-2]]
            third  = self.scene.fvalues[current_pos]
            norm_lens_pos = float(current_pos) / (self.scene.step_count - 1)
            evaluator = featuresfirststep.firststep_feature_evaluator(
                first, second, third, norm_lens_pos)
            first_size = evaluatetree.evaluate_tree(
                self.params.first_size_tree, evaluator)

            if first_size == "coarse":
                current_pos = self._clamp_to_scene(current_pos + direction * 8)
            else:
                current_pos = min(scene.step_count - 1, 
                                  max(0, current_pos + direction))
            lens_positions.append(current_pos)

            return first_size == "coarse"


    def _sweep(self, direction, initial_positions):
        """Sweep the lens in one direction and return a
        tuple (success state, positions visited along the way.
        """
        lens_positions = list(initial_positions)
        previously_coarse_step = self._first_step_is_coarse(direction,
            self.scene, lens_positions)
        current_pos = lens_positions[-1]

        while 0 < current_pos < self.scene.step_count - 1:
            # Determine next step size.
            focus_values = self.scene.get_focus_values(lens_positions[-3:])
            if previously_coarse_step:
                coarse_now = coarse_if_previously_coarse(*focus_values)
            else:
                coarse_now = coarse_if_previously_fine(*focus_values)

            # Move the lens forward.
            if coarse_now:
                current_pos = self._clamp_to_scene(current_pos + direction * 8)
            else:
                current_pos = self._clamp_to_scene(current_pos + direction)

            lens_positions.append(current_pos)
            previously_coarse_step = coarse_now
       
            if self.perfect_classification is None:
                # Obtain the ML classification at the new lens position.
                evaluator = featuresturn.action_feature_evaluator(direction, 
                    self.scene.fvalues, lens_positions, self.scene.step_count)
                classification = evaluatetree.evaluate_tree(
                    self.params.action_tree, evaluator)
            else:
                key = featuresturn.make_key(str(direction), lens_positions[0], 
                                            current_pos)
                classification = self.perfect_classification[key]

            if classification != "continue":
                assert (classification == "turn_peak" or
                        classification == "backtrack")
                return classification, lens_positions

        # We've reached an edge, but the decision tree still does not want
        # to turn back, so what do we do now?
        # After thinking a lot about it, I think the best thing to do is to
        # introduce a condition manually. It's a bit ad-hoc, but we really need
        # to be able to handle this case robustly, as there are lot of cases
        # (i.e., landscape shots) where peaks will be at the edge.
        ratio_min_to_max = featuresturn.ratio_min_to_max(
            focus_values=self.scene.fvalues, lens_positions=lens_positions)
        if ratio_min_to_max > 0.8:
            return "backtrack", lens_positions
        else:
            return "turn_peak", lens_positions


    def _backtrack(self, current_lens_pos, previous_direction):
        """From the current lens position, go back to the lens position we
        were at before and look on the other side."""

        new_direction = previous_direction.reverse()

        # We need to reinitialize the list of lens positions used as a feature
        # during the sweep algorithm, to avoid mixing it with the positions
        # of the previous sweep. 
        initial_positions = first_three_lens_pos(self.initial_pos, 
                                                 self.params.step_size)
        if new_direction.is_left():
            initial_positions.reverse()

        # Go back to where we started.
        distance_from_initial = abs(self.initial_pos - current_lens_pos)
        self._go_back_to_start(distance_from_initial, 
            new_direction, initial_positions)

        # Sweep again the other way.
        result, positions = self._sweep(new_direction, initial_positions)

        # Don't count the initial positions which we've added already.
        self.visited_positions.extend(positions[len(initial_positions):])

        if result == "turn_peak":
            self._go_to_max(positions)
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

        # Initial sweep
        result, positions = self._sweep(direction, initial_positions)

        # Don't count the initial positions which we've added already.
        self.visited_positions.extend(positions[len(initial_positions):])

        if result == "turn_peak":
            self._go_to_max(positions)
        elif result == "backtrack":
            self._backtrack(positions[-1], direction)
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


def benchmark_scene(params, scene, steps_count_list):
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
    sum_t_pos = 0
    sum_f_pos = 0
    sum_t_neg = 0
    sum_f_neg = 0
    sum_perct = 0
    sum_steps = 0
    sum_avgds = 0

    for scene in scenes:
        t_pos, f_pos, t_neg, f_neg, steps, avg_distance = \
            benchmark_scene(params, scene, steps_count_list)
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
           [--specific-scene=<a scene's filename, will print R script]
           [--perfect-file=<use classification from file instead of tree>]"""


def main(argv):
    # Parse script arguments
    try:
        opts, _ = getopt.getopt(argv, "d:uo",
            ["left-right-tree=", "first-size-tree=",
             "action-tree=", "double-step",
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
        else:
            print_script_usage()
            sys.exit(2)

    # Make sure simulator has everything it needs.
    if params.missing_params():
        print_script_usage()
        sys.exit(2)

    scenes = load_scenes(folder="focusraw/",
        excluded_scenes=["cat.txt", "moon.txt"])
    if use_only_file:
        scenes = [scene for scene in scenes if scene.filename == use_only_file]

    if specific_scene is None:
        benchmark_scenes(params, scenes)
    else:
        benchmark_specific(params, scenes, specific_scene)


main(sys.argv[1:])