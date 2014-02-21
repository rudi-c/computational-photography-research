#!/usr/bin/python

# This script evaluate classifications for a given decision tree as well
# as the correct classifications, for comparison.
#
# The output is meant to be convenient to plot in R.
#
# The tree should be of the form :
# node = | (function_name falsetree truetree)
#        | leaf (left or right)
# Example : (ratio2_7 (upTrend left right) left)

import getopt
import inspect
import os
import sys

from evaluatetree      import *
from featuresleftright import *
from rtools            import *
from scene             import *

def print_script_usage():
   print  """Script usage : ./makeclassifierplot.py 
             [-s <scene to load (.txt)>]
             [-t <decision tree to evaluate>]
             [-c <classifier (highest, nearest, near_high)>]
             [-d <double step size used>]"""


def print_R_script(scene, tree, classifier, step_size):

    print "# " + scene.fileName + "\n"

    # Some R functions for plotting.
    print "library(scales)" # for alpha blending
    print_plot_focus_measures(scene.measuresValues)

    # The correct classifications.
    classes = [ "left" if classifier(scene, lens_pos) else "right"
                for lens_pos in range(2 * step_size, scene.measuresCount) ]

    # What we actually get.
    results = []
    for lens_pos in range(2 * step_size, scene.measuresCount):
        first  = scene.measuresValues[lens_pos - step_size * 2]
        second = scene.measuresValues[lens_pos - step_size]
        third  = scene.measuresValues[lens_pos]
        norm_lens_pos = float(lens_pos) / (scene.measuresCount - 1)

        evaluator = leftright_feature_evaluator(first, second, 
                                                third, norm_lens_pos)

        evaluation = evaluate_tree(tree, evaluator)
        results.append(evaluation)

    print_classification_points(classes, results, ["left", "right"])

    print "# Plot me!\n"


def main(argv):

    # Parse script arguments
    try:
        opts, args = getopt.getopt(argv, "s:t:c:d",
                                  ["scene=", "tree=", "classifier=",
                                   "double-step"])
    except getopt.GetoptError:
        print_script_usage
        sys.exit(2)

    functions = { key : value for (key, _, value) in all_features() }

    scene = None
    tree = None
    classifier = None
    step_size = 1

    for opt, arg in opts:
        if opt in ("-s", "--scene"):
            scene = Scene(arg)
        elif opt in ("-t", "--tree"):
            tree = read_decision_tree(arg, functions)
        elif opt in ("-c", "--classifier"):
            if arg == "highest":
                classifier = highest_on_left
            elif arg == "nearest":
                classifier = nearest_on_left
            elif arg == "near_high":
                classifier = highest_and_near_on_left
        elif opt in ("-d", "--double-step"):
            step_size = 2

    if scene == None or tree == None:
        print_script_usage()
        sys.exit(2)

    load_maxima_into_measures([scene])
    print_R_script(scene, tree, classifier, step_size)


main(sys.argv[1:])