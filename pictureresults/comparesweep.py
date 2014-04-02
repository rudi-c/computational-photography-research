#!/usr/bin/python

import coarsefine
from scene import load_scenes

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

    for scene in scenes:
        success_count = 0
        total_count = 0
        initial_positions = range(step_size, scene.step_count)
        for ini_pos in initial_positions:
            step_count = 1
            if scene.fvalues[ini_pos] < scene.fvalues[ini_pos - step_size]:
                lens_pos = ini_pos - step_size
                max_pos = 0 # default maximum found is at the edge, if we
                            # didn't find anything during the sweep
                # Sweep left
                while lens_pos > 0:
                    step_count += 1
                    new_pos = max(0, lens_pos - step_size)
                    if scene.fvalues[new_pos] < scene.fvalues[lens_pos] * 0.9:
                        lens_pos = new_pos
                        # Found peak, go back the other way until we found a max
                        while lens_pos < scene.step_count - 1:
                            step_count += 1
                            lens_pos += 1
                            if (scene.fvalues[lens_pos] < 
                                scene.fvalues[lens_pos - 1]):
                                step_count += 1
                                max_pos = lens_pos - 1
                                break
                        # Stop searching
                        break
                    lens_pos = new_pos
            else:
                lens_pos = ini_pos
                # default maximum found is at the edge, if we
                # didn't find anything during the sweep
                max_pos = scene.step_count - 1 
                # Sweep right
                while lens_pos < scene.step_count - 1:
                    step_count += 1
                    new_pos = min(scene.step_count - 1, lens_pos + step_size)
                    if scene.fvalues[new_pos] < scene.fvalues[lens_pos] * 0.9:
                        lens_pos = new_pos
                        # Found peak, go back the other way until we found a max
                        while lens_pos > 0:
                            step_count += 1
                            lens_pos -= 1
                            if (scene.fvalues[lens_pos] < 
                                scene.fvalues[lens_pos + 1]):
                                step_count += 1
                                max_pos = lens_pos + 1
                                break
                        # Stop searching
                        break
                    lens_pos = new_pos

            if scene.distance_to_closest_peak(max_pos) <= 1:
                total_count += step_count
                success_count += 1

        print "%s | %.2f | %.1f" % (scene.filename, 
            float(success_count) / len(initial_positions),
            float(total_count) / success_count)


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



def main():
    scenes = load_scenes(folder="lowlightgaussraw/")
    search_perfect(scenes)
    print "\n"
    search_simple(scenes)
    print "\n"
    search_sweep(scenes, False)
    print "\n"
    search_sweep(scenes, True)
    print "\n"
    search_camera(scenes)


main()