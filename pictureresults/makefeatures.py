#!/usr/bin/python

# This script will look in a folder called 'FocusMeasures' for .txt
# files with two numbers in each line : (row #, focus value).
#
# The script also expects a file called 'maxima.txt' that contains
# the output of running localMax.exe
#
# If no argument is provided, an ARFF file with every possible feature
# is generated. If arguments are provided, then only those features will
# be used. For example, ./makefeature ratio2 ratio3 will produce rows of
# data with only ratio2,ratio3,class
#
# OPTIONS
# --three-measures : uses features that take 3 focus measures
#                    by default, only 2 measures are used
# --all-features : uses all features, including brackets
# --highest : classify by the location of the highest peak (default)
# --nearest : classify by the location of the nearest peak
# --high-and-near : classify by both height and distance of peaks
# --double-step : use measures at intervals of two lens positions
#                 by default, interval is one lens position
# --dup-edges : duplicates features near the first and last lens positions
# --dup-peaks : duplicates features near peaks

import os
import sys

from featuresleftright import *

def get_arff_header(features):
    """Return a string representing the header of the ARFF file"""

    return ("@RELATION autofocus_dir\n\n"
            + ''.join([ "@ATTRIBUTE " + attr + " " + values + " \n" 
                     for attr, values, _ in features])
            + "@ATTRIBUTE direction {left, right} \n")


def convert_true_false(value):
    """Convert True to 1, False to 0 and leave everything else as it is."""
    if value is True:
        return 1
    elif value is False:
        return 0
    else:
        return value


def get_data_lines(scenes, classifier, position_selector, 
                   features, step_size):
    lines = []

    for scene in scenes:
        for lens_pos in position_selector(scene):
            values = [feature(
                first  = scene.fvalues[lens_pos - 2 * step_size],
                second = scene.fvalues[lens_pos - 1 * step_size],
                third  = scene.fvalues[lens_pos],
                lens_pos = float(lens_pos) / (scene.step_count - 1))
                      for _, _, feature in features]

            classification = (",left" if classifier(scene, lens_pos)
                                      else ",right")
            lines.append(','.join([str(convert_true_false(value))
                                  for value in values])
                      + classification)

    return lines


def create_lens_position_selector(dup_edges, step_size):
    def f(scene):
        # Default lens positions
        lens_positions = range(step_size * 2, scene.step_count)

        if dup_edges:
            # Add some positions close to edges
            lens_positions += range(step_size * 2, 
                int(scene.step_count * 0.2 + step_size * 2))
            lens_positions += range(int(scene.step_count * 0.8), 
                                    scene.step_count)
            # Add more of the closest ones
            lens_positions += range(step_size * 2, 
                int(scene.step_count * 0.1 + step_size * 2))
            lens_positions += range(int(scene.step_count * 0.9), 
                                    scene.step_count)
        return lens_positions
    return f


def main(argv):
    filters = []
    features = two_measure_features
    classifier = highest_on_left
    step_size = 1
    dup_edges = False
    dup_peaks = False

    # Process command line options. Anything remaining will be considered
    # to be filters for features.
    for arg in argv:
        if arg == "--three-measures":
            features = measure_features
        elif arg == "--all-features":
            features = all_features
        elif arg == "--highest":
            # Default
            pass
        elif arg == "--nearest":
            classifier = nearest_on_left
        elif arg == "--high-and-near":
            classifier = highest_and_near_on_left
        elif arg == "--double-step":
            step_size = 2
        elif arg == "--dup-edges":
            dup_edges = True
        else:
            filters.append(arg)

    scenes = load_scenes()

    # Print the contents of the ARFF file to screen (use output
    # redirection to save to file)
    print get_arff_header(features(filters))
    print "@DATA"
    for line in get_data_lines(scenes, 
                               classifier, 
                               create_lens_position_selector(dup_edges, 
                                                             step_size),
                               features(filters), 
                               step_size):
        print line



# Entry point.
main(sys.argv[1:])