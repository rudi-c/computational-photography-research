# Data structure for storing the focus measures at each lens position
# for each scene.

# Where to find the data.
scenes_folder = "focusmeasures/"
maxima_file = "maxima.txt"

class Scene:
    def __get_focus_value(self, string):
        parts = string.split()
        if len(parts) != 2:
            print "Lines in focus measures files should only have two columns."
            raise
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

    def __init__(self, file_name):
        self.fileName = file_name
        self.__load_file()
        self.maxima = []


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