#!/usr/bin/python

# This script simulates a sweep using the decision tree learned in
# "An autofocus heuristic for digital cameras based on supervised 
#  machine learning" and outputs the classification obtained at every
# lens position it visits.

import getopt
import inspect
import os
import sys

from scene      import *
from coarsefine import *
from rtools     import *


def simulate(scene):

    last_step_coarse = True
    f_cur   = scene.measuresValues[0]
    f_prev  = scene.measuresValues[0]
    f_prev2 = scene.measuresValues[0]
    position = 1

    output = []

    while position < scene.measuresCount:
        f_prev2 = f_prev
        f_prev = f_cur
        f_cur = scene.measuresValues[position]
        if last_step_coarse:
            step_coarse = coarse_if_previously_coarse(f_cur, f_prev, f_prev2)
        else:
            step_coarse = coarse_if_previously_fine(f_cur, f_prev, f_prev2)

        if step_coarse:
            output.append((position, 0))
            position += 8
        else:
            output.append((position, 1))
            position += 1

        last_step_coarse = step_coarse

    return output


def print_R_script(scene):

    print "# " + scene.fileName + "\n"

    # Some R functions for plotting.
    print "library(scales)" # for alpha blending
    print_plot_focus_measures(scene.measuresValues)

    # Axis to indicate that the bottom points mean coarse and
    # the top points means fine.
    print "axis(2, at=0, labels=\"coarse\", padj=-2)"
    print "axis(2, at=1.0, labels=\"fine\", padj=-2)"

    # Points to plot
    results = simulate(scene)
    xs, ys = ([ a for a,b in results ], [ b for a,b in results ]) # unzip

    # Indicate the correct classes (coarse or fine) and the predicted classes. 
    # The predicted classes is slightly offset to avoid overlapping.
    print_plot_point_pairs(xs, ys, 22, "black", "blue", False, 0.3, 0.5)

    # In the title, indicate how many steps are used.
    print "title(main=paste(\"coarse steps:\", %d, " \
                           "\"fine steps:\", %d, " \
                           "\"total steps:\", %d))" \
          % (ys.count(0), ys.count(1), len(ys))

    print "# Plot me!\n"


def print_script_usage():
   print  """Script usage : ./evaluatestepsize.py 
             [-t <decision tree to evaluate>]
             [-c <classifier (highest, nearest, near_high)>]
             [-d <double step size used>]"""


def main(argv):

    # Parse script arguments
    try:
        opts, args = getopt.getopt(argv,"s:",
                                  ["scene="])
    except getopt.GetoptError:
        print_script_usage()
        sys.exit(2)

    scene = None

    for opt, arg in opts:
        if opt in ("-s", "--scene"):
            scene = Scene(arg)

    if scene == None:
        print_script_usage()
        sys.exit(2)

    load_maxima_into_measures([scene])
    print_R_script(scene)


main(sys.argv[1:])