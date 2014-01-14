/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkRecursiveGaussianImageFilter.txx,v $
  Language:  C++
  Date:      $Date: 2004/02/02 21:42:22 $
  Version:   $Revision: 1.32 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkRecursiveGaussianImageFilter_txx
#define _itkRecursiveGaussianImageFilter_txx

#include "itkRecursiveGaussianImageFilter.h"
#include "itkObjectFactory.h"
#include "itkImageLinearIteratorWithIndex.h"
#include <new>


namespace itk
{
  
template <typename TInputImage, typename TOutputImage>
RecursiveGaussianImageFilter<TInputImage,TOutputImage>
::RecursiveGaussianImageFilter()
{
  m_Sigma = 1.0;
  m_NormalizeAcrossScale = false;
  m_Order = ZeroOrder;
}

/**
 *   Explicitly set a zeroth order derivative.
 */
template <typename TInputImage, typename TOutputImage>
void
RecursiveGaussianImageFilter<TInputImage,TOutputImage>
::SetZeroOrder()
{
  this->SetOrder( ZeroOrder );
}

/**
 *   Explicitly set a first order derivative.
 */
template <typename TInputImage, typename TOutputImage>
void
RecursiveGaussianImageFilter<TInputImage,TOutputImage>
::SetFirstOrder()
{
  this->SetOrder( FirstOrder );
}

/**
 *   Explicitly set a second order derivative.
 */
template <typename TInputImage, typename TOutputImage>
void
RecursiveGaussianImageFilter<TInputImage,TOutputImage>
::SetSecondOrder()
{
  this->SetOrder( SecondOrder );
}


/**
 *   Compute filter for Gaussian kernel.
 */
template <typename TInputImage, typename TOutputImage>
void
RecursiveGaussianImageFilter<TInputImage,TOutputImage>
::SetUp(RealType spacing)
{ 
  const RealType spacingTolerance = 1e-8;
  
  /**  Parameters of exponential series. */
  RealType A1[3];
  RealType B1[3];
  RealType W1;
  RealType L1;
  RealType A2[3];
  RealType B2[3];
  RealType W2;
  RealType L2;

  RealType direction = 1.0;
  if( spacing < 0.0 )
    {
    direction = -1.0;
    spacing = -spacing;
    }

  if( spacing < spacingTolerance )
    {
    itkExceptionMacro(<<"The spacing " << spacing << "is suspiciosly small in this image");
    } 
  
  const RealType sigmad = m_Sigma/spacing;
  RealType across_scale_normalization = 1.0;
  if( this->GetNormalizeAcrossScale() )
    {
    across_scale_normalization = sigmad;
    }

  A1[0] = static_cast<RealType>(  1.3530 );
  B1[0] = static_cast<RealType>(  1.8151 );
  W1    = static_cast<RealType>(  0.6681 );
  L1    = static_cast<RealType>( -1.3932 );
  A2[0] = static_cast<RealType>( -0.3531 );
  B2[0] = static_cast<RealType>(  0.0902 );
  W2    = static_cast<RealType>(  2.0787 );
  L2    = static_cast<RealType>( -1.3732 );

  A1[1] = static_cast<RealType>( -0.6724 );
  B1[1] = static_cast<RealType>( -3.4327 );
  A2[1] = static_cast<RealType>(  0.6724 );
  B2[1] = static_cast<RealType>(  0.6100 );

  A1[2] = static_cast<RealType>( -1.3563 );
  B1[2] = static_cast<RealType>(  5.2318 );
  A2[2] = static_cast<RealType>(  0.3446 );
  B2[2] = static_cast<RealType>( -2.2355 );

  RealType SD, DD, ED;
  this->ComputeDCoefficients(sigmad, W1, L1, W2, L2, SD, DD, ED);
  RealType SN, DN, EN;
  
  
  switch( m_Order ) 
    {
    case ZeroOrder: // Approximation of convolution with a gaussian.
      {
      ComputeNCoefficients(sigmad,
			   A1[0], B1[0], W1, L1,
			   A2[0], B2[0], W2, L2,
			   m_N0, m_N1, m_N2, m_N3,
			   SN, DN, EN);

      RealType alpha0 = 2 * SN / SD - m_N0;
      m_N0 *= across_scale_normalization / alpha0;
      m_N1 *= across_scale_normalization / alpha0;
      m_N2 *= across_scale_normalization / alpha0;
      m_N3 *= across_scale_normalization / alpha0;
      const bool symmetric = true;
      this->ComputeRemainingCoefficients(symmetric);
      break;
      }
    case FirstOrder: // Approximation of convolution with 
                     // the first derivative of a Gaussian.
      {
      ComputeNCoefficients(sigmad,
			   A1[1], B1[1], W1, L1,
			   A2[1], B2[1], W2, L2,
			   m_N0, m_N1, m_N2, m_N3,
			   SN, DN, EN);

      RealType alpha1 = 2 * (SN*DD - DN*SD) / (SD*SD);
      // If negative spacing, negate the first derivative response.
      alpha1 *= direction;
      
      m_N0 *= across_scale_normalization / alpha1;
      m_N1 *= across_scale_normalization / alpha1;
      m_N2 *= across_scale_normalization / alpha1;
      m_N3 *= across_scale_normalization / alpha1;
      
      const bool symmetric = false;
      this->ComputeRemainingCoefficients(symmetric);
      break;
      }
    case SecondOrder: // Approximation of convolution with 
                      // the second derivative of a Gaussian.
      {
      RealType N0_0, N1_0, N2_0, N3_0;
      RealType N0_2, N1_2, N2_2, N3_2;
      RealType SN0, DN0, EN0;
      RealType SN2, DN2, EN2;
      ComputeNCoefficients(sigmad,
			   A1[0], B1[0], W1, L1,
			   A2[0], B2[0], W2, L2,
			   N0_0, N1_0, N2_0, N3_0,
			   SN0, DN0, EN0);
      ComputeNCoefficients(sigmad,
			   A1[2], B1[2], W1, L1,
			   A2[2], B2[2], W2, L2,
			   N0_2, N1_2, N2_2, N3_2,
			   SN2, DN2, EN2);

      RealType beta = -(2*SN2 - SD*N0_2) / (2*SN0 - SD*N0_0);
      m_N0 = N0_2 + beta * N0_0;
      m_N1 = N1_2 + beta * N1_0;
      m_N2 = N2_2 + beta * N2_0;
      m_N3 = N3_2 + beta * N3_0;
      SN = SN2 + beta * SN0;
      DN = DN2 + beta * DN0;
      EN = EN2 + beta * EN0;

      RealType alpha2;
      alpha2  = EN*SD*SD - ED*SN*SD - 2*DN*DD*SD + 2*DD*DD*SN;
      alpha2 /= SD*SD*SD;
      
      m_N0 *= across_scale_normalization / alpha2;
      m_N1 *= across_scale_normalization / alpha2;
      m_N2 *= across_scale_normalization / alpha2;
      m_N3 *= across_scale_normalization / alpha2;
      
      const bool symmetric = true;
      this->ComputeRemainingCoefficients(symmetric);
      break;
      }
    default:
      {
      itkExceptionMacro(<<"Unknown Order");
      return;
      }
    }
}


/**
 * Compute the N coefficients.
 */
template <typename TInputImage, typename TOutputImage>
void
RecursiveGaussianImageFilter<TInputImage,TOutputImage>
::ComputeNCoefficients(RealType sigmad,
		       RealType A1, RealType B1, RealType W1, RealType L1,
		       RealType A2, RealType B2, RealType W2, RealType L2,
		       RealType& N0, RealType& N1, RealType& N2, RealType& N3,
		       RealType& SN, RealType& DN, RealType& EN)
{
  RealType Sin1 = sin(W1 / sigmad);
  RealType Sin2 = sin(W2 / sigmad);
  RealType Cos1 = cos(W1 / sigmad);
  RealType Cos2 = cos(W2 / sigmad);
  RealType Exp1 = exp(L1 / sigmad);
  RealType Exp2 = exp(L2 / sigmad);
  
  N0  = A1 + A2;
  N1  = Exp2 * (B2*Sin2 - (A2+2*A1) * Cos2); 
  N1 += Exp1 * (B1*Sin1 - (A1+2*A2) * Cos1); 
  N2  = (A1+A2) * Cos2*Cos1;
  N2 -= B1*Cos2*Sin1 + B2*Cos1*Sin2;
  N2 *= 2*Exp1*Exp2;
  N2 += A2*Exp1*Exp1 + A1*Exp2*Exp2;
  N3  = Exp2*Exp1*Exp1 * (B2*Sin2 - A2*Cos2);
  N3 += Exp1*Exp2*Exp2 * (B1*Sin1 - A1*Cos1);

  SN = N0 + N1 + N2 + N3;
  DN = N1 + 2*N2 + 3*N3;
  EN = N1 + 4*N2 + 9*N3;
}


/**
 * Compute the D coefficients.
 */
template <typename TInputImage, typename TOutputImage>
void
RecursiveGaussianImageFilter<TInputImage,TOutputImage>
::ComputeDCoefficients(RealType sigmad,
		       RealType W1, RealType L1, RealType W2, RealType L2,
		       RealType& SD, RealType& DD, RealType& ED) 
{
  RealType Sin1 = sin(W1 / sigmad);
  RealType Sin2 = sin(W2 / sigmad);
  RealType Cos1 = cos(W1 / sigmad);
  RealType Cos2 = cos(W2 / sigmad);
  RealType Exp1 = exp(L1 / sigmad);
  RealType Exp2 = exp(L2 / sigmad);
  
  m_D4  = Exp1*Exp1*Exp2*Exp2;
  m_D3  = -2*Cos1*Exp1*Exp2*Exp2;
  m_D3 += -2*Cos2*Exp2*Exp1*Exp1;
  m_D2  =  4*Cos2*Cos1*Exp1*Exp2;
  m_D2 +=  Exp1*Exp1 + Exp2*Exp2;
  m_D1  =  -2*(Exp2*Cos2 + Exp1*Cos1);

  SD = 1.0 + m_D1 + m_D2 + m_D3 + m_D4;
  DD = m_D1 + 2*m_D2 + 3*m_D3 + 4*m_D4;
  ED = m_D1 + 4*m_D2 + 9*m_D3 + 16*m_D4;
}


/**
 * Compute the M coefficients and the boundary coefficients.
 */
template <typename TInputImage, typename TOutputImage>
void
RecursiveGaussianImageFilter<TInputImage,TOutputImage>
::ComputeRemainingCoefficients(bool symmetric) 
{
  if( symmetric )
    {
    m_M1 = m_N1 - m_D1 * m_N0;
    m_M2 = m_N2 - m_D2 * m_N0;
    m_M3 = m_N3 - m_D3 * m_N0;
    m_M4 =      - m_D4 * m_N0;
    }
  else
    {
    m_M1 = -( m_N1 - m_D1 * m_N0 );
    m_M2 = -( m_N2 - m_D2 * m_N0 );
    m_M3 = -( m_N3 - m_D3 * m_N0 );
    m_M4 =           m_D4 * m_N0;
    }

  // Compute coefficients to be used at the boundaries
  // in order to simulate edge extension boundary conditions.
  const RealType SN = m_N0 + m_N1 + m_N2 + m_N3;
  const RealType SM = m_M1 + m_M2 + m_M3 + m_M4;
  const RealType SD = 1.0 + m_D1 + m_D2 + m_D3 + m_D4;

  m_BN1 = m_D1 * SN / SD;
  m_BN2 = m_D2 * SN / SD;
  m_BN3 = m_D3 * SN / SD;
  m_BN4 = m_D4 * SN / SD;

  m_BM1 = m_D1 * SM / SD;
  m_BM2 = m_D2 * SM / SD;
  m_BM3 = m_D3 * SM / SD;
  m_BM4 = m_D4 * SM / SD;
}


template <typename TInputImage, typename TOutputImage>
void
RecursiveGaussianImageFilter<TInputImage,TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);

  os << "Sigma: " << m_Sigma << std::endl; 
  os << "Order: " << m_Order << std::endl; 
  os << "NormalizeAcrossScale: " << m_NormalizeAcrossScale << std::endl;
}

} // end namespace itk

#endif

