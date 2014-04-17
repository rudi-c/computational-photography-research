"""Class definition for an object simulating a camera. The public interface
is the only interface allowed to control the camera. The camera only knows the
focus measures of positions it has visited.

The camera also knows which direction in which it is headed. If a coarse step 
is taken in the opposite backlash, backlash has the option to be simulated.

The camera model can also simulate measurement noise.

The camera model is one-use.
"""

import rtools
from scene      import Scene
from direction  import Direction
from random     import random, randint

noise_factor = 0.05
maximum_backlash = 3

class CameraModel(object):
    """See file docstring."""

    def _take_measure(self):
        """Take a measurement of focus value at the current lens position.
        """
        if self.simulate_noise:
            noise = random() * self._max_noise
        else:
            noise = 0    
        self._measures[self.position] = \
            self.scene.fvalues[self.position] + noise

    def _move(self, direction, count, step_size):
        """Move a given distance in a specified direction and number of times.
        """
        for _ in range(count):
            if not self.has_hit_edge():
                self.position = min(self.scene.step_count - 1, max(0,
                    self.position + direction * step_size))
                self._take_measure()
                self.steps_taken += 1
                self.visited_positions.append(self.position)

    def move_coarse(self, direction, count=1):
        """Move the camera in coarse steps in a given direction.
        """
        if self._direction != direction and self.simulate_backlash:
            self._direction = direction
            backlash_offset = randint(-maximum_backlash, maximum_backlash)
            self._move(direction, 1, 8 + backlash_offset)
            self._move(direction, count - 1, 8)
            self.backlash_count += 1
        else:
            self._direction = direction
            self._move(direction, count, 8)    

    def move_fine(self, direction, count=1):
        """Move the camera in fine steps in a given direction.
        """
        if self._direction != direction and self.simulate_backlash:
            self._direction = direction
            self._move(direction, count, 1)
            self.backlash_count += 1
        else:
            self._direction = direction
            self._move(direction, count, 1)    

    def get_fvalue(self, position):
        """Get the focus value measured at a given position. The function will
        throw an exception is no focus value was measured at that position.
        """
        return self.get_fvalues([position])[0]

    def get_fvalues(self, positions):
        """Get a set of focus values measured at a set of positions. The 
        function will throw an exception is no focus value was measured at
        any of those positions.
        """
        if all(self._measures.has_key(p) for p in positions):
            return [ self._measures[p] for p in positions ]
        else:
            raise Exception("Trying to obtain focus value at"
                            " non-visted positions %s" % str(positions))

    def has_hit_edge(self):
        """Whether the camera has hit an edge by moving in its present
        direction.
        """
        return self.will_hit_edge(self._direction)

    def will_hit_edge(self, direction):
        """Whether the camera will hit an edge by moving in the specified
        direction.
        """
        return ((self.position <= 0 and direction.is_left()) or        
                (self.position >= self.scene.step_count - 1 and 
                    direction.is_right()))

    def print_script(self, evaluation):
        """Print an R script that shows the movement of the lens in this
        particular simulation."""

        assert evaluation in ("true positive", "false positive", 
                              "true negative", "false negative",
                              "succeeded", "failed")

        print "# %s at %d, %s\n" % (self.scene.filename, 
                                    self.visited_positions[0], evaluation)

        # Some R functions for plotting.
        rtools.print_set_window_division(1, 1)
        print "library(scales)" # for alpha blending

        rtools.print_plot_focus_measures(self.scene.fvalues, show_grid=True)

        xs = self.visited_positions
        ys = [ float(i) / max(10, len(self.visited_positions))
               for i in range(0, len(self.visited_positions)) ]

        rtools.print_plot_point_pairs(xs, ys, 25, "blue", "blue", True)

        result = self.last_position()
        if result >= 0:
            print "segments(%d, 0.0, %d, 1.0)" % (result, result)

        print "\n# Plot me!\n"

    def last_position(self):
        """Return the last visited lens position."""
        return self.visited_positions[-1]

    def last_fmeasure(self):
        """Return the last measured focus value."""
        return self.get_fvalue(self.last_position())

    def __init__(self, scene, initial_pos,
                 simulate_backlash=False, simulate_noise=False):
        self.scene = scene
        self.position = initial_pos
        self.simulate_backlash = simulate_backlash
        self.simulate_noise = simulate_noise
        self.steps_taken = 0
        self.visited_positions = [ initial_pos ]
        self.backlash_count = 0

        self._measures = { initial_pos: scene.fvalues[initial_pos] }
        self._direction = None
        self._max_noise = min(self.scene.fvalues) * noise_factor