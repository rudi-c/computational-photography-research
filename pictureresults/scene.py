# Data structure for storing the focus measures at each lens position
# for each scene.

import copy
import os

# Where to find the data.
scenes_folder = "focusraw/"
maxima_file = "maxima.txt"

class Scene:
    def __get_focus_value(self, string):
        parts = string.split()
        if len(parts) != 2:
            raise Exception("Lines in focus measures files " \
                            "should only have two columns.")
        return float(parts[1])

    def __load_file(self):
        try:
            f = open(scenes_folder + self.fileName)
        except IOError:
            print "File " + self.fileName + " not found."
            raise

        focus_values = [self.__get_focus_value(line) for
                        line in f.readlines()]
        self.measuresCount = len(focus_values)
        self.measuresValues = focus_values
        # Normalized version as tuples.
        # self.measuresValues = [(float(i) / self.measuresCount, focus_values[i]) 
        #                       for i in range(0, len(focus_values)) ]

        assert self.measuresCount > 0

    def inverse_copy(self):
        """Return an inverted copy of this scene."""
        i_scene = copy.copy(self)
        i_scene.measuresValues = list(self.measuresValues)
        i_scene.measuresValues.reverse()
        i_scene.maxima = [ self.measuresCount - m - 1 for m in self.maxima ]
        i_scene.norm_maxima = [ float(m) / self.measuresCount 
                                for m in i_scene.maxima ]
        return i_scene

    def distance_to_closest_peak(self, lens_pos):
        if len(self.maxima) == 0:
            raise Exception("Error : No maxima loaded for this scene")
        min_dist = self.measuresCount
        for maximum in self.maxima:
            min_dist = min(min_dist, abs(lens_pos - maximum))
        return min_dist

    def __init__(self, file_name = None):
        self.fileName = file_name
        self.name = file_name[:file_name.rfind(".")] # remove extension
        if not file_name is None:
            self.__load_file()
        self.maxima = []
        self.norm_maxima = []


def load_scenes(excluded_scenes):
    return [ Scene(f) 
             for f in os.listdir(scenes_folder) 
             if os.path.isfile(scenes_folder + f)
             if not f in excluded_scenes ]


def load_maxima_into_measures(scenes):
    try:
        f = open(maxima_file)
    except IOError:
        print maxima_file + " not found."
        raise

    lines = f.readlines()

    if len(lines) % 2 != 0:
        print "Expecting an even number of lines in " + maxima_file
        raise

    # Organize scenes by their filename to find them more conveniently.
    scenes_dict = {}
    for scene in scenes:
        scenes_dict[scene.fileName] = scene

    for i in range(0, len(lines), 2):
        filename = lines[i].strip()
        maxima = lines[i + 1].split()

        if not scenes_dict.has_key(filename):
            # print "Warning : " + filename + " was found in " + maxima_file \
            #       + " but not in " + scenes_folder
            continue

        scene = scenes_dict[filename]

        # Peaks are normalized to a range [0, 1]
        scene.maxima = [ int(position) for position in maxima ]
        scene.norm_maxima = [ float(position) / scene.measuresCount for 
                              position in maxima ]

        for maximum in scene.norm_maxima:
            if maximum < 0 or maximum > 1:
                print "Warning : " + filename + " has a maximum at a" \
                      + " position outside of the range [0, 1]."


def peak_count(scenes):
    return sum(len(scene.maxima) for scene in scenes)