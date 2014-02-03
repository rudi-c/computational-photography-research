# Functions that evaluate features.

# Features are lambda expressions taking an arbitrary number of
# keyword arguments. Current arguments used include :
# first (first focus measurement, [0, 1])
# second (second focus measurement, [0, 1])
# third (third focus measurement, [0, 1])
# lens_pos (0 for the first lens position, 1 for the last lens position)

from math import log

from scene import *

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

    # Ratio calculations are split in two to maintain symmetry.
    # e.g. 0.75 and 1.25 are not symmetrical ratios, but 0.75 and 1.333 are
    features += [("ratio2_" + str(k), "{0,1}", ratio2((10 + k) / 20.0) )
                 for k in range(0, 11)] \
             +  [("ratio2_" + str(k + 10), "{0,1}", ratio2(20.0 / (10 + k)) )
                 for k in range(1, 11)]
    features += [("logRatio2_" + str(k), "{0,1}", logRatio2((10 + k) / 20.0) )
                 for k in range(0, 11)] \
             +  [("logRatio2_" + str(k + 10), "{0,1}", logRatio2(20.0 / (10 + k)) )
                 for k in range(1, 11)]

    features += [("diffRatioAvg2_" + str(k), "{0,1}", 
                   diffRatioAvg2((5 - k) / 10.0) )
                 for k in range(0, 11)]
    features += [("diffRatioMin2_" + str(k), "{0,1}", 
                   diffRatioMin2((5 - k) / 10.0) )
                 for k in range(0, 11)]
    features += [("diffRatioMax2_" + str(k), "{0,1}", 
                   diffRatioMax2((5 - k) / 10.0) )
                 for k in range(0, 11)]

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

    # Ratio calculations are split in two to maintain symmetry.
    # e.g. 0.75 and 1.25 are not symmetrical ratios, but 0.75 and 1.333 are
    features += [("ratio3_" + str(k), "{0,1}", ratio3((10 + k) / 20.0) )
                 for k in range(0, 11)] \
             +  [("ratio3_" + str(k + 10), "{0,1}", ratio3(20.0 / (10 + k)) )
                 for k in range(1, 11)]
    features += [("logRatio3_" + str(k), "{0,1}", logRatio3((10 + k) / 20.0) )
                 for k in range(0, 11)] \
             +  [("logRatio3_" + str(k + 10), "{0,1}", logRatio3(20.0 / (10 + k)) )
                 for k in range(1, 11)]

    features += [("diffRatioAvg3_" + str(k), "{0,1}", 
                   diffRatioAvg3((5 - k) / 10.0) )
                 for k in range(0, 11)]
    features += [("diffRatioMin3_" + str(k), "{0,1}", 
                   diffRatioMin3((5 - k) / 10.0) )
                 for k in range(0, 11)]
    features += [("diffRatioMax3_" + str(k), "{0,1}", 
                   diffRatioMax3((5 - k) / 10.0) )
                 for k in range(0, 11)]

    features += [("curving_" + str(k), "{0,1}", curving((k - 8.0) / 4.0) )
                 for k in range(1, 16)]
    features += [("curvingRatio_" + str(k), "{0,1}", curving((k - 8.0) / 4.0) )
                 for k in range(1, 16)]

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

### Coarse vs Fine ###

def coarse_if_previously_fine(f_cur, f_prev, f_prev2):
    """Assumes the previous step taken was a fine step. Return true
    if the next step should be a coarse step."""
    if not safeRatioMoreThan(f_cur, f_prev, 8.0 / 8.0):
        return True
    else:
        return False

def coarse_if_previously_coarse(f_cur, f_prev, f_prev2):
    """Assumes the previous step taken was a coarse step. Return true
    if the next step should be a coarse step."""

    def log_feature():
        if f_cur <= f_prev:
            return False
        elif f_prev == 0:
            return f_cur - f_prev > 1
        else:
            return safeRatioMoreThan(log(f_cur - f_prev), 
                log(f_prev), 6.0 / 8.0)

    # Using 'if not' instead of 'if' to follow the layout of the algorithm
    # as written in the paper.

    # ratio(10, 8)
    if not safeRatioMoreThan(f_cur, f_prev, 10.0 / 8.0):
        # downSlope(9, 8)
        if not safeRatioMoreThan(f_prev2 * f_cur, f_prev * f_prev, 9.0 / 8.0):
            # Coarse
            return True
        else:
            # ratioI(9, 8)
            if not safeRatioMoreThan(f_prev, f_cur, 9.0 / 8.0):
                # logDiff(6, 8)
                if not log_feature():
                    # upSlope(8, 4)
                    if not safeRatioMoreThan(f_cur * f_prev2, 
                                             f_prev * f_prev, 8.0 / 4.0):
                        # Coarse
                        return True
                    else:
                        # Fine
                        return False
                else:
                    # upTrend
                    if not (f_cur >= f_prev >= f_prev2):
                        # Fine
                        return False
                    else:
                        # Coarse
                        return True
            else:
                # Coarse
                return True
    else:
        # downSlope(9, 8)
        if not safeRatioMoreThan(f_prev2 * f_cur, f_prev * f_prev, 9.0 / 8.0):
            # ratio(11, 8)
            if not safeRatioMoreThan(f_cur, f_prev, 11.0 / 8.0):
                # Coarse
                return True
            else:
                # Fine
                return False
        else:
            # Fine
            return False


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