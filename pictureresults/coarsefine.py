"""
Functions that evaluate whether to take a fine step or a coarse step
depending on previous focus values, as described in
   'An Autofocus Heuristic for Digital Cameras Based on 
    Supervised Machine Learning'
"""

from math import log
from featuresfirststep import safe_ratio_gt

def coarse_if_previously_fine(f_prev2, f_prev, f_cur):
    """Assumes the previous step taken was a fine step. Return true
    if the next step should be a coarse step."""
    if not safe_ratio_gt(f_cur, f_prev, 8.0 / 8.0):
        # Coarse
        return True
    else:
        # Fine
        return False

def coarse_if_previously_coarse(f_prev2, f_prev, f_cur):
    """Assumes the previous step taken was a coarse step. Return true
    if the next step should be a coarse step."""

    def log_feature():
        """Handles the edge cases of logDiff described in the paper."""
        if f_cur <= f_prev:
            return False
        elif f_prev == 0:
            return f_cur - f_prev > 1
        else:
            return safe_ratio_gt(log(f_cur - f_prev), 
                log(f_prev), 6.0 / 8.0)

    # Using 'if not' instead of 'if' to follow the layout of the algorithm
    # as written in the paper.

    # ratio(10, 8)
    if not safe_ratio_gt(f_cur, f_prev, 10.0 / 8.0):
        # downSlope(9, 8)
        if not safe_ratio_gt(f_prev2 * f_cur, f_prev * f_prev, 9.0 / 8.0):
            # Coarse
            return True
        else:
            # ratioI(9, 8)
            if not safe_ratio_gt(f_prev, f_cur, 9.0 / 8.0):
                # logDiff(6, 8)
                if not log_feature():
                    # upSlope(8, 4)
                    if not safe_ratio_gt(f_cur * f_prev2, 
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
        if not safe_ratio_gt(f_prev2 * f_cur, f_prev * f_prev, 9.0 / 8.0):
            # ratio(11, 8)
            if not safe_ratio_gt(f_cur, f_prev, 11.0 / 8.0):
                # Coarse
                return True
            else:
                # Fine
                return False
        else:
            # Fine
            return False