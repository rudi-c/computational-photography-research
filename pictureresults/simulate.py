#!/usr/bin/python
"""
Simulates sweeping through scenes in either direction in order to create
instances of data that will be used to train a classifier for deciding
whether we should turn back to peak, backtrack or continue.
"""

import getopt
import random
import sys

import coarsefine
import rtools
from direction import Direction
from scene import load_scenes
from featuresfirststep import first_three_lens_pos
from featuresturn import *

seed = 1

# How many times more instances with the classification "continue" we want
# compared to one of the other two classifications. We generally want an
# unbalanced dataset so that the decision tree is biased towards "continue".
# It's more acceptable to take a few more steps than to backtrack too early
# and fail.
continue_multiplier = 2.0

def make_instance(scene, features, params, instances, direction,
                  focus_measures, classification, weight):
    """Adds a new instance by evaluate the features on the data acquired so
    far. If the outlier handling mode is SAMPLING, an array of
    (evaluated features, classification) is returned. If the outlier handling
    mode is WEIGHTING, an array (evaluated features, classification, weights)
    is returned."""
    evaluator = action_feature_evaluator(focus_measures, scene.step_count)

    # Randomly select only a subset of instances based on their weight.
    if params.outlierHandling == OutlierHandling.SAMPLING:
        if random.random() <= weight * params.uniformSamplingRate:
            instance = ( [ evaluator(feature) for _, feature in features ], 
                         classification )
            instances.append(instance)
    elif params.outlierHandling == OutlierHandling.WEIGHTING:
        if random.random() <= params.uniformSamplingRate:
            instance = ( [ evaluator(feature) for _, feature in features ], 
                           classification, weight )
            instances.append(instance)
    else:
        assert False


def get_balancing_probabilities(instances):
    """Calculate the probabilities of needed to removing instances of each
    class to get a balanced dataset."""
    count_continue  = sum(classification == Action.CONTINUE 
                          for _, classification in instances)
    count_turn_peak = sum(classification == Action.TURN_PEAK
                          for _, classification in instances)
    count_backtrack = sum(classification == Action.BACKTRACK
                          for _, classification in instances)

    min_count = min(count_continue, count_turn_peak, count_backtrack)
    probabilities = { Action.CONTINUE  : continue_multiplier * 
                                         float(min_count) / count_continue,
                      Action.TURN_PEAK : float(min_count) / count_turn_peak,
                      Action.BACKTRACK : float(min_count) / count_backtrack }
    return probabilities


def balance_dataset_sampling(instances):
    """Balance the dataset so that each class has approximately
    the same number of instances."""
    probabilities = get_balancing_probabilities(instances)
    new_instances = [ (features, classification) 
                      for features, classification in instances 
                      if random.random() < probabilities[classification] ]

    return new_instances


def assert_balanced_sampling(instances):
    """Make sure we have approximately the same number of instances of each
    class, within 10%"""
    probabilities = get_balancing_probabilities(instances)
    total = continue_multiplier + 2
    assert (probabilities[Action.CONTINUE] > continue_multiplier / total * 0.9
            and probabilities[Action.TURN_PEAK] > 1.0 / total * 0.9
            and probabilities[Action.BACKTRACK] > 1.0 / total * 0.9)


def get_balancing_weight_factors(instances):
    """Calculate the factors by which to multiply weights to get an equal
    sum of weights in each class."""
    sum_continue  = sum(w for _, classification, w in instances
                        if classification == Action.CONTINUE)
    sum_turn_peak = sum(w for _, classification, w in instances
                        if classification == Action.TURN_PEAK)
    sum_backtrack = sum(w for _, classification, w in instances
                        if classification == Action.BACKTRACK)

    min_sum = min(sum_continue, sum_turn_peak, sum_backtrack)
    factors = { Action.CONTINUE  : continue_multiplier *
                                   float(min_sum) / sum_continue,
                Action.TURN_PEAK : float(min_sum) / sum_turn_peak,
                Action.BACKTRACK : float(min_sum) / sum_backtrack }
    return factors


def balance_dataset_weighting(instances):
    """Balance the dataset so the sum of weights for each class is the same"""
    factors = get_balancing_weight_factors(instances)
    new_instances = [ (features, classification, w * factors[classification]) 
                      for features, classification, w in instances ]
    return new_instances


def assert_balanced_weighting(instances):
    """Make sure we have approximately the same the sum of weights in 
    instances of each class, within 10%"""
    factors = get_balancing_weight_factors(instances)
    total = continue_multiplier + 2
    assert (factors[Action.CONTINUE] > continue_multiplier / total * 0.9
            and factors[Action.TURN_PEAK] > 1.0 / total * 0.9
            and factors[Action.BACKTRACK] > 1.0 / total * 0.9)


def simulate_sweep(scene, features, instances, initial_lens_position, 
                   direction, classifier, params):
    """Perform the sweep, taking small and large steps as needed."""
    focus_measures = [ scene.fvalues[initial_lens_position] ]
    current_pos = initial_lens_position

    keep_ratio = 1.0

    smallest = min(scene.fvalues)

    while current_pos > 0 and current_pos < scene.step_count - 1:
        # Move the lens forward.
        current_pos = min(scene.step_count - 1, 
                          max(0, current_pos + direction * 8))

        # Measure the focus at the current lens possible. Simulate
        # a bit of noise that could occur in practice due to camera shake,
        # etc.
        focus_measures.append(scene.fvalues[current_pos] + 
                              random.random() * 0.05 * smallest)

        # Obtain the correct classification at the new lens position.
        classification = classifier(initial_lens_position, current_pos,
                                    scene.fvalues, scene.maxima, params)

        # Reduce the weight as we go along.
        if classification == Action.CONTINUE:
            keep_ratio *= params.continueRatio
        else:
            assert (classification == Action.TURN_PEAK or
                    classification == Action.BACKTRACK)
            keep_ratio *= params.turnbackRatio

        # Create a new instance for this lens position after the second step.
        if len(focus_measures) >= 3:
            make_instance(scene, features, params, instances, direction,
                          focus_measures, classification, keep_ratio)


def simulate_scenes(scenes, features, step_size, params):
    """Perform simulation for every scene."""
    instances = []

    for scene in scenes:
        # We simulate a sweep at each starting position. Note that we are 
        # assuming that the initial three focus measures were obtained 
        # by moving right.
        for lens_pos in range(step_size * 2, scene.step_count):

            # Going right.
            simulate_sweep(scene, features, instances, lens_pos, 
                Direction("right"), get_move_right_classification, params)

            # Going left. 
            simulate_sweep(scene, features, instances, lens_pos, 
                Direction("left"), get_move_left_classification, params)

    # Balance datasets.
    if params.outlierHandling == OutlierHandling.SAMPLING:
        instances = balance_dataset_sampling(instances)
        assert_balanced_sampling(instances)
    elif params.outlierHandling == OutlierHandling.WEIGHTING:
        instances = balance_dataset_weighting(instances)
        assert_balanced_weighting(instances)
    else:
        assert False

    return instances


def print_R_script(scene, lens_positions, instances):

    assert len(lens_positions) == len(instances)

    print "# " + scene.filename + "\n"

    xs_continue = []
    ys_continue = []
    xs_turn_peak = []
    ys_turn_peak = []
    xs_backtrack = []
    ys_backtrack = []

    for lens_pos, (_, classif, weight) in zip(lens_positions, instances):
        if classif == Action.CONTINUE:
            xs_continue.append(lens_pos)
            ys_continue.append(weight)
        if classif == Action.TURN_PEAK:
            xs_turn_peak.append(lens_pos)
            ys_turn_peak.append(weight)
        if classif == Action.BACKTRACK:
            xs_backtrack.append(lens_pos)
            ys_backtrack.append(weight)

    # Some R functions for plotting.
    rtools.print_set_window_division(1, 1)
    print "library(scales)" # for alpha blending
    rtools.print_plot_focus_measures(scene.fvalues, (-0.1, 1))

    rtools.print_plot_point_pairs(xs_continue, 
        ys_continue, 25, "black", "black")
    rtools.print_plot_point_pairs(xs_turn_peak, 
        ys_turn_peak, 25, "green", "green")
    rtools.print_plot_point_pairs(xs_backtrack, 
        ys_backtrack, 25, "red", "red")

    print "\n# Plot me!\n"


def simulate_samples(scenes, features, step_size, params):
    """Does a few random simulations and prints an R 
    script for visualization."""

    # Need to set the parameters so that all the instances are kept.
    params.outlierHandling = OutlierHandling.WEIGHTING
    params.uniformSamplingRate = 1.0

    for _ in range(20):
        scene = random.choice(scenes)
        instances = []

        # Always sweep right to make things easier. So invert the focus
        # measures half the time.
        if random.random() < 0.5:
            scene = scene.inverse_copy()

        lens_pos = random.randint(step_size * 2 + 1, scene.step_count - 1)

        initial_lens_positions = first_three_lens_pos(lens_pos, step_size)
        lens_positions = simulate_sweep(scene, features, instances,
            initial_lens_positions, +1, get_move_right_classification, params)

        # Don't want the initial three positions we used to decide whether
        # to move left or right anymore.
        lens_positions = lens_positions[3:]

        print_R_script(scene, lens_positions, instances)


def simulate_full(scenes, features, step_size, params):

    # Print the contents of the ARFF file to screen (use output
    # redirection to save to file)
    print get_arff_header(features)
    print "@DATA"
    instances = simulate_scenes(scenes, features, step_size, params)

    if params.outlierHandling == OutlierHandling.SAMPLING:
        for features, classification in instances:
            print "%s,%s" % (",".join("%.3f" % f for f in features),
                             class_names[classification])
    elif params.outlierHandling == OutlierHandling.WEIGHTING:
        for features, classification, weights in instances:
            print "%s,%s,{%.3f}" % (",".join("%.3f" % f for f in features),
                                    class_names[classification], weights)
    else:
        assert False


def get_arff_header(features):
    """Return a string representing the header of the ARFF file"""

    return ("@RELATION autofocus_action\n\n"
            + ''.join("@ATTRIBUTE " + attr + " numeric\n" 
                      for attr, _ in features)
            + "@ATTRIBUTE action {continue, turn_peak, backtrack } \n")


def print_script_usage():
    print >> sys.stderr, \
        """Script usage : ./simulate.py 
           [-d, --double-step <double step size used>]
           [--closest-peak, --backtrack-faster, -use-weights : simulation options]
           [--show-random-sample : instead of full simulation, randomly
                                   simulate a few scenes for plotting ]
           [-lv, --leave-out=<name of a file to leave out>]"""


def main(argv):
    # Parse script arguments
    try:
        opts, _ = getopt.getopt(argv, "d:lv",
            ["double-step", "closest-peak", "backtrack-faster",
             "use-weights", "show-random-sample",
             "leave-out="])
    except getopt.GetoptError:
        print_script_usage()
        sys.exit(2)

    features = all_features
    step_size = 1
    show_random_sample = False
    leave_out = ""

    params = ParameterSet()

    # Process command line options. Anything remaining will be considered
    # to be filters for features.
    for opt, arg in opts:
        if opt in ("-d", "--double-step"):
            step_size = 2
        elif opt in ("-lv", "--leave-out"):
            leave_out = arg
        elif opt == "--closest-peak":
            params.peakHandling = PeakHandling.CLOSEST
        elif opt == "--backtrack-faster":
            params.backtrackHandling = BacktrackHandling.FASTER
        elif opt == "--use-weights":
            params.outlierHandling = OutlierHandling.WEIGHTING
            params.uniformSamplingRate = 0.10
        elif opt == "--show-random-sample":
            show_random_sample = True
        else:
            print_script_usage()
            sys.exit(2)

    random.seed(seed)

    scenes = load_scenes(folder="focusraw/",
        excluded_scenes=["cat.txt", "moon.txt",
                         "projector2.txt", "projector3.txt", leave_out])

    if show_random_sample:
        simulate_samples(scenes, features(), step_size, params)
    else:
        simulate_full(scenes, features(), step_size, params)


main(sys.argv[1:])