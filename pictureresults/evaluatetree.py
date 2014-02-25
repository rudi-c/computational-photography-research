"""
Function that load and evaluate a Weka decisiontree that has been parsed 
and converted into json format.
"""

import operator
import json
import os

# Conveniently convert string operators into lambdas.
comparator_map = { "<" : operator.lt, "<=" : operator.le, 
                   ">" : operator.gt, ">=" : operator.ge,
                   "=" : operator.eq }

def read_decision_tree(filename, features):
    """Transform a JSON representation of the decision tree into one that can
    be used directly (with lambdas for feature evaluation and comparison).
    Takes a dictionnary of { feature names : features (lambdas) } 
    """
    def parse_tree(json_tree):
        if isinstance(json_tree, str) or isinstance(json_tree, unicode):
            # Reached leaf.
            return str(json_tree)
        else:
            feature, children = json_tree
        if not features.has_key(feature):
            raise Exception("Feature %s not found." % feature)
        return (features[feature], 
                [ (comparator_map[comparator], value, parse_tree(subtree))
                  for comparator, value, subtree in children ])

    if os.path.isfile(filename):
        f = open(filename)
        lines = f.readlines()
        f.close()
    else:
        raise Exception("Error : File %s not found!" % filename)

    json_tree = json.loads("".join(lines))
    return parse_tree(json_tree)


def evaluate_tree(tree, evaluator):
    """Evaluate a tree loaded with read_decision_tree. Takes an evaluator,
    which is a function taking a particular feature at a node and evalulating
    that featuring on some data.
    """
    if isinstance(tree, str):
        # Reached leaf.
        return tree
    if isinstance(tree, unicode):
        raise Exception("Expecting non-unicode string")

    function, children = tree
    node_evaluation = evaluator(function)
    for comparator, value, subtree in children:
        if isinstance(value, list):
            # Handle ranges of values.
            if value[0] < node_evaluation <= value[1]:
                return evaluate_tree(subtree, evaluator)
        elif comparator(node_evaluation, value):
            return evaluate_tree(subtree, evaluator)
    raise Exception("No match in tree for evaluated feature!")
    