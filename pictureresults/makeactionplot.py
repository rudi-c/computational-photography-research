#!/usr/bin/python

import getopt
import inspect
import os
import sys

from scene import *
from featuresturn import *
from rtools import *

PCHS = [ 3, 1, 7 ]
COLORS = [ "black", "green", "red" ]

def segmentation(array):
    segments = [ 0 ]
    current = array[0]
    for i in range(1, len(array)):
        if array[i] != current:
            segments.append(i)
            current = array[i]
    segments.append(len(array))
    return segments


def print_left_classes(scene, params):

    segments_continue = []
    segments_turn_peak = []
    segments_backtrack = []

    for start_pos in range(0, scene.step_count):
        y = float(start_pos) / scene.step_count
        classes = [ get_move_left_classification(start_pos, pos, 
                        scene.fvalues, scene.maxima, params)
                    for pos in range(0, start_pos + 1) ]
        segments = segmentation(classes)
        for start, end in zip(segments[:-1], segments[1:]):
            fstart = float(start) / scene.step_count
            fend   = float(end)   / scene.step_count
            if classes[start] == Action.CONTINUE:
                segments_continue.append((fstart, fend, y))
            elif classes[start] == Action.TURN_PEAK:
                segments_turn_peak.append((fstart, fend, y))
            elif classes[start] == Action.BACKTRACK:
                segments_backtrack.append((fstart, fend, y))
            else:
                assert False

    print_array_assignment("xstarts", [ a for a, _, _ in segments_continue ])
    print_array_assignment("xends",   [ b for _, b, _ in segments_continue ])
    print_array_assignment("ys",      [ c for _, _, c in segments_continue ])
    print "for (i in 1:%d) { segments(xstarts[i], ys[i], xends[i], ys[i]," \
          " col=\"black\") }" % len(segments_continue)
    print_array_assignment("xstarts", [ a for a, _, _ in segments_turn_peak ])
    print_array_assignment("xends",   [ b for _, b, _ in segments_turn_peak ])
    print_array_assignment("ys",      [ c for _, _, c in segments_turn_peak ])
    print "for (i in 1:%d) { segments(xstarts[i], ys[i], xends[i], ys[i]," \
          " col=\"green\") }" % len(segments_turn_peak)
    print_array_assignment("xstarts", [ a for a, _, _ in segments_backtrack ])
    print_array_assignment("xends",   [ b for _, b, _ in segments_backtrack ])
    print_array_assignment("ys",      [ c for _, _, c in segments_backtrack ])
    print "for (i in 1:%d) { segments(xstarts[i], ys[i], xends[i], ys[i]," \
          " col=\"red\") }" % len(segments_backtrack)



def print_right_classes(scene, params):

    segments_continue = []
    segments_turn_peak = []
    segments_backtrack = []

    for start_pos in range(0, scene.step_count):
        y = float(start_pos) / scene.step_count
        classes = [ get_move_right_classification(start_pos, pos, 
                        scene.fvalues, scene.maxima, params)
                    for pos in range(start_pos, scene.step_count) ]
        segments = segmentation(classes)
        for start, end in zip(segments[:-1], segments[1:]):
            fstart = float(start + start_pos) / scene.step_count
            fend   = float(end + start_pos)   / scene.step_count
            if classes[start] == Action.CONTINUE:
                segments_continue.append((fstart, fend, y))
            elif classes[start] == Action.TURN_PEAK:
                segments_turn_peak.append((fstart, fend, y))
            elif classes[start] == Action.BACKTRACK:
                segments_backtrack.append((fstart, fend, y))
            else:
                assert False

    print_array_assignment("xstarts", [ a for a, _, _ in segments_continue ])
    print_array_assignment("xends",   [ b for _, b, _ in segments_continue ])
    print_array_assignment("ys",      [ c for _, _, c in segments_continue ])
    print "for (i in 1:%d) { segments(xstarts[i], ys[i], xends[i], ys[i]," \
          " col=\"black\") }" % len(segments_continue)
    print_array_assignment("xstarts", [ a for a, _, _ in segments_turn_peak ])
    print_array_assignment("xends",   [ b for _, b, _ in segments_turn_peak ])
    print_array_assignment("ys",      [ c for _, _, c in segments_turn_peak ])
    print "for (i in 1:%d) { segments(xstarts[i], ys[i], xends[i], ys[i]," \
          " col=\"green\") }" % len(segments_turn_peak)
    print_array_assignment("xstarts", [ a for a, _, _ in segments_backtrack ])
    print_array_assignment("xends",   [ b for _, b, _ in segments_backtrack ])
    print_array_assignment("ys",      [ c for _, _, c in segments_backtrack ])
    print "for (i in 1:%d) { segments(xstarts[i], ys[i], xends[i], ys[i]," \
          " col=\"red\") }" % len(segments_backtrack)


def print_R_script(scene, params):

    print "# " + scene.filename + "\n"

    # Some R functions for plotting.
    print_set_window_division(3, 1)
    print "frame()"
    print "library(scales)" # for alpha blending

    print_right_classes(scene, params)

    print ""
    print_plot_focus_measures(scene.fvalues, (-0.1, 1))
    print ""

    print "plot.new()"
    print_left_classes(scene, params)

    print "\n# Plot me!\n"


def main(argv):
    params = ParameterSet()

    # Process command line options.
    for arg in argv:
        if arg == "--closest-peak":
            params.peakHandling = PeakHandling.CLOSEST
        elif arg == "--backtrack-faster":
            params.backtrackHandling = BacktrackHandling.FASTER

    scenes = load_scenes()

    for scene in scenes:
        print_R_script(scene, params)


main(sys.argv[1:])