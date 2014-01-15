#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <string>
#include <iostream>

#include "imageTools.h"

using namespace std;

int compute_median(unsigned char* buffer, int x, int y, int window_size)
{
    const int w = 1056;
    const int h =  704;
    const int n = w * h;

    int k = x*w + y;
    int start_delta = -window_size / 2;
    int end_delta = window_size / 2;
    int median_index = window_size * window_size / 2;
    std::vector <int> window;

    for (int x_delta = start_delta; x_delta <= end_delta; x_delta++) {
      for (int y_delta = start_delta; y_delta <= end_delta; y_delta++) {
        window.push_back(buffer[k + x_delta * w + y_delta]);
      }
    }
    std::nth_element(window.begin(), window.begin() + median_index, window.end());

    return window[median_index];
}

int compute_adaptive_median(unsigned char* buffer, int x, int y, int window_size)
{
    const int w = 1056;
    const int h =  704;
    const int n = w * h;
    int k = x*w + y;
    int this_val = buffer[k];
    int start_delta = -window_size / 2;
    int end_delta = window_size / 2;
    int median_index = window_size * window_size / 2;
    std::vector <int> window;
    for (int x_delta = start_delta; x_delta <= end_delta; x_delta++) {
      for (int y_delta = start_delta; y_delta <= end_delta; y_delta++) {
        window.push_back(buffer[k + x_delta * w + y_delta]);
      }
    }
    int window_min = *min_element(window.begin(),window.end());
    int window_max = *max_element(window.begin(),window.end());
    std::nth_element(window.begin(), window.begin() + median_index, window.end());
    int window_med = window[median_index];

    if (window_med - window_min <= 0 || window_med - window_max >= 0) {
      if (x + start_delta - 1 >= 0 && y + start_delta - 1 >= 0 &&
          x + end_delta + 1 < h && y + end_delta + 1 < w && window_size < 10) {
        return compute_adaptive_median(buffer, x, y, window_size + 2);
      } else {
        return window_med;
      }
    }

    if (this_val - window_min <= 0 || this_val - window_max >= 0) {
      return window_med;
    }

    return this_val;
}

void print_usage(char* progname)
{
    cerr << "Usage: " << progname << " [OPTIONS] [FILES]" << endl;
    cerr << "\t Valid options include --output-png --compute-median " << endl;
    cerr << "\t --compute_adaptive_median" << endl;
    exit(1);
}

int
main( int argc, char *argv[] )
{
    // Default image sizes.
    const int w = 1056;
    const int h =  704;
    const int n = w * h;

    // Store input/output images.
    unsigned char buffer[n], result[n];

    // Function to process the image with.
    int (*compute)(unsigned char*, int, int, int) = compute_median;

    // Number of command-line options passed onto this program
    // (things that start with --)
    int optionsCount = 0;
    bool outputPNG = false;

    for (int i = 1; i < argc; i++)
    {
        string option(argv[i]);
        if (option == "--median")
            compute = compute_median;
        else if (option == "--adaptive-median")
            compute = compute_adaptive_median;
        else if (option == "--output-png")
            outputPNG = true;
        else if (option[0] == '-' && option[1] == '--')
            // This option isn't recognized.
            print_usage(argv[0]);
        else
            // A file - we can stop reading options now.
            break;

        optionsCount++;
    }

    // Process all files passed to this program.
    for (int i = 1 + optionsCount; i < argc; i++)
    {
        string inputFile(argv[i]);

        ImageTools::readGray( inputFile.c_str(), n, buffer );

        // TODO: Is this loop needed?
        for (int i = 0; i < n; i++)
            result[i] = buffer[i];

        for( int i = 1; i < h-1; i++ )
            for( int j = 1; j < w-1; j++ )
                result[i*w + j] = compute(buffer, i, j, 3);

        string outputFile = inputFile + ".median";
        ImageTools::saveGray( outputFile.c_str(), n, result );

        if (outputPNG)
        {
            string pngOutputFile = inputFile + ".png";
            ImageTools::saveGrayPng( pngOutputFile.c_str(), result, w, h);
        }
    }
}
