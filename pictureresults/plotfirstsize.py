#!/usr/bin/python
"""
This script evaluate classifications (first step is fine or coarse) for a given 
decision tree as well as the correct classifications, for comparison.

The output is an R script for plotting the results.
"""

import getopt
import sys

from scene import Scene, load_maxima
import evaluatetree
import featuresfirststep
import rtools

def print_script_usage():
    print >> sys.stderr, \
        """Script usage : ./plotfirstsize.py 
           [-s <scene to load (.txt)>]
           [-t <decision tree to evaluate>]
           [-d <double step size used>]"""


def print_R_script(scene, tree, step_size):

    print "# " + scene.filename + "\n"

    # Some R functions for plotting.
    print "library(scales)" # for alpha blending
    rtools.print_plot_focus_measures(scene.fvalues)

    classes = []
    results = []

    for lens_pos in range(2 * step_size, scene.step_count):

        # The correct classification.
        go_left = featuresfirststep.nearest_on_left(scene, lens_pos)
        initial_positions = featuresfirststep.first_three_lens_pos(
            lens_pos, step_size)
        if go_left:
            initial_positions.reverse()
            last_pos = initial_positions[-1]
            if scene.distance_to_closest_left_peak(last_pos) <= 15:
                classes.append("fine")
            else:
                classes.append("coarse")
        else:
            last_pos = initial_positions[-1]
            if scene.distance_to_closest_right_peak(last_pos) <= 15:
                classes.append("fine")
            else:
                classes.append("coarse")

        # The classification obtained by evaluating the decision tree.
        first, second, third = scene.get_focus_values(initial_positions)
        norm_lens_pos = float(lens_pos) / (scene.step_count - 1)


        evaluator = featuresfirststep.firststep_feature_evaluator(
            first, second, third, norm_lens_pos)

        evaluation = evaluatetree.evaluate_tree(tree, evaluator)
        results.append(evaluation)

    rtools.print_classification_points(classes, results, ["coarse", "fine"])

    print "# Plot me!\n"


def main(argv):

    # Parse script arguments
    try:
        opts, _ = getopt.getopt(argv, "s:t:c:d",
                                ["scene=", "tree=", "classifier=",
                                 "double-step"])
    except getopt.GetoptError:
        print_script_usage()
        sys.exit(2)

    features = featuresfirststep.all_features_dict()

    scene = None
    tree = None
    step_size = 1

    for opt, arg in opts:
        if opt in ("-s", "--scene"):
            scene = Scene(arg)
            load_maxima([scene])
        elif opt in ("-t", "--tree"):
            tree = evaluatetree.read_decision_tree(arg, features)
        elif opt in ("-d", "--double-step"):
            step_size = 2

    if scene is None or tree is None:
        print_script_usage()
        sys.exit(2)

    print_R_script(scene, tree, step_size)


main(sys.argv[1:])