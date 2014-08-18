#!/usr/bin/python
"""Generate an R script to plot the location of the nearest peak vs the
two relative focus measures used as features for prediction in (Han, 2011)
"""

from scene import load_scenes
import rtools
import sys

def ratio(x, y):
    return (x - y) / y

def main(argv):
    scenes_folder = "focusraw/"
    scenes = load_scenes(folder=scenes_folder)

    ratio1 = [ ratio(scene.fvalues[0], scene.fvalues[8])
               for scene in scenes ]
    ratio2 = [ ratio(scene.fvalues[8], scene.fvalues[16])
               for scene in scenes ]
    first_maxima = [ scene.maxima[0] for scene in scenes ]

    print "library(plotrix)"

    rtools.print_array_assignment("ratio1", ratio1)
    rtools.print_array_assignment("ratio2", ratio2)
    rtools.print_array_assignment("first_maxima", first_maxima)

    print "plot(ratio1, ratio2, xlim=c(-0.070,0.030), ylim=c(-0.070,0.010))"
    print "# To avoid overlapping labels"
    print "thigmophobe.labels(ratio1,ratio2,labels=first_maxima,cex=1.0)"


main(sys.argv[1:])