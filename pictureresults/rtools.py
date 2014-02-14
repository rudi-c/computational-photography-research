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


def print_plot_focus_measures(focus_measures, yrange=(0,1)):
    """Plot the focus measures divided by the maximum. An optional range
    can be specified as a tuple."""
    maximum = max(focus_measures)
    print_array_assignment("focusmeasures", [ float(v) / maximum for v in
                                              focus_measures ] )
    print "plot(focusmeasures, pch=8, ylim=c(%d,%d))" % yrange
    print "lines(focusmeasures)"


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