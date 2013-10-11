/*
 *  Focus measure: map an image f of size h x w to a value
 *  that represents the degree of focus of the image.
 */
#ifndef _FOCUSMEASURE_H
#define _FOCUSMEASURE_H

typedef unsigned char uchar;

class FocusMeasure
{
    public:
	FocusMeasure();
	~FocusMeasure();
	double firstorder3x3( uchar *f, int w, int h );
	double roberts3x3( uchar *f, int w, int h );
	double prewitt3x3( uchar *f, int w, int h );
	double scharr3x3( uchar *f, int w, int h );
	double sobel3x3( uchar *f, int w, int h );
	double sobel5x5( uchar *f, int w, int h );
	double sobel3x3so( uchar *f, int w, int h );
	double sobel5x5so( uchar *f, int w, int h );
	double sobel3x3soCross( uchar *f, int w, int h );
	double sobel5x5soCross( uchar *f, int w, int h );
	double laplacian3x3( uchar *f, int w, int h );
	double laplacian5x5( uchar *f, int w, int h );
	double brenner( uchar *f, int w, int h, int threshold = 0 );
	double thresholdGradient( uchar *f, int w, int h, int threshold = 0 );
	double squaredGradient( uchar *f, int w, int h, int threshold = 0 );
	double MMHistogram( uchar *f, int w, int h );
	double rangeHistogram( uchar *f, int w, int h );
	double MGHistogram( uchar *f, int w, int h );
	double entropyHistogram( uchar *f, int w, int h );
	double th_cont( uchar *f, int w, int h, int threshold = 150 );
	double num_pix( uchar *f, int w, int h, int threshold = 150 );
	double power( uchar *f, int w, int h, int threshold = 0 );
	double var( uchar *f, int w, int h );
	double nor_var( uchar *f, int w, int h );
	double vollath4( uchar *f, int w, int h );
	double vollath5( uchar *f, int w, int h );
	double autoCorrelation( uchar *f, int w, int h, int k = 0 );
	double firstDerivGaussian( uchar *f, int w, int h );
	double LoG( uchar *f, int w, int h );
	double curvature( uchar *f, int w, int h );

    private:
	double combine( int vx, int vy );
	double determineMean( uchar *f, int w, int h );
	double determineVariance( uchar *f, int w, int h, double mean );
	void determineMinMaxIntensities( uchar *f, int w, int h, int &min, int &max );
	double computeEntropy( int histogram[], int w, int h );
	void computeHistogram( uchar *f, int w, int h, int histogram[] );
};

#endif // _FOCUSMEASURE_H
