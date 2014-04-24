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
from cameramodel import CameraModel
from coarsefine import *
from direction import Direction
from featuresfirststep import first_three_lens_pos
from scene import Scene, load_scenes

seed = 1
two_step_tolerance = False

class BenchmarkParameters(object):

    def __init__(self):
        self.left_right_tree = None
        self.action_tree = None
        self.step_size = 1
        self.perfect_classification = None
        self.backlash = False
        self.noise = False

    def missing_params(self):
        """Returns whether enough parameters have been set for simulation."""
        missing_trees = (self.left_right_tree is None or 
                         self.action_tree is None)
        return self.perfect_classification is None and missing_trees

    def initial_pos_range(self, scene):
        """Possible values for the initial lens position."""
        return range(0, scene.step_count - 2 * self.step_size)
        

class Simulator(object):

    def __init__(self, params, scene, initial_pos):
        self.scene = scene
        self.params = params
        self.initial_pos = initial_pos
        self.status = "none"

        self.camera = CameraModel(scene, initial_pos,
            simulate_backlash=self.params.backlash,
            simulate_noise=self.params.noise)

        if params.perfect_classification is None:
            self.perfect_classification = None
        else:
            self.perfect_classification = \
                params.perfect_classification[scene.filename]

    def _do_local_search(self, direction, rev_direction):
        """Perform a local search (incremental hillclimbing in a 
        given direction). The hillclimbing has a tolerance of two steps.
        i.e., Up to two steps that don't increase the focus value can be taken
        before we stop climbing."""
        while not self.camera.will_hit_edge(direction):
            prev_fmeasure = self.camera.last_fmeasure()
            self.camera.move_fine(direction)

            if self.camera.last_fmeasure() < prev_fmeasure:
                if (two_step_tolerance and 
                    not self.camera.will_hit_edge(direction)):
                    # We've seen a decrease. Consider moving one extra step.
                    prev_fmeasure = self.camera.last_fmeasure()
                    self.camera.move_fine(direction)

                    if (self.camera.last_fmeasure() < prev_fmeasure):
                        # Seen a decrease again, backtrack and stop.
                        self.camera.move_fine(rev_direction, 2)
                        break
                else:
                    # Backtrack and stop.
                    self.camera.move_fine(rev_direction)
                    break

    def _go_to_max(self):
        """Return to the location of the largest focus value seen so far and
        perform a local search to find the exact location of the peak."""
        current_pos = self.camera.last_position()
        maximum_pos = max(self.camera.visited_positions,
            key=(lambda pos : self.camera.get_fvalue(pos)))

        if maximum_pos < current_pos:
            direction = Direction("left")
        elif maximum_pos > current_pos:
            direction = Direction("right")
        elif current_pos < self.camera.visited_positions[-2]:
            direction = Direction("left")
        else:
            direction = Direction("right")
        rev_direction = direction.reverse()

        # Take as many coarse steps as needed to go back to the maximum
        # without going over it.
        distance = abs(current_pos - maximum_pos)
        coarse_steps = distance / 8

        self.camera.move_coarse(direction, coarse_steps)

        # Keep going in fine steps to see if we can find a higher position.
        start_pos = self.camera.last_position()
        self._do_local_search(direction, rev_direction)

        # If we didn't move further, we might want to look in the other
        # direction too.
        if start_pos == self.camera.last_position():
            self._do_local_search(rev_direction, direction)

        self.status = "foundmax"

    def _get_first_direction(self):
        """Direction in which we should start sweeping initially."""
        first, second, third = self.camera.get_fvalues(
            self.camera.visited_positions[-3:])
        norm_lens_pos = float(self.initial_pos) / (self.scene.step_count - 1)

        evaluator = featuresfirststep.firststep_feature_evaluator(
            first, second, third, norm_lens_pos)
        return Direction(evaluatetree.evaluate_tree(
            self.params.left_right_tree, evaluator))

    def _sweep(self, direction):
        """Sweep the lens in one direction and return a
        tuple (success state, number of steps taken) along the way.
        """
        initial_position = self.camera.last_position()
        sweep_fvalues = [ self.camera.last_fmeasure() ]

        while not self.camera.will_hit_edge(direction):
            # Move the lens forward.
            self.camera.move_coarse(direction)
            sweep_fvalues.append(self.camera.last_fmeasure())

            # Take at least two steps before we allow turning back.
            if len(sweep_fvalues) < 3:
                continue
       
            if self.perfect_classification is None:
                # Obtain the ML classification at the new lens position.
                evaluator = featuresturn.action_feature_evaluator(
                    sweep_fvalues, self.scene.step_count)
                classification = evaluatetree.evaluate_tree(
                    self.params.action_tree, evaluator)
            else:
                key = featuresturn.make_key(str(direction), initial_position, 
                                            self.camera.last_position())
                classification = self.perfect_classification[key]

            if classification != "continue":
                assert (classification == "turn_peak" or
                        classification == "backtrack")
                return classification, len(sweep_fvalues) - 1

        # We've reached an edge, but the decision tree still does not want
        # to turn back, so what do we do now?
        # After thinking a lot about it, I think the best thing to do is to
        # introduce a condition manually. It's a bit ad-hoc, but we really need
        # to be able to handle this case robustly, as there are lot of cases
        # (i.e., landscape shots) where peaks will be at the edge.
        min_val = min(self.camera.get_fvalues(self.camera.visited_positions))
        max_val = max(self.camera.get_fvalues(self.camera.visited_positions))
        if float(min_val) / max_val > 0.8:
            return "backtrack", len(sweep_fvalues) - 1
        else:
            return "turn_peak", len(sweep_fvalues) - 1


    def _backtrack(self, previous_direction, step_count):
        """From the current lens position, go back to the lens position we
        were at before and look on the other side."""

        new_direction = previous_direction.reverse()

        # Go back to where we started.
        self.camera.move_coarse(new_direction, step_count)

        # Sweep again the other way.
        result, step_count = self._sweep(new_direction)

        if result == "turn_peak":
            self._go_to_max()
        elif result == "backtrack":
            # If we need to backtrack a second time, we failed.
            self.status = "failed"
        else:
            assert False

    def evaluate(self):
        """For every scene and every lens position, run a simulation and
        store the statistics."""

        # Take the first two steps, as to get three focus measures with which
        # to decide which direction to sweep.
        self.camera.move_fine(Direction("right"), 2)

        # Decide initial direction in which to look.
        direction = self._get_first_direction()
            
        # Search in that direction.
        result, step_count = self._sweep(direction)

        if result == "turn_peak":
            self._go_to_max()
        elif result == "backtrack":
            self._backtrack(direction, step_count)
        else:
            assert False

    def is_true_positive(self):
        """Whether a peak was found and the peak is close to a real peak."""
        return (self.status == "foundmax" and 
                self.scene.distance_to_closest_peak(
                    self.camera.last_position()) <= 1)

    def is_false_positive(self):
        """Whether a peak was found and the peak not close to a real peak."""
        return (self.status == "foundmax" and 
                self.scene.distance_to_closest_peak(
                    self.camera.last_position()) > 1)

    def is_true_negative(self):
        """Whether we failed to find a peak and we didn't come 
        close to a real peak."""
        return (self.status == "failed" and 
                all(self.scene.distance_to_closest_peak(pos) > 1
                    for pos in self.camera.visited_positions))

    def is_false_negative(self):
        """Whether we failed to find a peak but we did come 
        close to a real peak."""
        return (self.status == "failed" and 
                any(self.scene.distance_to_closest_peak(pos) <= 1
                    for pos in self.camera.visited_positions))

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
    total_steps = sum(len(evaluator.camera.visited_positions) 
                      for evaluator in evaluators)
    avg_distance = sum(scene.distance_to_closest_peak(initial_pos)
                       for initial_pos in params.initial_pos_range(scene))
    avg_distance = float(avg_distance) / len(params.initial_pos_range(scene))

    steps_count_list.extend(
        [len(evaluator.camera.visited_positions) for evaluator in evaluators])
    backlash_count_list.extend(
        [evaluator.camera.backlash_count for evaluator in evaluators])

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
            "%.1f" % perct, "%.1f" % steps, "%.1f" % avg_distance))

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

                evaluator.camera.print_script(evaluator.get_evaluation())


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
           --action-tree=<decision tree for deciding action to take>]
           [--low-light <evaluate low light benchmarks>]
           [-d, --double-step <double step size used>]
           [--backlash <simulate backlash noise>]
           [--specific-scene=<a scene's filename, will print R script]
           [--perfect-file=<use classification from file instead of tree>]"""


def main(argv):
    # Parse script arguments
    try:
        opts, _ = getopt.getopt(argv, "d:uo",
            ["left-right-tree=", "lowlight", "low-light",
             "lowlightgauss", "low-light-gauss",
             "action-tree=", "double-step", "backlash", "noise",
             "specific-scene=", "perfect-file=",
             "use-only="])
    except getopt.GetoptError:
        print_script_usage()
        sys.exit(2)

    params = BenchmarkParameters()
    specific_scene = None
    use_only_file = None
    scenes_folder = "focusraw/"

    for opt, arg in opts:
        if opt in ("-d", "--double-step"):
            params.step_size = 2
            raise Exception("Simulator does not support double step size yet.")
        elif opt in ("-uo", "--use-only"):
            use_only_file = arg
        elif opt in ("--lowlight", "--low-light"):
            scenes_folder = "lowlightraw/"
        elif opt in ("--lowlightgauss", "--low-light-gauss"):
            scenes_folder = "lowlightgaussraw/"
        elif opt == "--left-right-tree":
            params.left_right_tree = evaluatetree.read_decision_tree(
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
        elif opt == "--noise":
            params.noise = True
        else:
            print_script_usage()
            sys.exit(2)

    random.seed(seed)

    # Make sure simulator has everything it needs.
    if params.missing_params():
        print_script_usage()
        sys.exit(2)

    scenes = load_scenes(folder=scenes_folder,
        excluded_scenes=["cat.txt", "moon.txt", 
                         "projector2.txt", "projector3.txt"])
    if use_only_file:
        scenes = [scene for scene in scenes if scene.filename == use_only_file]

    if specific_scene is None:
        benchmark_scenes(params, scenes)
    else:
        benchmark_specific(params, scenes, specific_scene)


main(sys.argv[1:])