
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
void readGray( char *fileName, int n, unsigned char *buffer );

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
  cerr << "Usage: " << progname << " mode filename" << endl;
  exit(1);
}

int
main( int argc, char *argv[] )
{
    const int w = 1056;
    const int h =  704;
    const int n = w * h;
    unsigned char buffer[n], result[n];
    double measure[argc];
    int (*compute)(unsigned char*, int, int, int);
    string mode(argv[1]);

    if (mode == "--median") {
      compute = compute_median;
    } else if (mode == "--adaptive-median") {
      compute = compute_adaptive_median;
    } else {
      print_usage(argv[0]);
    }

    double v = 0;
    readGray( argv[2], n, buffer );
    for (int i = 0; i < n; i++) {
      result[i] = buffer[i];
    }

    for( int i = 1; i < h-1; i++ ) {
      for( int j = 1; j < w-1; j++ ) {
        result[i*w + j] = compute(buffer, i, j, 3);
      }
    }
    FILE *fp;
    string file(argv[2]);
    file += ".median";
    fp = fopen( file.c_str(), "w" );
    fwrite( result, 1, n, fp );
    fclose(fp);
}

/*
 *  Read the gray values from a file into buffer.
 */
void readGray( char *fileName, int n, unsigned char *buffer )
{
    FILE *fp;

    fp = fopen( fileName, "rb" );
    if( fp == NULL ) {
	fprintf( stderr, "No such file: %s\n", fileName );
	exit( 1 );
    }

    int result = fread( buffer, 1, n, fp );
    if( result != n ) {
	fprintf( stderr, "Wrong number of bytes: %s\n", fileName );
	exit( 1 );
    }


    fclose( fp );
}

