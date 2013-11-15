#!/usr/bin/python

import getopt
import json
import operator
import os
import re # regex
import sys

numerics_regex_obj = re.compile("^[0-9]*(\.[0-9]*)?")

def convert_unicode(input):

    # JSON files are loaded with text in the form of unicode arrays
    # instead of strings, which is inconvenient. So we convert everything
    # to strings recursively.
    # See http://stackoverflow.com/questions/956867
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
    try:
        float(s)
        return True
    except ValueError:
        return False


def leading_numerics(string):

    # Returns all the leading digit characters and converts to number.
    # This is useful to get rid of units in a string.
    return float(numerics_regex_obj.search(string).group())


def filter_out_units(data):

    # Remove all units from numerical values in the array.
    return [leading_numerics(datum) for datum in data]


def load_exifs(folders):

    exifs = []

    # Handle non-list inputs.
    if isinstance(folders, basestring):
        folders = [folders]

    for folder in folders:
        # Make sure the folder exists
        if not os.path.isdir(folder):
            print "No folder " + folder + " found. Skipping."
            return

        for files in os.listdir(folder):
            if files.endswith(".json"):
                file_path = folder + "/" + files

                try:
                    f = open(file_path)
                    contents = f.read()
                    exifs.append(convert_unicode(json.loads(contents)))
                except Exception as e:
                    print "Failed to read " + file_path
                    print e

    return exifs


def tally_exifs(exifs):

    # Returns the dictonnary of exif data that appears in all the
    # exif files that have been loaded and their frequencies.

    tally = {}

    for exif in exifs:
        for key in exif:
            if tally.has_key(key):
                tally[key] += 1
            else:
                tally[key] = 1

    return tally


def get_field_data(exifs, field, contexts=[], conditions={}):

    # Return a sorted list of the value in a given field,
    # whenever present.
    # If the contexts argument contains strings, then the
    # list returned is a list of tuple where the first
    # element of the tuple is the value of the field searched for,
    # and the rest are values of the fields in contexts (possibly nothing).

    data = []

    # If every instance of the field as a numerical value, we'll convert it
    # to a number for convenience.
    numerical = True
    for exif in exifs:
        if exif.has_key(field) and not is_number(exif[field]):
            numerical = False
            break

    for exif in exifs:
        if exif.has_key(field):

            meets_conditions = True
            for condition_field in conditions:
                if exif.has_key(condition_field):
                    elem = exif[condition_field]
                else:
                    elem = ""
                if not eval(conditions[condition_field]):
                    meets_conditions = False

            if meets_conditions:
                if numerical:
                    value = float(exif[field])
                else:
                    value = exif[field]

                if len(contexts) == 0:
                    data.append(value)
                else:
                    data.append([value] + 
                        [exif[field] if exif.has_key(context) else ""
                         for context in contexts])

    if len(contexts) == 0:
        return sorted(data)
    else:
        return sorted(data, key=operator.itemgetter(0))


def main(argv):

    # Parse script arguments
    try:
       opts, args = getopt.getopt(argv, "lts:", ["tally", "list", "select"])
    except getopt.GetoptError:
       print  """Script usage : ./FlickrAggregate
                 [-t <display exif fields tally>]
                 [-l <list the exif fields alphabetically] 
                 [-s <display all the data of a particular exif field] """
       sys.exit(2)

    exifs = load_exifs(args)

    for opt, arg in opts:
        if opt in ("-l", "--list"):

            tally = tally_exifs(exifs)
            # Convert to list and sort alphabetically.
            sorted_tally = sorted(tally.iteritems(), key=operator.itemgetter(0))
            for field in sorted_tally:
                print field[0] + " " + str(field[1])

        elif opt in ("-t", "--tally"):
            tally = tally_exifs(exifs)
            # Convert to list and sort by frequency.
            sorted_tally = sorted(tally.iteritems(), 
                key=operator.itemgetter(1), reverse=True)
            for field in sorted_tally:
                print field[0] + " " + str(field[1])

        elif opt in ("-s", "--select"):
            data = get_field_data(exifs, arg)
            print data


# Entry point.
main(sys.argv[1:])