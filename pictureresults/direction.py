"""Class definition for a object representing direction (left or right),
which can be used both as a string and as a number (-1, +1)"""

class Direction(object):
    """Represents a direction. Prints as a string and multiples as +/- 1."""

    def __init__(self, direction):
        if direction == "left" or direction < 0:
            self._string_direction = "left"
            self._int_direction = -1
        elif direction == "right" or direction > 0:
            self._string_direction = "right"
            self._int_direction = +1
        else:
            assert False

    def __add__(self, other):
        return self._int_direction + other

    def __radd__(self, other):
        return self._int_direction + other

    def __mul__(self, other):
        return self._int_direction * other

    def __rmul__(self, other):
        return self._int_direction * other

    def __eq__(self, other):
        return (self._int_direction == other or
                self._string_direction == other or
                str(self) == str(other))

    def __ne__(self, other):
        return not self == other

    def __repr__(self):
        return self._string_direction

    def is_left(self):
        """Direction represents 'left'"""
        return self._int_direction == -1

    def is_right(self):
        """Direction represents 'right'"""
        return self._int_direction == +1

    def reverse(self):
        """Returns an object representing the opposite direction."""
        if self._string_direction == "left":
            return Direction("right")
        else:
            return Direction("left")
