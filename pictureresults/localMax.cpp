/*
 *  Print the local maxima.
 *  Assumes that the focus measure data
 *  has been normalized to [0, 1].
 */

#include <stdio.h>
#include <stdlib.h>

void printLocalMax( int n, double phi[] );

int
main( int argc, char *argv[] )
{
    double fm[256];

    int i;
    float f;
    int n = 0;
    while( fscanf( stdin, "%d %f\n", &i, &f ) != EOF ) {
	if( i != n ) {
	    fprintf( stderr, "trouble on input (%d != %d)\n", i, n );
	    exit( 0 );
        }
	fm[i] = (double)f;
	n++;
    }

    /*
     *  Print all local maximums.
     */
    printLocalMax( n, fm );

    exit( 0 );

}

void
printLocalMax( int n, double fm[] )
{
    /*
     *  Smooth the data by performing a weighted moving average.
     */
    double s[256];
    s[0] = (3.0*fm[0] +
	        fm[1]) / 4.0;
    for( int i = 1; i < n-1; i++ ) {
	s[i] = (    fm[i-1] +
	        2.0*fm[i  ] +
		    fm[i+1]) / 4.0;
    }
    s[n-1] = (    fm[n-2] +
	      3.0*fm[n-1]) / 4.0;

    /*
     *  Determine the local maxima.
     */
    int found = 0;
    for( int i = 0; i < n; i++ ) {
	int k;

	/*
	 *  Determine the height on the left.
	 */
	for( k = 0; (i-k > 0) && (s[i-k] > s[i-k-1]); k++ ) {
	    ;
	}
	int index_l = k;
	double height_l = (s[i] - s[i-k]);

	/*
	 *  Determine the height on the right.
	 */
	for( k = 0; (i+k+1 < n) && (s[i+k] > s[i+k+1]); k++ ) {
	    ;
	}
	int index_r = k;
	double height_r = (s[i] - s[i+k]);

	/*
	 *  Conditions for being a local max.
	 */
	const double delta = 0.01;
	const double epsilon = 0.005;
	bool cond_l = (i == 0) ||
		     ((i == 1) && (height_l > epsilon)) ||
		     ((index_l > 3) && (height_l > epsilon)) ||
		      (height_l > delta);
	bool cond_r = (i == n-1) ||
		     ((i == n-2) && (height_r > epsilon)) ||
		     ((index_r > 3) && (height_r > epsilon)) ||
		      (height_r > delta);
	if( cond_l && cond_r ) {
	    printf( " %d", i );
	    //printf( "Local max: s[%d] = %12.8f\n", i, s[i] );
	    //printf( "\t%d, diff = %12.8f\n", index_l, height_l );
	    //printf( "\t%d, diff = %12.8f\n", index_r, height_r );
	    found++;
	}
    }

    if( found > 0 ) {
	printf( "\n" );
    }
    else {
	/*
	 *  No local max found according to above conditions,
	 *  so output the global maximums.
	 */
	double max = 0.0;
	for( int i = 0; i < n; i++ ) {
	    if( max < s[i] ) {
		max = s[i];
	    }
	}
	for( int i = 0; i < n; i++ ) {
	    if( max == s[i] ) {
		printf( " %d", i );
	    }
	}
	printf( "\n" );
    }
}

