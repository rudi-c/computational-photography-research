# Functions useful for printing R scripts for plotting and visualizing results.

def print_array_assignment(var_name, array):
    """Print the assignment of an array of floats to a variable in R
    to up to 3 decimal places - e.g. array <- c(1.000, 2.000, 3.000)"""
    print var_name + " <- c(" + \
          ",".join(["%.3f" % v if isinstance(v, float) else str(v) 
                    for v in array]) + ")"


def print_set_window_division(rows, cols):
    """Print command to set the number of window subdivisions (number
    plots that fit in one window)"""
    print "par(mfrow=c(%d,%d))" % (rows, cols)


def print_plot_focus_measures(focus_measures, yrange=(0,1), show_grid=False):
    """Plot the focus measures divided by the maximum. An optional range
    can be specified as a tuple."""
    maximum = max(focus_measures)
    print_array_assignment("focusmeasures", [ float(v) / maximum for v in
                                              focus_measures ] )
    # Type 'o' means plot both points and lines joining the points.
    print "plot(x=seq(0,%d), y=focusmeasures, " \
          "pch=8, type=\"o\", ylim=c(%d,%d))" % \
          (len(focus_measures) - 1, yrange[0], yrange[1])
    if show_grid:
        print "grid(nx=32,ny=1)"


def print_plot_point_pairs(xs, ys, pch, point_color, bg_color, lines=False,
    point_alpha=0.5, bg_alpha=0.5):
    """Plot pairs of points (x, y) with a given shape and color. The default
    transparency is 0.5"""
    assert len(xs) == len(ys)

    print_array_assignment("xs", xs)
    print_array_assignment("ys", ys)
    print "points(xs, ys, pch=%d, col=alpha(\"%s\", %.2f), " \
          "bg=alpha(\"%s\", %.2f))" % (pch, point_color, point_alpha, \
          bg_color, bg_alpha)
    if lines:
        print "lines(xs, ys)"


def print_classification_points(classes, results, leaves):

    # Axis to indicate that the bottom points mean left and
    # the top points means right.
    for index, name in enumerate(leaves):
        print "axis(2, at=%.2f, labels=\"%s\", padj=-2)" % \
            (float(index) / (len(leaves) - 1), name)

    # Legend to differentiate correct and predicted.
    # pch indicates the shape of the points 
    print "legend(\"left\", pch=c(25, 22), col=c(\"brown\", \"blue\"), " \
          "legend=c(\"correct\", \"predicted\"))"

    classes_ys = [float(leaves.index(c)) / (len(leaves) - 1) for c in classes]
    results_ys = [float(leaves.index(r)) / (len(leaves) - 1) for r in results]

    print_array_assignment("classes", classes_ys)
    print_array_assignment("results", results_ys)

    # Indicate the correct classes (left or right) and
    # the predicted classes. The predicted classes is
    # slightly offset to avoid overlapping.
    print "points(classes, pch=25, col=alpha(\"black\", 0.3), " \
          "bg=alpha(\"brown\", 0.5))"
    print "points(results - 0.02, pch=22, col=alpha(\"black\", 0.3), " \
          "bg=alpha(\"blue\", 0.5))"
