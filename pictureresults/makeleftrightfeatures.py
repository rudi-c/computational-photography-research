#!/usr/bin/python
"""
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
--highest : classify by the location of the highest peak (default)
--nearest : classify by the location of the nearest peak
--high-and-near : classify by both height and distance of peaks
--double-step : use measures at intervals of two lens positions
                by default, interval is one lens position
--dup-edges : duplicates features near the first and last lens positions
"""

import sys

import featuresfirststep
from scene import load_scenes

def get_arff_header(features):
    """Return a string representing the header of the ARFF file"""
    return ("@RELATION autofocus_dir\n\n"
            + ''.join("@ATTRIBUTE %s %s \n" % (attr, values)
                      for attr, values, _ in features)
            + "@ATTRIBUTE direction {left, right} \n")


def convert_true_false(value):
    """Convert True to 1, False to 0 and leave everything else as it is."""
    if value is True:
        return 1
    elif value is False:
        return 0
    else:
        return value


def get_instances(scenes, classifier, position_selector, 
                  features, step_size):
    """Create an array of instances in ARFF string representation."""
    lines = []

    for scene in scenes:
        for lens_pos in position_selector(scene):
            # Data at this lens position.
            initial_positions = featuresfirststep.first_three_lens_pos(
                lens_pos, step_size)
            first, second, third = scene.get_focus_values(initial_positions)
            normalized_lens_pos = float(lens_pos) / (scene.step_count - 1)

            # Evaluate features on data at this lens position.
            values = [feature(first=first, second=second, third=third,
                              lens_pos=normalized_lens_pos)
                      for _, _, feature in features]

            classification = (",left" if classifier(scene, lens_pos)
                                      else ",right")
            instance = ','.join(str(convert_true_false(value))
                                  for value in values) + classification
            lines.append(instance)

    return lines


def create_lens_position_selector(duplicate_edges, step_size):
    """Return a function that takes a scene as argument and returns the
    list of lens positions we want to use to create an instance.
    
    We do it this way (rather than just iterating over all lens positions
    in order to duplicate some lens positions that are more important, to help
    the machine learning algorithm.
    """
    def selector(scene):
        # Default lens positions
        lens_positions = range(step_size * 2, scene.step_count)

        if duplicate_edges:
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
    return selector


def main(argv):
    filters = []
    features = featuresfirststep.two_measure_features
    classifier = featuresfirststep.highest_on_left
    step_size = 1
    duplicate_edges = False

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
        elif arg == "--dup-edges":
            duplicate_edges = True
        else:
            filters.append(arg)

    scenes = load_scenes()

    # Print the contents of the ARFF file to screen (use output
    # redirection to save to file)
    print get_arff_header(features(filters))
    print "@DATA"
    instances = get_instances(scenes, classifier, 
        create_lens_position_selector(duplicate_edges, step_size),
        features(filters), step_size)
    for instance in instances:
        print instance


# Entry point.
main(sys.argv[1:])