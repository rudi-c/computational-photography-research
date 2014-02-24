#!/usr/bin/python

# This script will look in a folder called 'FocusMeasures' for .txt
# files with two numbers in each line : (row #, focus value).
#
# The script also expects a file called 'maxima.txt' that contains
# the output of running localMax.exe
#
# The script generates R code that plots the maxima of each data set,
# in order to visualize how our maxima are distributed.

import sys

from scene import *

def main():
    scenes = load_scenes()

    xs = []
    ys = []

    for scene in scenes:
        for maxima in scene.maxima:
            xs.append(float(maxima) / scene.step_count)
            ys.append(scene.fvalues[maxima])

    # For alpha blending
    print "library(scales)"
    print "plot(-1, -1, xlim=c(0,1), ylim=c(0,1))"
    for x, y in zip(xs, ys):
        print "segments(%f, 0, %f, %f, col=alpha(\"black\", 0.5))" % (x, x, y)

    return


# Entry point.
main()