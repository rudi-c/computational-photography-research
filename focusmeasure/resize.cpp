#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "imageTools.h"

using namespace std;

int
main( int argc, char *argv[] )
{
    if( argc <= 2 ) {
        fprintf( stderr, "Usage: resize scale fileName0.gray ...\n" );
        fprintf( stderr, "\tscale -- scaling factor \n" );
        fprintf( stderr, "\tfileName0.gray -- file containing gray levels\n" );
        exit( 6 );
    }

    const int w = 1056;
    const int h =  704;
    const int n = w * h;

    double scale = atof(argv[1]);

    int newW = (int)(w * scale);
    int newH = (int)(h * scale);

    for( int i = 2; i < argc; i++ )
    {
        uchar * buffer = new uchar[n];
        char * inputName = argv[i];
        ImageTools::readGray( inputName, n, buffer );

        ImageTools::scale( buffer, w, h, newW, newH,
                           ImageTools::AreaAverage );

        // We're assuming that the file name from the input does indeed
        // finish with .gray.
        string outputName( inputName );
        outputName = outputName.substr(0, outputName.length() - 4);
        outputName += "png";

        ImageTools::saveGrayPng(outputName.c_str(), buffer, newW, newH);

        delete [] buffer;
    }

    return 0;
}