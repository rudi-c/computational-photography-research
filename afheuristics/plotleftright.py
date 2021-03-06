#!/usr/bin/python
"""
This script evaluate classifications (look left or right) for a given 
decision tree as well as the correct classifications, for comparison.

The output is an R script for plotting the results.
"""

import getopt
import sys

from scene import Scene, load_scene
import evaluatetree
import featuresfirststep
import rtools

def print_script_usage():
    print >> sys.stderr, \
        """Script usage : ./plotleftright.py 
           [-s <scene to load (.txt)>]
           [-t <decision tree to evaluate>]
           [-c <classifier (highest, nearest, near_high)>]
           [-d <double step size used>]"""


def print_R_script(scene, tree, classifier, step_size):

    print "# " + scene.filename + "\n"

    # Some R functions for plotting.
    print "library(scales)" # for alpha blending
    rtools.print_plot_focus_measures(scene.fvalues)

    # The correct classifications.
    classes = [ "left" if classifier(scene, lens_pos) else "right"
                for lens_pos in range(2 * step_size, scene.step_count) ]

    # What we actually get.
    results = []
    for lens_pos in range(2 * step_size, scene.step_count):
        initial_positions = featuresfirststep.first_three_lens_pos(
            lens_pos, step_size)
        first, second, third = scene.get_focus_values(initial_positions)
        norm_lens_pos = float(lens_pos) / (scene.step_count - 1)

        evaluator = featuresfirststep.firststep_feature_evaluator(
            first, second, third, norm_lens_pos)

        evaluation = evaluatetree.evaluate_tree(tree, evaluator)
        results.append(evaluation)

    rtools.print_classification_points(classes, results, ["left", "right"])

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
    classifier = None
    step_size = 1

    for opt, arg in opts:
        if opt in ("-s", "--scene"):
            scene = load_scene(arg)
        elif opt in ("-t", "--tree"):
            tree = evaluatetree.read_decision_tree(arg, features)
        elif opt in ("-c", "--classifier"):
            if arg == "highest":
                classifier = featuresfirststep.highest_on_left
            elif arg == "nearest":
                classifier = featuresfirststep.nearest_on_left
            elif arg == "near_high":
                classifier = featuresfirststep.highest_and_near_on_left
        elif opt in ("-d", "--double-step"):
            step_size = 2

    if scene is None or tree is None:
        print_script_usage()
        sys.exit(2)

    print_R_script(scene, tree, classifier, step_size)


main(sys.argv[1:])