#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "ImageTools.h"

using namespace std;

#define DEFAULT_WIDTH 1056
#define DEFAULT_HEIGHT 704

int
main( int argc, char *argv[] )
{
    if( argc <= 3 ) {
        fprintf( stderr, "Usage: addlowlight darkenFactor noiseFactor [files 0..n]\n" );
        fprintf( stderr, "\t darkenFactor -- amount of light to remove (0-1) \n" );
        fprintf( stderr, "\t noiseFactor -- amount of noise to add (0-1)\n" );
        exit( 6 );
    }

    float darkenFactor = atof(argv[1]);
    float noiseFactor  = atof(argv[2]);

    for( int i = 3; i < argc; i++ ) 
    {
        vector<uchar> buffer(DEFAULT_WIDTH * DEFAULT_HEIGHT);
        ImageTools::readGray( argv[i], DEFAULT_WIDTH * DEFAULT_HEIGHT, 
                              &buffer[0] );

        ImageTools::addLowLight(darkenFactor, noiseFactor,
                                DEFAULT_WIDTH, DEFAULT_HEIGHT, &buffer[0]);

        // Save both a .gray and a .png version of the new image.
        string filename(argv[i]);
        string grayOut("LowLightOut/");
        grayOut += filename;
        string jpegOut("LowLightOut/");
        pngOut += filename.substr(0, filename.length() - 4) + "png";

        ImageTools::saveGray(grayOut.c_str(), &buffer[0]);
        ImageTools::saveGrayPng(pngOut.c_str(), &buffer[0], 
                                DEFAULT_WIDTH, DEFAULT_HEIGHT);
    }
}