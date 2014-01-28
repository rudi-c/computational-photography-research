#!/usr/bin/python

# This script will look in a folder called 'FocusMeasures' for .txt
# files with two numbers in each line : (row #, focus value).
#
# The script also expects a file called 'maxima.txt' that contains
# the output of running localMax.exe

import os
import sys

from scene import *

# Number of lens positions to calculate probabilities for.
# For example, steps = 5 would mean calculate probabilities at
# [0, .25, .5, .75, 1]
steps = 250

def smaller(a, b):
    return a < b

def greater(a, b):
    return a > b

left = smaller
right = greater

def valid_comparator(comparator):
    return comparator == left or comparator == right or \
           comparator == smaller or \
           comparator == greater


def probability_peak_exist(lens_pos, scenes, comparator):
    assert valid_comparator(comparator)

    count = 0
    for scene in scenes:
        for n_maximum in scene.norm_maxima:
            if comparator(n_maximum, lens_pos):
                count += 1
                break
    return float(count) / len(scenes)

def probability_left_peak(lens_pos, scenes):
    """P(peak exists to left | lens position)"""
    return probability_peak_exist(lens_pos, scenes, left)


def probability_not_left_peak(lens_pos, scenes):
    """P(peak does not exists to left | lens position)"""
    return 1 - probability_left_peak(lens_pos, scenes)


def probability_right_peak(lens_pos, scenes):
    """P(peak exists to right | lens position)"""
    return probability_peak_exist(lens_pos, scenes, right)


def probability_not_right_peak(lens_pos, scenes):
    """P(peak does not exists to right | lens position)"""
    return 1 - probability_right_peak(lens_pos, scenes)


def probability_highest_peak(lens_pos, scenes, comparator):
    assert valid_comparator(comparator)
    assert lens_pos >= 0 and lens_pos <= 1
    
    count = 0
    for scene in scenes:
        highest = 0
        for i in range(1, scene.measuresCount):
            if scene.measuresValues[i] > scene.measuresValues[highest]:
                highest = i

        if comparator(float(highest), lens_pos * scene.measuresCount):
            count += 1
    return float(count) / len(scenes)


def probability_highest_left(lens_pos, scenes):
    """P(highest peak to left | lens position)"""
    return probability_highest_peak(lens_pos, scenes, left)


def probability_highest_right(lens_pos, scenes):
    """P(highest peak to right | lens position)"""
    return probability_highest_peak(lens_pos, scenes, right)


def probability_most_peaks(lens_pos, scenes, comparator):
    assert valid_comparator(comparator)

    count = 0
    for scene in scenes:
        left_peaks = [ n_maximum for n_maximum in scene.norm_maxima
                                 if left(n_maximum, lens_pos) ]
        right_peaks = [ n_maximum for n_maximum in scene.norm_maxima
                                 if right(n_maximum, lens_pos) ]
        if comparator(len(left_peaks), len(right_peaks)):
            count += 1
    return float(count) / len(scenes)


def probability_most_left(lens_pos, scenes):
    """P( most peaks to the left | lens position )"""
    return probability_most_peaks(lens_pos, scenes, greater)


def probability_most_right(lens_pos, scenes):
    """P( most peaks to the left | lens position )"""
    return probability_most_peaks(lens_pos, scenes, smaller)

def probability_nearest(lens_pos, scenes, comparator):
    assert valid_comparator(comparator)

    count = 0
    for scene in scenes:
        nearest = scene.norm_maxima[0]
        for n_maximum in scene.norm_maxima:
            if abs(lens_pos - n_maximum) < abs(lens_pos - nearest):
                nearest = n_maximum
        if comparator(nearest, lens_pos):
            count += 1
    return float(count) / len(scenes)

def probability_nearest_left(lens_pos, scenes):
    """P( nearest peak to the left | lens position )"""
    return probability_nearest(lens_pos, scenes, left)

def probability_nearest_right(lens_pos, scenes):
    """P( nearest peak to the right | lens position )"""
    return probability_nearest(lens_pos, scenes, right)


def print_statistics(scenes, lens_positions, function):
    print "# " + function.__doc__
    probabilities = [ function(pos, scenes) for pos in lens_positions ]
    print str(probabilities).translate(None, '[] ')


def main(argv):
    if not os.path.isdir(scenes_folder):
        print scenes_folder + " folder not found."
        return

    scenes = load_scenes()

    load_maxima_into_measures(scenes)

    # Make sure that the number of steps is appropriate.
    if steps < max([len(scene.maxima) for scene in scenes]):
        print "Warning - number of steps smaller than the number of lens " \
              "positions in the largest scene."

    # Calculate various statistics.
    lens_positions = [ float(i) / (steps - 1) for i in range(0, steps)]

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


# Entry point.
main(sys.argv[1:])