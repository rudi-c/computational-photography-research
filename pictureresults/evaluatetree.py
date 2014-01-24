#!/usr/bin/python

# This script evaluate classifications for a given decision tree as well
# as the correct classifications, for comparison.
#
# The output is meant to be convenient to plot in R.
#
# The tree should be of the form :
# node = | (function_name falsetree truetree)
#        | leaf (0 or 1)
# Example : (ratio2_7 (upTrend 0 1) 1)

import getopt
import inspect
import os
import sys

from scene    import *
from features import *

def parse_tree(input, functions, current_index = 0):
    """Parse a string into tree form, as a nested array."""
    
    first_char = input[current_index]
    while first_char == ' ':
        current_index += 1
        first_char = input[current_index]

    assert first_char == '(' or first_char == "0" or first_char == "1"

    if not first_char == '(':
        return (int(first_char), current_index + 1)

    # Parse function name.
    next_whitespace = input.find(' ', current_index)
    function_name = input[current_index + 1 : next_whitespace]
    function = functions[function_name]

    # Parse left node.
    left_tree, next_index = parse_tree(input, functions, next_whitespace + 1)

    # Parse right node.
    right_tree, next_index = parse_tree(input, functions, next_index)

    assert input[next_index] == ')'

    if current_index == 0:
        # Return root
        return [function, left_tree, right_tree]
    else:
        return ([function, left_tree, right_tree], next_index + 1)

def tree_eval(scene, lens_pos, tree):
    # Reached a leaf.
    if tree == 0 or tree == 1:
        return tree

    # Pattern matching is so convenient.
    [ function, left_tree, right_tree ] = tree

    arg_count = len(inspect.getargspec(function)[0])
    if arg_count == 2:
        if function(scene.measuresValues[lens_pos - 1],
                    scene.measuresValues[lens_pos]):
            return tree_eval(scene, lens_pos, right_tree)
        else:
            return tree_eval(scene, lens_pos, left_tree)
    elif arg_count == 3:
        if function(scene.measuresValues[lens_pos - 2],
                    scene.measuresValues[lens_pos - 1],
                    scene.measuresValues[lens_pos]):
            return tree_eval(scene, lens_pos, right_tree)
        else:
            return tree_eval(scene, lens_pos, left_tree)
    else:
        raise Exception("Function should have exactly 2 or 3 arguments")

def print_script_usage():
   print  """Script usage : ./FlickrRandom 
             [-s <scene to load (.txt)>]
             [-t <decision tree to evaluate>]
             [-c <classifier (highest, nearest, near_high)>]"""

def print_array_assignment(var_name, array):
    print var_name + " <- c(" + \
          str(array).translate(None, '[] ') + ")"

def print_R_script(scene, tree, classifier):

    # Print the focus measures first.
    print_array_assignment("focusmeasures", scene.measuresValues)

    # Then the correct classifications.
    classes = [ 1 if classifier(scene, lens_pos) else 0
                for lens_pos in range(2, scene.measuresCount) ]
    print_array_assignment("classes", classes)

    # Then what we actually get.
    results = [ tree_eval(scene, lens_pos, tree)
                for lens_pos in range(2, scene.measuresCount) ]
    print_array_assignment("results", results)

    # Some R functions for plotting.
    print "plot(focusmeasures, pch=8)"
    print "lines(focusmeasures)"
    print "points(classes - 0.02, pch=25, bg=\"brown\")"
    print "points(results - 0.02, pch=22, bg=\"blue\")"
    print "# Plot me!"


def main(argv):

    # Parse script arguments
    try:
        opts, args = getopt.getopt(argv,"s:t:c:",
                                  ["scene=", "tree=", "classifier="])
    except getopt.GetoptError:
        print_script_usage
        sys.exit(2)

    functions = { key : value for (key, value) 
                  in two_measure_features() + three_measure_features() }

    scene = None
    tree = None
    classifier = None

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

    if scene == None or tree == None:
        print_script_usage()
        sys.exit(2)

    load_maxima_into_measures([scene])
    print_R_script(scene, tree, classifier)





main(sys.argv[1:])