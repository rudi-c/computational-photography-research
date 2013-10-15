#ifndef _ImageTools_H
#define _ImageTools_H

typedef unsigned char uchar;

class ImageTools
{
public:
	enum ScalingMethod
	{
		NearestNeighbor,
		Bilinear,
		Bicubic,
		AreaAverage
	};

	/*
	 *  Read the gray values from a file into buffer.
	 */
	static void readGray( char *fileName, int n, uchar *buffer );

	/*
	 * Replaces an image of size (w, h) with a scaled version of size
	 * (newW, newH)
	 */
	static void  scale( uchar *&image, int w, int h, 
		int newW, int newH, ScalingMethod method = NearestNeighbor);
	/*
	 * Returns a a scale version of size (newW, newH) of an image 
	 * of size (w, h).
	 */
	static unsigned char * scaleCopy ( uchar *image, int w, int h,
		int newW, int newH, ScalingMethod method);

private:

	static uchar * scaleNearestNeighbor( uchar * image, int w, int h,
		int newW, int newH);
	static uchar * scaleBilinear( uchar * image, int w, int h,
		int newW, int newH);
	static uchar * scaleBicubic( uchar * image, int w, int h,
		int newW, int newH);
	static uchar * scaleAreaAverage( uchar * image, int w, int h, 
		int newW, int newH);

	/*
	 * Given (-1, p0), (0, p1), (1, p2), (2, p3), use cubic spline
	 * interpolation to find the value y for point (x, y) on the curve.
	 * It is assumed that 0 < x < 1. In other words, we are interpolating
	 * between two points with values p1 and p2.
	 */
	static double cubicInterpolate( double p1, double p2, 
		double p3, double p4, double x );
};

#endif