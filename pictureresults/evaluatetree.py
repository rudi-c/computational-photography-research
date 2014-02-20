import json
import os

def less(a, b):
    return a < b

def less_or_equal(a, b):
    return a <= b

def greater(a, b):
    return a > b

def greater_or_equal(a, b):
    return a >= b

def equals(a, b):
    return a == b

comparator_map = { "<" : less, "<=" : less_or_equal, 
                   ">" : greater, ">=" : greater_or_equal,
                   "=" : equals }

def read_decision_tree(filename, features):
    """Transform a JSON representation of the decision tree into one that can
    be used directly (with lambdas for feature evaluation and comparison)"""
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
        print "Error : File %s not found!" % filename
        sys.exit(1)

    json_tree = json.loads("".join(lines))
    return parse_tree(json_tree)


def evaluate_tree(tree, evaluator):
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
    