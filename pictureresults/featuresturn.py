"""
Functions that evaluate features.
Set of functions relating to classification during the sweep process, where at
each step we must decide one of three actions :
1) Turn back to peak (we found a peak)
2) Backtrack (give up looking in one direction and try the other)
3) Continue

Features are lambda expressions taking an arbitrary number of
keyword arguments. Current arguments used include :
lens_positions (an array of all the lens positions visited so far,
                the current one at the end)
focus_values (a mapping from lens positions to calculated focus values. This
              should have as many elements as the number of lens positions)
total_positions (total number of lens positions for this lens)
Note : Some features assume that at least one step has been taken (i.e.,
       that len(lens_positions) >= 2)
"""

from direction import Direction
import math

step_size = 8

def make_key(direction, initial_pos, current_pos):
    """Returns a key to identify a particular instance (from direction,
    initial position and current position."""
    return "%s-%d-%d" % (direction, initial_pos, current_pos)

### Features related to the number of steps and distance ###

def distance_swept(**kwargs):
    """Number of steps taken up to now, normalized by the total number of 
    lens positions."""
    total_positions = kwargs["total_positions"]
    focus_measures = kwargs["focus_measures"]

    return float(len(focus_measures) - 1) / total_positions

functions_steps_and_distance = [ distance_swept ]

### Scagnostics ###

def rank(vector):
    """Calculate the ranks of elements in a vector - i.e., for each x, find
    the number of elements smaller than x"""
    sorted_vector = sorted(vector)
    return [sorted_vector.index(x) for x in vector]


def monotonicity(**kwargs):
    """Measure of monotonicity can be obtained by using Spearman's Rank
    Correlation coefficient, with gives +1 if the function is perfectly
    monotonic and increasing and -1 if the function is perfectly monotonic
    and decreasing"""
    focus_measures = kwargs["focus_measures"]

    ranks_xs = rank(range(len(focus_measures)))
    ranks_ys = rank([f for f in focus_measures])
    mean_xs = float(sum(ranks_xs)) / len(ranks_xs)
    mean_ys = float(sum(ranks_ys)) / len(ranks_ys)
    covariance = sum( (x - mean_xs) * (y - mean_ys) 
                      for x, y in zip(ranks_xs, ranks_ys) )
    variance_x = sum( (x - mean_xs) ** 2 for x in ranks_xs )
    variance_y = sum( (y - mean_ys) ** 2 for y in ranks_ys )

    assert variance_x != 0
    if variance_y == 0:
        # Variance of 0 means the function is neither increasing or decreasing
        # not much we can do here, but this should be very rare.
        return 0

    return float(covariance) / math.sqrt(variance_x * variance_y)


def abs_monotonicity(**kwargs):
    """Absolute value of monotonicity"""
    return abs(monotonicity(**kwargs))


def alternation_ratio(**kwargs):
    """Measure of noise by counting the number of triples of lens positions
    which are not monotonic (value goes down and up or up and down)."""
    focus_measures = kwargs["focus_measures"]

    triples = zip(focus_measures, focus_measures[1:], focus_measures[2:])
    count = sum( (f1 < f2) != (f2 < f3)
                 for f1, f2, f3 in triples )
    return float(count) / (len(focus_measures) - 2)


functions_scagnostics = [ monotonicity, abs_monotonicity, alternation_ratio ]

### Features related to comparison with the maximum we've got so far ###

def ratio_to_max(**kwargs):
    """How the focus value at the current position compares to the largest
    value found so far"""
    focus_measures = kwargs["focus_measures"]

    max_so_far = float(max(focus_measures))
    return focus_measures[-1] / max_so_far


def ratio_to_range(**kwargs):
    """How the focus value at the current position compares to the range of
    values (min and max) found so far"""
    focus_measures = kwargs["focus_measures"]

    latest = focus_measures[-1]
    max_so_far = float(max(focus_measures))
    min_so_far = float(min(focus_measures))
    if min_so_far == max_so_far:
        return 0.5 # I think this is a fair "in-between" value
    return (latest - min_so_far) / (max_so_far - min_so_far)


def distance_to_max(**kwargs):
    """The distance (in steps taken) between the current position and the
    position of the largest value found so far, normalized by the total number
    of lens positions"""
    total_positions = kwargs["total_positions"]
    focus_measures = kwargs["focus_measures"]

    max_so_far = float(max(focus_measures))
    max_distances = ( len(focus_measures) - i 
                      for i, focus_value in enumerate(focus_measures) 
                      if focus_value == max_so_far )
    # Though unlikely, there could be multiple peaks with the same value.
    closest = min(max_distances)
    return float(closest) / total_positions


def ratio_min_to_max(**kwargs):
    """How the smallest focus value found so far compares to the largest focus
    value found so far."""
    focus_measures = kwargs["focus_measures"]

    max_so_far = float(max(focus_measures))
    min_so_far = float(min(focus_measures))
    return float(min_so_far) / max_so_far


functions_maximum = [ ratio_to_max, ratio_to_range, distance_to_max,
                      ratio_min_to_max ]

### Features related to the overall trend of focus values ###

def slope(steps_in_between, f1, f2, total_positions):
    normalized_diff = float(steps_in_between * step_size) / total_positions
    average = (f2 + f1) / 2
    return (f2 - f1) / normalized_diff / average


def simple_slope(**kwargs):
    """Simple calculation of the slope using only the endpoints,
    normalized by the total number of lens positions and the
    average value of the two endpoints."""
    total_positions = kwargs["total_positions"]
    focus_measures = kwargs["focus_measures"]

    first  = focus_measures[0]
    latest = focus_measures[-1]
    return slope(len(focus_measures), first, latest, total_positions)
           

def regression_slope(**kwargs):
    """Slope as obtained by linear regression (least squares),
    normalized by the total number of lens positions and the
    average value of the positions."""
    total_positions = kwargs["total_positions"]
    focus_measures = kwargs["focus_measures"]

    xs = [ float(x * step_size) / total_positions 
           for x in range(len(focus_measures)) ]
    mean_xs = sum(xs) / len(focus_measures)
    mean_ys = float(sum(focus_measures)) / len(focus_measures)
    covariance = sum( (x - mean_xs) * (focus_value - mean_ys)
                      for x, focus_value in zip(xs, focus_measures) )
    variance_x = sum( (x - mean_xs) ** 2 for x in xs )
    assert variance_x != 0
    return covariance / variance_x / mean_ys


def simple_slope_up(**kwargs):
    return 0 if simple_slope(**kwargs) < 0 else 1


def regression_slope_up(**kwargs):
    return 0 if regression_slope(**kwargs) < 0 else 1

functions_trend_slope = [ simple_slope, regression_slope,
                          simple_slope_up, regression_slope_up ]

### Features related to the local slope ###

def current_slope(**kwargs):
    """Slope between the current lens position and the previous 
    lens position, normalized by the total number of lens positions
    and the average value of the two positions."""
    total_positions = kwargs["total_positions"]
    focus_measures = kwargs["focus_measures"]

    latest = focus_measures[-1]
    previous = focus_measures[-2]
    return slope(len(focus_measures), previous, latest, total_positions)


def current_slope_large(**kwargs):
    """Slope between the current lens position and the previous previous 
    lens position, normalized by the total number of lens positions
    and the average value of the two positions."""
    total_positions = kwargs["total_positions"]
    focus_measures = kwargs["focus_measures"]

    latest = focus_measures[-1]
    previous = focus_measures[-3]
    return slope(len(focus_measures), previous, latest, total_positions)


def current_slope_up(**kwargs):
    return 0 if current_slope(**kwargs) < 0 else 1


def current_slope_large_up(**kwargs):
    return 0 if current_slope_large(**kwargs) < 0 else 1


def downslope_1st_half(**kwargs):
    """Proportion of lens movements where the focus value went down in the
    first half of our lens movements"""
    focus_measures = kwargs["focus_measures"]
    
    halfpoint = int(math.ceil(float(len(focus_measures)) / 2))
    focus_measures = focus_measures[:halfpoint]
    count = sum( f1 > f2 for f1, f2 
                 in zip(focus_measures[:-1], focus_measures[1:]) )
    return float(count) / (len(focus_measures) - 1)


def downslope_2nd_half(**kwargs):
    """Proportion of lens movements where the focus value went down in the
    second half of our lens movements"""
    focus_measures = kwargs["focus_measures"]
    
    halfpoint = int(math.floor(float(len(focus_measures)) / 2))
    focus_measures = focus_measures[halfpoint:]
    count = sum( f1 > f2 for f1, f2 
                 in zip(focus_measures[:-1], focus_measures[1:]) )
    return float(count) / (len(focus_measures) - 1)




functions_local_slope = [ current_slope, current_slope_large,
                          current_slope_up, current_slope_large_up,
                          downslope_1st_half, downslope_2nd_half ]

### For convenience ###

def all_features(filters=None):
    """Returns an array of (attribute name, function)"""
    if filters is None:
        filters = []
    functions = (functions_steps_and_distance +
                 functions_scagnostics + functions_maximum +
                 functions_trend_slope + functions_local_slope)

    if len(filters) > 0:
        return [ (f.__name__, f) for f in functions if
                         f.__name__ in filters ]
    else:
        return [ (f.__name__, f) for f in functions ]
    

def all_features_dict(filters=None):
    """Returns a dictionnary { feature name: function }"""
    if filters is None:
        filters = []
    return { name: function for name, function in all_features(filters) }


def action_feature_evaluator(focus_measures, total_positions):
    """Returns a function to evaluate an action feature."""
    assert len(focus_measures) >= 3
    feature_args = { "focus_measures"  : list(focus_measures),
                     "total_positions" : total_positions}
    def evaluate(feature):
        return feature(**feature_args)

    return evaluate

### Classifiers ###

# Enums
class Action:
    CONTINUE, TURN_PEAK, BACKTRACK = range(0, 3)

class PeakHandling:
    ALWAYSTURN, CLOSEST = range(0, 2)

class BacktrackHandling:
    NOPEAKSONLY, FASTER = range(0, 2)

class OutlierHandling:
    WEIGHTING, SAMPLING = range(0, 2)

class_names = { Action.CONTINUE : "continue", 
                Action.TURN_PEAK : "turn_peak", 
                Action.BACKTRACK : "backtrack" }

class ParameterSet:
    def __init__(self):
        self.peakHandling = PeakHandling.ALWAYSTURN
        self.backtrackHandling = BacktrackHandling.NOPEAKSONLY
        # Probability that any given instance will be kept (this is to
        # reduce the amount of data, which we can afford since it's quite
        # redudant)
        self.uniformSamplingRate = 0.4

        # Whether we weight less important instance or sample them at
        # lower rates
        self.outlierHandling = OutlierHandling.SAMPLING

        # Factor by which to multiply the weight or sampling rate when we
        # encounter a lens position where the classification is to continue
        self.continueRatio = 0.99

        # Factor by which to multiply the weight or sampling rate when we
        # encounter a lens position where the classification is to turn back
        self.turnbackRatio = 0.93


def _successor(value, array):
    """Find the smallest value greater than a specified number"""
    smallest = None
    for a in array:
        if a > value:
            if smallest is None:
                smallest = a
            elif a < smallest:
                smallest = a
    return smallest


def _predecessor(value, array):
    """Find the largest value smaller than or equal to a specified number"""
    largest = None
    for a in array:
        if a <= value:
            if largest is None:
                largest = a
            elif a > largest:
                largest = a
    return largest
    

def get_move_right_classification(start_lens_pos, current_lens_pos,
                                  focus_measures, maxima, params):

    visited_maxima = [ maximum for maximum in maxima 
                       if maximum >= start_lens_pos ]

    # # If we don't wait until there is at least a few lens positions before 
    # # turning back, the classification suggests that we might turn back 
    # # immediately before we chose to go left or right.
    # if abs(current_lens_pos - start_lens_pos) < 6:
    #     return Action.CONTINUE

    # Find the closest maxima on the left.
    left_closest = _predecessor(current_lens_pos, maxima)
    left_closest_visited = _predecessor(current_lens_pos, visited_maxima)
    right_closest = _successor(current_lens_pos, maxima)

    if left_closest_visited is None:
        if params.backtrackHandling == BacktrackHandling.FASTER:
            # Determine if it would be faster to just go the other way.
            if right_closest is None:
                return Action.BACKTRACK
            elif (abs(right_closest - current_lens_pos) >
                  abs(current_lens_pos - left_closest)):
                return Action.BACKTRACK
        else:
            assert params.backtrackHandling == BacktrackHandling.NOPEAKSONLY

        left_closest_to_start = _predecessor(start_lens_pos, maxima)
        if (left_closest_to_start is not None and
            start_lens_pos - left_closest_to_start <= 8 and
            current_lens_pos - left_closest_to_start > 8):
            return Action.TURN_PEAK
    else:
        # If we're at the last lens position, we need need to turn back to
        # the peak -now-.
        if current_lens_pos == len(focus_measures) - 1:
            return Action.TURN_PEAK

        # If we just passed peak, we should continue looking
        # forward for just a bit to confirm that this is a peak.
        if current_lens_pos - left_closest_visited <= 8:
            return Action.CONTINUE
        elif params.peakHandling == PeakHandling.CLOSEST:
            if right_closest is None:
                return Action.TURN_PEAK
            elif (current_lens_pos - left_closest_visited - 8 <
                  right_closest - current_lens_pos):
                return Action.TURN_PEAK
            else:
                return Action.CONTINUE
        else:
            assert params.peakHandling == PeakHandling.ALWAYSTURN
            return Action.TURN_PEAK

    # If there are no more peaks to the right, we should backtrack.
    if right_closest is None and abs(current_lens_pos - start_lens_pos) > 32:
        return Action.BACKTRACK

    # Default case : continue looking
    return Action.CONTINUE 



def get_move_left_classification(start_lens_pos, current_lens_pos,
                                 focus_measures, maxima, params):
    def reverse(pos):
        return len(focus_measures) - pos - 1
    # To classify the correct action when we are moving left is to
    # classify the correct action when we are moving right on the
    # reverse data.
    reversed_measures = list(focus_measures)
    reversed_measures.reverse()
    reversed_maxima = [reverse(maximum) for maximum in maxima]
    return get_move_right_classification(
        reverse(start_lens_pos), reverse(current_lens_pos), 
        reversed_measures, reversed_maxima, params)
