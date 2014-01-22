#!/usr/bin/python

# This script will look in a folder called 'FocusMeasures' for .txt
# files with two numbers in each line : (row #, focus value).
#
# The script also expects a file called 'maxima.txt' that contains
# the output of running localMax.exe

import os
import sys

scenes_folder = "FocusMeasure/"
maxima_file = "maxima.txt"

# Number of lens positions to calculate probabilities for.
# For example, steps = 5 would mean calculate probabilities at
# [0, .25, .5, .75, 1]
steps = 250

class Scene:
    def __get_focus_value(self, string):
        parts = string.split()
        if len(parts) != 2:
            print "Lines in focus measures files should only have two columns."
            raise
        return float(parts[1])

    def __load_file(self):
        try:
            f = open(scenes_folder + self.fileName)
        except IOError:
            print "File " + self.fileName + " not found."
            raise

        focus_values = [self.__get_focus_value(line) for
                        line in f.readlines()]
        self.measuresCount = len(focus_values)
        self.measuresValues = focus_values
        # Normalized version as tuples.
        # self.measuresValues = [(float(i) / self.measuresCount, focus_values[i]) 
        #                       for i in range(0, len(focus_values)) ]

    def __init__(self, file_name):
        self.fileName = file_name
        self.__load_file()
        self.maxima = []


def load_maxima_into_measures(scenes):
    try:
        f = open(maxima_file)
    except IOError:
        print maxima_file + " not found."
        raise

    lines = f.readlines()

    if len(lines) % 2 != 0:
        print "Expecting an even number of lines in " + maxima_file
        raise

    # Organize scenes by their filename to find them more conveniently.
    scenes_dict = {}
    for scene in scenes:
        scenes_dict[scene.fileName] = scene

    for i in range(0, len(lines), 2):
        filename = lines[i].strip()
        maxima = lines[i + 1].split()

        if not scenes_dict.has_key(filename):
            print "Warning : " + filename + " was found in " + maxima_file \
                  + " but not in " + scenes_folder

        scene = scenes_dict[filename]

        # Peaks are normalized to a range [0, 1]
        scene.maxima = maxima
        scene.norm_maxima = [ float(position) / scene.measuresCount for 
                              position in maxima ]

        for maximum in scene.norm_maxima:
            if maximum < 0 or maximum > 1:
                print "Warning : " + filename + " has a maximum at a" \
                      + " position outside of the range [0, 1]."


def peak_count(scenes):
    return sum(len(scene.maxima) for scene in scenes)

def left(a, b):
    return a < b

def right(a, b):
    return a > b

smaller = left
greater = right

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


def print_statistics(scenes, lens_positions, function):
    print "# " + function.__doc__
    probabilities = [ function(pos, scenes) for pos in lens_positions ]
    print str(probabilities).translate(None, '[] ')


def main(argv):
    if not os.path.isdir(scenes_folder):
        print scenes_folder + " folder not found."
        return

    scenes = [ Scene(f) 
               for f in os.listdir(scenes_folder) 
               if os.path.isfile(scenes_folder + f) ]

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


# Entry point.
main(sys.argv[1:])