/*
 *  Focus measure: map an image f of size h x w to a value
 *  that represents the degree of focus of the image.
 *
 *  Pixel (i,j) in the image is given by f[i*w + j],
 *  where
 *      i = 0, ..., h-1 (rows)
 *      j = 0, ..., w-1 (columns)
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "focusMeasure.h"

FocusMeasure::FocusMeasure()
{
}

FocusMeasure::~FocusMeasure()
{
};

/*
 *  For the case where filters come in pairs, the resulting two
 *  values can be combined in several ways: max of the two values,
 *  add the absolute value of the two values, find the magnitude
 *  (sum the squares of the values, take square root). Thresholding
 *  can also be performed.
 */
double
FocusMeasure::combine( int vx, int vy )
{
    /*
     *  (a) Sum of absolute values.
     */
    //return( abs(vx) + abs(vy) );

    /*
     *  (b) Max of absolute values.
     */
    //vx = abs( vx );
    //vy = abs( vy );
    //if( vx > vy ) return( vx );
    //else	  return( vy );

    /*
     *  (c) Sum of squared values.
     */
    return( vx*vx + vy*vy );

    /*
     *  (d) Sqrt of sum of squared values (magnitude of gradient).
     */
    //return( sqrt( (double) vx*vx + vy*vy ) );

    /*
     *  (e) Max of squared values.
     */
    //vx = vx*vx;
    //vy = vy*vy;
    //if( vx > vy ) return( vx );
    //else	  return( vy );

    /*
     *  (f) vx squared value.
     */
    //return( vx*vx );

    /*
     *  (g) vy squared value.
     */
    //return( vy*vy );

    /*
     *  (h) vx absolute value.
     */
    //return( abs( vx ) );

    /*
     *  (i) vy absolute value.
     */
    //return( abs( vy ) );

}

/*
 *  Use a first-order 3x3 difference operator to determine a focus measure.
 */
double
FocusMeasure::firstorder3x3( uchar *f, int w, int h )
{
    double sum = 0;
    /*
     *  Ignore the boundaries of the image since applying the
     *  operator at, for example, pixel (0,0) is undefined.
     *  I.e., i = 1, ..., h-2 (rows)
     *        j = 1, ..., w-2 (columns)
     */
    for( int i = 1; i < h-1; i++ )
    for( int j = 1; j < w-1; j++ ) {
	int k = i*w + j;
	/*
	 *				       0  0  0
	 *  Horizontal 3x3 difference kernel: -1  0 +1
	 *				       0  0  0
	 */
	int vx =  0*f[k-w-1] + 0*f[k-w] + 0*f[k-w+1] +
		 -1*f[k  -1] + 0*f[k  ] + 1*f[k  +1] +
		  0*f[k+w-1] + 0*f[k+w] + 0*f[k+w+1];

	/*
	 *			            0 -1  0
	 *  Vertical 3x3 difference kernel: 0  0  0
	 *			            0  1  0
	 */
	int vy = 0*f[k-w-1] + -1*f[k-w] + 0*f[k-w+1] +
		 0*f[k  -1] +  0*f[k  ] + 0*f[k  +1] +
		 0*f[k+w-1] +  1*f[k+w] + 0*f[k+w+1];

	sum += combine( vx, vy );
	if( sum < 0 ) {
	    fprintf( stdout, "error: overflow in firstorder3x3\n" );
	    fprintf( stderr, "error: overflow in firstorder3x3\n" );
	}
    }

    return( sum );
}

/*
 *  Use a first derivative 3x3 Roberts operator to determine a focus measure.
 */
double
FocusMeasure::roberts3x3( uchar *f, int w, int h )
{
    double sum = 0;
    /*
     *  Ignore the boundaries of the image since applying the
     *  operator at, for example, pixel (0,0) is undefined.
     *  I.e., i = 1, ..., h-2 (rows)
     *        j = 1, ..., w-2 (columns)
     */
    for( int i = 1; i < h-1; i++ )
    for( int j = 1; j < w-1; j++ ) {
	int k = i*w + j;
	/*
	 *				    0  0  0
	 *  Horizontal 3x3 Roberts kernel:  0  1  0
	 *				   -1  0  0
	 */
	int vx =  0*f[k-w-1] +  0*f[k-w] +  0*f[k-w+1] +
		  0*f[k  -1] +  1*f[k  ] +  0*f[k  +1] +
		 -1*f[k+w-1] +  0*f[k+w] +  0*f[k+w+1];

	/*
	 *			         0  0  0
	 *  Vertical 3x3 Roberts kernel: 0  1  0
	 *			         0  0 -1
	 */
	int vy = 0*f[k-w-1] +  0*f[k-w] +  0*f[k-w+1] +
		 0*f[k  -1] +  1*f[k  ] +  0*f[k  +1] +
		 0*f[k+w-1] +  0*f[k+w] + -1*f[k+w+1];

	sum += combine( vx, vy );
	if( sum < 0 ) {
	    fprintf( stdout, "error: overflow in roberts3x3\n" );
	    fprintf( stderr, "error: overflow in roberts3x3\n" );
	}
    }

    return( sum );
}

/*
 *  Use a first derivative 3x3 Prewitt operator to determine a focus measure.
 */
double
FocusMeasure::prewitt3x3( uchar *f, int w, int h )
{
    double sum = 0;
    /*
     *  Ignore the boundaries of the image since applying the
     *  operator at, for example, pixel (0,0) is undefined.
     *  I.e., i = 1, ..., h-2 (rows)
     *        j = 1, ..., w-2 (columns)
     */
    for( int i = 1; i < h-1; i++ )
    for( int j = 1; j < w-1; j++ ) {
	int k = i*w + j;
	/*
	 *			           -1  0 +1
	 *  Horizontal 3x3 Prewitt kernel: -1  0 +1
	 *			           -1  0 +1
	 */
	int vx = -1*f[k-w-1] +  0*f[k-w] +  1*f[k-w+1] +
		 -1*f[k  -1] +  0*f[k  ] +  1*f[k  +1] +
		 -1*f[k+w-1] +  0*f[k+w] +  1*f[k+w+1];

	/*
	 *				 -1 -1 -1
	 *  Vertical 3x3 Prewitt kernel:  0  0  0
	 *				 +1 +1 +1
	 */
	int vy = -1*f[k-w-1] + -1*f[k-w] + -1*f[k-w+1] +
		  0*f[k  -1] +  0*f[k  ] +  0*f[k  +1] +
		  1*f[k+w-1] +  1*f[k+w] +  1*f[k+w+1];

	sum += combine( vx, vy );
	if( sum < 0 ) {
	    fprintf( stdout, "error: overflow in prewitt3x3\n" );
	    fprintf( stderr, "error: overflow in prewitt3x3\n" );
	}
    }

    return( sum );
}

/*
 *  Use a first derivative 3x3 Scharr operator to determine a focus measure.
 */
double
FocusMeasure::scharr3x3( uchar *f, int w, int h )
{
    double sum = 0;
    /*
     *  Ignore the boundaries of the image since applying the
     *  operator at, for example, pixel (0,0) is undefined.
     *  I.e., i = 1, ..., h-2 (rows)
     *        j = 1, ..., w-2 (columns)
     */
    for( int i = 1; i < h-1; i++ )
    for( int j = 1; j < w-1; j++ ) {
	int k = i*w + j;
	/*
	 *			           -3  0  +3
	 *  Horizontal 3x3 Scharr kernel: -10  0 +10
	 *			           -3  0  +3
	 */
	int vx =  -3*f[k-w-1] +  0*f[k-w] +   3*f[k-w+1] +
		 -10*f[k  -1] +  0*f[k  ] +  10*f[k  +1] +
		  -3*f[k+w-1] +  0*f[k+w] +   3*f[k+w+1];

	/*
	 *				-3 -10  -3
	 *  Vertical 3x3 Scharr kernel:  0   0   0
	 *				+3 +10  +3
	 */
	int vy = -3*f[k-w-1] + -10*f[k-w] + -3*f[k-w+1] +
		  0*f[k  -1] +   0*f[k  ] +  0*f[k  +1] +
		  3*f[k+w-1] +  10*f[k+w] +  3*f[k+w+1];

	sum += combine( vx, vy );
	if( sum < 0 ) {
	    fprintf( stdout, "error: overflow in scharr3x3\n" );
	    fprintf( stderr, "error: overflow in scharr3x3\n" );
	}
    }

    return( sum );
}


/*
 *  Use a first derivative 3x3 Sobel operator to determine a focus measure.
 */
double
FocusMeasure::sobel3x3( uchar *f, int w, int h )
{
    double sum = 0;
    /*
     *  Ignore the boundaries of the image since applying the
     *  operator at, for example, pixel (0,0) is undefined.
     *  I.e., i = 1, ..., h-2 (rows)
     *        j = 1, ..., w-2 (columns)
     */
    for( int i = 1; i < h-1; i++ )
    for( int j = 1; j < w-1; j++ ) {
	int k = i*w + j;
	/*
	 *			         -1 0 +1
	 *  Horizontal 3x3 Sobel kernel: -2 0 +2
	 *			         -1 0 +1
	 */
	int vx = -1*f[k-w-1] +  0*f[k-w] +  1*f[k-w+1] +
		 -2*f[k  -1] +  0*f[k  ] +  2*f[k  +1] +
		 -1*f[k+w-1] +  0*f[k+w] +  1*f[k+w+1];

	/*
	 *			       -1 -2 -1
	 *  Vertical 3x3 Sobel kernel:  0  0  0
	 *			       +1 +2 +1
	 */
	int vy = -1*f[k-w-1] + -2*f[k-w] + -1*f[k-w+1] +
		  0*f[k  -1] +  0*f[k  ] +  0*f[k  +1] +
		  1*f[k+w-1] +  2*f[k+w] +  1*f[k+w+1];

	sum += combine( vx, vy );
	if( sum < 0 ) {
	    fprintf( stdout, "error: overflow in sobel3x3\n" );
	    fprintf( stderr, "error: overflow in sobel3x3\n" );
	}
    }

    return( sum );
}

/*
 *  Use a first derivative 5x5 Sobel operator to determine a focus measure.
 */
double
FocusMeasure::sobel5x5( uchar *f, int w, int h )
{
    double sum = 0;
    /*
     *  Ignore the boundaries of the image since applying the
     *  operator at, for example, pixel (1,1) is undefined.
     *  I.e., i = 2, ..., h-3 (rows)
     *        j = 2, ..., w-3 (columns)
     */
    for( int i = 2; i < h-2; i++ )
    for( int j = 2; j < w-2; j++ ) {
        int k = i*w + j;
        /*
         *                               -1  -2   0   2   1
         *                               -4  -8   0   8   4
         *  Horizontal 5x5 Sobel kernel: -6 -12   0  12   6
         *                               -4  -8   0   8   4
         *                               -1  -2   0   2   1
         */
        int vx =
	    -1*f[k-2*w-2] +  -2*f[k-2*w-1] + 0*f[k-2*w] +  2*f[k-2*w+1] + 1*f[k-2*w+2] +
	    -4*f[k-  w-2] +  -8*f[k-  w-1] + 0*f[k-  w] +  8*f[k-  w+1] + 4*f[k-  w+2] +
	    -6*f[k    -2] + -12*f[k    -1] + 0*f[k    ] + 12*f[k    +1] + 6*f[k    +2] +
	    -4*f[k+  w-2] +  -8*f[k+  w-1] + 0*f[k+  w] +  8*f[k+  w+1] + 4*f[k+  w+2] +
	    -1*f[k+2*w-2] +  -2*f[k+2*w-1] + 0*f[k+2*w] +  2*f[k+2*w+1] + 1*f[k+2*w+2];

        /*
         *                             -1  -4  -6  -4  -1
         *                             -2  -8 -12  -8  -2
         *  Vertical 5x5 Sobel kernel:  0   0   0   0   0
         *                              2   8  12   8   2
         *                              1   4   6   4   1
         */
        int vy =
	    -1*f[k-2*w-2] + -4*f[k-2*w-1] +  -6*f[k-2*w] + -4*f[k-2*w+1] + -1*f[k-2*w+2] +
	    -2*f[k-  w-2] + -8*f[k-  w-1] + -12*f[k-  w] + -8*f[k-  w+1] + -2*f[k-  w+2] +
	     0*f[k    -2] +  0*f[k    -1] +   0*f[k    ] +  0*f[k    +1] +  0*f[k    +2] +
	     2*f[k+  w-2] +  8*f[k+  w-1] +  12*f[k+  w] +  8*f[k+  w+1] +  2*f[k+  w+2] +
	     1*f[k+2*w-2] +  4*f[k+2*w-1] +   6*f[k+2*w] +  4*f[k+2*w+1] +  1*f[k+2*w+2];

        sum += combine( vx, vy );
	if( sum < 0 ) {
	    fprintf( stdout, "error: overflow in sobel5x5\n" );
	    fprintf( stderr, "error: overflow in sobel5x5\n" );
	}
    }

    return( sum );
}

/*
 *  Use a second derivative 3x3 Laplacian operator to determine a focus measure.
 */
double
FocusMeasure::laplacian3x3( uchar *f, int w, int h )
{
    double sum = 0;
    /*
     *  Ignore the boundaries of the image since applying the
     *  operator at, for example, pixel (0,0) is undefined.
     *  I.e., i = 1, ..., h-2 (rows)
     *        j = 1, ..., w-2 (columns)
     */
    for( int i = 1; i < h-1; i++ )
    for( int j = 1; j < w-1; j++ ) {
	int k = i*w + j;
	/*
	 *			  -1  -1  -1
	 *  3x3 Laplacian kernel: -1   8  -1
	 *			  -1  -1  -1
	 */
	int v = -1*f[k-w-1] + -1*f[k-w] + -1*f[k-w+1] +
		-1*f[k  -1] +  8*f[k  ] + -1*f[k  +1] +
		-1*f[k+w-1] + -1*f[k+w] + -1*f[k+w+1];

	//sum += abs( v );
	sum += v*v;
    }

    return( sum );
}

/*
 *  Use a second derivative 5x5 Laplacian operator to determine a focus measure.
 */
double
FocusMeasure::laplacian5x5( uchar *f, int w, int h )
{
    double sum = 0;
    /*
     *  Ignore the boundaries of the image since applying the
     *  operator at, for example, pixel (1,1) is undefined.
     *  I.e., i = 2, ..., h-3 (rows)
     *        j = 2, ..., w-3 (columns)
     */
    for( int i = 2; i < h-2; i++ )
    for( int j = 2; j < w-2; j++ ) {
        int k = i*w + j;
        /*
         *                        -1 -3 -4 -3 -1
         *                        -3  0  6  0 -3
         *  5x5 Laplacian kernel: -4  6 20  6 -4
         *                        -3  0  6  0 -3
         *                        -1 -3 -4 -3 -1
         */
        int v =
	    -1*f[k-2*w-2] + -3*f[k-2*w-1] + -4*f[k-2*w] + -3*f[k-2*w+1] + -1*f[k-2*w+2] +
	    -3*f[k-  w-2] +  0*f[k-  w-1] +  6*f[k-  w] +  0*f[k-  w+1] + -3*f[k-  w+2] +
	    -4*f[k    -2] +  6*f[k    -1] + 20*f[k    ] +  6*f[k    +1] + -4*f[k    +2] +
	    -3*f[k+  w-2] +  0*f[k+  w-1] +  6*f[k+  w] +  0*f[k+  w+1] + -3*f[k+  w+2] +
	    -1*f[k+2*w-2] + -3*f[k+2*w-1] + -4*f[k+2*w] + -3*f[k+2*w+1] + -1*f[k+2*w+2];

        //sum += abs( v );
	sum += v*v;
    }

    return( sum );
}

/*
 *  Use a second derivative 3x3 Sobel operator to determine a focus measure.
 */
double
FocusMeasure::sobel3x3so( uchar *f, int w, int h )
{
    double sum = 0;
    /*
     *  Ignore the boundaries of the image since applying the
     *  operator at, for example, pixel (0,0) is undefined.
     *  I.e., i = 1, ..., h-2 (rows)
     *        j = 1, ..., w-2 (columns)
     */
    for( int i = 1; i < h-1; i++ )
    for( int j = 1; j < w-1; j++ ) {
	int k = i*w + j;
	/*
	 *                                                1   2   1
	 *  Horizontal second derivative Sobel operator: -2  -4  -2
	 *                                                1   2   1
	 */
/*
	int vx =  1*f[k-w-1] +  2*f[k-w] +  1*f[k-w+1] +
		 -2*f[k  -1] + -4*f[k  ] + -2*f[k  +1] +
		  1*f[k+w-1] +  2*f[k+w] +  1*f[k+w+1];
*/

	/*
	 *                                             1  -2   1
	 *  Vertical second derivative Sobel operator: 2  -4   2
	 *                                             1  -2   1
	 */
	int vy =  1*f[k-w-1] + -2*f[k-w] +  1*f[k-w+1] +
		  2*f[k  -1] + -4*f[k  ] +  2*f[k  +1] +
		  1*f[k+w-1] + -2*f[k+w] +  1*f[k+w+1];

        //sum += combine( vx, vy );
	sum += vy*vy;
    }

    return( sum );
}

/*
 *  Use a second derivative 5x5 Sobel operator to determine a focus measure.
 */
double
FocusMeasure::sobel5x5so( uchar *f, int w, int h )
{
    double sum = 0;
    /*
     *  Ignore the boundaries of the image since applying the
     *  operator at, for example, pixel (1,1) is undefined.
     *  I.e., i = 2, ..., h-3 (rows)
     *        j = 2, ..., w-3 (columns)
     */
    for( int i = 2; i < h-2; i++ )
    for( int j = 2; j < w-2; j++ ) {
        int k = i*w + j;
        /*
         *                                                  1   4   6   4   1
         *                                                  0   0   0   0   0
         *  Horizontal 5x5 second derivative Sobel kernel: -2  -8 -12  -8  -2
         *                                                  0   0   0   0   0
         *                                                  1   4   6   4   1
         */
/*
        int vx =
	     1*f[k-2*w-2] +  4*f[k-2*w-1] +   6*f[k-2*w] +  4*f[k-2*w+1] +  1*f[k-2*w+2] +
	     0*f[k-  w-2] +  0*f[k-  w-1] +   0*f[k-  w] +  0*f[k-  w+1] +  0*f[k-  w+2] +
	    -2*f[k    -2] + -8*f[k    -1] + -12*f[k    ] + -8*f[k    +1] + -2*f[k    +2] +
	     0*f[k+  w-2] +  0*f[k+  w-1] +   0*f[k+  w] +  0*f[k+  w+1] +  0*f[k+  w+2] +
	     1*f[k+2*w-2] +  4*f[k+2*w-1] +   6*f[k+2*w] +  4*f[k+2*w+1] +  1*f[k+2*w+2];
*/

        /*
         *                                                1   0   -2   0   1
         *                                                4   0   -8   0   4
         *  Vertical 5x5 second derivative Sobel kernel:  6   0  -12   0   6
         *                                                4   0   -8   0   4
         *                                                1   0   -2   0   1
         */
        int vy =
	     1*f[k-2*w-2] +  0*f[k-2*w-1] +  -2*f[k-2*w] +  0*f[k-2*w+1] + 1*f[k-2*w+2] +
	     4*f[k-  w-2] +  0*f[k-  w-1] +  -8*f[k-  w] +  0*f[k-  w+1] + 4*f[k-  w+2] +
	     6*f[k    -2] +  0*f[k    -1] + -12*f[k    ] +  0*f[k    +1] + 6*f[k    +2] +
	     4*f[k+  w-2] +  0*f[k+  w-1] +  -8*f[k+  w] +  0*f[k+  w+1] + 4*f[k+  w+2] +
	     1*f[k+2*w-2] +  0*f[k+2*w-1] +  -2*f[k+2*w] +  0*f[k+2*w+1] + 1*f[k+2*w+2];

        //sum += combine( vx, vy );
	sum += vy*vy;
    }

    return( sum );
}

double
FocusMeasure::sobel3x3soCross( uchar *f, int w, int h )
{
    double sum = 0;
    for( int i = 1; i < h-1; i++ )
    for( int j = 1; j < w-1; j++ ) {
	int k = i*w + j;
	/*
	 *                                              -1   0   1
	 *  3x3 second derivative cross Sobel operator:  0   0   0
	 *                                               1   0  -1
	 */
	int v =  -1*f[k-w-1] +  0*f[k-w] +  1*f[k-w+1] +
		  0*f[k  -1] +  0*f[k  ] +  0*f[k  +1] +
		  1*f[k+w-1] +  0*f[k+w] + -1*f[k+w+1];

	//sum += abs( v );
	sum += v*v;
    }

    return( sum );
}

double
FocusMeasure::sobel5x5soCross( uchar *f, int w, int h )
{
    double sum = 0;
    for( int i = 2; i < h-2; i++ )
    for( int j = 2; j < w-2; j++ ) {
        int k = i*w + j;
        /*
         *                                            -1  -2   0   2   1
         *                                            -2  -4   0   4   2
         *  5x5 second derivative cross Sobel kernel:  0   0   0   0   0
         *                                             2   4   0  -4  -2
         *                                             1   2   0  -2  -1
         */
        int v =
	    -1*f[k-2*w-2] + -2*f[k-2*w-1] +   0*f[k-2*w] +  2*f[k-2*w+1] +  1*f[k-2*w+2] +
	    -2*f[k-  w-2] + -4*f[k-  w-1] +   0*f[k-  w] +  4*f[k-  w+1] +  2*f[k-  w+2] +
	     0*f[k    -2] +  0*f[k    -1] +   0*f[k    ] +  0*f[k    +1] +  0*f[k    +2] +
	     2*f[k+  w-2] +  4*f[k+  w-1] +   0*f[k+  w] + -4*f[k+  w+1] + -2*f[k+  w+2] +
	     1*f[k+2*w-2] +  2*f[k+2*w-1] +   0*f[k+2*w] + -2*f[k+2*w+1] + -1*f[k+2*w+2];

        //sum += abs( v );
	sum += v*v;
    }

    return( sum );
}

double
FocusMeasure::brenner( uchar *f, int w, int h, int threshold )
{
    double sum = 0;

    for( int i = 0; i < h; ++i ) {
        for( int j = 0; j < w - 2; ++j ) {

            int firstDiff = abs( f[i*w + j+2] - f[i*w + j] );

            if( firstDiff >= threshold ) {
                sum += pow( firstDiff, 2 );
            }
        }
    }

    return( sum );
}

double
FocusMeasure::thresholdGradient( uchar *f, int w, int h, int threshold )
{
    double sum = 0;

    for( int i = 0; i < h; ++i ) {
        for( int j = 0; j < w - 1; ++j ) {

            int firstDiff = abs( f[i*w + j+1] - f[i*w + j] );

            if( firstDiff >= threshold ) {
                sum += abs( firstDiff );
            }
        }
    }

    return( sum );
}

double
FocusMeasure::squaredGradient( uchar *f, int w, int h, int threshold )
{
    double sum = 0;

    for( int i = 0; i < h; ++i ) {
        for( int j = 0; j < w - 1; ++j ) {

            int firstDiff = abs( f[i*w + j+1] - f[i*w + j] );

            if( firstDiff >= threshold )  {
                sum += pow( firstDiff, 2 );
            }
        }
    }

    return( sum );
}

void
FocusMeasure::determineMinMaxIntensities( uchar *f, int w, int h, int &min, int &max )
{
    min = f[0];
    max = f[0];

    for( int i = 0; i < h; i++ )
    for( int j = 0; j < w; j++ ) {
	int k = i*w + j;
	if( f[k] < min ) min = f[k];
	if( f[k] > max ) max = f[k];
    }
}

/*
 *  Convert a grayscale image (represented as uchar *f) into
 *  a histogram (represented as an array of integers).
 */
void
FocusMeasure::computeHistogram( uchar *f, int w, int h, int histogram[] )
{
    for( int i = 0; i < h; i++ )
    for( int j = 0; j < w; j++ ) {
	int intensity = f[i*w + j];
	histogram[intensity]++;
    }
}

int
coordToIndex( int row, int col, int w )
{
    return( row*w + col );
}

double
computeDelta( uchar *f, int i, int j, int w, int h )
{
    double delta =
	2 * pow(f[coordToIndex(i, j-1, w)] - f[coordToIndex(i, j+1, w)], 2) +
	2 * pow(f[coordToIndex(i-1, j, w)] - f[coordToIndex(i+1, j, w)], 2) +
	    pow(f[coordToIndex(i-1, j-1, w)] - f[coordToIndex(i+1, j+1, w)], 2) +
	    pow(f[coordToIndex(i-1, j+1, w)] - f[coordToIndex(i+1, j-1, w)], 2);

    return( delta );
}

// computes the Mason-Green threshhold measure for a given uchar array
int
determineMGThreshhold( uchar *f, int w, int h )
{
    double numerator = 0;
    double denominator = 0;
    for( int i = 1; i < h-1; i++ )
    for( int j = 1; j < w-1; j++ ) {
	int intensity = f[i*w + j];
	double delta = computeDelta( f, i, j, w, h );
	numerator += delta * intensity;
	denominator += delta;
    }

    int threshold = (int)(numerator / denominator);

    return( threshold );
}

double
FocusMeasure::MMHistogram( uchar *f, int w, int h )
{
    int histogram[256];
    for( int i = 0; i < 256; i++ ) {
	histogram[i] = 0;
    }
    computeHistogram( f, w, h, histogram );

    int mean = 128; // could be set better (actual mean from first image)
    double sum = 0;
    for( int i = mean; i < 256; i++)  {
        sum += i * histogram[i];
    }

    return( sum );
}

double
FocusMeasure::rangeHistogram( uchar *f, int w, int h )
{
    int min, max;
    determineMinMaxIntensities( f, w, h, min, max );

    return( max - min );
}

double
FocusMeasure::MGHistogram( uchar *f, int w, int h )
{
    int histogram[256];
    for( int i = 0; i < 256; i++ ) {
	histogram[i] = 0;
    }
    computeHistogram( f, w, h, histogram );

    int threshhold = determineMGThreshhold( f, w, h );
    double sum = 0;
    for( int i = threshhold + 1; i < 256; i++ ) {
        sum += histogram[i] * (i - threshhold);
    }

    return( sum );
}

double
FocusMeasure::computeEntropy( int histogram[], int w, int h )
{
    double total = w * h;
    double sum = 0;
    double log2 = log( 2.0 );

    for( int i = 0; i < 256; i++ ) {
        double p = (double)histogram[i] / total;
	if( p > 0 ) {
	    sum += p * (log(p) / log2);
	}
    }
    sum *= -1;

    return( sum );
}

double
FocusMeasure::entropyHistogram( uchar *f, int w, int h )
{
    int histogram[256];
    for( int i = 0; i < 256; i++ ) {
	histogram[i] = 0;
    }
    computeHistogram( f, w, h, histogram );

    return( computeEntropy( histogram, w, h) );
}

double
FocusMeasure::th_cont( uchar *f, int w, int h, int threshold )
{
    int result = 0;

    for ( int i = 0; i < h; ++i )
    for ( int j = 0; j < w; ++j ) {
        if( f[i*w + j] >= threshold ) {
            result += f[i*w + j];
        }
    }

    return( result );
}

double
FocusMeasure::num_pix( uchar *f, int w, int h, int threshold )
{
    int result = 0;

    for( int i = 0; i < h; i++ )
    for( int j = 0; j < w; j++ ) {
	if( f[i*w + j] < threshold ) {
	    result++;
	}
    }

    return( result );
}

double
FocusMeasure::power( uchar *f, int w, int h, int threshold )
{
    double result = 0;

    for( int i = 0; i < h; i++ )
    for( int j = 0; j < w; j++ ) {
	if( f[i*w + j] >= threshold ) {
	    result += pow( f[i*w + j], 2 );
	}
    }

    return( result );
}

double
FocusMeasure::determineMean( uchar *f, int w, int h )
{
    double aggregate = 0;
    for( int i = 0; i < h; i++ )
    for( int j = 0; j < w; j++ ) {
	aggregate += f[i*w + j];
    }

    double mean = aggregate / (w * h);

    return( mean );
}

double
FocusMeasure::var( uchar *f, int w, int h )
{
    double result = 0;
    double mean = determineMean( f, w, h );

    for( int i = 0; i < h; ++i )
    for( int j = 0; j < w; ++j ) {
	result += pow( f[i*w + j] - mean, 2 );
    }

    return( result / ( w * h ) );
}

double
FocusMeasure::nor_var( uchar *f, int w, int h )
{
    double result = 0;
    double mean = determineMean( f, w, h );

    for( int i = 0; i < h; ++i )
    for( int j = 0; j < w; ++j ) {
	result += pow( f[i*w + j] - mean, 2 );
    }

    return( result / ( w * h * mean ) );
}

double
FocusMeasure::vollath4( uchar *f, int w, int h )
{
    double sum1 = 0;
    for( int i = 0; i < h-1; i++ )
    for( int j = 0; j < w;   j++ ) {
	sum1 += f[coordToIndex(i, j, w)] * f[coordToIndex(i+1, j, w)];
    }

    double sum2 = 0;
    for( int i = 0; i < h-2; i++ )
    for( int j = 0; j < w;   j++ ) {
	sum2 += f[coordToIndex(i, j, w)] * f[coordToIndex(i+2, j, w)];
    }

    return( sum1 - sum2 );
}

double
FocusMeasure::vollath5( uchar *f, int w, int h )
{
    double sum = 0;
    for( int i = 0; i < h-1; i++ )
    for( int j = 0; j < w;   j++ ) {
	sum += f[coordToIndex(i, j, w)] * f[coordToIndex(i+1, j, w)];
    }

    double mean = determineMean( f, w, h );
    sum -= h * w * pow( mean, 2 );

    return( sum );
}

double
FocusMeasure::determineVariance( uchar *f, int w, int h, double mean )
{
    double variance = 0;
    for( int i = 0; i < h; i++ )
    for( int j = 0; j < w; j++ ) {
	variance += pow( f[coordToIndex(i, j, w)] - mean, 2 );
    }

    return( variance / (w * h) );
}

double
FocusMeasure::autoCorrelation( uchar *f, int w, int h, int k )
{
    double mean = determineMean( f, w, h );
    double variance = determineVariance( f, w, h, mean );

    int n = w * h;
    double innerSum = 0;
    for( int i = 0; i < n - k; i++ ) {
        innerSum += (f[i] - mean) * (f[i+k] - mean);
    }

    double sum = 1.0 - (1.0 / ((n-k)*variance)) * innerSum;

    return( sum );
}

/*
 *  First derivative of Gaussian.
 *  Generated by gaussian.c
 */
double
FocusMeasure::firstDerivGaussian( uchar *f, int w, int h )
{
    double sum = 0;
    // remember to adjust range of for loops
    for( int i = 3; i < h-3; i++ )
    for( int j = 3; j < w-3; j++ ) {
        int k = i*w + j;

// sigma =  0.8
double vx =
  -0.00006142*f[k-3*w-3] +
  -0.00203564*f[k-3*w-2] +
  -0.01060592*f[k-3*w-1] +
   0.00000000*f[k-3*w+0] +
   0.01060592*f[k-3*w+1] +
   0.00203564*f[k-3*w+2] +
   0.00006142*f[k-3*w+3] +

  -0.00305346*f[k-2*w-3] +
  -0.10119599*f[k-2*w-2] +
  -0.52724320*f[k-2*w-1] +
   0.00000000*f[k-2*w+0] +
   0.52724320*f[k-2*w+1] +
   0.10119599*f[k-2*w+2] +
   0.00305346*f[k-2*w+3] +

  -0.03181775*f[k-1*w-3] +
  -1.05448641*f[k-1*w-2] +
  -5.49400034*f[k-1*w-1] +
   0.00000000*f[k-1*w+0] +
   5.49400034*f[k-1*w+1] +
   1.05448641*f[k-1*w+2] +
   0.03181775*f[k-1*w+3] +

  -0.06949635*f[k+0*w-3] +
  -2.30321007*f[k+0*w-2] +
 -12.00000000*f[k+0*w-1] +
   0.00000000*f[k+0*w+0] +
  12.00000000*f[k+0*w+1] +
   2.30321007*f[k+0*w+2] +
   0.06949635*f[k+0*w+3] +

  -0.03181775*f[k+1*w-3] +
  -1.05448641*f[k+1*w-2] +
  -5.49400034*f[k+1*w-1] +
   0.00000000*f[k+1*w+0] +
   5.49400034*f[k+1*w+1] +
   1.05448641*f[k+1*w+2] +
   0.03181775*f[k+1*w+3] +

  -0.00305346*f[k+2*w-3] +
  -0.10119599*f[k+2*w-2] +
  -0.52724320*f[k+2*w-1] +
   0.00000000*f[k+2*w+0] +
   0.52724320*f[k+2*w+1] +
   0.10119599*f[k+2*w+2] +
   0.00305346*f[k+2*w+3] +

  -0.00006142*f[k+3*w-3] +
  -0.00203564*f[k+3*w-2] +
  -0.01060592*f[k+3*w-1] +
   0.00000000*f[k+3*w+0] +
   0.01060592*f[k+3*w+1] +
   0.00203564*f[k+3*w+2] +
   0.00006142*f[k+3*w+3];

double vy =
  -0.00006142*f[k-3*w-3] +
  -0.00305346*f[k-3*w-2] +
  -0.03181775*f[k-3*w-1] +
  -0.06949635*f[k-3*w+0] +
  -0.03181775*f[k-3*w+1] +
  -0.00305346*f[k-3*w+2] +
  -0.00006142*f[k-3*w+3] +

  -0.00203564*f[k-2*w-3] +
  -0.10119599*f[k-2*w-2] +
  -1.05448641*f[k-2*w-1] +
  -2.30321007*f[k-2*w+0] +
  -1.05448641*f[k-2*w+1] +
  -0.10119599*f[k-2*w+2] +
  -0.00203564*f[k-2*w+3] +

  -0.01060592*f[k-1*w-3] +
  -0.52724320*f[k-1*w-2] +
  -5.49400034*f[k-1*w-1] +
 -12.00000000*f[k-1*w+0] +
  -5.49400034*f[k-1*w+1] +
  -0.52724320*f[k-1*w+2] +
  -0.01060592*f[k-1*w+3] +

   0.00000000*f[k+0*w-3] +
   0.00000000*f[k+0*w-2] +
   0.00000000*f[k+0*w-1] +
   0.00000000*f[k+0*w+0] +
   0.00000000*f[k+0*w+1] +
   0.00000000*f[k+0*w+2] +
   0.00000000*f[k+0*w+3] +

   0.01060592*f[k+1*w-3] +
   0.52724320*f[k+1*w-2] +
   5.49400034*f[k+1*w-1] +
  12.00000000*f[k+1*w+0] +
   5.49400034*f[k+1*w+1] +
   0.52724320*f[k+1*w+2] +
   0.01060592*f[k+1*w+3] +

   0.00203564*f[k+2*w-3] +
   0.10119599*f[k+2*w-2] +
   1.05448641*f[k+2*w-1] +
   2.30321007*f[k+2*w+0] +
   1.05448641*f[k+2*w+1] +
   0.10119599*f[k+2*w+2] +
   0.00203564*f[k+2*w+3] +

   0.00006142*f[k+3*w-3] +
   0.00305346*f[k+3*w-2] +
   0.03181775*f[k+3*w-1] +
   0.06949635*f[k+3*w+0] +
   0.03181775*f[k+3*w+1] +
   0.00305346*f[k+3*w+2] +
   0.00006142*f[k+3*w+3];
/*
*/

	//sum += fabs(vx) + fabs(vy);

	//vx = fabs( vx );
	//vy = fabs( vy );
	//if( vx > vy ) sum += vx;
	//else          sum += vy;

	sum += vx*vx + vy*vy;

	//sum += sqrt( vx*vx + vy*vy );

	//vx = vx*vx;
	//vy = vy*vy;
	//if( vx > vy ) sum += vx;
	//else	        sum += vy;

	//sum += vx*vx;

	//sum += vy*vy;

    }

    return( sum );
}

/*
 *  Laplacian of Gaussian.
 *  Generated by gaussian.c
 */
double
FocusMeasure::LoG( uchar *f, int w, int h )
{
    double sum = 0;
    // remember to adjust range of for loops
    for( int i = 4; i < h-4; i++ )
    for( int j = 4; j < w-4; j++ ) {
        int k = i*w + j;

// sigma =  1.2
double v =
  -0.00604456*f[k-4*w-4] +
  -0.05218375*f[k-4*w-3] +
  -0.22921201*f[k-4*w-2] +
  -0.53574532*f[k-4*w-1] +
  -0.70445656*f[k-4*w+0] +
  -0.53574532*f[k-4*w+1] +
  -0.22921201*f[k-4*w+2] +
  -0.05218375*f[k-4*w+3] +
  -0.00604456*f[k-4*w+4] +

  -0.05218375*f[k-3*w-4] +
  -0.40539537*f[k-3*w-3] +
  -1.53989454*f[k-3*w-2] +
  -3.07029812*f[k-3*w-1] +
  -3.73463936*f[k-3*w+0] +
  -3.07029812*f[k-3*w+1] +
  -1.53989454*f[k-3*w+2] +
  -0.40539537*f[k-3*w+3] +
  -0.05218375*f[k-3*w+4] +

  -0.22921201*f[k-2*w-4] +
  -1.53989454*f[k-2*w-3] +
  -4.42144171*f[k-2*w-2] +
  -5.18823798*f[k-2*w-1] +
  -3.87881214*f[k-2*w+0] +
  -5.18823798*f[k-2*w+1] +
  -4.42144171*f[k-2*w+2] +
  -1.53989454*f[k-2*w+3] +
  -0.22921201*f[k-2*w+4] +

  -0.53574532*f[k-1*w-4] +
  -3.07029812*f[k-1*w-3] +
  -5.18823798*f[k-1*w-2] +
   6.10318853*f[k-1*w-1] +
  18.45137170*f[k-1*w+0] +
   6.10318853*f[k-1*w+1] +
  -5.18823798*f[k-1*w+2] +
  -3.07029812*f[k-1*w+3] +
  -0.53574532*f[k-1*w+4] +

  -0.70445656*f[k+0*w-4] +
  -3.73463936*f[k+0*w-3] +
  -3.87881214*f[k+0*w-2] +
  18.45137170*f[k+0*w-1] +
  40.00000000*f[k+0*w+0] +
  18.45137170*f[k+0*w+1] +
  -3.87881214*f[k+0*w+2] +
  -3.73463936*f[k+0*w+3] +
  -0.70445656*f[k+0*w+4] +

  -0.53574532*f[k+1*w-4] +
  -3.07029812*f[k+1*w-3] +
  -5.18823798*f[k+1*w-2] +
   6.10318853*f[k+1*w-1] +
  18.45137170*f[k+1*w+0] +
   6.10318853*f[k+1*w+1] +
  -5.18823798*f[k+1*w+2] +
  -3.07029812*f[k+1*w+3] +
  -0.53574532*f[k+1*w+4] +

  -0.22921201*f[k+2*w-4] +
  -1.53989454*f[k+2*w-3] +
  -4.42144171*f[k+2*w-2] +
  -5.18823798*f[k+2*w-1] +
  -3.87881214*f[k+2*w+0] +
  -5.18823798*f[k+2*w+1] +
  -4.42144171*f[k+2*w+2] +
  -1.53989454*f[k+2*w+3] +
  -0.22921201*f[k+2*w+4] +

  -0.05218375*f[k+3*w-4] +
  -0.40539537*f[k+3*w-3] +
  -1.53989454*f[k+3*w-2] +
  -3.07029812*f[k+3*w-1] +
  -3.73463936*f[k+3*w+0] +
  -3.07029812*f[k+3*w+1] +
  -1.53989454*f[k+3*w+2] +
  -0.40539537*f[k+3*w+3] +
  -0.05218375*f[k+3*w+4] +

  -0.00604456*f[k+4*w-4] +
  -0.05218375*f[k+4*w-3] +
  -0.22921201*f[k+4*w-2] +
  -0.53574532*f[k+4*w-1] +
  -0.70445656*f[k+4*w+0] +
  -0.53574532*f[k+4*w+1] +
  -0.22921201*f[k+4*w+2] +
  -0.05218375*f[k+4*w+3] +
  -0.00604456*f[k+4*w+4];
/*
*/

	//sum += fabs( v );
	sum += v*v;
    }

    return( sum );
}

double curvature( uchar *f, int w, int h );

/*
 *  Sum of curvature values at each point:
 *      f"(x) / (1 + f'(x)^2)^1.5
 */
double
FocusMeasure::curvature( uchar *f, int w, int h )
{
    double sum = 0;

    for( int i = 2; i < h-2; i++ )
    for( int j = 2; j < w-2; j++ ) {
	int k = i*w + j;

	/*
	 *  Central (fourth order) finite difference scheme
	 *  for first derivative.
	 */
	double fx = (-f[k+2] + 8.0*f[k+1] - 8.0*f[k-1] + f[k-2]) / 12.0;

	/*
	 *  Central (second order) finite difference scheme
	 *  for fourth derivative.
	 */
	double fxx = (-f[k+2] + 16.0*f[k+1] - 30.0*f[k] + 16.0*f[k-1] - f[k-2]) / 12.0;

	sum += fxx / pow( 1.0 + fx * fx, 1.5 );
    }

    return( sum );
}

