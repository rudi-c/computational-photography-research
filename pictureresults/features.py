# Functions that evaluate features.

from math import log

from scene import *

def safeRatioLessThan(a, b, k):
    """Compare the ratio of a, b with k and handle division by zero."""
    return b != 0 and float(a) / b < k

### Features taking the focus measure value at two lens positions ###

def ratio2(k):
    # positive numbers for k only
    def r(first, second):
        return safeRatioLessThan(first, second, k)
    return r

def ratio2Inverse(k):
    # positive numbers for k only
    def r(first, second):
        return safeRatioLessThan(second, first, k)
    return r

def logRatio2(k):
    # positive numbers for k only
    def r(first, second):
        return safeRatioLessThan(log(first + 1.0), log(second + 1.0), k)
    return r

def logRatio2Inverse(k):
    # positive numbers for k only
    def r(first, second):
        return safeRatioLessThan(log(second + 1.0), log(first + 1.0), k)
    return r

def two_measure_features(filters=[]):
    """ Returns an array of (attribute name, function) where the functions
        take two arguments."""

    # We're using arrays instead of dicts to maintain a constant order.
    features = []
    features += [("ratio2_" + str(k), ratio2(k / 8.0) )
                 for k in range(1, 16)]
    features += [("ratio2Inverse_" + str(k), ratio2Inverse(k / 8.0) )
                 for k in range(1, 16)]
    features += [("logRatio2_" + str(k), logRatio2(k / 8.0) )
                 for k in range(1, 16)]
    features += [("logRatio2Inverse_" + str(k), logRatio2Inverse(k / 8.0) )
                 for k in range(1, 16)]

    if len(filters) > 0:
        return [(name, feature) for name, feature in features 
                if name in filters]
    else:
        return features

### Features taking the focus measure value at three lens positions ###

def ratio3(k):
    # positive numbers for k only
    def r(first, second, third):
        return safeRatioLessThan(float(first), third, k)
    return r

def ratio3Inverse(k):
    # positive numbers for k only
    def r(first, second, third):
        return safeRatioLessThan(float(third), first, k)
    return r

def logRatio3(k):
    # positive numbers for k only
    def r(first, second, third):
        return safeRatioLessThan(log(first + 1.0), log(third + 1.0), k)
    return r

def logRatio3Inverse(k):
    # positive numbers for k only
    def r(first, second, third):
        return safeRatioLessThan(log(third + 1.0), log(first + 1.0), k)
    return r

def curving(k):
    # positive and negative numbers for k alike
    def r(first, second, third):
        return safeRatioLessThan(first + third - 2 * second, second, k)
    return r

def curvingRatio(k):
    # positive and negative numbers for k alike
    def r(first, second, third):
        return safeRatioLessThan(first - second, second - third, k)
    return r

def downTrend(first, second, third):
    return first <= second and second <= third

def upTrend(first, second, third):
    return first >= second and second >= third

def three_measure_features(filters=[]):
    """ Returns an array of (attribute name, function) where the functions
        take three arguments."""

    # We're using arrays instead of dicts to maintain a constant order.
    features = [ ("downTrend", downTrend), ("upTrend", upTrend) ]
    features += [("ratio3_" + str(k), ratio3(k / 8.0) )
                 for k in range(1, 16)]
    features += [("ratio3Inverse_" + str(k), ratio3Inverse(k / 8.0) )
                 for k in range(1, 16)]
    features += [("logRatio3_" + str(k), logRatio3(k / 8.0) )
                 for k in range(1, 16)]
    features += [("logRatio3Inverse_" + str(k), logRatio3Inverse(k / 8.0) )
                 for k in range(1, 16)]
    features += [("curving_" + str(k), curving((k - 8.0) / 4.0) )
                 for k in range(1, 16)]
    features += [("curvingRatio_" + str(k), curving((k - 8.0) / 4.0) )
                 for k in range(1, 16)]

    if len(filters) > 0:
        return [(name, feature) for name, feature in features 
                if name in filters]
    else:
        return features

### Classifiers ###

def highest_on_left(scene, lens_pos):
    # Find the location of the highest peak
    highest = 0
    for maxima in scene.maxima:
        if scene.measuresValues[maxima] > scene.measuresValues[highest]:
            highest = maxima
    return highest < lens_pos

def nearest_on_left(scene, lens_pos):
    # Find the location of the nearest peak
    nearest = 0
    for maxima in scene.maxima:
        if abs(lens_pos - maxima) < abs(lens_pos - nearest):
            nearest = maxima
    return nearest < lens_pos

def highest_and_near_on_left(scene, lens_pos):
    # Find the location of the peak that maximizes height
    # and distance, equally weighted in a product
    best = 0
    for maxima in scene.maxima:
        # Need to add a + 1 to the distance to avoid division by zero.
        if scene.measuresValues[maxima] / (abs(lens_pos - maxima) + 1) > \
           scene.measuresValues[best] / (abs(lens_pos - best) + 1):
            best = maxima
    return best < lens_pos