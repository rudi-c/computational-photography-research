#!/usr/bin/python

import os
import random
import sys

from scene import *
from coarsefine import *
from featuresturn import *
from rtools import *

def make_instance(scene, features, params, instances, direction,
                  lens_positions, classification, weight):
    """Adds a new instance by evaluate the features on the data acquired so
    far. If the outlier handling mode is SAMPLING, an array of
    (evaluated features, classification) is returned. If the outlier handling
    mode is WEIGHTING, an array (evaluated features, classification, weights)
    is returned."""
    evaluator = action_feature_evaluator(direction, scene.measuresValues,
        lens_positions, scene.measuresCount)

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
    count_continue  = len( [ True for _, classification in instances
                             if classification == Action.CONTINUE ] )
    count_turn_peak = len( [ True for _, classification in instances
                             if classification == Action.TURN_PEAK ] )
    count_backtrack = len( [ True for _, classification in instances
                             if classification == Action.BACKTRACK ] )

    min_count = min(count_continue, count_turn_peak, count_backtrack)
    probabilities = { Action.CONTINUE  : float(min_count) / count_continue,
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
    assert all( [ probabilities[k] > 0.9 for k in probabilities.keys() ] )


def get_balancing_weight_factors(instances):
    """Calculate the factors by which to multiply weights to get an equal
    sum of weights in each class."""
    sum_continue  = sum( [ w for _, classification, w in instances
                           if classification == Action.CONTINUE ] )
    sum_turn_peak = sum( [ w for _, classification, w in instances
                           if classification == Action.TURN_PEAK ] )
    sum_backtrack = sum( [ w for _, classification, w in instances
                           if classification == Action.BACKTRACK ] )

    min_sum = min(sum_continue, sum_turn_peak, sum_backtrack)
    factors = { Action.CONTINUE  : float(min_sum) / sum_continue,
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
    assert all( [ factors[k] > 0.9 for k in factors.keys() ] )


def simulate_sweep(scene, features, instances, initial_lens_positions, 
                   direction, classifier, params):
    assert abs(direction) == 1

    lens_positions = initial_lens_positions
    current_pos = lens_positions[-1]
    previously_coarse_step = True

    keep_ratio = 1.0

    while current_pos > 0 and current_pos < scene.measuresCount - 1:
        # Determine next step size.
        if previously_coarse_step:
            coarse_now = coarse_if_previously_coarse(
                scene.measuresValues[current_pos],
                scene.measuresValues[lens_positions[-2]],
                scene.measuresValues[lens_positions[-3]])
        else:
            coarse_now = coarse_if_previously_fine(
                scene.measuresValues[current_pos],
                scene.measuresValues[lens_positions[-2]],
                scene.measuresValues[lens_positions[-3]])

        # Move the lens forward.
        if coarse_now:
            current_pos = min(scene.measuresCount - 1, 
                              max(0, current_pos + direction * 8))
        else:
            current_pos = min(scene.measuresCount - 1, 
                              max(0, current_pos + direction))
        lens_positions.append(current_pos)
        previously_coarse_step = coarse_now
   
        # Obtain the correct classification at the new lens position.
        classification = classifier(initial_lens_positions[0], current_pos,
                                    scene.measuresValues, scene.maxima, params)

        # Reduce the weight as we go along.
        if classification == Action.CONTINUE:
            keep_ratio *= params.continueRatio
        else:
            assert classification == Action.TURN_PEAK or    \
                   classification == Action.BACKTRACK
            keep_ratio *= params.turnbackRatio

        # Create a new instance for this lens position.
        make_instance(scene, features, params, instances, direction,
                      lens_positions, classification, keep_ratio)

    return lens_positions


def simulate_scenes(scenes, features, step_size, params):
    instances = []
    for scene in scenes:

        # We simulate a sweep at each starting position.
        for lens_pos in range(step_size * 2, scene.measuresCount):

            # Going left. Note that we are assuming that the initial
            # three focus measures were obtained by moving right.
            initial_lens_positions = [ lens_pos, lens_pos - step_size,
                                       lens_pos - step_size * 2 ]
            simulate_sweep(scene, features, instances, initial_lens_positions, 
                -1, get_move_left_classification, params)

            # Right
            initial_lens_positions = [ lens_pos - step_size * 2,
                                       lens_pos - step_size, lens_pos ]
            simulate_sweep(scene, features, instances, initial_lens_positions, 
                +1, get_move_right_classification, params)

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

    print "# " + scene.fileName + "\n"

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
    print_set_window_division(1, 1)
    print "library(scales)" # for alpha blending
    print_plot_focus_measures(scene.measuresValues, (-0.1, 1))

    print_plot_point_pairs(xs_continue, ys_continue, 25, "black", "black")
    print_plot_point_pairs(xs_turn_peak, ys_turn_peak, 25, "green", "green")
    print_plot_point_pairs(xs_backtrack, ys_backtrack, 25, "red", "red")

    print "\n# Plot me!\n"


def simulate_samples(scenes, features, step_size, params):
    """Does a few random simulations and prints an R 
    script for visualization."""

    # Need to set the parameters so that all the instances are kept.
    params.outlierHandling = OutlierHandling.WEIGHTING
    params.uniformSamplingRate = 1.0

    for i in range(0, 20):
        scene = random.choice(scenes)
        instances = []

        # Always sweep right to make things easier. So invert the focus
        # measures half the time.
        if random.random() < 0.5:
            scene = scene.inverse_copy()

        lens_pos = random.randint(step_size * 2 + 1, scene.measuresCount - 1)

        initial_lens_positions = [ lens_pos - step_size * 2,
                                   lens_pos - step_size, lens_pos ]
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
            print ",".join([ "%.3f" % f for f in features ] ) \
                  + "," + class_names[classification]
    elif params.outlierHandling == OutlierHandling.WEIGHTING:
        for features, classification, weights in instances:
            print ",".join([ "%.3f" % f for f in features ] ) \
                  + "," + class_names[classification] + ",{%.3f}" % weights
    else:
        assert False


def get_arff_header(features):
    """Return a string representing the header of the ARFF file"""

    return "@RELATION autofocus_action\n\n" \
           + ''.join([ "@ATTRIBUTE " + attr + " numeric\n" 
                     for attr, _ in features]) \
           + "@ATTRIBUTE action {continue, turn_peak, backtrack } \n"


def main(argv):

    if not os.path.isdir(scenes_folder):
        print scenes_folder + " folder not found."
        return

    features = all_features
    filters = []
    step_size = 1
    show_random_sample = False
    seed = 1

    params = ParameterSet()

    # Process command line options. Anything remaining will be considered
    # to be filters for features.
    for arg in argv:
        if arg == "--double-step":
            step_size = 2
        elif arg == "--closest-peak":
            params.peakHandling = PeakHandling.CLOSEST
        elif arg == "--backtrack-faster":
            params.backtrackHandling = BacktrackHandling.FASTER
        elif arg == "--use-weights":
            params.outlierHandling = OutlierHandling.WEIGHTING
            params.uniformSamplingRate = 0.10
        elif arg == "--show-random-sample":
            show_random_sample = True
        else:
            filters.append(arg)

    random.seed(seed)

    scenes = load_scenes()
    load_maxima_into_measures(scenes)

    if show_random_sample:
        simulate_samples(scenes, features(filters), step_size, params)
    else:
        simulate_full(scenes, features(filters), step_size, params)


main(sys.argv[1:])