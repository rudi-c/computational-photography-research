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
#
#
#


import os
import sys

from featuresleftright import *

def get_arff_header(features):
    """Return a string representing the header of the ARFF file"""

    return ("@RELATION autofocus_size\n\n"
            + ''.join(["@ATTRIBUTE " + attr + " " + values + " \n" 
                       for attr, values, _ in features])
            + "@ATTRIBUTE step_size {coarse, fine} \n")


def convert_true_false(value):
    """Convert True to 1, False to 0 and leave everything else as it is."""
    if value is True:
        return 1
    elif value is False:
        return 0
    else:
        return value


def get_data_lines(scenes, classifier, 
                   features, step_size):
    lines = []

    for scene in scenes:
        for lens_pos in range(2 * step_size, scene.measuresCount):

            go_left =  classifier(scene, lens_pos)

            if go_left:
                pos_1st = lens_pos
                pos_2nd = lens_pos - 1 * step_size
                pos_3rd = lens_pos - 2 * step_size
                if scene.distance_to_closest_left_peak(pos_3rd) <= 15:
                    classification = ",fine"
                else:
                    classification = ",coarse"
            else:
                pos_1st = lens_pos - 2 * step_size
                pos_2nd = lens_pos - 1 * step_size
                pos_3rd = lens_pos
                if scene.distance_to_closest_right_peak(pos_3rd) <= 15:
                    classification = ",fine"
                else:
                    classification = ",coarse"

            values = [feature(
                first  = scene.measuresValues[pos_1st],
                second = scene.measuresValues[pos_2nd],
                third  = scene.measuresValues[pos_3rd],
                lens_pos = float(pos_3rd) / (scene.measuresCount - 1))
                      for _, _, feature in features]

            lines.append(','.join([str(convert_true_false(value))
                                  for value in values])
                      + classification)

    return lines


def main(argv):
    if not os.path.isdir(scenes_folder):
        print scenes_folder + " folder not found."
        return

    filters = []
    features = two_measure_features
    classifier = highest_on_left
    step_size = 1

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
        else:
            filters.append(arg)

    scenes = load_scenes()
    load_maxima_into_measures(scenes)

    # Print the contents of the ARFF file to screen (use output
    # redirection to save to file)
    print get_arff_header(features(filters))
    print "@DATA"
    for line in get_data_lines(scenes, 
                               classifier,
                               features(filters), 
                               step_size):
        print line



# Entry point.
main(sys.argv[1:])