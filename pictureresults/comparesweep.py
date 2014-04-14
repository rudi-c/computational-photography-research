#!/usr/bin/python

import getopt
import sys

import coarsefine
from cameramodel import CameraModel
from direction   import Direction
from scene       import load_scenes

simulate_backlash = True
simulate_noise = True

def print_aligned_data_rows(rows):
    """Print rows of data such that each column is aligned."""
    column_lengths = [ len(max(cols, key=len)) for cols in zip(*rows) ]
    for row in rows:
        print "|".join(" " * (length - len(col)) + col
                       for length, col in zip(column_lengths, row))


def search_perfect(scenes):
    print "# Perfect search"

    for scene in scenes:
        total_count = 0
        initial_positions = range(2, scene.step_count)
        for ini_pos in initial_positions:
            step_count = 2
            lens_pos = ini_pos
            passed_peak = False
            if (scene.distance_to_closest_left_peak(ini_pos) <
                scene.distance_to_closest_right_peak(ini_pos)):
                # Sweep left
                lens_pos -= 2
                while lens_pos > 0:
                    diff = scene.fvalues[lens_pos] - scene.fvalues[lens_pos+1]
                    if scene.distance_to_closest_right_peak(lens_pos) < 4:
                        lens_pos -= 1 #fine
                    elif passed_peak:
                        lens_pos += 4
                        break
                    elif (scene.distance_to_closest_left_peak(lens_pos) <= 10
                          or diff > 0.01):
                        lens_pos -= 1 #fine
                    else:
                        lens_pos -= 8 #coarse

                    if lens_pos in scene.maxima:
                        passed_peak = True
                    step_count += 1
            else:
                # Sweep right
                while lens_pos < scene.step_count - 1:
                    diff = scene.fvalues[lens_pos] - scene.fvalues[lens_pos-1]
                    if scene.distance_to_closest_left_peak(lens_pos) < 4:
                        lens_pos += 1 #fine
                    elif passed_peak:
                        lens_pos += 4
                        break
                    elif (scene.distance_to_closest_right_peak(lens_pos) <= 10
                          or diff > 0.01):
                        lens_pos += 1 #fine
                    else:
                        lens_pos += 8 #coarse

                    if lens_pos in scene.maxima:
                        passed_peak = True
                    step_count += 1

            total_count += step_count

        print "%s | %.1f" % (scene.filename, 
                             float(total_count) / len(initial_positions))


def search_simple(scenes):
    print "# Simple search"
    step_size = 8

    data_rows = []

    for scene in scenes:
        success_count = 0
        total_step_count = 0

        initial_positions = range(0, scene.step_count - step_size)
        for initial_position in initial_positions:
            camera = CameraModel(scene, initial_position,
                simulate_backlash=simulate_backlash, 
                simulate_noise=simulate_noise)

            first_measure = camera.last_fmeasure()
            camera.move_coarse(Direction("right"))

            # Determine whether to start moving left or right.
            if camera.last_fmeasure() < first_measure:
                direction = Direction("left")
            else:
                direction = Direction("right")

            # If the first step decreases focus value, switch direction.
            # This is a simple backtracking, basically.
            first_measure = camera.last_fmeasure()
            camera.move_coarse(direction)
            if camera.last_fmeasure() < first_measure:
                direction = direction.reverse()

            # Sweep
            max_value = camera.last_fmeasure()
            while not camera.will_hit_edge(direction):
                camera.move_coarse(direction)
                max_value = max(max_value, camera.last_fmeasure())

                # Have we found a peak?
                if camera.last_fmeasure() < max_value * 0.9:
                    # Hillclimb until we're back at the peak.
                    while not camera.will_hit_edge(direction.reverse()):
                        prev_measure = camera.last_fmeasure()
                        camera.move_fine(direction.reverse())
                        if prev_measure > camera.last_fmeasure():
                            camera.move_fine(direction)
                            break
                    # Stop searching
                    break

            # Record if we succeeded.
            if scene.distance_to_closest_peak(camera.last_position()) <= 1:
                success_count += 1
            total_step_count += camera.steps_taken

        line = (scene.name, 
                "%.1f" % (float(success_count) / len(initial_positions) * 100), 
                "%.1f" % (float(total_step_count) / len(initial_positions)))
        data_rows.append(line)

    print_aligned_data_rows(data_rows)


def search_sweep(scenes, always_coarse):
    print "# Sweep and stop search"

    if always_coarse:
        print "# Always coarse"
    else:
        print "# Use heuristics"

    data_rows = []

    for scene in scenes:
        last_step_coarse = True
        max_val = scene.fvalues[0]
        f_cur, f_prev, f_prev2 = scene.get_focus_values([0, 0, 0])
        current_pos = 1

        step_count = 1

        # Sweep in search of a maxima.
        while current_pos < scene.step_count - 1:
            # Size of the next step.
            if always_coarse:
                step_coarse = True
            else:
                f_prev2, f_prev, f_cur = \
                    f_prev, f_cur, scene.fvalues[current_pos]

                # Decide on size of the next step using the right decision tree
                if last_step_coarse:
                    step_coarse = coarsefine.coarse_if_previously_coarse(
                        f_prev2, f_prev, f_cur)
                else:
                    step_coarse = coarsefine.coarse_if_previously_fine(
                        f_prev2, f_prev, f_cur)

            if step_coarse:
                current_pos = min(scene.step_count - 1, current_pos + 8)
            else:
                current_pos = min(scene.step_count - 1, current_pos + 1)
            step_count += 1

            max_val = max(max_val, scene.fvalues[current_pos])
            if scene.fvalues[current_pos] < 0.7 * max_val:
                break

            last_step_coarse = step_coarse

        # Go back to peak using local search hillclimbing.
        while current_pos > 0:
            if scene.fvalues[current_pos] < scene.fvalues[current_pos - 1]:
                current_pos -= 1
                step_count += 1
            elif (current_pos > 1 and 
                  scene.fvalues[current_pos] < scene.fvalues[current_pos - 2]):
                # Tolerance of two fine steps.
                current_pos -= 2
                step_count += 2
            else:
                # Number of steps to move forward and back, 
                # due to two step tolerance
                step_count += 4
                break

        first_column = "%s (%d)" % (scene.filename, len(scene.maxima))
        if scene.distance_to_closest_peak(current_pos) < 1:
            if scene.distance_to_highest_peak(current_pos) <= 1:
                line = (first_column, "found highest", str(step_count))
            else:
                line = (first_column, "found a peak", str(step_count))
        else:
            line = (first_column, "failed", str(step_count))
        data_rows.append(line)

    print_aligned_data_rows(data_rows)


def search_camera(scenes):
    print "# Camera search"
    sweep_steps = 19

    data_rows = []

    for scene in scenes:
        # The camera does a full sweep.
        highest_pos = 0
        for pos in range(0, scene.step_count, 8):
            if scene.fvalues[pos] > scene.fvalues[highest_pos]:
                highest_pos = pos

        # Number of large steps needed to go back to the highest position.
        large_steps = (scene.step_count - 1 - highest_pos) / 8
        current_pos = (scene.step_count - 1) - (large_steps * 8)
        fine_steps = 0

        # Local search.
        while current_pos > 0:
            if scene.fvalues[current_pos] < scene.fvalues[current_pos - 1]:
                current_pos -= 1
                fine_steps += 1
            elif (current_pos > 1 and 
                  scene.fvalues[current_pos] < scene.fvalues[current_pos - 2]):
                # Tolerance of two fine steps.
                current_pos -= 2
                fine_steps += 2
            else:
                # Number of steps to move forward and back, 
                # due to two step tolerance
                fine_steps += 4
                break

        step_count = sweep_steps + large_steps + fine_steps

        first_column = "%s (%d)" % (scene.filename, len(scene.maxima))
        if scene.distance_to_closest_peak(current_pos) <= 1:
            if scene.distance_to_highest_peak(current_pos) <= 1:
                line = (first_column, "found highest", str(step_count))
            else:
                line = (first_column, "found a peak", str(step_count))
        else:
            line = (first_column, "failed", str(step_count))
        data_rows.append(line)

    print_aligned_data_rows(data_rows)


def print_script_usage():
    print >> sys.stderr, \
        """Script usage : ./benchmark.py 
           [-ll, --low-light <evaluate low light benchmarks>]"""


def main(argv):
    # Parse script arguments
    try:
        opts, _ = getopt.getopt(argv, "", [ "lowlight" ])
    except getopt.GetoptError:
        print_script_usage()
        sys.exit(2)

    scenes_folder = "focusraw/"

    for opt, arg in opts:
        if opt == "--lowlight":
            scenes_folder = "lowlightraw/"
        else:
            print_script_usage()
            sys.exit(2)

    scenes = load_scenes(folder=scenes_folder,
        excluded_scenes=["cat.txt", "moon.txt", 
                         "projector2.txt", "projector3.txt"])
    search_perfect(scenes)
    print "\n"
    search_simple(scenes)
    print "\n"
    search_sweep(scenes, False)
    print "\n"
    search_sweep(scenes, True)
    print "\n"
    search_camera(scenes)


main(sys.argv[1:])