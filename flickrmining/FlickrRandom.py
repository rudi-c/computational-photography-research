#!/usr/bin/python

import getopt
import json
import subprocess
import os
import random
import urllib
import sys
import time
import traceback

import xml.etree.ElementTree as ET

# Rudi's Flickr API key
api_key = "170cf699c060c0e54061e8138752dada"

# For logging. Remember to use the global key word when using these
# inside functions.
search_attempts = 0
download_attempts = 0

# Stuff for logging
def log_search_attempt():
    global search_attempts
    search_attempts += 1

def log_download_attempt():
    global download_attempts
    download_attempts += 1

def get_search_log():
    global search_attempts
    return search_attempts

def get_download_log():
    global download_attempts
    return download_attempts

# Returns XML root
def call_api(function, params):

    string_params = "&api_key=" + api_key
    for key in params:
        string_params += "&" + key + "=" + str(params[key])

    url = "http://api.flickr.com/services/rest/" \
          "?method=" + function + string_params

    print "Calling API function " + function + "..."

    result = urllib.urlopen(url)
    tree = ET.parse(result)

    status = tree.getroot().get('stat') 

    if status == 'ok':
        return tree.getroot()
    else:
        print "API Called Failed, stat=" + status
        print "URL : " + url
        print "XML : "
        print ET.tostring(tree.getroot())
        raise


def get_fileformat_from_id(photo_id, secret):

    # Download the photo's info.
    params = {"photo_id" : photo_id, "secret" : secret}
    root = call_api("flickr.photos.getInfo", params)

    # XML is of the form
    # <rsp ...>
    #   <photo ...>
    #     ...
    #   </photo>
    # </rsp>
    format = root.find('photo').get('originalformat')
    return format


def get_exif_from_id(photo_id, secret):

    # Download the photo's exif data
    params = {"photo_id" : photo_id, "secret" : secret}
    root = call_api("flickr.photos.getExif", params)

    # XML is of the form
    # <rsp ...>
    #   <photo ...>
    #     <exif ...>
    #       <raw>...</raw>
    #       <clean>...</clean> (sometimes present)
    #     </exif>
    #     ...
    #   </photo>
    # </rsp>

    # Compile exif data
    exif_data = {}
    for exif in root.find('photo').iter('exif'):
        # exif.get('label') # more readable version of 'tag'
        exif_data[exif.get('tag')] = exif.find('raw').text

    return exif_data


def download_image_file(download_folder, farm_id, server_id, photo_id, secret):

    # Downloads requested picture.
    # Returns the path in which file was downloaded

    photo_url = "http://farm%s.staticflickr.com/%s/%s_%s.jpg" \
                % (farm_id, server_id, photo_id, secret)
    target_photo = download_folder + photo_id + ".jpg"
    
    result = subprocess.call(["/usr/bin/wget", "-O", target_photo, photo_url])
    if result == 0:
        return target_photo
    else:
        raise


def get_dictionnary_unix():
    # Need to remove the newline character that comes with each word.
    return [word.strip() for word in list(open('/usr/share/dict/words'))]


def get_random_word(dictionnary):
    return random.choice(dictionnary)


def get_number_of_results_from_word(word):

    # Logging
    log_search_attempt()

    # Find out how many pictures match our query.
    params = {"per_page" : 1, "text" : word}
    root = call_api("flickr.photos.search", params)

    return int(root.find('photos').get('total'))


def silent_remove_file(file):

    # Remove files and do not throw an exception if
    # the file has already been deleted.
    try: 
        os.remove(file)
    except OSError:
        # Incase deletion failed.
        if os.path.isfile(file):
            raise

def download_by_photo_id(download_folder, photo_id, 
    farm_id, server_id, secret):

    # Returns whether the picture was downloaded successfully.

    exif_file_path = download_folder + photo_id + ".json"
    image_file_path = ""

    try:
        # Make sure the picture is in jpg format.
        # Most pictures will be in jpg format anyway and other format
        # could be result of significant amounts of editing.
        format = get_fileformat_from_id(photo_id, secret)
        if not format is None:
            format = format.lower()
        if not (format == 'jpg' or format == 'jpeg'):
            raise RuntimeError("Not jpg format.")

        # Save exif as a json file
        f = open(exif_file_path, 'w+')
        try:
            f.write(json.dumps(get_exif_from_id(photo_id, secret)))
        except:
            raise RuntimeError("Failed to save exif.")

        # Download the photo
        try:
            image_file_path = download_image_file(download_folder, 
                                        farm_id, server_id, photo_id, secret)
        except:
            raise RuntimeError("Failed to download photo.")

    except RuntimeError as e:
        print e

        # Delete lingering/incompleted exif and image files.
        silent_remove_file(exif_file_path)
        if image_file_path == "":
            silent_remove_file(image_file_path)

        # Failure
        return False

    # Success
    return True

def download_search_result(word, result_index, download_folder):

    # Download the picture corresponding to a specific search result.
    # Returns whether the picture was downloaded successfully.

    # Logging
    log_download_attempt()

    # Get information for the picture we've chosen.
    params = {"per_page" : 1, "page" : result_index, "text" : word}
    root = call_api("flickr.photos.search", params)

    # Extract info
    photo = root.find('photos').find('photo')
    farm_id     = photo.get('farm')
    server_id   = photo.get('server')
    photo_id    = photo.get('id')
    secret      = photo.get('secret')

    return download_by_photo_id(download_folder, photo_id, 
        farm_id, server_id, secret)


def download_random_picture(download_folder, word_dict):

    # Keep using random words for search until we find a word
    # for which a search result exists and succeed at downloading
    # a picture from the search results.

    # Note : XML for flicker.photos.search will look like this:
    # <rsp>
    #     <photos page="2" pages="89" perpage="10" total="881">
    #         <photo id="2636" owner="47058503995@N01" 
    #             secret="a123456" server="2" title="test_04"
    #             ispublic="1" isfriend="0" isfamily="0" />
    #         ...
    #     </photos>
    # </rsp>

    while True:

        # Get a random word to use our search with.
        search_word = get_random_word(word_dict)

        print "Try to get a random picture from search " \
              "for word \"" + search_word + "\""

        # Search.
        try:
            number_of_results = get_number_of_results_from_word(search_word)
        except Exception as e:
            print e
            print "Failed to get the number of results " \
                  "for word \"" + search_word + "\""
            continue

        if number_of_results == 0:
            print "No results found."
            continue

        # Choose a random picture among the search query.
        result_index = random.randint(0, number_of_results - 1)

        print "Downloading result # " + str(result_index) + \
              " out of " + str(number_of_results) + \
              " from search for \"" + search_word + "\""

        try:
            success = download_search_result(search_word, 
                result_index, download_folder)

            # Exit at the first successful download.
            if success:
                break
            else:
                print "Download failed."

        except Exception as e:
            print e

    print "Download successful"


def download_most_recent_picture(download_folder):

    # Keep trying to download the most recent picture.

    # Note : XML for flicker.photos.getRecent will look like this:
    # <rsp>
    #     <photos page="1" pages="10" perpage="100" total="1000">
    #         <photo id="2636" owner="47058503995@N01" 
    #             secret="a123456" server="2" title="test_04"
    #             ispublic="1" isfriend="0" isfamily="0" />
    #         ...
    #     </photos>
    # </rsp>

    while True:

        # Logging
        log_download_attempt()

        try:
            print "Getting most recent picture info..."

            # Get information for the picture we've chosen.
            params = {"per_page" : 1}
            root = call_api("flickr.photos.getRecent", params)

            # Extract info
            photo = root.find('photos').find('photo')
            title       = photo.get('title')
            farm_id     = photo.get('farm')
            server_id   = photo.get('server')
            photo_id    = photo.get('id')
            secret      = photo.get('secret')

            print "Trying to download \"" + title + "\""

            success = download_by_photo_id(download_folder, photo_id, 
                farm_id, server_id, secret)

            if success:
                break
            else:
                raise

        except Exception as e:
            print e
            print "Failed to download recent picture."

    print "Download successful"



def main(argv):

    # Default values for the script arguments.
    # Download 1 picture to ./default/
    download_folder = os.getcwd() + '/default/'
    num_pics_to_dl = 1
    option = "search"

    # Parse script arguments
    try:
       opts, args = getopt.getopt(argv,"o:f:n:",
                                  ["option=", "folder=","num_pics="])
    except getopt.GetoptError:
       print  """Script usage : ./FlickrRandom 
                 [-f <download folder> (default=./default)]
                 [-n <number of files to download> (default=1)]
                 [-option <how to choose pictures to download> (default=search)]"""
       sys.exit(2)

    for opt, arg in opts:
        if opt in ("-f", "--folder"):
            download_folder = "/" + arg + "/"
        elif opt in ("-n", "--num_pics"):
            num_pics_to_dl = int(arg)
        elif opt in ("-o", "--option"):
            option = arg

    # Make sure the directory for downloaded files exists.
    try: 
        os.makedirs(download_folder)
    except OSError:
        if not os.path.isdir(download_folder):
            raise

    # Since Flickr does not provide a getrandompicture api function,
    # we can find pictures semi-randomly by picking random results
    # from random searches.
    word_dict = get_dictionnary_unix()

    # Timer, it's interesting to know how long it took to download.
    start = time.time()

    codes = {"search" : "download_random_picture(download_folder, word_dict)",
             "recent" : "download_most_recent_picture(download_folder)"}
    download_code = codes[option]
    
    print "Executing : " + download_code

    # Download as many pictures as needed.
    for i in range(num_pics_to_dl):
        # How the pictures are downloaded depends on the command-line args.
        exec download_code

    elapsed = (time.time() - start)

    # Output log.
    print "Finished downloading " + str(num_pics_to_dl) + " pictures."
    print "Elapsed : " + str(elapsed) + "s"
    print str(get_search_log()) + " search queries were made."
    print str(get_download_log()) + " download attempts were made."


# Entry point.
main(sys.argv[1:])