# Functions that evaluate features.

# Features are lambda expressions taking an arbitrary number of
# keyword arguments. Current arguments used include :
# lens_positions (an array of all the lens positions visited so far,
#                 the current one at the end)
# focus_values (a mapping from lens positions to calculated focus values. This
#               should have as many elements as the number of lens positions)
# total_positions (total number of lens positions for this lens)
#
# Note : Some features assume that at least one step has been taken (i.e.,
#        that len(lens_positions) >= 2)

from math import sqrt

def count_step_size(lens_positions):
    """Returns a tuple (# of small steps taken, # large steps taken)"""
    small = len([1 for p1, p2 in zip(lens_positions[:-1], lens_positions[1:])
                 if abs(p1 - p2) == 1])
    large = len([1 for p1, p2 in zip(lens_positions[:-1], lens_positions[1:])
                 if abs(p1 - p2) > 1])
    return (small, large)


### It might be useful for the machine learning algorithm to know explicitely
### if we're moving left (-1) or right (+1). 
def left_or_right(**kwargs):
    lens_positions = kwargs["lens_positions"]
    if lens_positions[0] < lens_positions[-1]:
        return +1
    else:
        return -1

### Features related to the number of steps and distance ###

def steps_taken(**kwargs):
    """Number of steps taken up to now, regardless of step size,
    normalized by the total number of lens positions."""
    total_positions = kwargs["total_positions"]
    lens_positions = kwargs["lens_positions"]
    return float(len(lens_positions) - 1) / total_positions


def large_steps_taken(**kwargs):
    """Number of large steps taken up to now, normalized by the 
    total number of lens positions."""
    total_positions = kwargs["total_positions"]
    lens_positions = kwargs["lens_positions"]
    small, large = count_step_size(lens_positions)
    return float(large) / total_positions


def small_steps_taken(**kwargs):
    """Number of large steps taken up to now, normalized by the 
    total number of lens positions."""
    total_positions = kwargs["total_positions"]
    lens_positions = kwargs["lens_positions"]
    small, large = count_step_size(lens_positions)
    return float(small) / total_positions


def ratio_small_steps(**kwargs):
    """Proportion of our steps taken that were small steps."""
    lens_positions = kwargs["lens_positions"]
    small, large = count_step_size(lens_positions)
    return float(small) / (small + large) # avoid division by zero


def ratio_large_steps(**kwargs):
    """Proportion of our steps taken that were large steps."""
    lens_positions = kwargs["lens_positions"]
    small, large = count_step_size(lens_positions)
    return float(large) / (small + large) # avoid division by zero


def distance_taken(**kwargs):
    """Number of lens position since initial points, normalized by
    the total number of lens positions"""
    total_positions = kwargs["total_positions"]
    lens_positions = kwargs["lens_positions"]
    first  = lens_positions[0]
    latest = lens_positions[-1]
    return float(abs(latest - first)) / total_positions


# I think in general it's better to normalize, but it could be that the
# calculation for some other features (such as slope) gives bogus results
# when there are only 1-2 steps, so I'm hoping these two features can help
# the machine learning algorithm deal with those.

def absolute_steps_taken(**kwargs):
    """Number of steps taken up to now, regardless of step size,
    not normalized."""
    lens_positions = kwargs["lens_positions"]
    return float(len(lens_positions) - 1)


def absolute_distance_taken(**kwargs):
    """Number of lens position since initial points, not normalized"""    
    lens_positions = kwargs["lens_positions"]
    first  = lens_positions[0]
    latest = lens_positions[-1]
    return float(abs(latest - first))


### Features related to the distances to the last lens positions on either side

def distance_to_left(**kwargs):
    """Number of lens positions from the leftmost position, normalized by
    the total number of lens positions"""
    total_positions = kwargs["total_positions"]
    lens_positions = kwargs["lens_positions"]
    latest = lens_positions[-1]
    return float(latest) / (total_positions - 1)


def distance_to_right(**kwargs):
    """Number of lens positions from the rightmost position, normalized by
    the total number of lens positions"""
    total_positions = kwargs["total_positions"]
    lens_positions = kwargs["lens_positions"]
    latest = lens_positions[-1]
    return float(total_positions - latest - 1) / (total_positions - 1)


def ratio_left_to_right(**kwargs):
    """How the distance to the leftmost position compares with the distance
    to the rightmost positions. Maxes at 10.0."""
    total_positions = kwargs["total_positions"]
    lens_positions = kwargs["lens_positions"]
    latest = lens_positions[-1]
    dist_to_right = total_positions - latest - 1
    if dist_to_right == 0:
        return 10.0
    return min(float(latest) / dist_to_right, 10.0)


def ratio_right_to_left(**kwargs):
    """How the distance to the leftmost position compares with the distance
    to the rightmost positions. Maxes at 10.0."""
    total_positions = kwargs["total_positions"]
    lens_positions = kwargs["lens_positions"]
    latest = lens_positions[-1]
    if latest == 0:
        return 10.0
    return min(float(total_positions - latest - 1) / latest, 10.0)


### Scagnostics ###

def rank(vector):
    """Calculate the ranks of elements in a vector - i.e., for each x, find
    the number of elements smaller than x"""
    sorted_vector = sorted(vector)
    return [sorted_vector.index(x) for x in vector]


def monotoniticy(**kwargs):
    """Measure of monotoniticy can be obtained by using Spearman's Rank
    Correlation coefficient, with gives +1 if the function is perfectly
    monotonic and increasing and -1 if the function is perfectly monotonic
    and decreasing"""
    focus_values = kwargs["focus_values"]
    lens_positions = kwargs["lens_positions"]
    ranks_xs = rank(lens_positions)
    ranks_ys = rank([focus_values[x] for x in lens_positions])
    mean_xs = float(sum(ranks_xs)) / len(ranks_xs)
    mean_ys = float(sum(ranks_ys)) / len(ranks_ys)
    covariance = sum([ (x - mean_xs) * (y - mean_ys) 
                       for x, y in zip(ranks_xs, ranks_ys)])
    variance_x = sum([ (x - mean_xs) ** 2 for x in ranks_xs ])
    variance_y = sum([ (y - mean_ys) ** 2 for y in ranks_ys ])

    assert variance_x != 0
    if variance_y == 0:
        # Variance of 0 means the function is neither increasing or decreasing
        # not much we can do here, but this should be very rare.
        return 0

    return float(covariance) / sqrt(variance_x * variance_y)


### Features related to comparison with the maximum we've got so far ###

def ratio_to_max(**kwargs):
    """How the focus value at the current position compares to the largest
    value found so far"""
    focus_values = kwargs["focus_values"]
    lens_positions = kwargs["lens_positions"]
    latest = lens_positions[-1]
    max_so_far = float(max( [ focus_values[x] for x in lens_positions ] ))
    return focus_values[latest] / max_so_far


def ratio_to_range(**kwargs):
    """How the focus value at the current position compares to the range of
    values (min and max) found so far"""
    focus_values = kwargs["focus_values"]
    lens_positions = kwargs["lens_positions"]
    latest = lens_positions[-1]
    max_so_far = float(max( [ focus_values[x] for x in lens_positions ] ))
    min_so_far = float(min( [ focus_values[x] for x in lens_positions ] ))
    if min_so_far == max_so_far:
        return 0.5 # I think this is a fair "in-between" value
    return (focus_values[latest] - min_so_far) / (max_so_far - min_so_far)


def distance_to_max(**kwargs):
    """The distance (in lens positions) between the current position and the
    position of the largest value found so far, normalized by the total number
    of lens positions"""
    total_positions = kwargs["total_positions"]
    focus_values = kwargs["focus_values"]
    lens_positions = kwargs["lens_positions"]
    latest = lens_positions[-1]
    max_so_far = float(max( [ focus_values[x] for x in lens_positions ] ))
    max_positions = [ x for x in lens_positions 
                        if focus_values[x] == max_so_far ]
    # There may be more than one maximum in exceptional cases - pick the
    # closest one.
    closest = min( [ abs(latest - max_pos) for max_pos in max_positions ] )
    return float(closest) / total_positions


### Features related to the overall trend of focus values ###

def simple_slope(**kwargs):
    """Simple calculation of the slope using only the endpoints,
    normalized by the total number of lens positions."""
    focus_values = kwargs["focus_values"]
    total_positions = kwargs["total_positions"]
    lens_positions = kwargs["lens_positions"]
    first  = lens_positions[0]
    latest = lens_positions[-1]
    normalized_diff = float(latest - first) / total_positions
    return (focus_values[latest] - focus_values[first]) / normalized_diff
           

def regression_slope(**kwargs):
    """Slope as obtained by linear regression (least squares),
    normalized by the total number of lens positions."""
    focus_values = kwargs["focus_values"]
    total_positions = kwargs["total_positions"]
    lens_positions = kwargs["lens_positions"]

    mean_xs = float(sum(lens_positions)) / len(lens_positions) / total_positions
    mean_ys = float(sum([focus_values[x] for x in lens_positions])) / \
                                                        len(lens_positions)
    covariance = sum( [ (float(x) / total_positions - mean_xs) * 
                            (focus_values[x] - mean_ys)
                        for x in lens_positions ] )
    variance_x = sum( [ (float(x) / total_positions - mean_xs) ** 2 
                        for x in lens_positions ] )
    assert variance_x != 0
    return covariance / variance_x


### Features related to the local slope ###

def current_slope(**kwargs):
    """Slope between the current lens position and the previous 
    lens position, normalized by the total number of lens positions"""
    focus_values = kwargs["focus_values"]
    total_positions = kwargs["total_positions"]
    lens_positions = kwargs["lens_positions"]
    latest = lens_positions[-1]
    previous = lens_positions[-2]
    normalized_diff = float(latest - previous) / total_positions
    return (focus_values[latest] - focus_values[previous]) / normalized_diff


def current_slope_large(**kwargs):
    """Slope between the current lens position and another position at least
    4 lens positions away, normalized by the total number of lens positions"""
    # The point of this feature is that it should be more noise-resilient than
    # current slope when the last step taken was a fine step
    focus_values = kwargs["focus_values"]
    total_positions = kwargs["total_positions"]
    lens_positions = kwargs["lens_positions"]
    latest = lens_positions[-1]
    remaining = lens_positions[:-1]
    while len(remaining) > 1 and abs(latest - remaining[-1]) < 4:
        remaining = remaining[:-1]
    previous = remaining[-1]
    normalized_diff = float(latest - previous) / total_positions
    return (focus_values[latest] - focus_values[previous]) / normalized_diff


def downslope_1st_half(**kwargs):
    """Proportion of lens movements where the focus value went down in the
    first half of our lens movements"""
    focus_values = kwargs["focus_values"]
    total_positions = kwargs["total_positions"]
    lens_positions = kwargs["lens_positions"]
    
    major_half = len(lens_positions) / 2 + 1
    lens_positions = lens_positions[:max(2, major_half)]
    print focus_values
    count = [ focus_values[x1] > focus_values[x2] for x1, x2 
              in zip(lens_positions[:-1], lens_positions[1:]) ].count(True)
    return float(count) / (len(lens_positions) - 1)


def downslope_2nd_half(**kwargs):
    """Proportion of lens movements where the focus value went down in the
    second half of our lens movements"""
    focus_values = kwargs["focus_values"]
    total_positions = kwargs["total_positions"]
    lens_positions = kwargs["lens_positions"]
    
    half_rounded_down = len(lens_positions) / 2
    lens_positions = lens_positions[min(len(lens_positions) - 2, 
                                        half_rounded_down):]
    count = [ focus_values[x1] > focus_values[x2] for x1, x2 
              in zip(lens_positions[:-1], lens_positions[1:]) ].count(True)
    return float(count) / (len(lens_positions) - 1)
