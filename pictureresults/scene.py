"""Class definition for an object storing the focus measures at each lens
position and the maxima (peaks) for each scene."""

import copy
import os

# Where to find the data.
SCENES_FOLDER = "focusraw/"
#SCENES_FOLDER = "lowlightgaussraw/"
MAXIMA_FILE = "maxima.txt"

class Scene(object):
    """Stores a the data of a particular scene, which is composed of focus
    values obtained from every lens position as well as the location of the
    peaks/maxima.
    """

    def __get_focus_value(self, string):
        """Extract the numeric focus value from a line."""
        parts = string.split()
        if len(parts) != 2:
            raise Exception("Lines in focus measures files "
                            "should only have two columns.")
        return float(parts[1])

    def __load_from_file(self, folder):
        """Load scene information from file."""
        try:
            f = open(folder + self.filename)
            lines = f.readlines()
            f.close()
        except IOError:
            raise IOError("Could not open scene file \"%d\"." % self.filename)

        focus_values = [self.__get_focus_value(line) for
                        line in lines]
        self.step_count = len(focus_values)
        self.fvalues = focus_values

        assert self.step_count > 0

    def inverse_copy(self):
        """Return an inverted copy of this scene."""
        i_scene = copy.copy(self)
        i_scene.fvalues = list(self.fvalues)
        i_scene.fvalues.reverse()
        i_scene.maxima = [ self.step_count - m - 1 for m in self.maxima ]
        i_scene.norm_maxima = [ float(m) / self.step_count 
                                for m in i_scene.maxima ]
        return i_scene

    def distance_to_closest_peak(self, lens_pos, condition=None):
        """Distance to the closest peak from a particular lens position."""
        if len(self.maxima) == 0:
            raise Exception("Error : No maxima loaded for this scene")
        min_dist = self.step_count
        for maximum in self.maxima:
            if condition is None or condition(maximum):
                min_dist = min(min_dist, abs(lens_pos - maximum))
        return min_dist
    
    def distance_to_closest_left_peak(self, lens_pos):
        """Distance to the closest peak on the left side of a lens position."""
        return self.distance_to_closest_peak(lens_pos,
            lambda max : max <= lens_pos)
    
    def distance_to_closest_right_peak(self, lens_pos):
        """Distance to the closest peak on the right side of a lens position."""
        return self.distance_to_closest_peak(lens_pos,
            lambda max : max >= lens_pos)

    def get_focus_values(self, positions):
        """Return the focus values corresponding to multiple lens positions."""
        return [self.fvalues[pos] for pos in positions]

    def __init__(self, filename, folder):
        self.filename = filename
        self.name = filename[:filename.rfind(".")] # remove extension
        self.step_count = 0
        self.fvalues = []
        self.maxima = []
        self.norm_maxima = []

        self.__load_from_file(folder)


def load_scenes(excluded_scenes=None, folder=None):
    """Loads scenes (sets of pictures at every lens position). Expects a folder
    focusraw/ and a file maxima.txt to exist in the same directory.
    """
    if excluded_scenes is None:
        excluded_scenes = []
    if folder is None:
        folder = SCENES_FOLDER

    if not os.path.isdir(folder):
        raise IOError("Scenes folder \"%s\" folder not found." % folder)

    scenes = [ Scene(f, folder) 
               for f in os.listdir(folder) 
               if f.endswith(".txt")
               if os.path.isfile(folder + "/" + f)
               if not f in excluded_scenes ]
    load_maxima(scenes)
    return scenes


def load_maxima(scenes):
    """Looks in a file named maxima.txt for the location of the peaks for
    each scene.
    """
    try:
        f = open(MAXIMA_FILE)
        lines = f.readlines()
        f.close()
    except IOError:
        raise IOError("Could not open maxima file \"%s\"." % MAXIMA_FILE)

    if len(lines) % 2 != 0:
        raise IOError("Expected an even number of lines in %s." % MAXIMA_FILE)

    # Organize scenes by their filename to find them more conveniently.
    scenes_dict = { scene.filename: scene for scene in scenes }

    for i in range(0, len(lines), 2):
        filename = lines[i].strip()
        maxima = lines[i + 1].split()

        try:
            scene = scenes_dict[filename]
        except KeyError:
            continue

        # Peaks are normalized to a range [0, 1]
        scene.maxima = [ int(position) for position in maxima ]
        scene.norm_maxima = [ float(position) / scene.step_count
                              for position in maxima ]

        for maximum in scene.norm_maxima:
            if maximum < 0 or maximum > 1:
                print "Warning : " + filename + " has a maximum at a" \
                      + " position outside of the range [0, 1]."
