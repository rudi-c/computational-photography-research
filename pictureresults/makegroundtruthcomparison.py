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
from statistics        import *

def print_R_script(ground_probabilities, our_probabilities):
    print_array_assignment("ground", ground_probabilities)
    print_array_assignment("our", our_probabilities)

    # pch is the shape of the points
    print "plot(ground, ylim=c(0,1), pch=4)"
    print "points(our, pch=19)"
    print "lines(ground)"
    print "lines(our)"

    # Legend to differentiate correct and predicted point
    print "legend(\"bottomright\", pch=c(4, 19), " \
          "legend=c(\"ground\", \"predicted\"))"

    # Summarize the difference in % correct classification between
    # our classification and the ground truth.
    print "summary(ground)"
    print "summary(our)"
    print "summary(our - ground)"


def print_script_usage():
   print  """Script usage : ./makegroundtruthcomparison.py 
             [-t <decision tree to evaluate>]
             [-c <classifier (highest, nearest, near_high)>]
             [-d <double step size used>]"""


def main(argv):

    # Parse script arguments
    try:
        opts, args = getopt.getopt(argv, "s:t:c:d",
                                  ["tree=", "classifier=", "double-step"])
    except getopt.GetoptError:
        print_script_usage()
        sys.exit(2)

    functions = { key : value for key, _, value in all_features() }

    tree = None
    classifier = None
    ground_probability = None
    step_size = 1

    for opt, arg in opts:
        if opt in ("-t", "--tree"):
            tree = read_decision_tree(arg, functions)
        elif opt in ("-c", "--classifier"):
            if arg == "highest":
                classifier = highest_on_left
                ground_probability = probability_highest_left
            elif arg == "nearest":
                classifier = nearest_on_left
                ground_probability = probability_nearest_left
            elif arg == "near_high":
                classifier = highest_and_near_on_left
                ground_probability = probability_nearhighest_left
        elif opt in ("-d", "--double-step"):
            step_size = 2

    if tree == None:
        print_script_usage()
        sys.exit(2)

    scenes = load_scenes()
    load_maxima_into_measures(scenes)

    # We need to calculate the probability of getting the correct
    # classification at certain intervals, but not every scene has
    # the same number of lens positions. Here we just use the least
    # number of intervals amongst all scenes (which should also be
    # the majority).
    steps = min([scene.measuresCount for scene in scenes])
    assert steps > 0

    # Start at 2 * step_size to match the number of classifications
    # that we get from the decision tree (which is less than the total
    # number of lens position since we're sampling at 3 different places)
    lens_positions = [ float(i) / (steps - 1) 
                       for i in range(2 * step_size, steps)]

    # Calculate the probability of getting the right classification among
    # all scenes if we rely purely on statistics (majority classification
    # for all lens positions).
    ground_probabilities = [ ground_probability(pos, scenes) 
                             for pos in lens_positions ]
    ground_probabilities = [ max(p, 1.0 - p) for p in ground_probabilities]

    # Calculate the probability of getting the right classification given
    # the input decision tree.
    correct_bins = [ 0 ] * steps
    incorrect_bins = [ 0 ] * steps
    for scene in scenes:
        for lens_pos in range(2 * step_size, scene.measuresCount):

            # The classifier function returns True if the answer is Left
            correct_class = classifier(scene, lens_pos)

            # The tree returns 0 if the answer is Left, so we need to change
            # it to True.
            first  = scene.measuresValues[lens_pos - step_size * 2]
            second = scene.measuresValues[lens_pos - step_size]
            third  = scene.measuresValues[lens_pos]
            norm_lens_pos = float(lens_pos) / (scene.measuresCount - 1)
            evaluator = leftright_feature_evaluator(first, second, 
                                                    third, norm_lens_pos)

            dectree_class = evaluate_tree(tree, evaluator)
            dectree_class = (dectree_class == "left")

            # Need to round at a few decimal points to make sure that floating
            # point precisions errors don't make it so that i != lens_pos
            # when scene.measuresCount == steps.
            i = int(round(float(lens_pos) / scene.measuresCount * steps, 4))

            if correct_class == dectree_class:
                correct_bins[i] += 1
            else:
                incorrect_bins[i] += 1

    our_probabilities = [ float(ncorrect) / (ncorrect + nincorrect)
                          for ncorrect, nincorrect 
                          in zip(correct_bins[2 * step_size:], 
                                 incorrect_bins[2 * step_size:]) ]

    print_R_script(ground_probabilities, our_probabilities)


main(sys.argv[1:])
