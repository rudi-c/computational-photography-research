#!/usr/bin/python
"""
Outputs statistics about available scenes in the form of R code.
The statistics are :
P(peak exists on the left | lens position)
P(peak does not exists on the left | lens position)
P(peak exists on the right | lens position)
P(peak does not exists on the right | lens position)
P(highest peak on the left | lens position)
P(highest peak on the right | lens position)
P(most peaks on the left | lens position)
P(most peaks on the right | lens position)
P(nearest peak on the left | lens position)
P(nearest peak on the right | lens position)
P(nearhighest peak on the left | lens position)
P(nearhighest peak on the right | lens position)

This script will look in the folder defined in scene.py for .txt
files with two numbers in each line : (row #, focus value).

The script also expects a file called 'maxima.txt' that contains
the output of running localMax.exe
"""

import os
import sys

from scene import load_scenes
from statistics import *

# Number of lens positions to calculate probabilities for.
# For example, steps = 5 would mean calculate probabilities at
# [0, .25, .5, .75, 1]
steps = 250

def print_statistics(scenes, lens_positions, function):
    print "# %s" % function.__doc__
    probabilities = [ function(pos, scenes) for pos in lens_positions ]
    print "data <- c(%s)" % ",".join(["%.3f" % p for p in probabilities])
    print "plot(data)"


def main(argv):
    scenes = load_scenes()

    # Make sure that the number of steps is appropriate.
    if steps < max(len(scene.maxima) for scene in scenes):
        print ("Warning - number of steps smaller than the number of lens "
               "positions in the largest scene.")

    # Calculate various statistics.
    lens_positions = [float(i) / (steps - 1) for i in range(steps)]

    print_statistics(scenes, lens_positions, probability_left_peak)
    print_statistics(scenes, lens_positions, probability_not_left_peak)
    
    print_statistics(scenes, lens_positions, probability_right_peak)
    print_statistics(scenes, lens_positions, probability_not_right_peak)
    
    print_statistics(scenes, lens_positions, probability_highest_left)
    print_statistics(scenes, lens_positions, probability_highest_right)
    
    print_statistics(scenes, lens_positions, probability_most_left)
    print_statistics(scenes, lens_positions, probability_most_right)
    
    print_statistics(scenes, lens_positions, probability_nearest_left)
    print_statistics(scenes, lens_positions, probability_nearest_right)

    print_statistics(scenes, lens_positions, probability_nearhighest_left)
    print_statistics(scenes, lens_positions, probability_nearhighest_right)


# Entry point.
main(sys.argv[1:])