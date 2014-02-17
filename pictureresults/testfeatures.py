#!/usr/bin/python

# Tests for featuresturn.py

from featuresturn import *

import unittest

def make_feature_argument(position_value_pairs, total_pos):
    lens_positions = [ pos for pos, val in position_value_pairs ]
    focus_values = { pos : val for pos, val in position_value_pairs }
    return { "lens_positions" : lens_positions, "focus_values" : focus_values,
             "total_positions" : total_pos }


class StepFeaturesTests(unittest.TestCase):

    def testFineOnly(self):
        simulation = [ (4, 1.1), (5, 1.2), (6, 1.3), (7, 1.4), (8, 1.5) ]
        kwargs = make_feature_argument(simulation, 12)
        self.assertAlmostEqual(steps_taken(**kwargs), 4.0 / 12)
        self.assertAlmostEqual(large_steps_taken(**kwargs), 0.0)
        self.assertAlmostEqual(small_steps_taken(**kwargs), 4.0 / 12)
        self.assertAlmostEqual(ratio_small_steps(**kwargs), 1.0)
        self.assertAlmostEqual(ratio_large_steps(**kwargs), 0.0)
        self.assertAlmostEqual(distance_taken(**kwargs), 4.0 / 12)
        self.assertEqual( absolute_steps_taken(**kwargs), 4 )
        self.assertEqual( absolute_distance_taken(**kwargs), 4 )

    def testCoarseOnly(self):
        simulation = [ (4, 1.1), (12, 1.2), (20, 1.3), (28, 1.4), (36, 1.5) ]
        kwargs = make_feature_argument(simulation, 40)
        self.assertAlmostEqual(steps_taken(**kwargs), 4.0 / 40)
        self.assertAlmostEqual(large_steps_taken(**kwargs), 4.0 / 40)
        self.assertAlmostEqual(small_steps_taken(**kwargs), 0.0)
        self.assertAlmostEqual(ratio_small_steps(**kwargs), 0.0)
        self.assertAlmostEqual(ratio_large_steps(**kwargs), 1.0)
        self.assertAlmostEqual(distance_taken(**kwargs), 32.0 / 40)
        self.assertEqual( absolute_steps_taken(**kwargs), 4 )
        self.assertEqual( absolute_distance_taken(**kwargs), 32 )

    def testFineCoarse(self):
        simulation = [ (4, 1.1), (12, 1.2), (13, 1.3), (21, 1.4), (22, 1.5) ]
        kwargs = make_feature_argument(simulation, 40)
        self.assertAlmostEqual(steps_taken(**kwargs), 4.0 / 40)
        self.assertAlmostEqual(large_steps_taken(**kwargs), 2.0 / 40)
        self.assertAlmostEqual(small_steps_taken(**kwargs), 2.0 / 40)
        self.assertAlmostEqual(ratio_small_steps(**kwargs), 0.5)
        self.assertAlmostEqual(ratio_large_steps(**kwargs), 0.5)
        self.assertAlmostEqual(distance_taken(**kwargs), 18.0 / 40)
        self.assertEqual( absolute_steps_taken(**kwargs), 4 )
        self.assertEqual( absolute_distance_taken(**kwargs), 18 )


class EdgeDistanceTests(unittest.TestCase):

    def testAtLeftEdge(self):
        simulation = [ (4, 1.1), (3, 1.2), (2, 1.3), (1, 1.4), (0, 1.5) ]
        kwargs = make_feature_argument(simulation, 12)
        self.assertAlmostEqual(distance_to_left(**kwargs), 0.0)
        self.assertAlmostEqual(distance_to_right(**kwargs), 1.0)
        self.assertAlmostEqual(ratio_left_to_right(**kwargs), 0.0)
        self.assertAlmostEqual(ratio_right_to_left(**kwargs), 10.0)

    def testAtRightEdge(self):
        simulation = [ (7, 1.1), (8, 1.2), (9, 1.3), (10, 1.4), (11, 1.5) ]
        kwargs = make_feature_argument(simulation, 12)
        self.assertAlmostEqual(distance_to_left(**kwargs), 1.0)
        self.assertAlmostEqual(distance_to_right(**kwargs), 0.0)
        self.assertAlmostEqual(ratio_left_to_right(**kwargs), 10.0)
        self.assertAlmostEqual(ratio_right_to_left(**kwargs), 0.0)

    def testAtCenter(self):
        simulation = [ (0, 1.1), (8, 1.2), (9, 1.3), (10, 1.4), (18, 1.5) ]
        kwargs = make_feature_argument(simulation, 37)
        self.assertAlmostEqual(distance_to_left(**kwargs), 0.5)
        self.assertAlmostEqual(distance_to_right(**kwargs), 0.5)
        self.assertAlmostEqual(ratio_left_to_right(**kwargs), 1.0)
        self.assertAlmostEqual(ratio_right_to_left(**kwargs), 1.0)


class ScagnosticsTests(unittest.TestCase):

    def testRank(self):
        self.assertEqual(rank(range(0, 10)), range(0, 10))
        self.assertEqual(rank(range(10, 0, -1)), range(9, -1, -1))
        self.assertEqual(rank([1, 5, -4, 3, 4, -6]), [2, 5, 1, 3, 4, 0] )

    def testMonotoniticity(self):
        self.assertAlmostEqual(monotoniticy(**make_feature_argument(
            zip(range(0, 10), range(0, 10)), 20)), 1.0)
        self.assertAlmostEqual(monotoniticy(**make_feature_argument(
            zip(range(0, 10), range(10, 0, -1)), 20)), -1.0)
        self.assertLess(abs(monotoniticy(**make_feature_argument(
            zip(range(0, 7), [4, 1, 5, 2, 5, 3, 4]), 20))), 0.8)

    def testAllEqual(self):
        simulation = [ (1, 1.1), (9, 1.1), (10, 1.1), (18, 1.1) ]
        kwargs = make_feature_argument(simulation, 20)
        self.assertEqual(monotoniticy(**kwargs), 0.0)

    def testAlternation(self):
        self.assertAlmostEqual(alternation_ratio(**make_feature_argument(
            zip(range(0, 10), range(0, 10)), 20)), 0.0)
        self.assertAlmostEqual(alternation_ratio(**make_feature_argument(
            zip(range(0, 10), range(10, 0, -1)), 20)), 0.0)
        self.assertAlmostEqual(alternation_ratio(**make_feature_argument(
            zip(range(0, 7), [4, 1, 5, 2, 5, 3, 4]), 20)), 1.0)


class MaximumTests(unittest.TestCase):

    def testAllEqual(self):
        simulation = [ (1, 1.1), (9, 1.1), (10, 1.1), (18, 1.1) ]
        kwargs = make_feature_argument(simulation, 20)
        self.assertAlmostEqual(ratio_to_max(**kwargs), 1.0)
        self.assertAlmostEqual(ratio_to_range(**kwargs), 0.5)
        self.assertAlmostEqual(distance_to_max(**kwargs), 0.0)
        self.assertAlmostEqual(ratio_min_to_max(**kwargs), 1.0)


    def testNormal(self):
        simulation = [ (1, 1.1), (9, 1.5), (10, 1.6), (18, 1.4) ]
        kwargs = make_feature_argument(simulation, 20)
        self.assertAlmostEqual(ratio_to_max(**kwargs), 1.4 / 1.6)
        self.assertAlmostEqual(ratio_to_range(**kwargs), 0.3 / 0.5)
        self.assertAlmostEqual(distance_to_max(**kwargs), 8.0 / 20)
        self.assertAlmostEqual(ratio_min_to_max(**kwargs), 1.1 / 1.6)


class TrendTests(unittest.TestCase):

    def testAllEqual(self):
        simulation = [ (1, 1.1), (9, 1.1), (10, 1.1), (18, 1.1) ]
        kwargs = make_feature_argument(simulation, 20)
        self.assertAlmostEqual(simple_slope(**kwargs), 0.0)
        self.assertAlmostEqual(regression_slope(**kwargs), 0.0)

    def testSlopeOne(self):
        simulation = [ (1, 0.1), (9, 0.9), (10, 1.0), (18, 1.8) ]
        kwargs = make_feature_argument(simulation, 20)
        self.assertAlmostEqual(simple_slope(**kwargs), 0.1 * 20 / (1.9 / 2))
        self.assertAlmostEqual(regression_slope(**kwargs), 0.1 * 20 / (1.9 / 2))

    def testSlopeMinusOne(self):
        simulation = [ (1, 1.8), (9, 1.0), (10, 0.9), (18, 0.1) ]
        kwargs = make_feature_argument(simulation, 20)
        self.assertAlmostEqual(simple_slope(**kwargs), -0.1 * 20 / (1.9 / 2))
        self.assertAlmostEqual(regression_slope(**kwargs), 
            -0.1 * 20 / (1.9 / 2))


class LocalSlopeTests(unittest.TestCase):

    def testAllEqual(self):
        simulation = [ (1, 1.1), (9, 1.1), (10, 1.1), (18, 1.1), (19, 1.1) ]
        kwargs = make_feature_argument(simulation, 20)
        self.assertAlmostEqual(current_slope(**kwargs), 0.0)
        self.assertAlmostEqual(current_slope_large(**kwargs), 0.0)
        self.assertEqual(downslope_1st_half(**kwargs), 0.0)
        self.assertEqual(downslope_2nd_half(**kwargs), 0.0)

    def testPeak(self):
        simulation = [ (1, 1.1), (9, 1.4), (10, 1.5), (18, 1.4), (19, 1.0) ]
        kwargs = make_feature_argument(simulation, 20)
        self.assertAlmostEqual(current_slope(**kwargs), -0.4 * 20 / 1.2)
        self.assertAlmostEqual(current_slope_large(**kwargs), 
            -0.5 / 9 * 20 / 1.25)
        self.assertEqual(downslope_1st_half(**kwargs), 0.0)
        self.assertEqual(downslope_2nd_half(**kwargs), 1.0)



def main():
    unittest.main()

main()