
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "focusMeasure.h"
#include "imageTools.h"

#define DEFAULT_WIDTH 1056
#define DEFAULT_HEIGHT 704
#define RESIZE 1

int
main( int argc, char *argv[] )
{
    if( argc <= 2 ) {
        fprintf( stderr, "Usage: apply measure fileName0.gray ...\n" );
        fprintf( stderr, "\tmeasure -- focus measure to apply to image\n" );
        fprintf( stderr, "\tfileName0.gray -- file containing gray levels\n" );
        exit( 6 );
    }

    int apply = atoi( argv[1] );

    double measure[argc];

    FocusMeasure focus;

    double v = 0;
    //double min = INT_MAX;
    double min = HUGE_VAL;
    double max = 0;
    for( int i = 2; i < argc; i++ ) {

    	uchar * buffer = new uchar[DEFAULT_WIDTH * DEFAULT_HEIGHT];
	ImageTools::readGray( argv[i], DEFAULT_WIDTH * DEFAULT_HEIGHT, buffer );

	int w = DEFAULT_WIDTH;
	int h = DEFAULT_HEIGHT;

	if (RESIZE)
	{
		ImageTools::scale( buffer, w, h, w / 2, h / 2,
			ImageTools::NearestNeighbor );
		w /= 2;
		h /= 2;
	}
	
	switch( apply ) {
	    case  0: v = focus.firstorder3x3( buffer, w, h );	break;
	    case  1: v = focus.roberts3x3( buffer, w, h );	break;
	    case  2: v = focus.prewitt3x3( buffer, w, h );	break;
	    case  3: v = focus.scharr3x3( buffer, w, h );	break;
	    case  4: v = focus.sobel3x3( buffer, w, h );	break;
	    case  5: v = focus.sobel5x5( buffer, w, h );	break;
	    case  6: v = focus.laplacian3x3( buffer, w, h );	break;
	    case  7: v = focus.laplacian5x5( buffer, w, h );	break;
	    case  8: v = focus.sobel3x3so( buffer, w, h );	break;
	    case  9: v = focus.sobel5x5so( buffer, w, h );	break;
	    case 10: v = focus.brenner( buffer, w, h );		break;
	    case 11: v = focus.thresholdGradient( buffer, w, h ); break;
	    case 12: v = focus.squaredGradient( buffer, w, h ); break;
	    case 13: v = focus.MMHistogram( buffer, w, h );	break;
	    case 14: v = focus.rangeHistogram( buffer, w, h );	break;
	    case 15: v = focus.MGHistogram( buffer, w, h );	break;
	    case 16: v = focus.entropyHistogram( buffer, w, h ); break;
	    case 17: v = focus.th_cont( buffer, w, h );		break;
	    case 18: v = focus.num_pix( buffer, w, h );		break;
	    case 19: v = focus.power( buffer, w, h );		break;
	    case 20: v = focus.var( buffer, w, h );		break;
	    case 21: v = focus.nor_var( buffer, w, h );		break;
	    case 22: v = focus.vollath4( buffer, w, h );	break;
	    case 23: v = focus.vollath5( buffer, w, h );	break;
	    case 24: v = focus.autoCorrelation( buffer, w, h, 2 ); break;
	    case 25: v = focus.sobel3x3soCross( buffer, w, h ); break;
	    case 26: v = focus.sobel5x5soCross( buffer, w, h ); break;
	    case 27: v = focus.firstDerivGaussian( buffer, w, h ); break;
	    case 28: v = focus.LoG( buffer, w, h ); break;
	    case 29: v = focus.curvature( buffer, w, h ); break;
	}

	if( max < v ) {
	    max = v;
	}
	if( min > v ) {
	    min = v;
	}
	measure[i-2] = (double)v;

    	delete [] buffer;
    }

    for( int i = 0; i < argc-2; i++ ) {
	/*
	 *  Normalized focus measure.
	 */
	printf( "%d %0.5f\n", i, (measure[i] - min)/(double)(max - min) );
	/*
	 *  Raw focus measure.
	 */
	//printf( "%d %0.0f\n", i, measure[i] );
	/*
	 *  Both raw and normalized focus measure.
	 */
	//printf( "%d %0.0f %0.5f\n", i, measure[i], (measure[i] - min)/(double)(max - min) );
    }

    return( 0 );
}
