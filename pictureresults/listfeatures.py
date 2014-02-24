#!/usr/bin/python
"""This script lists all the features we use in machine learning."""

import featuresfirststep
import featuresturn

def main():
    print "# Left-right features taking 2 focus measures :"
    for name, values, _ in featuresfirststep.two_measure_features():
        print name + " : " + values
    print "# Left-right features taking 3 focus measures :"
    for name, values, _ in featuresfirststep.three_measure_features():
        print name + " : " + values
    print "# Left-right other features :"
    for name, values, _ in featuresfirststep.other_features():
        print name + " : " + values
    print "# Action features :"
    for name, _ in featuresturn.all_features():
        print name
    return

# Entry point.
main()