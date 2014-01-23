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

from scene import *
from features import *

### Classifiers ###

def highest_on_left(scene, lens_pos):
    # Find the location of the highest peak
    highest = 0
    for maxima in scene.maxima:
        if scene.measuresValues[maxima] > scene.measuresValues[highest]:
            highest = maxima
    return highest < lens_pos

def nearest_on_left(scene, lens_pos):
    # Find the location of the nearest peak
    nearest = 0
    for maxima in scene.maxima:
        if abs(lens_pos - maxima) < abs(lens_pos - nearest):
            nearest = maxima
    return nearest < lens_pos

def highest_and_near_on_left(scene, lens_pos):
    # Find the location of the peak that maximizes height
    # and distance, equally weighted in a product
    best = 0
    for maxima in scene.maxima:
        # Need to add a + 1 to the distance to avoid division by zero.
        if scene.measuresValues[maxima] / (abs(lens_pos - maxima) + 1) > \
           scene.measuresValues[best] / (abs(lens_pos - best) + 1):
            best = maxima
    return best < lens_pos


####

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

    scenes = [ Scene(f) 
               for f in os.listdir(scenes_folder) 
               if os.path.isfile(scenes_folder + f) ]

    load_maxima_into_measures(scenes)

    # Print the contents of the ARFF file to screen (use output
    # redirection to save to file)
    print get_arff_header(argv, True)
    print "@DATA"
    for line in get_data_lines(scenes, highest_and_near_on_left, argv, True):
        print line



# Entry point.
main(sys.argv[1:])