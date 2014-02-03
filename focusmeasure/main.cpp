#include <iostream>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "focusMeasure.h"
#include "imageTools.h"

#define DEFAULT_WIDTH 1056
#define DEFAULT_HEIGHT 704

// Leaving only a fifth of the width and a third
// of the height approximate Canon's AF window.
#define CROP_FACTOR_X 5
#define CROP_FACTOR_Y 3

using namespace std;

void print_usage()
{
    cerr << "Usage: apply measure [OPTIONS] [FILES]" << endl;
    cerr << "\t measure -- focus measure to apply to image (0-33) " << endl;
    cerr << "\t Valid options include :" << endl;
    cerr << "\t --scalehalf : reduce each dimension of the image by 1/2" << endl;
    cerr << "\t --crop : keep only a small center portion of the image" << endl;
    cerr << "\t --varylight : randomly uniformly darken/brighten image at each step" << endl;
    cerr << "\t --raw : output the raw (non-normalized) the data" << endl;
    cerr << "\t --norm-and-raw : output both raw and normalized data" << endl;
    exit(1);
}

int
main( int argc, char *argv[] )
{
    if ( argc <= 3 )
        print_usage();

    int apply = atoi( argv[1] );

    int optionsCount = 0;
    bool optionScaleHalf = false;
    bool optionCrop = false;
    bool optionVaryLight = false;
    bool printRaw = false;
    bool printRawAndNorm = false;

    for (int i = 2; i < argc; i++)
    {
        string option(argv[i]);
        if (option == "--scalehalf")
            optionScaleHalf = true;
        else if (option == "--crop")
            optionCrop = true;
        else if (option == "--varylight")
            optionVaryLight = true;
        else if (option == "--raw")
            printRaw = true;
        else if (option == "--norm-and-raw")
            printRawAndNorm = true;
        else if (option[0] == '-' && option[1] == '-')
            // This option isn't recognized.
            print_usage();
        else
            // A file - we can stop reading options now.
            break;

        optionsCount++;
    }

    double measure[argc];

    FocusMeasure focus;

    double v = 0;
    //double min = INT_MAX;
    double min = HUGE_VAL;
    double max = 0;

    for( int i = 2 + optionsCount; i < argc; i++ )
    {
        uchar * buffer = new uchar[DEFAULT_WIDTH * DEFAULT_HEIGHT];
        ImageTools::readGray( argv[i], DEFAULT_WIDTH * DEFAULT_HEIGHT, buffer );

        int w = DEFAULT_WIDTH;
        int h = DEFAULT_HEIGHT;

        if (optionScaleHalf)
        {
            ImageTools::scale( buffer, w, h, w / 2, h / 2,
                               ImageTools::NearestNeighbor );
            w /= 2;
            h /= 2;
        }

        if (optionCrop)
        {
            int left = (w - w / CROP_FACTOR_X) / 2;
            int right = left + w / CROP_FACTOR_X;
            int top = (h - h / CROP_FACTOR_Y) / 2;
            int bottom = left + h / CROP_FACTOR_Y;
            ImageTools::crop( buffer, w, h, left, right, top, bottom );
            w = right - left;
            h = bottom - top;
        }

        if (optionVaryLight)
        {
            // In an experimental setup, we found a case where the average
            // pixel brightness for an outlier was 30% lower (and the median
            // was 50% lower). So we use a factor that's between -0.3 and 0.3
            float factor = (float)rand() / RAND_MAX * 0.6f - 0.3f;
            ImageTools::changeBrightness( factor, w, h, buffer );
        }
        
        switch( apply )
        {
            case  0: v = focus.firstorder3x3( buffer, w, h );   break;
            case  1: v = focus.roberts3x3( buffer, w, h );      break;
            case  2: v = focus.prewitt3x3( buffer, w, h );      break;
            case  3: v = focus.scharr3x3( buffer, w, h );       break;
            case  4: v = focus.sobel3x3( buffer, w, h );        break;
            case  5: v = focus.sobel5x5( buffer, w, h );        break;
            case  6: v = focus.laplacian3x3( buffer, w, h );    break;
            case  7: v = focus.laplacian5x5( buffer, w, h );    break;
            case  8: v = focus.sobel3x3so( buffer, w, h );      break;
            case  9: v = focus.sobel5x5so( buffer, w, h );      break;
            case 10: v = focus.brenner( buffer, w, h );         break;
            case 11: v = focus.thresholdGradient( buffer, w, h ); break;
            case 12: v = focus.squaredGradient( buffer, w, h );   break;
            case 13: v = focus.MMHistogram( buffer, w, h );       break;
            case 14: v = focus.rangeHistogram( buffer, w, h );    break;
            case 15: v = focus.MGHistogram( buffer, w, h );       break;
            case 16: v = focus.entropyHistogram( buffer, w, h );  break;
            case 17: v = focus.th_cont( buffer, w, h );           break;
            case 18: v = focus.num_pix( buffer, w, h );           break;
            case 19: v = focus.power( buffer, w, h );             break;
            case 20: v = focus.var( buffer, w, h );               break;
            case 21: v = focus.nor_var( buffer, w, h );           break;
            case 22: v = focus.vollath4( buffer, w, h );          break;
            case 23: v = focus.vollath5( buffer, w, h );          break;
            case 24: v = focus.autoCorrelation( buffer, w, h, 2 ); break;
            case 25: v = focus.sobel3x3soCross( buffer, w, h );    break;
            case 26: v = focus.sobel5x5soCross( buffer, w, h );    break;
            case 27: v = focus.firstDerivGaussian( buffer, w, h ); break;
            case 28: v = focus.LoG( buffer, w, h );                break;
            case 29: v = focus.curvature( buffer, w, h );          break;
            case 30: v = focus.firstDerivGaussian2( buffer, w, h ); break;
            case 31: v = focus.firstDerivGaussian3( buffer, w, h ); break;
            case 32: v = focus.LoG2( buffer, w, h ); break;
            case 33: v = focus.LoG3( buffer, w, h ); break;
        }

        if( max < v ) {
            max = v;
        }
        if( min > v ) {
            min = v;
        }

        int fileIndex = i - 2 - optionsCount;
        measure[fileIndex] = (double)v;

        delete [] buffer;
    }

    int fileCount = argc - 2 - optionsCount;

    if (printRaw)
        /*
         *  Raw focus measure.
         */
        for (int i = 0; i < fileCount; i++)
            printf( "%d %0.0f\n", i, measure[i] );
    else if (printRawAndNorm)
        /*
         *  Both raw and normalized focus measure.
         */
        for (int i = 0; i < fileCount; i++)
            printf( "%d %0.0f %0.5f\n", i, measure[i], 
                (measure[i] - min)/(double)(max - min) );
    else
        /*
         *  Normalized focus measure.
         */
        for (int i = 0; i < fileCount; i++)
            printf( "%d %0.5f\n", i, (measure[i] - min)/(double)(max - min) );

    return( 0 );
}
