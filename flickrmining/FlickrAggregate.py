#!/usr/bin/python

# This file contains tools that help process exif data saved in json
# format (supposedly downloaded with FlickrRandom.py)
#
# Although this file contains a script that can be called directly
# from the terminal, the best way to use it is to load its functions
# in the python interpreter via :
#
# >>> execfile("FlickrAggregate.py")
#
# Then, type help() to see a list of available functions or
# help(function_name) to get more information on a given function.

import getopt
import json
import operator
import os
import re # regex
import sys

from sets import Set

numerics_regex_obj = re.compile("^[0-9]*(\.[0-9]*)?")


def filter_nikon_and_canon(exif):
    """
    filter_nikon_and_canon(exif) - This is a filter function.
    Returns true if the exif is from a picture taken by a Canon or Nikon
    branded camera.
    """
    if exif.has_key("Model"):
        model = exif["Model"]
        if model.tolower().contains("canon") or \
           model.tolower().contains("nikon"):
           return True

    return False


def filter_tag_exists(tag):
    """
    filter_tag_exists(tag) - Returns a filter function.
    The filter function returns true if the exif has a given tag.
    """
    def filter_internal(exif):
        return exif.has_key(tag)
    return filter_internal


def filter_tag_has_value(tag, value, compare_numbers=True):
    """
    filter_tag_has_value(tag, value, compare_numbers=True) 
    Returns a filter function.
    The filter function returns true if the exif has a tag of a given value.
    'compare_numbers' will convert from a string to a number and remove units.
    """
    if compare_numbers:
        def filter_internal(exif):
            try:
                return exif.has_key(tag) and \
                       is_number(leading_numerics(exif[tag])) and \
                       leading_numerics(exif[tag]) == value
            except ValueError:
                # Incase exif[tag] does not contain a leading number
                return False
        return filter_internal
    else:
        def filter_internal(exif):
            return exif.has_key(tag) and exif[tag] == value


def convert_unicode(input):
    """
    Converts every unicode string in an object to a normal string,
    """
    if isinstance(input, dict):
        return {convert_unicode(key): convert_unicode(value) 
                for key, value in input.iteritems()}
    elif isinstance(input, list):
        return [convert_unicode(element) for element in input]
    elif isinstance(input, unicode):
        return input.encode('utf-8')
    else:
        return input


def is_number(s):
    """
    is_number(s) - True if a string s is a number.
    """
    try:
        float(s)
        return True
    except ValueError:
        return False


def save_to_file(data, file_path):
    """
    save_to_file(data, file_path) - Save any object to file as a json format.
    If the object is a string, then the string is saved without quotation marks.
    """
    try:
        f = open(file_path, 'w+')

        if isinstance(data, basestring):
            f.write(data)
        else:
            f.write(json.dumps(data))

        # Ending with a newline makes the file nicer to printout to terminal.
        f.write('\n')

    except Exception as e:
        print "Failed to write file : "
        print e


def leading_numerics(s):
    """
    leading_numerics(s) - Returns the leading part of the string as a
    floating point number.
    """

    # The caller of this function should either guarantee that the input
    # will be parseable or be prepared to handle ValueError exceptions.

    # Returns all the leading digit characters and converts to number.
    # This is useful to get rid of units in a string.
    return float(numerics_regex_obj.search(s).group())


def remove_units(data):
    """
    remove_units(data) - Remove units from every string in an array and 
    return the string as a number.
    """

    # Remove all units from numerical values in the array.
    return [leading_numerics(datum) for datum in data]


def load_exifs(folders):
    """
    load_exifs(folders) - Loads all json files into a dict of <tag, value>
    folders can be a string or an array of strings
    """

    exifs = []

    # Handle non-list inputs (if the input is a single string,
    # convert to a list of one item)
    if isinstance(folders, basestring):
        folders = [folders]

    for folder in folders:

        # Make sure the folder exists
        if not os.path.isdir(folder):
            print "Folder " + folder + " not found. Skipping."
            continue

        json_files = (folder + "/" + path for path in os.listdir(folder) 
                      if path.endswith(".json"))
        for path in json_files:
            try:
                f = open(path)

                # JSON files are loaded with text in the form of unicode arrays
                # instead of strings, which is inconvenient. So we convert 
                # everything to strings recursively.
                # See http://stackoverflow.com/questions/956867
                contents = convert_unicode(json.loads(f.read()))

                # It is useful to keep track of the file ID even though it
                # is not part of the exif itself
                # The id is obtained by removing .json from the filename.
                # Assuming that the exif files come from FlickrRandom, that is.
                contents["id"] = os.path.basename(file_path)[:-5]

                exifs.append(contents)

            except Exception as e:
                print "Failed to read " + path
                print e

    return exifs


def tally_exifs(exifs):
    """ 
    tally_exifs(exifs) - Returns the dictonnary of exif tags that appears 
    in all the exif files and their frequencies.
    """

    tally = {}

    for exif in exifs:
        for key in exif:
            if tally.has_key(key):
                tally[key] += 1
            else:
                tally[key] = 1

    return tally


def tally_tag(exifs, tag, sort=False, sort_frequency=True,
    string_form=True, filters=[]):
    """
    tally_tag(exifs, tag, sort=False, sort_frequency=True, 
        string_form=True, filters=[])
    Returns the number of occurrences of each value of a given tag.
    'sort' and 'sort_frequency' will sort by value and value frequency,
        respectively. Only one can be True.
    'string_form' will return the results as a string instead of a python object.
    'filters' are additional filter functions for whether an exif gets counted
    """

    tally = {}

    # Avoid conflicting sort requests.
    if sort and sort_frequency:
        print """Error in usage of tally_tag : cannot have both arguments
                 sort=True and sort_frequency=True"""
        return

    # Tally
    for exif in exifs:
        if exif.has_key(tag) and all([f(exif) for f in filters]):
            value = exif[tag]
            if tally.has_key(value):
                tally[value] += 1
            else:
                tally[value] = 1

    # Return results in desired format.
    if sort or sort_frequency:
        if sort:
            sorted_tally = sorted(tally.iteritems(), key=operator.itemgetter(0))
        else:
            sorted_tally = sorted(tally.iteritems(), 
                                  key=operator.itemgetter(1), 
                                  reverse=True) # larger numbers first
        
        if string_form:
            return "\n".join("%s %s" % (elem[0], elem[1]) 
                             for elem in sorted_tally)
        else:
            return sorted_tally
    else:
        if string_form:
            return "\n".join("%s %s" % (key, tally[key]) 
                             for key in tally)
        else:
            return tally


def get_tag_data(exifs, tag, contexts=[], filters=[], make_numerical=True):
    """
    get_tag_data(exifs, tag, contexts=[], filters=[], make_numerical=True)
    Returns a sorted list of the value of each instance of a given tag.
    'contexts' is an array of tags (strings). If non-empty, the output
               will be an array of tuples where the first element of the tuple
               is the value for 'tag' and the rest, the values for the tags
               in 'contexts'
    'filters'  are additional filter functions for whether an exif included
    'make_numerical' will convert tag values to numbers whenever possible
    """

    data = []

    for exif in exifs:
        if exif.has_key(tag) and all([f(exif) for f in filters]):
            try:
                if make_numerical:
                    value = leading_numerics(exif[tag])
            except ValueError:
                # Failed to get a number, so leave it as it is.
                value = exif[tag]

            if len(contexts) == 0:
                data.append(value)
            else:
                # Append a tuple of values for each tag
                data.append([value] + 
                    [exif[context] if exif.has_key(context) else ""
                     for context in contexts])

    if len(contexts) == 0:
        return sorted(data)
    else:
        # Sort on the first element of the tuple.
        return sorted(data, key=operator.itemgetter(0))


def marginalize_over_component(data1, data2, sort=True):
    """
    marginalize_over_component(data1, data2, sort=True)
    'data1', 'data2' are arrays of tuples <a, b>
    The function returns a tuple <data1_new, data2_new> where data*_new
    is a random sample from the 'a's of data* such that both data1_new
    and data2_new contain the same number of instances of each value of 'b'
    'sort' will sort the data
    This function is useful to randomize over a variate.
    """

    sorted_data1 = {}
    sorted_data2 = {}

    for datum in data1:
        if sorted_data1.has_key(datum[1]):
            sorted_data1[datum[1]].append(datum[0])
        else:
            sorted_data1[datum[1]] = [datum[0]]
    for datum in data2:
        if sorted_data2.has_key(datum[1]):
            sorted_data2[datum[1]].append(datum[0])
        else:
            sorted_data2[datum[1]] = [datum[0]]

    all_keys = Set(sorted_data1.keys()).union(sorted_data2.keys())

    output1 = []
    output2 = []

    for key in all_keys:
        instances1 = len(sorted_data1[key]) if sorted_data1.has_key(key) else 0
        instances2 = len(sorted_data2[key]) if sorted_data2.has_key(key) else 0
        num_instances_to_sample = min(instances1, instances2)

        if num_instances_to_sample > 0:
            output1 += random.sample(sorted_data1[key], num_instances_to_sample)
            output2 += random.sample(sorted_data2[key], num_instances_to_sample)

    try:
        if sort:
            sorted_output1 = sorted(output1)
            sorted_output2 = sorted(output2)
            return (sorted_output1, sorted_output2)
        else:
            # Might not be sortable.
            raise
    except:
        return (output1, output2)


def help(obj=None):
    """
    help() - Provides help specific to FlickrAggregate
    """

    if obj is None:
        print """Test
                 ** Loading exifs.
                 - load_exifs

                 ** Aggregating data
                 - tally_exifs
                 - get_tag_data
                 - marginalize_over_component

                 ** Filter functions (passed as arguments to some functions).
                 - filter_nikon_and_canon
                 - filter_tag_exists
                 - filter_tag_has_value

                 ** Convenience functions.
                 - is_number
                 - leading_numerics
                 - remove_units
                 
                 End of help()"""
    else:
        print obj.__doc__.strip()


def main(argv):

    # Parse script arguments
    try:
       opts, args = getopt.getopt(argv, "lts:", ["tally", "list", "select"])
    except getopt.GetoptError:
       print  """Script usage : ./FlickrAggregate
                 [-t <display exif tags tally>]
                 [-l <list the exif tags alphabetically] 
                 [-s <display all the data of a particular exif tag] """
       sys.exit(2)

    exifs = load_exifs(args)

    for opt, arg in opts:
        if opt in ("-l", "--list"):

            tally = tally_exifs(exifs)
            # Convert to list and sort alphabetically.
            sorted_tally = sorted(tally.iteritems(), key=operator.itemgetter(0))
            for tag in sorted_tally:
                print tag[0] + " " + str(tag[1])

        elif opt in ("-t", "--tally"):
            tally = tally_exifs(exifs)
            # Convert to list and sort by frequency.
            sorted_tally = sorted(tally.iteritems(), 
                key=operator.itemgetter(1), reverse=True)
            for tag in sorted_tally:
                print tag[0] + " " + str(tag[1])

        elif opt in ("-s", "--select"):
            data = get_tag_data(exifs, arg)
            print data


# Entry point.
main(sys.argv[1:])
