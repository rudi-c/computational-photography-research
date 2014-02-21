#!/usr/bin/python

# Creates a JSON of the correct classification for every scene, every
# starting lens position, every direction and every lens position.

# The format is
# { "scene_name1" : { "dir-start-current" : "class" },
#   "scene_name2" : ... }
# Where dir is "left" or "right"
# and class is "continue", "turn_peak" or "backtrack"
#
# Example :
# { "cup.txt" : { "left-40-10" : "backtrack" } }

import json
import os
import sys

from scene import *
from featuresturn import *

def make_key(direction, initial_pos, current_pos):
    return direction + "-" + str(initial_pos) + "-" + str(current_pos)


def classify_for_scene(scene, params):
    left_moves = \
        { make_key("left", initial_pos, current_pos):
            class_names[get_move_left_classification(
                initial_pos, current_pos, scene.measuresValues,
                scene.maxima, params)]
            for initial_pos in range(0, scene.measuresCount)
            for current_pos in range(0, initial_pos + 1) }
    right_moves = \
        { make_key("right", initial_pos, current_pos):
            class_names[get_move_right_classification(
                initial_pos, current_pos, scene.measuresValues,
                scene.maxima, params)]
            for initial_pos in range(0, scene.measuresCount)
            for current_pos in range(initial_pos, scene.measuresCount) }

    return dict(left_moves.items() + right_moves.items())


def create_json(scenes, params):
    scene_classifications = \
        {scene.fileName: classify_for_scene(scene, params)
         for scene in scenes}
    return json.dumps(scene_classifications)


def main(argv):

    if not os.path.isdir(scenes_folder):
        print scenes_folder + " folder not found."
        return

    params = ParameterSet()

    # Process command line options.
    for arg in argv:
        if arg == "--closest-peak":
            params.peakHandling = PeakHandling.CLOSEST
        elif arg == "--backtrack-faster":
            params.backtrackHandling = BacktrackHandling.FASTER

    scenes = load_scenes()
    load_maxima_into_measures(scenes)

    json_file = create_json(scenes, params)
    print json_file


main(sys.argv[1:])