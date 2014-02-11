# Functions that evaluate features.

# Features are lambda expressions taking an arbitrary number of
# keyword arguments. Current arguments used include :
# first (first focus measurement, [0, 1])
# second (second focus measurement, [0, 1])
# third (third focus measurement, [0, 1])
# lens_pos (0 for the first lens position, 1 for the last lens position)

from scene import *
from math  import log

def safeRatioLessThan(a, b, k):
    """Compare the ratio of a, b with k and handle division by zero."""
    return b != 0 and float(a) / b < k

def safeRatioMoreThan(a, b, k):
    """Compare the ratio of a, b with k and handle division by zero."""
    return b != 0 and float(a) / b > k

### Features taking the focus measure value at two lens positions ###

def ratio2(k):
    # positive numbers for k only
    def r(**kwargs):
        first, second = kwargs["first"], kwargs["second"]
        return safeRatioLessThan(first, second, k)
    return r

# def ratio2Inverse(k):
#     # positive numbers for k only
#     def r(**kwargs):
#         first, second = kwargs["first"], kwargs["second"]
#         return safeRatioLessThan(second, first, k)
#     return r

def logRatio2(k):
    # positive numbers for k only
    def r(**kwargs):
        first, second = kwargs["first"], kwargs["second"]
        return safeRatioLessThan(log(first + 1.0), log(second + 1.0), k)
    return r

# def logRatio2Inverse(k):
#     # positive numbers for k only
#     def r(**kwargs):
#         first, second = kwargs["first"], kwargs["second"]
#         return safeRatioLessThan(log(second + 1.0), log(first + 1.0), k)
#     return r

def diffRatioAvg2(k):
    # numbers from -1 to 1 for k (in practice, -0.5 to 0.5)
    def r(**kwargs):
        first, second = kwargs["first"], kwargs["second"]
        return safeRatioLessThan(2.0 * (second - first), (second + first), k)
    return r

def diffRatioMin2(k):
    # numbers from -1 to 1 for k (in practice, -0.5 to 0.5)
    def r(**kwargs):
        first, second = kwargs["first"], kwargs["second"]
        return safeRatioLessThan(second - first, min(second, first), k)
    return r

def diffRatioMax2(k):
    # numbers from -1 to 1 for k (in practice, -0.5 to 0.5)
    def r(**kwargs):
        first, second = kwargs["first"], kwargs["second"]
        return safeRatioLessThan(second - first, max(second, first), k)
    return r

def two_measure_features(filters=[]):
    """ Returns an array of (attribute name, attribute range, function) 
        where the functions take two arguments."""

    # We're using arrays instead of dicts to maintain a constant order.
    features = []

    # Ratio are chosen to be symmetrical.
    # e.g. 0.75 and 1.25 are not symmetrical ratios, but 0.75 and 1.333 are
    ratios = [ 1.64, 1.32, 1.16, 1.08, 1.04, 1.02, 1.01, 1.00,
               1/1.01, 1/1.02, 1/1.04, 1/1.08, 1/1.16, 1/1.32, 1/1.64 ]
    features += [("ratio2_" + str(k), "{0,1}", ratio2(ratios[k]) )
                 for k in range(0, len(ratios))]
    features += [("logRatio2_" + str(k), "{0,1}", logRatio2(ratios[k]) )
                 for k in range(0, len(ratios))]

    ratios = [ -0.64, -0.32, -0.16, -0.08, -0.04, -0.02, -0.01, 0.0,
                0.01,  0.02,  0.04,  0.08,  0.16,  0.32,  0.64 ]
    features += [("diffRatioAvg2_" + str(k), "{0,1}", diffRatioAvg2(ratios[k]) )
                 for k in range(0, len(ratios))]
    features += [("diffRatioMin2_" + str(k), "{0,1}", diffRatioMin2(ratios[k]) )
                 for k in range(0, len(ratios))]
    features += [("diffRatioMax2_" + str(k), "{0,1}", diffRatioMax2(ratios[k]) )
                 for k in range(0, len(ratios))]

    if len(filters) > 0:
        return [(name, values, feature) for name, values, feature in features 
                if name in filters]
    else:
        return features

### Features taking the focus measure value at three lens positions ###

def ratio3(k):
    # positive numbers for k only
    def r(**kwargs):
        fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
        return safeRatioLessThan(float(fst), trd, k)
    return r

# def ratio3Inverse(k):
#     # positive numbers for k only
#     def r(**kwargs):
#         fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
#         return safeRatioLessThan(float(trd), fst, k)
#     return r

def logRatio3(k):
    # positive numbers for k only
    def r(**kwargs):
        fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
        return safeRatioLessThan(log(fst + 1.0), log(trd + 1.0), k)
    return r

# def logRatio3Inverse(k):
#     # positive numbers for k only
#     def r(**kwargs):
#         fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
#         return safeRatioLessThan(log(trd + 1.0), log(fst + 1.0), k)
#     return r

def diffRatioAvg3(k):
    # numbers from -1 to 1 for k (in practice, -0.5 to 0.5)
    def r(**kwargs):
        fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
        return safeRatioLessThan(2.0 * (trd - fst), (trd + fst), k)
    return r

def diffRatioMin3(k):
    # numbers from -1 to 1 for k (in practice, -0.5 to 0.5)
    def r(**kwargs):
        fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
        return safeRatioLessThan(trd - fst, min(trd, fst), k)
    return r

def diffRatioMax3(k):
    # numbers from -1 to 1 for k (in practice, -0.5 to 0.5)
    def r(**kwargs):
        fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
        return safeRatioLessThan(trd - fst, max(trd, fst), k)
    return r

def curving(k):
    # positive and negative numbers for k alike
    def r(**kwargs):
        fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
        return safeRatioLessThan(fst + trd - 2 * snd, snd, k)
    return r

def curvingRatio(k):
    # positive and negative numbers for k alike
    def r(**kwargs):
        fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
        return safeRatioLessThan(fst - snd, snd - trd, k)
    return r

def downTrend(**kwargs):
    fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
    return fst >= snd and snd >= trd

def upTrend(**kwargs):
    fst, snd, trd = kwargs["first"], kwargs["second"], kwargs["third"]
    return fst <= snd and snd <= trd

def three_measure_features(filters=[]):
    """ Returns an array of (attribute name, attribute range, function) 
        where the functions take three arguments."""

    # We're using arrays instead of dicts to maintain a constant order.
    features = [ ("downTrend", "{0,1}", downTrend), 
                 ("upTrend"  , "{0,1}", upTrend) ]

    # Ratio are chosen to be symmetrical.
    # e.g. 0.75 and 1.25 are not symmetrical ratios, but 0.75 and 1.333 are
    ratios = [ 1.64, 1.32, 1.16, 1.08, 1.04, 1.02, 1.01, 1.00,
               1/1.01, 1/1.02, 1/1.04, 1/1.08, 1/1.16, 1/1.32, 1/1.64 ]
    features += [("ratio3_" + str(k), "{0,1}", ratio3(ratios[k]) )
                 for k in range(0, len(ratios))]
    features += [("logRatio3_" + str(k), "{0,1}", logRatio3(ratios[k]) )
                 for k in range(0, len(ratios))]

    ratios = [ -0.64, -0.32, -0.16, -0.08, -0.04, -0.02, -0.01, 0.0,
                0.01,  0.02,  0.04,  0.08,  0.16,  0.32,  0.64 ]
    features += [("diffRatioAvg3_" + str(k), "{0,1}", diffRatioAvg3(ratios[k]) )
                 for k in range(0, len(ratios))]
    features += [("diffRatioMin3_" + str(k), "{0,1}", diffRatioMin3(ratios[k]) )
                 for k in range(0, len(ratios))]
    features += [("diffRatioMax3_" + str(k), "{0,1}", diffRatioMax3(ratios[k]) )
                 for k in range(0, len(ratios))]

    features += [("curving_" + str(k), "{0,1}", curving(ratios[k]) )
                 for k in range(0, len(ratios))]
    features += [("curvingRatio_" + str(k), "{0,1}", curvingRatio(ratios[k]) )
                 for k in range(0, len(ratios))]

    if len(filters) > 0:
        return [(name, values, feature) for name, values, feature in features 
                if name in filters]
    else:
        return features

### Other features ###

def bracket(brackets):
    # start and end to be numbers from 0 to 1
    def f(**kwargs):
        lens_pos = kwargs["lens_pos"]
        assert lens_pos >= brackets[0]

        # Return the index corresponding to the correct bracket
        for i in range(0, len(brackets) - 1):
            if lens_pos >= brackets[i] and lens_pos < brackets[i + 1]:
                return i
        return len(brackets) - 1

    return f

def other_features(filters=[]):
    """ Returns an array of (attribute name, attribute range, function)"""

    # I've decided not to distribute the brackets evenly because it matters
    # less if the lens is near the center than if it is near the end

    # Twelve brackets
    # brackets = [ 0.0, 0.05, 0.10, 0.15, 0.25, 0.35, 0.50, 0.65, 0.75, 0.85, 0.90, 0.95 ]
    
    # Five brackets. Experiments seem to suggest this is sufficient. Twelve
    # may lead to overfitting. Three might be to restrictive for the leftmost
    # and rightmost peaks.
    brackets = [ 0.0, 0.08, 0.20, 0.80, 0.92 ]

    bracket_range = "{" + \
        ",".join([str(i) for i in range(0, len(brackets))]) + "}"

    features = [ ( "bracket", bracket_range, bracket(brackets) )]

    if len(filters) > 0:
        return [(name, values, feature) for name, values, feature in features 
                if name in filters]
    else:
        return features

###

def measure_features(filters=[]):
    """ Returns an array of (attribute name, attribute range, function)"""
    return two_measure_features(filters) + three_measure_features(filters)

def all_features(filters=[]):
    """ Returns an array of (attribute name, attribute range, function)"""

    return two_measure_features(filters) + three_measure_features(filters) + \
           other_features(filters)


### Classifiers ###

def highest_on_left(scene, lens_pos):
    # Find the location of the highest peak
    highest = scene.maxima[0]
    for maxima in scene.maxima:
        if scene.measuresValues[maxima] > scene.measuresValues[highest]:
            highest = maxima
    return highest < lens_pos

def nearest_on_left(scene, lens_pos):
    # Find the location of the nearest peak
    nearest = scene.maxima[0]
    for maxima in scene.maxima:
        if abs(lens_pos - maxima) < abs(lens_pos - nearest):
            nearest = maxima
    return nearest < lens_pos

def highest_and_near_on_left(scene, lens_pos):
    # Find the location of the peak that maximizes height
    # and distance, equally weighted in a product
    best = scene.maxima[0]
    for maxima in scene.maxima:
        # Need to add a + 1 to the distance to avoid division by zero.
        if scene.measuresValues[maxima] / (abs(lens_pos - maxima) + 1) > \
           scene.measuresValues[best] / (abs(lens_pos - best) + 1):
            best = maxima
    return best < lens_pos
