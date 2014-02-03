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

from scene    import *
from features import *
from evaluatetree import *

def print_script_usage():
   print  """Script usage : ./makeclassifierplot.py 
             [-s <scene to load (.txt)>]
             [-t <decision tree to evaluate>]
             [-c <classifier (highest, nearest, near_high)>]
             [-d <double step size used>]"""

def print_array_assignment(var_name, array):
    print var_name + " <- c(" + \
          str(array).translate(None, '[] ') + ")"

def print_R_script(scene, tree, classifier, step_size):

    print "# " + scene.fileName + "\n"

    # Print the focus measures first.
    print_array_assignment("focusmeasures", scene.measuresValues)

    # Then the correct classifications.
    classes = [ 0 if classifier(scene, lens_pos) else 1
                for lens_pos in range(2 * step_size, scene.measuresCount) ]
    print_array_assignment("classes", classes)

    # Then what we actually get.
    results = [ tree_eval(scene, lens_pos, tree, step_size)
                for lens_pos in range(2 * step_size, scene.measuresCount) ]
    print_array_assignment("results", results)

    # Some R functions for plotting.
    print "library(scales)" # for alpha blending
    print "plot(focusmeasures, pch=8)"

    # Axis to indicate that the bottom points mean left and
    # the top points means right.
    print "axis(2, at=0, labels=\"left\", padj=-2)"
    print "axis(2, at=1.0, labels=\"right\", padj=-2)"

    # Legend to differentiate correct and predicted.
    # pch indicates the shape of the points 
    print "legend(\"left\", pch=c(25, 22), col=c(\"brown\", \"blue\"), " \
          "legend=c(\"correct\", \"predicted\"))"

    print "lines(focusmeasures)"

    # Indicate the correct classes (left or right) and
    # the predicted classes. The predicted classes is
    # slightly offset to avoid overlapping.
    print "points(classes, pch=25, col=alpha(\"black\", 0.3), " \
          "bg=alpha(\"brown\", 0.5))"
    print "points(results - 0.02, pch=22, col=alpha(\"black\", 0.3), " \
          "bg=alpha(\"blue\", 0.5))"
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
    classifier = None
    step_size = 1

    for opt, arg in opts:
        if opt in ("-s", "--scene"):
            scene = Scene(arg)
        elif opt in ("-t", "--tree"):
            tree = parse_tree(arg, functions)
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