#!/usr/bin/python

import getopt
import inspect
import os
import sys

from scene import *
from featuresturn import *

PCHS = [ 3, 1, 7 ]
COLORS = [ "black", "green", "red" ]

def print_array_assignment(var_name, array):
    print var_name + " <- c(" + \
          ",".join(["%.3f" % v if isinstance(v, float) else str(v) 
                    for v in array]) + ")"


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

    for start_pos in range(0, scene.measuresCount):
        y = float(start_pos) / scene.measuresCount
        classes = [ get_move_left_classification(start_pos, pos, 
                        scene.measuresValues, scene.maxima, params)
                    for pos in range(0, start_pos + 1) ]
        segments = segmentation(classes)
        for start, end in zip(segments[:-1], segments[1:]):
            fstart = float(start) / scene.measuresCount
            fend   = float(end)   / scene.measuresCount
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

    for start_pos in range(0, scene.measuresCount):
        y = float(start_pos) / scene.measuresCount
        classes = [ get_move_right_classification(start_pos, pos, 
                        scene.measuresValues, scene.maxima, params)
                    for pos in range(start_pos, scene.measuresCount) ]
        segments = segmentation(classes)
        for start, end in zip(segments[:-1], segments[1:]):
            fstart = float(start + start_pos) / scene.measuresCount
            fend   = float(end + start_pos)   / scene.measuresCount
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

    print "# " + scene.fileName + "\n"

    # Print the focus measures first. Normalize so that the maximum is 1 (but
    # without touching the minimum!) these so that they fit on the graph.
    maximum = max(scene.measuresValues)
    print_array_assignment("focusmeasures", [ float(v) / maximum for v in
                                              scene.measuresValues ] )

    # Some R functions for plotting.
    print "par(mfrow=c(3,1))"
    print "frame()"
    print "library(scales)" # for alpha blending

    print_right_classes(scene, params)

    print "\nplot(focusmeasures, pch=8, ylim=c(-0.1,1))"
    print "lines(focusmeasures)"

    print "plot.new()"
    print_left_classes(scene, params)

    print "\n# Plot me!\n"


def main(argv):

    if not os.path.isdir(scenes_folder):
        print scenes_folder + " folder not found."
        return

    params = ParameterSet()

    # Process command line options.
    for arg in argv:
        if arg == "--closest-peak":
            params.peakHandling = PeakHandling.CLOSEST
        elif arg == "--backtrack-faster":
            params.backtrackHandling = BacktrackHandling.FASTER

    scenes = load_scenes()
    load_maxima_into_measures(scenes)

    for scene in scenes:
        print_R_script(scene, params)


main(sys.argv[1:])