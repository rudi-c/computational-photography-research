"""Functions to evaluate statistics about scenes."""

import operator

smaller = left = operator.lt
greater = right = operator.gt


def valid_comparator(comparator):
    """Make sure that a lambda is valid for use in computing statistics."""
    return comparator == operator.lt or comparator == operator.gt


def probability_peak_exist(lens_pos, scenes, comparator):
    """Calculate probability that a peak exists in the direction given 
    by the comparator.
    """
    assert valid_comparator(comparator)
    count = 0
    for scene in scenes:
        for n_maximum in scene.norm_maxima:
            if comparator(n_maximum, lens_pos):
                count += 1
                break
    return float(count) / len(scenes)

def probability_left_peak(lens_pos, scenes):
    """P(peak exists to left | lens position)"""
    return probability_peak_exist(lens_pos, scenes, left)


def probability_not_left_peak(lens_pos, scenes):
    """P(peak does not exists to left | lens position)"""
    return 1 - probability_left_peak(lens_pos, scenes)


def probability_right_peak(lens_pos, scenes):
    """P(peak exists to right | lens position)"""
    return probability_peak_exist(lens_pos, scenes, right)


def probability_not_right_peak(lens_pos, scenes):
    """P(peak does not exists to right | lens position)"""
    return 1 - probability_right_peak(lens_pos, scenes)


def probability_highest_peak(lens_pos, scenes, comparator):
    """Calculate probability that the highest peak is in the direction given 
    by the comparator.
    """
    assert valid_comparator(comparator)
    assert lens_pos >= 0 and lens_pos <= 1
    
    count = 0
    for scene in scenes:
        highest = 0
        for i in range(1, scene.step_count):
            if scene.fvalues[i] > scene.fvalues[highest]:
                highest = i

        if comparator(float(highest), lens_pos * scene.step_count):
            count += 1
    return float(count) / len(scenes)


def probability_highest_left(lens_pos, scenes):
    """P(highest peak to left | lens position)"""
    return probability_highest_peak(lens_pos, scenes, left)


def probability_highest_right(lens_pos, scenes):
    """P(highest peak to right | lens position)"""
    return probability_highest_peak(lens_pos, scenes, right)


def probability_most_peaks(lens_pos, scenes, comparator):
    """Calculate probability that there are more peaks in the direction given 
    by the comparator.
    """
    assert valid_comparator(comparator)

    count = 0
    for scene in scenes:
        left_peaks = [ n_maximum for n_maximum in scene.norm_maxima
                                 if left(n_maximum, lens_pos) ]
        right_peaks = [ n_maximum for n_maximum in scene.norm_maxima
                                  if right(n_maximum, lens_pos) ]
        if comparator(len(left_peaks), len(right_peaks)):
            count += 1
    return float(count) / len(scenes)


def probability_most_left(lens_pos, scenes):
    """P( most peaks to the left | lens position )"""
    return probability_most_peaks(lens_pos, scenes, greater)


def probability_most_right(lens_pos, scenes):
    """P( most peaks to the left | lens position )"""
    return probability_most_peaks(lens_pos, scenes, smaller)


def probability_nearest(lens_pos, scenes, comparator):
    """Calculate probability that the nearest peak is in the direction given 
    by the comparator.
    """
    assert valid_comparator(comparator)

    count = 0
    for scene in scenes:
        nearest = scene.norm_maxima[0]
        for n_maximum in scene.norm_maxima:
            if abs(lens_pos - n_maximum) < abs(lens_pos - nearest):
                nearest = n_maximum
        if comparator(nearest, lens_pos):
            count += 1
    return float(count) / len(scenes)


def probability_nearest_left(lens_pos, scenes):
    """P( nearest peak to the left | lens position )"""
    return probability_nearest(lens_pos, scenes, left)


def probability_nearest_right(lens_pos, scenes):
    """P( nearest peak to the right | lens position )"""
    return probability_nearest(lens_pos, scenes, right)


def probability_nearhighest(lens_pos, scenes, comparator):
    """Calculate probability that the nearhighest peak is in the direction  
    given by the comparator.
    """
    assert valid_comparator(comparator)

    count = 0
    for scene in scenes:
        bestvalue = 0
        nearhighest = 0

        # Find the values that maximizes height / distance
        for maximum in scene.maxima:
            n_maximum = float(maximum) / scene.step_count

            # Ignore if the peak is exactly at this location, it will
            # give division by zero and is neither left or right.
            if not lens_pos == n_maximum:
                value = (scene.fvalues[maximum] /
                         abs(lens_pos - n_maximum))
            if value > bestvalue:
                nearhighest = n_maximum
                bestvalue = value
        if comparator(nearhighest, lens_pos):
            count += 1
    return float(count) / len(scenes)


def probability_nearhighest_left(lens_pos, scenes):
    """P( peak that maximizes height / distance is left | lens position )"""
    return probability_nearhighest(lens_pos, scenes, left)


def probability_nearhighest_right(lens_pos, scenes):
    """P( peak that maximizes height / distance is right | lens position )"""
    return probability_nearhighest(lens_pos, scenes, right)
