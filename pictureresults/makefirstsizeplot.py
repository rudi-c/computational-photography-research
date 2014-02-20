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
             [-d <double step size used>]"""


def print_R_script(scene, tree, step_size):

    print "# " + scene.fileName + "\n"

    # Some R functions for plotting.
    print "library(scales)" # for alpha blending
    print_plot_focus_measures(scene.measuresValues)

    classes = []
    results = []

    for lens_pos in range(2 * step_size, scene.measuresCount):

        # The correct classification.
        go_left =  nearest_on_left(scene, lens_pos)
        if go_left:
            pos_1st = lens_pos
            pos_2nd = lens_pos - 1 * step_size
            pos_3rd = lens_pos - 2 * step_size
            if scene.distance_to_closest_left_peak(pos_3rd) <= 15:
                classes.append("fine")
            else:
                classes.append("coarse")
        else:
            pos_1st = lens_pos - 2 * step_size
            pos_2nd = lens_pos - 1 * step_size
            pos_3rd = lens_pos
            if scene.distance_to_closest_right_peak(pos_3rd) <= 15:
                classes.append("fine")
            else:
                classes.append("coarse")

        # The classification obtained by evaluating the decision tree.
        first  = scene.measuresValues[pos_1st]
        second = scene.measuresValues[pos_2nd]
        third  = scene.measuresValues[pos_3rd]
        norm_lens_pos = float(lens_pos) / (scene.measuresCount - 1)

        evaluator = leftright_feature_evaluator(first, second, 
                                                third, norm_lens_pos)

        evaluation = evaluate_tree(tree, evaluator)
        results.append(evaluation)

    print_classification_points(classes, results, ["coarse", "fine"])

    print "# Plot me!\n"


def main(argv):

    # Parse script arguments
    try:
        opts, args = getopt.getopt(argv,"s:t:c:d",
                                  ["scene=", "tree=", "classifier=",
                                   "double-step"])
    except getopt.GetoptError:
        print_script_usage
        sys.exit(2)

    functions = { key : value for (key, _, value) in all_features() }

    scene = None
    tree = None
    step_size = 1

    for opt, arg in opts:
        if opt in ("-s", "--scene"):
            scene = Scene(arg)
        elif opt in ("-t", "--tree"):
            tree = read_decision_tree(arg, functions)
        elif opt in ("-d", "--double-step"):
            step_size = 2

    if scene == None or tree == None:
        print_script_usage()
        sys.exit(2)

    load_maxima_into_measures([scene])
    print_R_script(scene, tree, step_size)


main(sys.argv[1:])