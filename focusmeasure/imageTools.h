#ifndef _ImageTools_H
#define _ImageTools_H

#include <vector>

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
	static void readGray( const char *fileName, int n, uchar *buffer );

	/*
	 *  Save the gray values from a buffer into a file.
	 */
	static void saveGray( const char *fileName, int n, uchar *buffer );

	/*
	 *  Save an image of gray values to .png
	 */
	static void saveGrayPng( const char *filename, uchar *buffer,
							 int width, int height );

	/*
	 * Replaces an image of size (w, h) with subset of it.
	 */
	static void crop( uchar*&image, int w, int h, int left, int right,
		int top, int bottom );

	/*
	 * Replaces an image of size (w, h) with a scaled version of size
	 * (newW, newH)
	 */
	static void scale( uchar *&image, int w, int h, 
		int newW, int newH, ScalingMethod method = NearestNeighbor );

	/*
	 * Returns a a scale version of size (newW, newH) of an image 
	 * of size (w, h).
	 */
	static unsigned char * scaleCopy ( uchar *image, int w, int h,
		int newW, int newH, ScalingMethod method );

	/*
	 * Change the brightness of the image by some factor in [-1, 1]
	 * Negative values mean to darken the image.
	 * Positive values mean to brighten the image.
	 */
	static void changeBrightness( float factor, int w, int h, uchar * buffer );

	/*
	 * Reduces the brightness of an image and add low-light noise.
	 */
	static void addLowLight( float darkenFactor, float noiseFactor, 
							  int w, int h, uchar * buffer );

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

	// Returns a filter representing the 1D gaussian with parameter sigma.
	static std::vector<float> get1DGaussian(int filtersize, int sigma);

	// Applies gaussian blur on an image.
	static void gaussianBlur(float sigma, int w, int h,
							 uchar * in, uchar * out);
};

#endif