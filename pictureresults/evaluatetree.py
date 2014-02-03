

def find_next_whitespace(string, start_index = 0):
    """Finds the first occurence of whitespace (space, tab or newline)
    after start_index. Return -1 if non"""
    next_space   = string.find(' ',  start_index)
    next_tab     = string.find('\t', start_index)
    next_newline = string.find('\n', start_index)
    earliest = max(next_space, next_tab, next_newline)
    if next_space > 0:
        earliest = min(earliest, next_space)
    if next_tab > 0:
        earliest = min(earliest, next_tab)
    if next_newline > 0:
        earliest = min(earliest, next_newline)
    return earliest


def check_brackets(input):
    """Ensures that the brackets match (every opening bracket has a
    corresponding closing bracket)"""
    n_pairs = 0
    for i in range(0, len(input)):
        if input[i] == '(':
            n_pairs += 1
        elif input[i] == ')':
            if n_pairs == 0:
                raise Exception("Closing bracket without opening bracket : \n"
                                + input[:(i+1)])
            else:
                n_pairs -= 1
    if n_pairs != 0:
        raise Exception(str(n_pairs) + 
                        " opening brackets have not been closed")


def _parse_tree(input, functions, current_index):
    """Recursive helper function for parse_tree"""

    # Skip extra leading whitespaces
    first_char = input[current_index]
    while first_char == ' ' or first_char == '\t' or first_char == '\n':
        current_index += 1
        first_char = input[current_index]

    if not (first_char == '(' or input[current_index:].startswith("left") \
                              or input[current_index:].startswith("right")):
        raise Exception("Every child should either be a subtree, a "
                        "\"left\" leaf or a \"right\" leaf :\n"
                        + input[:(current_index+1)])

    # This is a leaf node.
    if not first_char == '(':
        if input[current_index:].startswith("left"):
            return (0, current_index + len("left"))
        else:
            return (1, current_index + len("right"))

    # Parse function name.
    next_whitespace = find_next_whitespace(input, current_index)
    function_name = input[current_index + 1 : next_whitespace]
    function = functions[function_name]

    next_index = next_whitespace + 1

    # Recursively parse all child nodes.
    children = []
    while input[next_index] != ')':
        child, next_index = _parse_tree(input, functions, next_index)
        children.append(child)

    if len(children) <= 0:
        raise Exception("Every non-leaf node should have at "
                        "least one child : \n" + input[:(next_index+1)])

    return ((function, children), next_index + 1)


def parse_tree(input, functions):
    """Parse a string into tree form, as a tuple of function
    and an array of children."""
    check_brackets(input)
    node, next_index = _parse_tree(input, functions, 0)
    return node
    

def tree_eval(scene, lens_pos, tree, step_size):
    """Evaluate the tree and return 0 if the root node is 'left',
    1 if the root node is 'move right'"""

    # Reached a leaf.
    if tree == 0 or tree == 1:
        return tree

    # Pattern matching is so convenient.
    function, children = tree

    node_value = function(
        first  = scene.measuresValues[lens_pos - 2 * step_size],
        second = scene.measuresValues[lens_pos - 1 * step_size],
        third  = scene.measuresValues[lens_pos],
        lens_pos = float(lens_pos) / (scene.measuresCount - 1))

    if node_value is True:
        return tree_eval(scene, lens_pos, children[1], step_size)
    elif node_value is False:
        return tree_eval(scene, lens_pos, children[0], step_size)
    elif isinstance(node_value, int):
        try:
            return tree_eval(scene, lens_pos, children[node_value], step_size)
        except IndexError:
            raise Exception("Insufficient number of children for a " \
                            "feature with integer values.")
    else:
        raise Exception("Features should produce a boolean or an integer.")

