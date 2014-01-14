#include "imageTools.h"
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace std;

void
ImageTools::readGray( char *fileName, int n, uchar *buffer )
{
	FILE *fp;

	fp = fopen( fileName, "rb" );
	if( fp == NULL )
	{
		fprintf( stderr, "No such file: %s\n", fileName );
		exit( 1 );
	}

	int result = fread( buffer, 1, n, fp );
	if( result != n ) 
	{
		fprintf( stderr, "Wrong number of bytes: %s\n", fileName );
		exit( 1 );
	}

	fclose( fp );
}

void
ImageTools::saveGray( char *filename, int n, uchar *buffer )
{
	FILE *fp;
    
	fp = fopen( fileName, "rb" );
	if( fp == NULL ) 
	{
		fprintf( stderr, "No such file: %s\n", fileName );
		exit( 1 );
	}

	int result = fwrite( buffer, sizeof(uchar), n, fp );
	if( result != n ) 
	{
		fprintf( stderr, "Wrong number of bytes: %s\n", fileName );
		exit( 1 );
    }
}

void
ImageTools::addLowLight( float darkenFactor, float noiseFactor, 
							  int w, int h, uchar * buffer )
{
	// I found experimentally (i.e., playing around in photoshop) that
	// something that looks like low-light noise can be generated by
	// first creating random noise where each of the 3 color channels (RGB)
	// is either 0 or 255, then applying a gaussian blur with sigma = 1.0

	// Darker parts of the image should have more noise (because it means
	// that there has been less light), perhaps twice as much as the brightest
	// parts.

	vector<uchar> noise(w * h);
	vector<uchar> blurredNoise(w * h);

	// Generate noise as described above.
	for (int i = 0; i < w * h; i++)
	{
		noise[i] = 0;
		if (rand() % 2 == 0) noise[i] += 255 / 3; // R
		if (rand() % 2 == 0) noise[i] += 255 / 3; // G
		if (rand() % 2 == 0) noise[i] += 255 / 3; // B
	}

	ImageTools::gaussianBlur(1.0, w, h, &noise[0], &blurredNoise[0]);

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			float pixel = buffer[x + y * w];
			pixel *= darkenFactor;

			// Want relativeNoiseFactor = noiseFactor *  1 when pixel = 0
			// 	    relativeNoiseFactor = noiseFactor * .5 when pixel = 255
			float relativeNoiseFactor = noiseFactor * (1.0f - pixel / 512.0f);

			pixel = pixel * (1.0f - relativeNoiseFactor) + 
				    noise[x + y * w] * relativeNoiseFactor;
		}
	}
}

void 
ImageTools::scale( uchar *&image, int w, int h, 
	int newW, int newH, ScalingMethod method)
{
	uchar * newImage = scaleCopy( image, w, h, newW, newH, method );
	if (newImage != image)
	{
		delete [] image;
		image = newImage;
	}
}

unsigned char *
ImageTools::scaleCopy ( uchar *image, int w, int h, 
	int newW, int newH, ScalingMethod method)
{
	if ( method == NearestNeighbor )
		return scaleNearestNeighbor( image, w, h, newW, newH );
	else if ( method == Bilinear )
		return scaleBilinear( image, w, h, newW, newH );
	else if ( method == Bicubic )
		return scaleBicubic( image, w, h, newW, newH );
	else if ( method == AreaAverage )
		return scaleAreaAverage( image, w, h, newW, newH );
	else
	{
		cerr << "Unknown scaling method, image not modified" << endl;
		return image;
	}
}

uchar *
ImageTools::scaleNearestNeighbor( uchar * image, int w, int h, 
	int newW, int newH)
{
	uchar * newImage = new uchar[newW * newH];

	double xScale = (double)w / newW;
	double yScale = (double)h / newH;

	for ( int i = 0; i < newH; i++ )
	for ( int j = 0; j < newW; j++ ) {
		int k = i * newW + j;

		double x = xScale * j;
		double y = yScale * i;

		// Need to add +0.5 in order to round to nearest
		// integer (as opposed to floor, when casting from
		// double to int).
		int nearestX = (int)(x + 0.5);
		int nearestY = (int)(y + 0.5);
		int index = nearestX + nearestY * w;
		newImage[k] = image[index];
	}

	return newImage;
}

uchar * 
ImageTools::scaleBilinear( uchar * image, int w, int h, 
	int newW, int newH)
{
	uchar * newImage = new uchar[newW * newH];

	double xScale = (double)w / newW;
	double yScale = (double)h / newH;

	for ( int i = 0; i < newH; i++ )
	for ( int j = 0; j < newW; j++ ) {
		int k = i * newW + j;

		double x = xScale * j;
		double y = yScale * i;

		int lowerX = (int)x;
		int lowerY = (int)y;
		int upperX = lowerX + 1;
		int upperY = lowerY + 1;

		double horizLower = 
			((double)upperX - x) * image[lowerX + lowerY * w] +
			(x - (double)lowerX) * image[upperX + lowerY * w];
		double horizUpper =
			((double)upperX - x) * image[lowerX + upperY * w] +
			(x - (double)lowerX) * image[upperX + upperY * w];

		double vertical = (upperY - y) * horizLower 
				+ (y - lowerY) * horizUpper;

		newImage[k] = (uchar)vertical;
	}

	return newImage;
}

uchar * 
ImageTools::scaleBicubic( uchar * image, int w, int h, 
	int newW, int newH)
{
	uchar * newImage = new uchar[newW * newH];

	double xScale = (double)w / newW;
	double yScale = (double)h / newH;

	for ( int i = 0; i < newH; i++ )
	for ( int j = 0; j < newW; j++ ) {
		int k = i * newW + j;

		double x = xScale * j;
		double y = yScale * i;

		int x1 = (int)x;
		int y1 = (int)y;
		int x0 = max(0, x1 - 1);
		int y0 = max(0, y1 - 1);
		int x2 = x1 + 1;
		int y2 = y1 + 1;
		int x3 = min(w - 1, x1 + 2);
		int y3 = min(h - 1, y1 + 2);

		double v0 = cubicInterpolate(
			image[x0 + y0 * w], image[x1 + y0 * w],
			image[x2 + y0 * w], image[x3 + y0 * w], x - x1);
		double v1 = cubicInterpolate(
			image[x0 + y1 * w], image[x1 + y1 * w],
			image[x2 + y1 * w], image[x3 + y1 * w], x - x1);
		double v2 = cubicInterpolate(
			image[x0 + y2 * w], image[x1 + y2 * w],
			image[x2 + y2 * w], image[x3 + y2 * w], x - x1);
		double v3 = cubicInterpolate(
			image[x0 + y3 * w], image[x1 + y3 * w],
			image[x2 + y3 * w], image[x3 + y3 * w], x - x1);

		newImage[k] = cubicInterpolate(v0, v1, v2, v3, y - y1);
	}

	return newImage;
}

uchar *
ImageTools::scaleAreaAverage( uchar * image, int w, int h, 
	int newW, int newH)
{
	uchar * newImage = new uchar[newW * newH];

	double xScale = (double)w / newW;
	double yScale = (double)h / newH;

	for ( int i = 0; i < newH; i++ )
	for ( int j = 0; j < newW; j++ ) {
		int k = i * newW + j;

		double xBegin = xScale * j;
		double xEnd = xScale * (j + 1);
		double yBegin = yScale * i;
		double yEnd = yScale * (i + 1);

		double sum = 0;
		double area = (xEnd - xBegin) * (yEnd - yBegin);

		// Note that here, lower referers to lower values of y,
		// not the lower part of the image as displayed on a screen.
		double lowerStripHeight = ceil(yBegin) - yBegin;
		double upperStripHeight = yEnd - ceil(yEnd);
		double leftStripWidth   = ceil(xBegin) - xBegin;
		double rightStripWidth  = xEnd - floor(xEnd); 

		// Number of full blocks in the middle.
		double nx = (int)round(floor(xEnd) - ceil(xBegin));
		double ny = (int)round(floor(yEnd) - ceil(yBegin));

		// Lower-left corner.
		sum += image[(int)xBegin + (int)yBegin * w] *
			leftStripWidth * lowerStripHeight;
		// Lower-right corner.
		sum += image[(int)xEnd + (int)yBegin * w] *
			rightStripWidth * lowerStripHeight;
		// Upper-left corner.
		sum += image[(int)xBegin + (int)yEnd * w] *
			leftStripWidth * upperStripHeight;
		// Upper-right corner.
		sum += image[(int)xEnd + (int)yEnd * w] *
			rightStripWidth * upperStripHeight;

		// Rest of bottom strip.
		for ( int x = 1; x <= nx; x++ )
			sum += image[(int)xBegin + x + (int)yBegin*w] * lowerStripHeight;
		// Rest of upper strip.
		for ( int x = 1; x <= nx; x++ )
			sum += image[(int)xBegin + x + (int)yEnd*w] * upperStripHeight;
		// Rest of left strip.
		for ( int y = 1; y <= ny; y++ )
			sum += image[(int)xBegin + (int)(y + yBegin)*w] * leftStripWidth;
		// Rest of right strip.
		for ( int y = 1; y <= ny; y++ )
			sum += image[(int)xEnd + (int)(y + yBegin)*w] * rightStripWidth;

		// Middle section
		for ( int y = 1; y <= ny; y++ )
			for (int x = 1; x <= nx; x++ )
				sum += image[(int)xBegin + x + (int)(yBegin + y) * w];

		newImage[k] = (uchar)(sum / area);
	}

	return newImage;
}

double
ImageTools::cubicInterpolate( double p0, double p1, 
	double p2, double p3, double x )
{
	// See paulinternet.nl/?page=bicubic

	// Want to find a, b, c, d for f(x) = ax^3 + bx^2 + cx + d
	// 
	// Solve the equations below.
	// f(0)  = d             = p1
	// f(1)  = a + b + c + d = p2
	// f'(0) = c             = (p2-p0)/2
	// f'(1) = 3a + 2b + c   = (p3-p1)/2
	double a = -0.5 * p0 + 1.5 * p1 - 1.5 * p2 + 0.5 * p3;
	double b = p0 - 2.5 * p1 + 2 * p2 - 0.5 * p3;
	double c = -0.5 * p0 + 0.5 * p2;
	double d = p1;

	return d + x * (c + x * (b + x * a));
}

vector<float>
ImageTools::get1DGaussian(int filtersize, int sigma)
{
    float coefficient = (1.0f /
        sqrt(2.0f * M_PI) / sigma);
    float exponentFactor = 2.0f * sigma * sigma;
    vector<float> gaussian(filtersize);

    float sum = 0.0;

    for (int i = 0; i < filtersize; i++)
    {
        float v = i - filtersize / 2;
        gaussian[i] = coefficient * exp(-(v * v) / exponentFactor);
        sum += gaussian[i];
    }

    // Since our filter size is not infinite, it does not integrate to 1
    // so we need to normalize it.
    for (int i = 0; i < filtersize; i++)
    	gaussian[i] /= sum;

    return gaussian;
}

void
ImageTools::gaussianBlur(float sigma, int w, int h,
						 uchar * in, uchar * out)
{
	int filtersize = sigma * 3.0;

	// The filter size should be odd.
	if (filtersize % 2 == 0) filtersize++;

	vector<float> filter = ImageTools::get1DGaussian(filtersize, sigma);
	vector<float> temp(w * h);

	int pad = filtersize / 2;

	// Horizontal blur.
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			float sum = 0.0;
			for (int i = 0; i < filtersize; i++)
			{
				int x2 = min(max(0, x + i - pad), w - 1);
				sum += in[x2 + y * w] * filter[i];
			}
			temp[x + y * w] = sum;
		}
	}

	// Vertical blur.
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			float sum = 0.0;
			for (int i = 0; i < filtersize; i++)
			{

				int y2 = min(max(0, y + i - pad), h - 1);
				sum += temp[x + y2 * w] * filter[i];
			}
			out[x + y * w] = sum;
		}
	}
}