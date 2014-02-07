#!/usr/bin/python

import os
import random
import sys

from scene import *
from coarsefine import *
from featuresturn import *

def print_instance(scene, features, lens_positions, classification):
    kwargs = { "lens_positions" : lens_positions, 
               "focus_values" : scene.measuresValues,
               "total_positions" : scene.measuresCount }

    # Uniformly randomly select only a subset of instances.
    if random.randint(0, 10) != 1:
        return

    print ",".join([ "%.3f" % feature(**kwargs) for _, feature in features ]) \
          + "," + ["continue", "turn_peak", "backtrack"][classification]


def simulate_sweep(scene, features, initial_lens_positions, 
                   direction, classifier,
                   peak_handling, backtrack_handling):
    assert abs(direction) == 1

    lens_positions = initial_lens_positions
    current_pos = lens_positions[-1]
    previously_coarse_step = False

    while current_pos > 0 and current_pos < scene.measuresCount - 1:
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

        print_instance(scene, features, lens_positions, 
            classifier(initial_lens_positions[0], current_pos,
                scene.measuresValues, scene.maxima, 
                peak_handling, backtrack_handling))

        if coarse_now:
            current_pos = min(scene.measuresCount - 1, 
                              max(0, current_pos + direction * 8))
        else:
            current_pos = min(scene.measuresCount - 1, 
                              max(0, current_pos + direction))
        lens_positions.append(current_pos)
        previously_coarse_step = coarse_now

    print_instance(scene, features, lens_positions, 
        classifier(initial_lens_positions[0], current_pos,
            scene.measuresValues, scene.maxima, 
            peak_handling, backtrack_handling))


def simulate_scenes(scenes, features, step_size, 
                    peak_handling, backtrack_handling):
    for scene in scenes:

        # We simulate a sweep at each starting position.
        for lens_pos in range(step_size * 2 + 1, scene.measuresCount - 1):

            # Going left. Note that we are assuming that the initial
            # three focus measures were obtained by moving right.
            initial_lens_positions = [ lens_pos, lens_pos - step_size,
                                       lens_pos - step_size * 2,
                                       lens_pos - step_size * 2 - 1 ]
            simulate_sweep(scene, features, initial_lens_positions, 
                -1, get_move_left_classification,
                peak_handling, backtrack_handling)

            # Right
            initial_lens_positions = [ lens_pos - step_size * 2,
                                       lens_pos - step_size, lens_pos,
                                       lens_pos + 1 ]
            simulate_sweep(scene, features, initial_lens_positions, 
                +1, get_move_right_classification,
                peak_handling, backtrack_handling)


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

    peak_handling = PeakHandling.ALWAYSTURN
    backtrack_handling = BacktrackHandling.NOPEAKSONLY

    # Process command line options. Anything remaining will be considered
    # to be filters for features.
    for arg in argv:
        if arg == "--double-step":
            step_size = 2
        elif arg == "--closest-peak":
            peak_handling = PeakHandling.CLOSEST
        elif arg == "--backtrack-faster":
            backtrack_handling = BacktrackHandling.FASTER
        else:
            filters.append(arg)

    scenes = load_scenes()
    load_maxima_into_measures(scenes)

    # Print the contents of the ARFF file to screen (use output
    # redirection to save to file)
    print get_arff_header(features(filters))
    print "@DATA"
    simulate_scenes(scenes, features(filters), step_size, 
                    peak_handling, backtrack_handling)



main(sys.argv[1:])