#!/usr/bin/python

# This script will list the features that we create.

import os
import sys

from features import *

def main(argv):
    print "# Features taking 2 focus measures :"
    for name, values, _ in two_measure_features():
        print name + " : " + values
    print "# Features taking 3 focus measures :"
    for name, values, _ in three_measure_features():
        print name + " : " + values
    print "# Other features :"
    for name, values, _ in other_features():
        print name + " : " + values
    return

# Entry point.
main(sys.argv[1:])