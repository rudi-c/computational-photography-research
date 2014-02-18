#!/usr/bin/python

# This script will list the features that we create.

import os
import sys

from featuresleftright import *
import featuresturn

def main(argv):
    print "# Left-right features taking 2 focus measures :"
    for name, values, _ in two_measure_features():
        print name + " : " + values
    print "# Left-right features taking 3 focus measures :"
    for name, values, _ in three_measure_features():
        print name + " : " + values
    print "# Left-right other features :"
    for name, values, _ in other_features():
        print name + " : " + values
    print "# Action features :"
    for name, _ in featuresturn.all_features():
        print name
    return

# Entry point.
main(sys.argv[1:])