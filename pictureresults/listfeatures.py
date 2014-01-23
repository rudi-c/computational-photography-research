#!/usr/bin/python

# This script will list the features that we create.

import os
import sys

from features import *

def main(argv):
    print "# Features taking 2 focus measures :"
    for name, function in two_measure_features():
        print name
    print "# Features taking 3 focus measures :"
    for name, function in three_measure_features():
        print name
    return

# Entry point.
main(sys.argv[1:])