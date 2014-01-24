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

import os
import sys

from features import *

def get_arff_header(filters, include_three_features=False):
    """Return a string representing the header of the ARFF file"""

    if include_three_features:
        return "@RELATION autofocus_dir\n\n" \
               + ''.join([ "@ATTRIBUTE " + attr + " {0, 1} \n" 
                         for attr, _ in two_measure_features(filters)]) \
               + ''.join([ "@ATTRIBUTE " + attr + " {0, 1} \n" 
                         for attr, _ in three_measure_features(filters)]) \
               + "@ATTRIBUTE direction {left, right} \n"
    else:
        return "@RELATION autofocus_dir\n\n" \
               + ''.join([ "@ATTRIBUTE " + attr + " {0, 1}\n" 
                         for attr, _ in two_measure_features(filters)]) \
               + "@ATTRIBUTE direction {left, right} \n"

def get_data_lines(scenes, classifier, filters,
                   include_three_features=False):
    lines = []

    two_features = two_measure_features(filters)
    three_features = three_measure_features(filters)

    for scene in scenes:
        for lens_pos in range(2, scene.measuresCount):
            features = [feature(scene.measuresValues[lens_pos - 1],
                                scene.measuresValues[lens_pos])
                        for _, feature in two_features]
            if include_three_features:
                features += [feature(scene.measuresValues[lens_pos - 2],
                                     scene.measuresValues[lens_pos - 1],
                                     scene.measuresValues[lens_pos])
                             for _, feature in three_features]

            classification = "left" if classifier(scene, lens_pos) \
                                    else "right"
            lines.append(''.join(["1," if feature else "0,"
                                  for feature in features])
                      + classification)

    return lines

def main(argv):
    if not os.path.isdir(scenes_folder):
        print scenes_folder + " folder not found."
        return

    filters = []
    three_measures = False
    classifier = highest_on_left

    # Process command line options. Anything remaining will be considered
    # to be filters for features.
    for arg in argv:
        if arg == "--three-measures":
            three_measures = True
        elif arg == "--highest":
            # Default
            pass
        elif arg == "--nearest":
            classifier = nearest_on_left
        elif arg == "--high-and-near":
            classifier = highest_and_near_on_left
        else:
            filters.append(arg)

    scenes = [ Scene(f) 
               for f in os.listdir(scenes_folder) 
               if os.path.isfile(scenes_folder + f) ]

    load_maxima_into_measures(scenes)

    # Print the contents of the ARFF file to screen (use output
    # redirection to save to file)
    print get_arff_header(filters, three_measures)
    print "@DATA"
    for line in get_data_lines(scenes, classifier, filters, three_measures):
        print line



# Entry point.
main(sys.argv[1:])