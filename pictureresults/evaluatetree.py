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

def find_next_whitespace(string, start_index = 0):
    """Finds the first occurence of whitespace (space, tab or newline)
    after start_index. Return -1 if non"""
    next_space   = string.find(' ',  start_index)
    next_tab     = string.find('\t', start_index)
    next_newline = string.find('\n', start_index)
    earliest = max(next_space, next_tab, next_newline)
    if next_space > 0:
        earliest = min(earliest, next_space)
    if next_tab > 0:
        earliest = min(earliest, next_tab)
    if next_newline > 0:
        earliest = min(earliest, next_newline)
    return earliest

def check_brackets(input):
    """Ensures that the brackets match (every opening bracket has a
    corresponding closing bracket)"""
    n_pairs = 0
    for i in range(0, len(input)):
        if input[i] == '(':
            n_pairs += 1
        elif input[i] == ')':
            if n_pairs == 0:
                raise Exception("Closing bracket without opening bracket : \n"
                                + input[:(i+1)])
            else:
                n_pairs -= 1
    if n_pairs != 0:
        raise Exception(str(n_pairs) + 
                        " opening brackets have not been closed")


def parse_tree(input, functions, current_index = 0):
    """Parse a string into tree form, as a tuple of function
    and an array of children."""
    
    # Skip extra leading whitespaces
    first_char = input[current_index]
    while first_char == ' ' or first_char == '\t' or first_char == '\n':
        current_index += 1
        first_char = input[current_index]

    if not (first_char == '(' or input[current_index:].startswith("left") \
                              or input[current_index:].startswith("right")):
        raise Exception("Every child should either be a subtree, a "
                        "\"left\" leaf or a \"right\" leaf :\n"
                        + input[:(current_index+1)])

    # This is a leaf node.
    if not first_char == '(':
        if input[current_index:].startswith("left"):
            return (0, current_index + len("left"))
        else:
            return (1, current_index + len("right"))

    # Parse function name.
    next_whitespace = find_next_whitespace(input, current_index)
    function_name = input[current_index + 1 : next_whitespace]
    function = functions[function_name]

    next_index = next_whitespace + 1

    # Recursively parse all child nodes.
    children = []
    while input[next_index] != ')':
        child, next_index = parse_tree(input, functions, next_index)
        children.append(child)

    if len(children) <= 0:
        raise Exception("Every non-leaf node should have at "
                        "least one child : \n" + input[:(next_index+1)])

    if current_index == 0:
        # Return root
        return (function, children)
    else:
        return ((function, children), next_index + 1)

def tree_eval(scene, lens_pos, tree, step_size):
    # Reached a leaf.
    if tree == 0 or tree == 1:
        return tree

    # Pattern matching is so convenient.
    function, children = tree

    node_value = function(
        first  = scene.measuresValues[lens_pos - 2 * step_size],
        second = scene.measuresValues[lens_pos - 1 * step_size],
        third  = scene.measuresValues[lens_pos],
        lens_pos = float(lens_pos) / (scene.measuresCount - 1))

    if node_value is True:
        return tree_eval(scene, lens_pos, children[1], step_size)
    elif node_value is False:
        return tree_eval(scene, lens_pos, children[0], step_size)
    elif isinstance(node_value, int):
        try:
            return tree_eval(scene, lens_pos, children[node_value], step_size)
        except IndexError:
            raise Exception("Insufficient number of children for a " \
                            "feature with integer values.")
    else:
        raise Exception("Features should produce a boolean or an integer.")

def print_script_usage():
   print  """Script usage : ./FlickrRandom 
             [-s <scene to load (.txt)>]
             [-t <decision tree to evaluate>]
             [-c <classifier (highest, nearest, near_high)>]"""

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
    print "points(classes, pch=25, bg=\"brown\")"
    print "points(results - 0.02, pch=22, bg=\"blue\")"
    print "# Plot me!\n"


def main(argv):

    # Parse script arguments
    try:
        opts, args = getopt.getopt(argv,"s:t:c:d",
                                  ["scene=", "tree=", "classifier=",
                                   "double"])
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
            check_brackets(arg)
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