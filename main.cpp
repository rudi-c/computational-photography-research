
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "focusMeasure.h"

void readGray( char *fileName, int n, uchar *buffer );

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

    const int w = 1056;
    const int h =  704;
    const int n = w * h;
    uchar buffer[n];
    double measure[argc];

    FocusMeasure focus;

    double v = 0;
    //double min = INT_MAX;
    double min = HUGE_VAL;
    double max = 0;
    for( int i = 2; i < argc; i++ ) {
	readGray( argv[i], n, buffer );
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

/*
 *  Read the gray values from a file into buffer.
 */
void readGray( char *fileName, int n, uchar *buffer )
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

// temporary: compute histogram
/*
int histogram[256];
for( int i = 0; i < 256; i++ ) {
  histogram[i] = 0;
}
for( int i = 0; i < n; i++ ) {
  histogram[buffer[i]]++;
}
for( int i = 0; i < 256; i++ ) {
  printf("%d\t%d\n",i,histogram[i]);
}
*/

    fclose( fp );
}

