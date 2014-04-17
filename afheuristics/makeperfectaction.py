#!/usr/bin/python
"""
Creates a JSON of the correct classification for every scene, every
starting lens position, every direction and every lens position.

The format is
{ "scene_name1" : { "dir-start-current" : "class" },
  "scene_name2" : ... }
Where dir is "left" or "right"
and class is "continue", "turn_peak" or "backtrack"
Example :
{ "cup.txt" : { "left-40-10" : "backtrack" } }
"""

import json
import sys

import featuresturn
from scene import load_scenes

def classify_for_scene(scene, params):
    """Get correct classification for a given scene."""
    left_moves = \
        { featuresturn.make_key("left", initial_pos, current_pos):
            featuresturn.class_names[featuresturn.get_move_left_classification(
                initial_pos, current_pos, scene.fvalues,
                scene.maxima, params)]
            for initial_pos in range(0, scene.step_count)
            for current_pos in range(0, initial_pos + 1) }
    right_moves = \
        { featuresturn.make_key("right", initial_pos, current_pos):
            featuresturn.class_names[featuresturn.get_move_right_classification(
                initial_pos, current_pos, scene.fvalues,
                scene.maxima, params)]
            for initial_pos in range(0, scene.step_count)
            for current_pos in range(initial_pos, scene.step_count) }

    return dict(left_moves.items() + right_moves.items())


def create_json(scenes, params):
    """Get correct classifications for every scene."""
    scene_classifications = \
        { scene.filename: classify_for_scene(scene, params)
          for scene in scenes}
    return json.dumps(scene_classifications)


def main(argv):
    params = featuresturn.ParameterSet()

    # Process command line options.
    for arg in argv:
        if arg == "--closest-peak":
            params.peakHandling = featuresturn.PeakHandling.CLOSEST
        elif arg == "--backtrack-faster":
            params.backtrackHandling = featuresturn.BacktrackHandling.FASTER

    scenes = load_scenes()

    json_file = create_json(scenes, params)
    print json_file


main(sys.argv[1:])