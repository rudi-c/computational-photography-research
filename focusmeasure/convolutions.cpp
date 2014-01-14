/*
 * This file contains a program to measure the time taken to compute
 * convolution of an image with the 2D gaussian or one of its derivatives.
 * That is, to measure how long it takes to apply a gaussian blur or
 * and compute the gradient of an image.
 *
 * The program is executed via ./convolve.exe [filtersize] [sigma] [filename]
 * larger values of filtersize will take longer to run. Filtersize should be
 * approximately 3x the value of sigma.
 *
 * The methods that are used include :
 * 1) O(n^2) Naive computation with floating-point
 * 2) O(n^2) Naive computation with integers
 * 3) O(n) Separable filter with floating-point
 * 4) O(n) Separable filter with intergers
 * 5) O(1) Recursive gaussian filter
 *
 * 2) and 4) don't give correct results (it's off by a constant factor) but
 * that shouldn't affect timing.
 * 5) is based on "Improving Deriche-Style Recursive Gaussian Filters" and
 * may contain errors.
 */

#include <algorithm>
#include <assert.h>
#include <cmath>
#include <ctime>
#include <iostream>
#include <stdlib.h>
#include <vector>

#include "imageTools.h"

#define DEFAULT_WIDTH 1056
#define DEFAULT_HEIGHT 704

#define REPETITIONS 20
#define INT_MULTIPLIER 1 << 5

using namespace std;

struct RecursiveCoefficientsSet
{
    enum Type
    {
        Gaussian,
        FirstDerivGaussian
    };

    const float sigma;

    const static float A1[3];
    const static float A2[3];
    const static float B1[3];
    const static float B2[3];
    const static float W1 =  0.6681;
    const static float W2 =  2.0787;
    const static float L1 = -1.3932;
    const static float L2 = -1.3732;

    float N0, N1, N2, N3;
    float M1, M2, M3, M4;
    float SN, DN, EN;
    float SM;
    float D1, D2, D3, D4;
    float SD, DD, ED;
    float BN1, BN2, BN3, BN4;
    float BM1, BM2, BM3, BM4;

    // Computes the coefficients of the recursive gaussian filter as
    // described in "Improving Deriche-Style Recursive Gaussian Filters"
    void Calculate(Type type)
    {
        float Sin1 = sin(W1 / sigma);
        float Sin2 = sin(W2 / sigma);
        float Cos1 = cos(W1 / sigma);
        float Cos2 = cos(W2 / sigma);
        float Exp1 = exp(L1 / sigma);
        float Exp2 = exp(L2 / sigma);

        int i = (type == Gaussian ? 0 : 1);
        bool symmetric = (type == Gaussian);
        
        N0  = A1[i] + A2[i];
        N1  = Exp2 * (B2[i]*Sin2 - (A2[i]+2*A1[i]) * Cos2);
        N1 += Exp1 * (B1[i]*Sin1 - (A1[i]+2*A2[i]) * Cos1);
        N2  = (A1[i]+A2[i]) * Cos2*Cos1;
        N2 -= B1[i]*Cos2*Sin1 + B2[i]*Cos1*Sin2;
        N2 *= 2*Exp1*Exp2;
        N2 += A2[i]*Exp1*Exp1 + A1[i]*Exp2*Exp2;
        N3  = Exp2*Exp1*Exp1 * (B2[i]*Sin2 - A2[i]*Cos2);
        N3 += Exp1*Exp2*Exp2 * (B1[i]*Sin1 - A1[i]*Cos1);

        SN = N0 + N1 + N2 + N3;
        DN = N1 + 2*N2 + 3*N3;
        EN = N1 + 4*N2 + 9*N3;

        D4  = Exp1*Exp1*Exp2*Exp2;
        D3  = -2*Cos1*Exp1*Exp2*Exp2;
        D3 += -2*Cos2*Exp2*Exp1*Exp1;
        D2  =  4*Cos2*Cos1*Exp1*Exp2;
        D2 +=  Exp1*Exp1 + Exp2*Exp2;
        D1  =  -2*(Exp2*Cos2 + Exp1*Cos1);

        SD = 1.0 + D1 + D2 + D3 + D4;
        DD = D1 + 2*D2 + 3*D3 + 4*D4;
        ED = D1 + 4*D2 + 9*D3 + 16*D4;

        float alpha = 1.0;
        if (type == Gaussian)
            alpha = 2 * SN / SD - N0;
        else
            alpha = 2 * (SN*DD - DN*SD) / (SD*SD);
        N0 /= alpha;
        N1 /= alpha;
        N2 /= alpha;
        N3 /= alpha;

        if ( symmetric )
        {
            M1 = N1 - D1 * N0;
            M2 = N2 - D2 * N0;
            M3 = N3 - D3 * N0;
            M4 =    - D4 * N0;
        }
        else
        {
            M1 = -( N1 - D1 * N0 );
            M2 = -( N2 - D2 * N0 );
            M3 = -( N3 - D3 * N0 );
            M4 =         D4 * N0;
        }

        SM = M1 + M2 + M3 + M4;

        BN1 = D1 * SN / SD;
        BN2 = D2 * SN / SD;
        BN3 = D3 * SN / SD;
        BN4 = D4 * SN / SD;

        BM1 = D1 * SM / SD;
        BM2 = D2 * SM / SD;
        BM3 = D3 * SM / SD;
        BM4 = D4 * SM / SD;
    }

    RecursiveCoefficientsSet(float s) : sigma(s)
    {

    }
};

const float RecursiveCoefficientsSet::A1[3] = {  1.3530, -0.6724, -1.3563 };
const float RecursiveCoefficientsSet::A2[3] = { -0.3531,  0.6724,  0.3446 };
const float RecursiveCoefficientsSet::B1[3] = {  1.8151, -3.4327,  5.2318 };
const float RecursiveCoefficientsSet::B2[3] = {  0.0902,  0.6100, -2.2355 };

// Creates a greyscale image where every pixel is a random number
// between 0 and 255.
void
getRandomImage(int w, int h, uchar * buffer)
{
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            buffer[x + y * w] = rand() % 256;
}

// Returns a square filter representing the x-derivative
// of the 2D gaussian with parameter sigma.
vector<float>
get2DGradientX(int filtersize, float sigma)
{
    float coefficient = -(1.0f / (2.0f * M_PI * sigma * sigma * sigma * sigma));
    float exponentFactor = 2.0f * sigma * sigma;

    vector<float> gradient(filtersize * filtersize);

    for (int y = 0; y < filtersize; y++)
    for (int x = 0; x < filtersize; x++)
    {
        float w = y - filtersize / 2;
        float v = x - filtersize / 2;

        gradient[x + y * filtersize] = coefficient * v *
            exp(-(v*v + w*w) / exponentFactor);
    }

    return gradient;
}

// Returns a square filter representing the y-derivative
// of the 2D gaussian with parameter sigma.
vector<float>
get2DGradientY(int filtersize, float sigma)
{
    float coefficient = -(1.0f / (2.0f * M_PI * sigma * sigma * sigma * sigma));
    float exponentFactor = 2.0f * sigma * sigma;

    vector<float> gradient(filtersize * filtersize);

    for (int y = 0; y < filtersize; y++)
    for (int x = 0; x < filtersize; x++)
    {
        float w = y - filtersize / 2;
        float v = x - filtersize / 2;

        gradient[x + y * filtersize] = coefficient * w *
            exp(-(v*v + w*w) / exponentFactor);
    }

    return gradient;
}

// Returns a filter representing the derivative 
// of the 1D gaussian with parameter sigma.
vector<float>
get1DGradient(int filtersize, int sigma)
{
    float coefficient = -(1.0f /
        sqrt(2.0f * M_PI) / sigma / sigma / sigma);
    float exponentFactor = 2.0f * sigma * sigma;
    vector<float> gradient(filtersize);

    for (int i = 0; i < filtersize; i++)
    {
        float v = i - filtersize / 2;
        gradient[i] = coefficient * v * exp(-(v * v) / exponentFactor);
    }

    return gradient;
}

// Returns a filter representing the 1D gaussian with parameter sigma.
vector<float>
get1DGaussian(int filtersize, int sigma)
{
    float coefficient = (1.0f /
        sqrt(2.0f * M_PI) / sigma);
    float exponentFactor = 2.0f * sigma * sigma;
    vector<float> gaussian(filtersize);

    for (int i = 0; i < filtersize; i++)
    {
        float v = i - filtersize / 2;
        gaussian[i] = coefficient * exp(-(v * v) / exponentFactor);
    }

    return gaussian;
}

// Converts every float in the input vector to an integer after
// multiplying them by some constant.
vector<int>
toIntVector(vector<float>& floats, int multiplier)
{
    vector<int> ints(floats.size());
    for (uint i = 0; i < floats.size(); i++)
        ints[i] = (int)(floats[i] * multiplier);
    return ints;
}

// Converts every int in the input vector to an floating point after
// dividing them by some constant.
vector<float>
toFloatVector(vector<int>& ints, int multiplier)
{
    vector<float> floats(ints.size());
    for (uint i = 0; i < ints.size(); i++)
        floats[i] = (float)ints[i] / multiplier;
    return floats;
}

// Calculate the gradient via convolution with 2D filters, representing
// the x and y partial derivatives of a 2D gaussian.
// This is a naive algorithm and takes O(n^2) where n filter size.
template<typename T> void
computeGradient(vector<uchar>& input, vector<T>& output,
                vector<T>& gradientx, vector<T>& gradienty,
                int filtersize)
{
    int pad = filtersize / 2;

    for (int iy = pad; iy < DEFAULT_HEIGHT - pad; iy++)
    {
        for (int ix = pad; ix < DEFAULT_WIDTH - pad; ix++)
        {
            T dx = 0;
            T dy = 0;

            for (int fy = -pad; fy <= pad; fy++)
            {
                for (int fx = -pad; fx <= pad; fx++)
                {
                    int filterIndex = (fx + pad) + (fy + pad) * filtersize;
                    dx += input[ix + fx + (iy + fy) * DEFAULT_WIDTH]
                        * gradientx[filterIndex];
                    dy += input[ix + fx + (iy + fy) * DEFAULT_WIDTH]
                        * gradienty[filterIndex];
                }
            }

            output[ix + iy * DEFAULT_WIDTH] = dx * dx + dy * dy;
        }
    }
}

// Calculate the gradient via convolution with 1D filters, a gaussian
// blur in one direction and a partial derivative in the other.
// This uses the property that d/dx i(x, y) (*) g(x, y) = 
//                                  i(x, y) (*) d/dx g(x) (*) g(y)
//                        and  d/dy i(x, y) (*) g(x, y) = 
//                                  i(x, y) (*) g(x) (*) d/dy g(y)
// where (*) is the convolution operator.
// This is a better algorithm and takes O(n) where n filter size.
template<typename T> void
computeGradientSeparated(vector<uchar>& input, vector<T>& output,
                         vector<T>& gradient1d,
                         vector<T>& gaussian1d,
                         int filtersize)
{
    // Static keyword within a function means that the variable is created 
    // only at the first function call. For temporary calculations.
    static vector<T> buffer(DEFAULT_WIDTH * DEFAULT_HEIGHT);

    int pad = filtersize / 2;

    // Caculate x-derivative first.
    for (int y = pad; y < DEFAULT_HEIGHT - pad; y++)
    {
        for (int x = 0; x < DEFAULT_WIDTH; x++)
        {
            T val = 0;
            for (int i = 0; i < filtersize; i++)
                val += input[x + (y + i - pad) * DEFAULT_WIDTH] * 
                       gaussian1d[i];
            buffer[x + y * DEFAULT_WIDTH] = val;
        }
    }
    for (int y = pad; y < DEFAULT_HEIGHT - pad; y++)
    {
        for (int x = pad; x < DEFAULT_WIDTH - pad; x++)
        {
            T val = 0;
            for (int i = 0; i < filtersize; i++)
                val += buffer[(x + i - pad) + y * DEFAULT_WIDTH] * 
                       gradient1d[i];
            output[x + y * DEFAULT_WIDTH] = val * val;
        }
    }

    // Caculate y-derivative second.
    for (int y = 0; y < DEFAULT_HEIGHT; y++)
    {
        for (int x = pad; x < DEFAULT_WIDTH - pad; x++)
        {
            T val = 0;
            for (int i = 0; i < filtersize; i++)
                val += input[(x + i - pad) + y * DEFAULT_WIDTH] * 
                       gaussian1d[i];
            buffer[x + y * DEFAULT_WIDTH] = val;
        }
    }
    for (int y = pad; y < DEFAULT_HEIGHT - pad; y++)
    {
        for (int x = pad; x < DEFAULT_WIDTH - pad; x++)
        {
            T val = 0;
            for (int i = 0; i < filtersize; i++)
                val += buffer[x + (y + i - pad) * DEFAULT_WIDTH] * 
                       gradient1d[i];
            output[x + y * DEFAULT_WIDTH] += val * val;
        }
    }
}

// Applies a recursive filter on a column x of the input.
template<typename T> void
computeRecursiveVertical(vector<T>& in, vector<float>& out,
                         RecursiveCoefficientsSet& c, int x)
{
    static vector<float> buf(DEFAULT_HEIGHT);

    // Save some typing.
    int h = DEFAULT_HEIGHT;

    // Initialize borders.
    float i0 = x + 0 * DEFAULT_WIDTH;
    float i1 = x + 1 * DEFAULT_WIDTH;
    float i2 = x + 2 * DEFAULT_WIDTH;
    float i3 = x + 3 * DEFAULT_WIDTH; 
    buf[0]  = c.N0  * in[i0] + c.N1  * in[i0] + c.N2  * in[i0] + c.N3  * in[i0]
            - c.BN1 * in[i0] + c.BN2 * in[i0] + c.BN3 * in[i0] + c.BN4 * in[i0];
    buf[1]  = c.N0 * in[i1] + c.N1  * in[i0] + c.N2  * in[i0] + c.N3  * in[i0]
            - c.D1 * buf[0] + c.BN2 * in[i0] + c.BN3 * in[i0] + c.BN4 * in[i0];
    buf[2]  = c.N0 * in[i2] + c.N1 * in[i1] + c.N2  * in[i0] + c.N3  * in[i0]
            - c.D1 * buf[1] + c.D2 * buf[0] + c.BN3 * in[i0] + c.BN4 * in[i0];
    buf[3]  = c.N0 * in[i3] + c.N1 * in[i2] + c.N2 * in[i1] + c.N3  * in[i0]
            - c.D1 * buf[2] + c.D2 * buf[1] + c.D3 * buf[0] + c.BN4 * in[i0];
    //buf[0] = buf[1] = buf[2] = buf[3] = 0;

    // Forward pass
    for (int y = 4; y < DEFAULT_HEIGHT; y++)
    {
        buf[y] =   c.N0 * in[x + y * DEFAULT_WIDTH]
                 + c.N1 * in[x + (y - 1) * DEFAULT_WIDTH]
                 + c.N2 * in[x + (y - 2) * DEFAULT_WIDTH]
                 + c.N3 * in[x + (y - 3) * DEFAULT_WIDTH]
                 - c.D1 * buf[y - 1]
                 - c.D2 * buf[y - 2]
                 - c.D3 * buf[y - 3]
                 - c.D4 * buf[y - 4];
    }

    for (int y = 0; y < DEFAULT_HEIGHT; y++)
        out[x + y * DEFAULT_WIDTH] = buf[y];

    // Initialize borders on the other side.
    float i_n1 = x + (DEFAULT_HEIGHT - 1) * DEFAULT_WIDTH;
    float i_n2 = x + (DEFAULT_HEIGHT - 2) * DEFAULT_WIDTH;
    float i_n3 = x + (DEFAULT_HEIGHT - 3) * DEFAULT_WIDTH;
    //float i_n4 = x + (DEFAULT_HEIGHT - 4) * DEFAULT_WIDTH; 
    buf[h - 1]  = c.M1  * in[i_n1] + c.M2  * in[i_n1] + c.M3  * in[i_n1] + c.M4  * in[i_n1]
                - c.BM1 * in[i_n1] + c.BM2 * in[i_n1] + c.BM3 * in[i_n1] + c.BM4 * in[i_n1];
    buf[h - 2]  = c.M1 * in[i_n1]   + c.M2  * in[i_n1] + c.M3  * in[i_n1] + c.M4  * in[i_n1]
                - c.D1 * buf[h - 1] + c.BM2 * in[i_n1] + c.BM3 * in[i_n1] + c.BM4 * in[i_n1];
    buf[h - 3]  = c.M1 * in[i_n2]   + c.M2 * in[i_n1]   + c.M3  * in[i_n1] + c.M4  * in[i_n1]
                - c.D1 * buf[h - 2] + c.D2 * buf[h - 1] + c.BM3 * in[i_n1] + c.BM4 * in[i_n1];
    buf[h - 4]  = c.M1 * in[i_n3]   + c.M2 * in[i_n2]   + c.M3 * in[i_n1]   + c.M4  * in[i_n1]
                - c.D1 * buf[h - 3] + c.D2 * buf[h - 2] + c.D3 * buf[h - 1] + c.BM4 * in[i_n1];
    //buf[h - 1] = buf[h - 2] = buf[h - 3] = buf[h - 4] = 0;

    // Backward pass.
    for (int y = h - 5; y >= 0; y--)
    {
        buf[y] =   c.M1 * in[x + (y + 1) * DEFAULT_WIDTH]
                 + c.M2 * in[x + (y + 2) * DEFAULT_WIDTH]
                 + c.M3 * in[x + (y + 3) * DEFAULT_WIDTH]
                 + c.M4 * in[x + (y + 4) * DEFAULT_WIDTH]
                 - c.D1 * buf[y + 1]
                 - c.D2 * buf[y + 2]
                 - c.D3 * buf[y + 3]
                 - c.D4 * buf[y + 4];
    }

    for (int y = 0; y < DEFAULT_HEIGHT; y++)
        out[x + y * DEFAULT_WIDTH] += buf[y];
}

// Applies a recursive filter on row y of the output.
template<typename T> void
computeRecursiveHorizontal(vector<T>& in, vector<float>& out,
                           RecursiveCoefficientsSet& c, int y)
{
    static vector<float> buf(DEFAULT_WIDTH);

    // Save some typing.
    int w = DEFAULT_WIDTH;

    // Initialize borders.
    float i0 = 0 + y * DEFAULT_WIDTH;
    float i1 = 1 + y * DEFAULT_WIDTH;
    float i2 = 2 + y * DEFAULT_WIDTH;
    float i3 = 3 + y * DEFAULT_WIDTH; 
    buf[0]  = c.N0  * in[i0] + c.N1  * in[i0] + c.N2  * in[i0] + c.N3  * in[i0]
            - c.BN1 * in[i0] + c.BN2 * in[i0] + c.BN3 * in[i0] + c.BN4 * in[i0];
    buf[1]  = c.N0 * in[i1] + c.N1  * in[i0] + c.N2  * in[i0] + c.N3  * in[i0]
            - c.D1 * buf[0] + c.BN2 * in[i0] + c.BN3 * in[i0] + c.BN4 * in[i0];
    buf[2]  = c.N0 * in[i2] + c.N1 * in[i1] + c.N2  * in[i0] + c.N3  * in[i0]
            - c.D1 * buf[1] + c.D2 * buf[0] + c.BN3 * in[i0] + c.BN4 * in[i0];
    buf[3]  = c.N0 * in[i3] + c.N1 * in[i2] + c.N2 * in[i1] + c.N3  * in[i0]
            - c.D1 * buf[2] + c.D2 * buf[1] + c.D3 * buf[0] + c.BN4 * in[i0];
    //buf[0] = buf[1] = buf[2] = buf[3] = 0;

    // Forward pass
    for (int x = 4; x < DEFAULT_WIDTH; x++)
    {
        buf[x] =   c.N0 * in[x + y * DEFAULT_WIDTH]
                 + c.N1 * in[x - 1 + y * DEFAULT_WIDTH]
                 + c.N2 * in[x - 2 + y * DEFAULT_WIDTH]
                 + c.N3 * in[x - 3 + y * DEFAULT_WIDTH]
                 - c.D1 * buf[x - 1]
                 - c.D2 * buf[x - 2]
                 - c.D3 * buf[x - 3]
                 - c.D4 * buf[x - 4];
    }

    for (int x = 0; x < DEFAULT_WIDTH; x++)
        out[x + y * DEFAULT_WIDTH] = buf[x];

    // Initialize borders on the other side.
    float i_n1 = DEFAULT_WIDTH - 1 + y * DEFAULT_WIDTH;
    float i_n2 = DEFAULT_WIDTH - 2 + y * DEFAULT_WIDTH;
    float i_n3 = DEFAULT_WIDTH - 3 + y * DEFAULT_WIDTH;
    //float i_n4 = DEFAULT_WIDTH - 4 + y * DEFAULT_WIDTH; 
    buf[w - 1]  = c.M1  * in[i_n1] + c.M2  * in[i_n1] + c.M3  * in[i_n1] + c.M4  * in[i_n1]
                - c.BM1 * in[i_n1] + c.BM2 * in[i_n1] + c.BM3 * in[i_n1] + c.BM4 * in[i_n1];
    buf[w - 2]  = c.M1 * in[i_n1]   + c.M2  * in[i_n1] + c.M3  * in[i_n1] + c.M4  * in[i_n1]
                - c.D1 * buf[w - 1] + c.BM2 * in[i_n1] + c.BM3 * in[i_n1] + c.BM4 * in[i_n1];
    buf[w - 3]  = c.M1 * in[i_n2]   + c.M2 * in[i_n1]   + c.M3  * in[i_n1] + c.M4  * in[i_n1] 
                - c.D1 * buf[w - 2] + c.D2 * buf[w - 1] + c.BM3 * in[i_n1] + c.BM4 * in[i_n1];
    buf[w - 4]  = c.M1 * in[i_n3]   + c.M2 * in[i_n2]   + c.M3 * in[i_n1]   + c.M4  * in[i_n1]
                - c.D1 * buf[w - 3] + c.D2 * buf[w - 2] + c.D3 * buf[w - 1] + c.BM4 * in[i_n1];
    //buf[w - 1] = buf[w - 2] = buf[w - 3] = buf[w - 4] = 0;

    // Backward pass.
    for (int x = w - 5; x >= 0; x--)
    {
        buf[x] =   c.M1 * in[x + 1 + y * DEFAULT_WIDTH]
                 + c.M2 * in[x + 2 + y * DEFAULT_WIDTH]
                 + c.M3 * in[x + 3 + y * DEFAULT_WIDTH]
                 + c.M4 * in[x + 4 + y * DEFAULT_WIDTH]
                 - c.D1 * buf[x + 1]
                 - c.D2 * buf[x + 2]
                 - c.D3 * buf[x + 3]
                 - c.D4 * buf[x + 4];
    }

    for (int x = 0; x < DEFAULT_HEIGHT; x++)
        out[x + y * DEFAULT_WIDTH] += buf[x];
}

// Calculates the first derivative of the gaussian in O(1) using
// the recursive gaussian filter.
void
computeGradientRecursive(vector<uchar>& in, vector<float>& out,
                         RecursiveCoefficientsSet& gaussSet,
                         RecursiveCoefficientsSet& derivSet,
                         int filtersize)
{
    // Static keyword within a function means that the variable is created 
    // only at the first function call. For temporary calculations.
    static vector<float> buffer1(DEFAULT_WIDTH * DEFAULT_HEIGHT);
    static vector<float> buffer2(DEFAULT_WIDTH * DEFAULT_HEIGHT);

    // Save some typing
    int pad = filtersize / 2;

    // Caculate x-derivative first.
    // Vertical gaussian blur.

    for (int x = 0; x < DEFAULT_WIDTH; x++)
        computeRecursiveVertical(in, buffer1, gaussSet, x);
    for (int y = 0; y < DEFAULT_HEIGHT; y++)
        computeRecursiveHorizontal(buffer1, buffer2, derivSet, y);

    for (int y = pad; y < DEFAULT_HEIGHT - pad; y++)
        for (int x = pad; x < DEFAULT_WIDTH - pad; x++)
            out[x + y * DEFAULT_WIDTH] =   buffer2[x + y * DEFAULT_WIDTH]
                                         * buffer2[x + y * DEFAULT_WIDTH];

    // Then calculate the y-derivative.
    // Vertical gaussian blur.
    for (int y = 0; y < DEFAULT_HEIGHT; y++)
        computeRecursiveHorizontal(in, buffer1, gaussSet, y);
    for (int x = 0; x < DEFAULT_WIDTH; x++)
        computeRecursiveVertical(buffer1, buffer2, derivSet, x);

    for (int y = pad; y < DEFAULT_HEIGHT - pad; y++)
        for (int x = pad; x < DEFAULT_WIDTH - pad; x++)
            out[x + y * DEFAULT_WIDTH] +=   buffer2[x + y * DEFAULT_WIDTH]
                                          * buffer2[x + y * DEFAULT_WIDTH];
}

// Computes the error between two sets of numbers pairwise, and prints
// a five-number statistics of the errors.
// i.e., (min, first quartile, median, third quartile, max)
void
analyzeError(vector<float>& base, vector<float>& diff, int filtersize)
{
    assert(base.size() == diff.size());

    int pad = filtersize / 2;

    // Small values close to zero can fluctuate too much in orders
    // of magnitude due to numerical errors, so we treat those separately.
    const float smallNumThreshold = 0.00001;
    int smallNumberPairsCount = 0;
    int smallNumberNonPairCount = 0;

    vector<float> errors;

    for (int y = pad; y < DEFAULT_HEIGHT - pad; y++)
    {
        for (int x = pad; x < DEFAULT_WIDTH - pad; x++)
        {
            int i = x + y * DEFAULT_WIDTH;

            if (base[i] < smallNumThreshold && diff[i] < smallNumThreshold)
                smallNumberPairsCount++;
            else if (base[i] < smallNumThreshold || 
                     diff[i] < smallNumThreshold)
                smallNumberNonPairCount++;
            else
            {
                errors.push_back(abs(diff[i] - base[i]) / base[i]);
            }
        }
    }

    sort(errors.begin(), errors.end());

    cout << smallNumberPairsCount 
         << " pairs of numbers were too small in "
            "magnitude for error calculation (ok)" << endl;
    cout << smallNumberNonPairCount 
         << " non-pair numbers were too small in "
            "magnitude for error calculation (not good)" << endl;

    cout << "5-Number Summary for Errors :" << endl;
    cout << "Min " << errors[0] 
         << " | 1st Quartile " << errors[errors.size() / 4]
         << " | Median " << errors[errors.size() / 2]
         << " | 3rd Quartile " << errors[errors.size() * 3 / 4]
         << " | Max " << errors[errors.size() - 1] 
         << endl;
}

int
main( int argc, char *argv[] )
{
    if( argc <= 2 )
    {
        cerr << "Usage: convolve [filtersize] "
                "[sigma] [filename] ..." << endl;
        cerr << "\tfiltersize -- size of the filter" << endl;
        cerr << "\tsigma -- sigma parameter for gaussian" << endl;
        cerr << "\tfilename -- image to use as benchmark (random image "
                  "if none provided)" << endl;
        exit( 6 );
    }

    int filtersize = atoi(argv[1]);
    float sigma    = atof(argv[2]);

    if (filtersize <= 1 || filtersize % 2 == 0)
    {
        cerr << "Filter size should be at least 3 and odd." << endl;
        exit( 6 );
    }

    vector<uchar> image(DEFAULT_WIDTH * DEFAULT_HEIGHT);

    if (argc == 3)
        getRandomImage(DEFAULT_WIDTH, DEFAULT_HEIGHT, &image[0]);
    else
        ImageTools::readGray( argv[3], 
            DEFAULT_WIDTH * DEFAULT_HEIGHT, &image[0] );

    // Filters
    vector<float> gradientx  = get2DGradientX(filtersize, sigma);
    vector<float> gradienty  = get2DGradientY(filtersize, sigma);
    vector<float> gradient1d = get1DGradient(filtersize, sigma);
    vector<float> gaussian1d = get1DGaussian(filtersize, sigma);
    vector<int> iGradientx  = toIntVector(gradientx, INT_MULTIPLIER);
    vector<int> iGradienty  = toIntVector(gradienty, INT_MULTIPLIER);
    vector<int> iGradient1d = toIntVector(gradient1d, INT_MULTIPLIER);
    vector<int> iGaussian1d = toIntVector(gaussian1d, INT_MULTIPLIER);
    RecursiveCoefficientsSet gaussSet = RecursiveCoefficientsSet(sigma);
    RecursiveCoefficientsSet derivSet = RecursiveCoefficientsSet(sigma);
    gaussSet.Calculate(RecursiveCoefficientsSet::Gaussian);
    derivSet.Calculate(RecursiveCoefficientsSet::FirstDerivGaussian);

    // Do benchmarks.
    clock_t start;
    double duration;
    vector<float> normalGradient(DEFAULT_WIDTH * DEFAULT_HEIGHT);
    vector<float> separatedGradient(DEFAULT_WIDTH * DEFAULT_HEIGHT);
    vector<float> recursiveGradient(DEFAULT_WIDTH * DEFAULT_HEIGHT);
    vector<int> iNormalGradient(DEFAULT_WIDTH * DEFAULT_HEIGHT);
    vector<int> iSeparatedGradient(DEFAULT_WIDTH * DEFAULT_HEIGHT);
    vector<float> fNormalGradient(DEFAULT_WIDTH * DEFAULT_HEIGHT);
    vector<float> fSeparatedGradient(DEFAULT_WIDTH * DEFAULT_HEIGHT);

    // Benchmark naive gradient calculation.
    start = std::clock();

    for (int i = 0; i < REPETITIONS; i++)
        computeGradient(image, normalGradient, gradientx, gradienty,
                        filtersize);
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

    cout << "Naive calculations took " << duration << " s." << endl;

    // Benchmark separated gradient calculation.
    start = std::clock();

    for (int i = 0; i < REPETITIONS; i++)
        computeGradientSeparated(image, separatedGradient, 
                                 gradient1d, gaussian1d, filtersize);
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

    cout << "Separated calculations took " << duration << " s." << endl;

    analyzeError(normalGradient, separatedGradient, filtersize);

    // Benchmark recursive gradient calculation.
    start = std::clock();

    for (int i = 0; i < REPETITIONS; i++)
        computeGradientRecursive(image, recursiveGradient,
                                 gaussSet, derivSet, filtersize);
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

    cout << "Recursive calculations took " << duration << " s." << endl;

    analyzeError(normalGradient, recursiveGradient, filtersize);


    // Benchmark integer naive gradient calculations.
    start = std::clock();

    for (int i = 0; i < REPETITIONS; i++)
        computeGradient(image, iNormalGradient, iGradientx, iGradienty,
                        filtersize);
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

    cout << "Int naive calculations took " << duration << " s." << endl;

    fNormalGradient = toFloatVector(iNormalGradient,
        INT_MULTIPLIER * INT_MULTIPLIER);
    //analyzeError(normalGradient, fNormalGradient, filtersize);

    // Benchmark integer separated gradient calculations.
    start = std::clock();

    for (int i = 0; i < REPETITIONS; i++)
        computeGradientSeparated(image, iSeparatedGradient, 
                                 iGradient1d, iGaussian1d, filtersize);
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

    cout << "Int separated calculations took " << duration << " s." << endl;

    fSeparatedGradient = toFloatVector(iSeparatedGradient,
        INT_MULTIPLIER * INT_MULTIPLIER);
    //analyzeError(normalGradient, fSeparatedGradient, filtersize);
}