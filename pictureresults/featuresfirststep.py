"""
Set of functions relating to the classification of focus values obtained in
the first few steps.

These are used to build evaluate decision trees to determine whether the first
step is a fine step or a coarse step and whether we should look left or right.

Features are lambda expressions taking an arbitrary number of
keyword arguments. Current arguments used include :
first (first focus measurement, [0, 1])
second (second focus measurement, [0, 1])
third (third focus measurement, [0, 1])
lens_pos (0 for the first lens position, 1 for the last lens position)
"""

import math
import scene


def safe_ratio_ls(numerator, denominator, k):
    """Compare the fraction with k and handle division by zero."""
    return denominator != 0 and float(numerator) / denominator < k

def safe_ratio_gt(numerator, denominator, k):
    """Compare the fraction with k and handle division by zero."""
    return denominator != 0 and float(numerator) / denominator > k

def make_ratio_features(feature_maker, outputs, ratios):
    """Create a list of features for a given family of features for various
    parameters k in the argument 'ratios'.
    """
    name = feature_maker.__name__
    return [("%s_%d" % (name, k), outputs, feature_maker(ratio))
            for k, ratio in enumerate(ratios)]

def first_three_lens_pos(lens_pos, step_size=1):
    """Return the first three lens positions that are visited every time in
    our algorithm, which are used to make the initial decision of whether
    to look left or right.
    """
    return [lens_pos - 2 * step_size, lens_pos - 1 * step_size, lens_pos]

### Features taking the focus measure value at two lens positions ###

def ratio2(k):
    """Ratio of two numbers."""
    assert k > 0
    def feature(**kwargs):
        first, second = kwargs["first"], kwargs["second"]
        return safe_ratio_ls(first, second, k)
    return feature

def log_ratio2(k):
    """Ratio of the log of two numbers."""
    assert k > 0
    def feature(**kwargs):
        first, second = kwargs["first"], kwargs["second"]
        return safe_ratio_ls(math.log(first + 1.0), math.log(second + 1.0), k)
    return feature

def diff_over_avg2(k):
    """Difference between two numbers normalized by their average."""
    assert -1 < k < 1
    def feature(**kwargs):
        first, second = kwargs["first"], kwargs["second"]
        return safe_ratio_ls(2.0 * (second - first), (second + first), k)
    return feature

def diff_over_min2(k):
    """Difference between two numbers normalized by the smallest of the two."""
    assert -1 < k < 1
    def feature(**kwargs):
        first, second = kwargs["first"], kwargs["second"]
        return safe_ratio_ls(second - first, min(second, first), k)
    return feature

def diff_over_max2(k):
    """Difference between two numbers normalized by the largest of the two."""
    assert -1 < k < 1
    def feature(**kwargs):
        first, second = kwargs["first"], kwargs["second"]
        return safe_ratio_ls(second - first, max(second, first), k)
    return feature

def two_measure_features(filters=None):
    """ Returns an array of (attribute name, attribute range, function) 
        where the functions take two arguments."""
    if filters is None:
        filters = []

    # We're using arrays instead of dicts here to maintain a constant order.
    features = []

    # Ratio are chosen to be symmetrical.
    # e.g. 0.75 and 1.25 are not symmetrical ratios, but 0.75 and 1.333 are
    ratios = [ 1.64, 1.32, 1.16, 1.08, 1.04, 1.02, 1.01, 1.00,
               1/1.01, 1/1.02, 1/1.04, 1/1.08, 1/1.16, 1/1.32, 1/1.64 ]
    features.extend(make_ratio_features(ratio2, "{0,1}", ratios))
    features.extend(make_ratio_features(log_ratio2, "{0,1}", ratios))

    ratios = [ -0.64, -0.32, -0.16, -0.08, -0.04, -0.02, -0.01, 0.0,
                0.01,  0.02,  0.04,  0.08,  0.16,  0.32,  0.64 ]
    features.extend(make_ratio_features(diff_over_avg2, "{0,1}", ratios))
    features.extend(make_ratio_features(diff_over_min2, "{0,1}", ratios))
    features.extend(make_ratio_features(diff_over_max2, "{0,1}", ratios))

    if filters:
        return [(name, outputs, feature) for name, outputs, feature in features 
                if name in filters]
    else:
        return features

### Features taking the focus measure value at three lens positions ###

def ratio3(k):
    """Ratio of two numbers."""
    assert k > 0
    def feature(**kwargs):
        fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
        return safe_ratio_ls(float(fst), trd, k)
    return feature

def log_ratio3(k):
    """Ratio of the log of two numbers."""
    assert k > 0
    def feature(**kwargs):
        fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
        return safe_ratio_ls(math.log(fst + 1.0), math.log(trd + 1.0), k)
    return feature

def diff_over_avg3(k):
    """Difference between two numbers normalized by their average."""
    assert -1 < k < 1
    def feature(**kwargs):
        fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
        return safe_ratio_ls(2.0 * (trd - fst), (trd + fst), k)
    return feature

def diff_over_min3(k):
    """Difference between two numbers normalized by the smallest of the two."""
    assert -1 < k < 1
    def feature(**kwargs):
        fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
        return safe_ratio_ls(trd - fst, min(trd, fst), k)
    return feature

def diff_over_max3(k):
    """Difference between two numbers normalized by the largest of the two."""
    assert -1 < k < 1
    def feature(**kwargs):
        fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
        return safe_ratio_ls(trd - fst, max(trd, fst), k)
    return feature

def curving(k):
    """Method 1 of calculating the second derivative."""
    def feature(**kwargs):
        fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
        return safe_ratio_ls(fst + trd - 2 * snd, snd, k)
    return feature

def curving_ratio(k):
    """Method 2 of calculating the second derivative."""
    def feature(**kwargs):
        fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
        return safe_ratio_ls(fst - snd, snd - trd, k)
    return feature

def downTrend(**kwargs):
    """Monotonically decreasing."""
    fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
    return fst >= snd and snd >= trd

def upTrend(**kwargs):
    """Monotonically increasing."""
    fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
    return fst <= snd and snd <= trd

def three_measure_features(filters=None):
    """ Returns an array of (attribute name, attribute range, function) 
        where the functions take three arguments."""
    if filters is None:
        filters = []

    # We're using arrays instead of dicts to maintain a constant order.
    features = [ ("downTrend", "{0,1}", downTrend), 
                 ("upTrend"  , "{0,1}", upTrend) ]

    # Ratio are chosen to be symmetrical.
    # e.g. 0.75 and 1.25 are not symmetrical ratios, but 0.75 and 1.333 are
    ratios = [ 1.64, 1.32, 1.16, 1.08, 1.04, 1.02, 1.01, 1.00,
               1/1.01, 1/1.02, 1/1.04, 1/1.08, 1/1.16, 1/1.32, 1/1.64 ]
    features.extend(make_ratio_features(ratio3, "{0,1}", ratios))
    features.extend(make_ratio_features(log_ratio3, "{0,1}", ratios))

    ratios = [ -0.64, -0.32, -0.16, -0.08, -0.04, -0.02, -0.01, 0.0,
                0.01,  0.02,  0.04,  0.08,  0.16,  0.32,  0.64 ]
    features.extend(make_ratio_features(diff_over_avg3, "{0,1}", ratios))
    features.extend(make_ratio_features(diff_over_min3, "{0,1}", ratios))
    features.extend(make_ratio_features(diff_over_max3, "{0,1}", ratios))
    features.extend(make_ratio_features(curving, "{0,1}", ratios))
    features.extend(make_ratio_features(curving_ratio, "{0,1}", ratios))

    if len(filters) > 0:
        return [(name, values, feature) for name, values, feature in features 
                if name in filters]
    else:
        return features

### Other features ###

def bracket(brackets):
    # start and end to be numbers from 0 to 1
    def feature(**kwargs):
        lens_pos = kwargs["lens_pos"]
        assert 0 <= lens_pos <= 1
        assert lens_pos >= brackets[0]

        # Return the index corresponding to the correct bracket
        for i in range(0, len(brackets) - 1):
            if lens_pos >= brackets[i] and lens_pos < brackets[i + 1]:
                return i
        return len(brackets) - 1

    return feature

def other_features(filters=None):
    """ Returns an array of (attribute name, attribute range, function)"""
    if filters is None:
        filters = []

    # I've decided not to distribute the brackets evenly because it matters
    # less if the lens is near the center than if it is near the end
    
    # Five brackets. Experiments seem to suggest this is sufficient. Twelve
    # may lead to overfitting. Three might be to restrictive for the leftmost
    # and rightmost peaks.
    brackets = [ 0.0, 0.08, 0.20, 0.80, 0.92 ]

    bracket_range = "{" + ",".join(str(i) for i in range(len(brackets))) + "}"
    features = [ ( "bracket", bracket_range, bracket(brackets) )]

    if len(filters) > 0:
        return [(name, values, feature) for name, values, feature in features 
                if name in filters]
    else:
        return features

### Convenience function to return groups of features ###

def measure_features(filters=None):
    """ Returns an array of (attribute name, attribute range, function)"""
    if filters is None:
        filters = []
    return two_measure_features(filters) + three_measure_features(filters)


def all_features(filters=None):
    """ Returns an array of (attribute name, attribute range, function)"""
    if filters is None:
        filters = []
    return (two_measure_features(filters) + three_measure_features(filters) +
            other_features(filters))


def firststep_feature_evaluator(first, second, third, lens_pos):
    """Returns a function to evaluate a feature for deciding left vs right.
    The lens_pos argument needs to be normalized to [0, 1]!"""
    assert 0 <= lens_pos <= 1
    feature_args = { "first" : first, "second"   : second,
                     "third" : third, "lens_pos" : lens_pos }
    def evaluate(feature):
        return feature(**feature_args)

    return evaluate


### Classifiers for deciding to look left or right ###

def highest_on_left(scene, lens_pos):
    """Return true if the highest peak in the scene is on the left of 
    the lens position.
    """
    # Find the location of the highest peak
    highest = scene.maxima[0]
    for maxima in scene.maxima:
        if scene.fvalues[maxima] > scene.fvalues[highest]:
            highest = maxima
    return highest < lens_pos

def nearest_on_left(scene, lens_pos):
    """Return true if the nearest peak in the scene to the lens position is
    on its left side.
    """
    # Find the location of the nearest peak
    nearest = scene.maxima[0]
    for maxima in scene.maxima:
        if abs(lens_pos - maxima) < abs(lens_pos - nearest):
            nearest = maxima
    return nearest < lens_pos

def highest_and_near_on_left(scene, lens_pos):
    """Return true if the peak that maximizes (height)/(distance) in the scene 
    is on the left of the lens position.
    """
    # Find the location of the peak that maximizes height
    # and distance, equally weighted in a product
    best = scene.maxima[0]
    for maxima in scene.maxima:
        # Need to add a + 1 to the distance to avoid division by zero.
        if (scene.fvalues[maxima] / (abs(lens_pos - maxima) + 1) >
            scene.fvalues[best] / (abs(lens_pos - best) + 1)):
            best = maxima
    return best < lens_pos
