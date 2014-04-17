#!/usr/bin/python
"""
This script generates an ARFF training file containing data for creating
a classifier that decides if our first step for the sweeping part of the
algorithm should be a fine step or a coarse step.

This script will look in the folder defined in scene.py for .txt
files with two numbers in each line : (row #, focus value).

The script also expects a file called 'maxima.txt' that contains
the output of running localMax.exe

If no argument is provided, an ARFF file with every possible feature
is generated. If arguments are provided, then only those features will
be used. For example, ./makefeature ratio2 ratio3 will produce rows of
data with only ratio2,ratio3,class

OPTIONS
--three-measures : uses features that take 3 focus measures
                   by default, only 2 measures are used
--all-features : uses all features, including brackets
--highest,--nearest,--high-and-near : how we created the leftright features
--double-step : use measures at intervals of two lens positions
                by default, interval is one lens position
"""

import sys

import featuresfirststep
from scene import load_scenes

def get_arff_header(features):
    """Return a string representing the header of the ARFF file"""
    return ("@RELATION autofocus_size\n\n"
            + ''.join("@ATTRIBUTE %s %s \n" % (attr, values)
                      for attr, values, _ in features)
            + "@ATTRIBUTE step_size {coarse, fine} \n")


def convert_true_false(value):
    """Convert True to 1, False to 0 and leave everything else as it is."""
    if value is True:
        return 1
    elif value is False:
        return 0
    else:
        return value


def get_instances(scenes, classifier, features, step_size):
    """Create an array of instances in ARFF string representation."""
    lines = []

    for scene in scenes:
        for lens_pos in range(2 * step_size, scene.step_count):

            go_left = classifier(scene, lens_pos)
            initial_positions = featuresfirststep.first_three_lens_pos(
                lens_pos, step_size)

            if go_left:
                initial_positions.reverse()
                last_pos = initial_positions[-1]
                if scene.distance_to_closest_left_peak(last_pos) <= 15:
                    classification = ",fine"
                else:
                    classification = ",coarse"
            else:
                last_pos = initial_positions[-1]
                if scene.distance_to_closest_right_peak(last_pos) <= 15:
                    classification = ",fine"
                else:
                    classification = ",coarse"

            first, second, third = scene.get_focus_values(initial_positions)
            normalized_lens_pos = float(lens_pos) / (scene.step_count - 1)
            values = [feature(first=first, second=second, third=third,
                              lens_pos=normalized_lens_pos)
                      for _, _, feature in features]

            instance = ','.join(str(convert_true_false(value))
                                  for value in values) + classification
            lines.append(instance)

    return lines


def main(argv):
    filters = []
    features = featuresfirststep.two_measure_features
    classifier = featuresfirststep.highest_on_left
    step_size = 1

    # Process command line options. Anything remaining will be considered
    # to be filters for features.
    for arg in argv:
        if arg == "--three-measures":
            features = featuresfirststep.measure_features
        elif arg == "--all-features":
            features = featuresfirststep.all_features
        elif arg == "--highest":
            # Default
            pass
        elif arg == "--nearest":
            classifier = featuresfirststep.nearest_on_left
        elif arg == "--high-and-near":
            classifier = featuresfirststep.highest_and_near_on_left
        elif arg == "--double-step":
            step_size = 2
        else:
            filters.append(arg)

    scenes = load_scenes()

    # Print the contents of the ARFF file to screen (use output
    # redirection to save to file)
    print get_arff_header(features(filters))
    print "@DATA"
    instances = get_instances(scenes, classifier, features(filters), step_size)
    for instance in instances:
        print instance



# Entry point.
main(sys.argv[1:])