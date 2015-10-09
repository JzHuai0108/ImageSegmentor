/******************************************************************
 * Modul name : colormrf.cpp
 * Author     : Mihaly Gara (gara@inf.u-szeged.hu) based on the code
 *              written by  Csaba Gradwohl (Gradwohl.Csaba@stud.u-szeged.hu) 
 *              with some  minor contributions from Zoltan Kato 
 *              (kato@inf.u-szeged.hu).
 *
 * Copyright  : GNU General Public License www.gnu.org/copyleft/gpl.html
 *
 * Description:
 * Color-based image segmentation using a Markov random field
 * segmentation model and four different optimization algorithms:
 * Metropolis - Simulated Annealing using Metropolis dynamics
 * Gibbs      - Simulated Annealing using a Gibbs sampler
 * ICM        - Iterated Conditional Modes, a deterministic suboptimal
 *              method (depends on a good initialization).
 * MMD        - Modified Metropolis Dynamics, a pseudo-stochastic
 *              suboptimal method which is less sensitive to
 *              initialization than ICM.
 *
 * The program GUI is written in wxWidgets hence the code can be
 * compiled and ran under Windows as well as under Linux/Unix.
 *
 * $Id: colormrf.cpp,v 1.1 2009/01/09 20:48:09 kato Exp $
 * $Revision: 1.1 $
 * $State: Exp $
 * $Log: colormrf.cpp,v $
 * Revision 1.1  2009/01/09 20:48:09  kato
 * Initial revision
 *
 * 
 *****************************************************************/
#ifndef lint
static char rcsid_colormrf_cpp[]="$Id: colormrf.cpp,v 1.1 2009/01/09 20:48:09 kato Exp $";
#endif
#include "stdafx.h"

#include <math.h>
#include <stdlib.h>

/* Random number generators
 */
#include "randomc.h"   // define classes for random number generators
#include <time.h>

/* Timer classes
 */

#include "colormrf.h"



/*********************************************************************
/* Functions of ColorMRF class
/********************************************************************/
ColorMRF::ColorMRF()
{
	in_data=NULL;
  out_data= NULL;
	l_data=u_data=v_data=NULL;
  no_regions = -1; // -1 ==> num. of regions has not been specified yet!
  beta = -1;
  T0 = -1;
  c = -1;
  K = 0;
  E = 0;
  T = 0;
  mean = variance = NULL;
  covariance = invcov = NULL;
  denom = NULL;
  alpha = 0.1;
}

ColorMRF::~ColorMRF()
{
	if(in_data)
	{
		delete[] in_data;
		delete[]l_data;
		delete[]u_data;
		delete[]v_data;
	
		for (int i=0; i<height; i++)
		{
			for(int j=0;j<width;j++)
				delete []in_image_data[i][j];
			delete []in_image_data[i];
			
		}
		delete[]in_image_data;
		delete[]out_data;
	}
	if(mean)
	{
		for (int j=0; j<3; j++)
		{
			delete []mean[j];
			delete []variance[j];
			delete []covariance[j];
			delete []invcov[j];
			
		}
		delete []mean;
		delete []variance;
		delete []covariance;
		delete []invcov;		
		delete []denom;
		
	}
	if(classes)
	{
		
		for (int i=0; i<height; ++i)
			delete[]	classes[i];
		
		delete []classes;
		delete []out_regions;
	}
}

unsigned char *ColorMRF::GetOrigImage()
{
  return in_data;
}

unsigned char *ColorMRF::GetLImage()
{
  return l_data;
}
unsigned char *ColorMRF::GetUImage()
{
  return u_data;
}

unsigned char *ColorMRF::GetVImage()
{
  return v_data;
}



bool ColorMRF::IsOutput()
{
  if (out_data== NULL) return false;
  else return true;
}


void ColorMRF::SetNoRegions(int n) 
{ 
  int j;
  no_regions = n;
  if (n == -1)
    {
      delete mean;
      delete variance;
      delete covariance;
      delete invcov;
      delete denom;
      mean = variance = NULL;
      covariance = invcov = NULL;
      denom = NULL;
    }
  else 
    {
      mean = new double*[3];
      variance = new double*[3]; 
      covariance = new double*[3];
      invcov = new double*[6];
      denom = new double[n];
      for (j=0; j<3; j++)
	{
	  mean[j] = new double[n];
	  variance[j] = new double[n]; 
	  covariance[j] = new double[n];
	  invcov[j] = new double[n];
	}
      for (j=3; j<6; j++)
	invcov[j] = new double[n];
      for (int i=0; i<n; ++i)
	{
	  for (j=0; j<3; j++)
	    mean[j][i] = variance[j][i] = covariance[j][i] = invcov[j][i] = -1;
	  for (j=3; j<6; j++)
	    invcov[j][i] = -1;
	}
    }
}

	
/* Compute mean vector and covariance matrix for a given label classification
 */
void ColorMRF::CalculateMeanAndCovariance(int*label)
{
	if (in_data!= NULL)
    {
		int i, j, k;
		int *count=new int[no_regions];
		double *sum=new double[no_regions];
		double *sum2=new double[no_regions];
		double *sum3=new double[no_regions];
		//  ((MyFrame *)frame)->GetRegion(x, y, w, h, region);
	
		
		for (k=0; k<3; k++)
		{
			memset(count,0,sizeof(int)*no_regions);
		memset(sum,0,sizeof(double)*no_regions);
		memset(sum2,0,sizeof(double)*no_regions);
		memset(sum3,0,sizeof(double)*no_regions);
			for (i=0; i<height; ++i)
				for (j=0; j<width; ++j)
				{
					int temp=label[i*width+j];
					++count[temp];
					sum[temp] += in_image_data[i][j][k];
					sum2[temp] += in_image_data[i][j][k]*in_image_data[i][j][k];
				}
				for(i=0;i<no_regions;++i)
				{
					mean[k][i] = sum[i]/count[i];
					int x=count[i];
					variance[k][i] = (sum2[i] - (sum[i]*sum[i])/count[i])/(count[i]-1);
				}
		}
		
		// compute covariances
		
		memset(sum,0,sizeof(double)*no_regions);
		memset(sum2,0,sizeof(double)*no_regions);
		memset(sum3,0,sizeof(double)*no_regions);
	
			for (i=0; i<height; ++i)
				for (j=0; j<width; ++j)
				{		// L-u covariance
					int temp=label[i*width+j];

					sum[temp] += (in_image_data[i][j][0]-mean[0][temp])*(in_image_data[i][j][1]-mean[1][temp]);
					// L-v covariance
					sum2[temp] += (in_image_data[i][j][0]-mean[0][temp])*(in_image_data[i][j][2]-mean[2][temp]);
					// u-v covariance
					sum3[temp] += (in_image_data[i][j][1]-mean[1][temp])*(in_image_data[i][j][2]-mean[2][temp]);
				}
		
		for(i=0;i<no_regions;++i)
		{
			covariance[0][i] = sum[i]/count[i];   // L-u covariance
			covariance[1][i] = sum2[i]/count[i];  // L-v covariance
			covariance[2][i] = sum3[i]/count[i];  // u-v covariance
			
			// Compute elements of inverse covariance matrix
			// element (1,1)
			invcov[0][i] = variance[2][i] * variance[1][i] - covariance[2][i]*covariance[2][i];
			
			// elements (1,2) and (2,1)
			invcov[1][i] = covariance[1][i] * covariance[2][i] - variance[2][i] * covariance[0][i];
			
			// elements (1,3) and (3,1)
			invcov[2][i] = covariance[0][i] * covariance[2][i] - variance[1][i] * covariance[1][i];
			
			// element (2,2)
			invcov[3][i] = variance[2][i] * variance[0][i] - covariance[1][i] * covariance[1][i];
			
			// elements (2,3) and (3,2)
			invcov[4][i] = covariance[0][i] * covariance[1][i] - variance[0][i] * covariance[2][i];
			
			// element (3,3)
			invcov[5][i] = variance[1][i] * variance[0][i] - covariance[0][i] * covariance[0][i];
			
			// denominator for computing elements of 
			// inverse covariance matrix
			denom[i] =  variance[0][i] * variance[1][i] * variance[2][i] - 
				variance[2][i] * covariance[0][i] * covariance[0][i] -
				variance[1][i] * covariance[1][i] * covariance[1][i] -
				variance[0][i] * covariance[2][i] * covariance[2][i] +
				covariance[0][i] * covariance[1][i] * covariance[2][i] * 2;
			
			if (denom[i] == 0)
				denom[i] = 1e-10;
		}
		for (k=0; k<3; k++)
		{
			for(i=0;i<no_regions;++i)
			{
				if (covariance[k][i] == 0) 
					covariance[k][i] = 1e-10;
				if (variance[k][i] == 0) 
					variance[k][i] = 1e-10;
			}
		}
			// print parameters in gaussians textfield
			//    *gaussians << region+1 << " (" << mean[0][region] << ", " << mean[1][region] << ", " <<
			//mean[2][region] << ")\t(" << variance[0][region] << ", " << variance[1][region] << ", "
			//	 << variance[2][region] << ")\t(" << covariance[0][region] << ", " << 
			//	covariance[1][region] << ", " << covariance[2][region] << ")\n";
    }
}
//set the mean variance and covariance of a region, mv[3][3], {mean variance, covariance}*luv
void ColorMRF::SetMeanVar(int region, double **mv)
{
	int k;
	for (k=0; k<3; k++)
	{	
		mean[k][region] = mv[0][k];
		variance[k][region] = mv[1][k];
		covariance[k][region] = mv[2][k];   // L-u covariance		
	}	
	
	// Compute elements of inverse covariance matrix
	// element (1,1)
	invcov[0][region] = variance[2][region] * variance[1][region] - covariance[2][region]*covariance[2][region];
	
	// elements (1,2) and (2,1)
	invcov[1][region] = covariance[1][region] * covariance[2][region] - variance[2][region] * covariance[0][region];
	
	// elements (1,3) and (3,1)
	invcov[2][region] = covariance[0][region] * covariance[2][region] - variance[1][region] * covariance[1][region];
	
	// element (2,2)
	invcov[3][region] = variance[2][region] * variance[0][region] - covariance[1][region] * covariance[1][region];
	
	// elements (2,3) and (3,2)
	invcov[4][region] = covariance[0][region] * covariance[1][region] - variance[0][region] * covariance[2][region];
	
	// element (3,3)
	invcov[5][region] = variance[1][region] * variance[0][region] - covariance[0][region] * covariance[0][region];
	
	// denominator for computing elements of 
	// inverse covariance matrix
	denom[region] =  variance[0][region] * variance[1][region] * variance[2][region] - 
		variance[2][region] * covariance[0][region] * covariance[0][region] -
		variance[1][region] * covariance[1][region] * covariance[1][region] -
		variance[0][region] * covariance[2][region] * covariance[2][region] +
		covariance[0][region] * covariance[1][region] * covariance[2][region] * 2;
	
	if (denom[region] == 0)
		denom[region] = 1e-10;
	
	for (k=0; k<3; k++)
	{
		
		if (covariance[k][region] == 0) 
			covariance[k][region] = 1e-10;
		if (variance[k][region] == 0) 
			variance[k][region] = 1e-10;
		
	}
	// print parameters in gaussians textfield
	//    *gaussians << region+1 << " (" << mean[0][region] << ", " << mean[1][region] << ", " <<
	//mean[2][region] << ")\t(" << variance[0][region] << ", " << variance[1][region] << ", "
	//	 << variance[2][region] << ")\t(" << covariance[0][region] << ", " << 
	//	covariance[1][region] << ", " << covariance[2][region] << ")\n";
    
}
double ColorMRF::Singleton(int i, int j, int label)
{
  double det;    // determinant of covariance matrix
  double gauss;  // exponential part of Gaussians

  det = variance[0][label]*variance[1][label]*variance[2][label] + 
    2 * covariance[0][label]*covariance[1][label]*covariance[0][label] - 
    covariance[0][label]*covariance[0][label]*variance[2][label] - 
    covariance[1][label]*covariance[1][label]*variance[1][label] - 
    covariance[2][label]*covariance[2][label]*variance[0][label];

  gauss = ((in_image_data[i][j][0]-mean[0][label]) * invcov[0][label] + 
	   (in_image_data[i][j][1]-mean[1][label]) * invcov[1][label] +
	   (in_image_data[i][j][2]-mean[2][label]) * invcov[2][label]) * (in_image_data[i][j][0]-mean[0][label]) + 
    ((in_image_data[i][j][0]-mean[0][label]) * invcov[1][label] + 
     (in_image_data[i][j][1]-mean[1][label]) * invcov[3][label] + 
     (in_image_data[i][j][2]-mean[2][label]) * invcov[4][label]) * (in_image_data[i][j][1]-mean[1][label]) +
    ((in_image_data[i][j][0]-mean[0][label]) * invcov[2][label] + 
     (in_image_data[i][j][1]-mean[1][label]) * invcov[4][label] + 
     (in_image_data[i][j][2]-mean[2][label]) * invcov[5][label]) * (in_image_data[i][j][2]-mean[2][label]);

  if (det==0)
    det = 1e-10;
  else if (det<0)
    {
      det = -det;
      //	  return - log(sqrt(2.0*3.141592653589793*det)) + 0.5 * (double)gauss / (double)denom[label];
    }
  return log(sqrt(2.0*3.141592653589793*det)) + 0.5 * (double)gauss / (double)denom[label];
}


double ColorMRF::Doubleton(int i, int j, int label)
{
  double energy = 0.0;

  if (i!=height-1) // south
    {
      if (label == classes[i+1][j]) energy -= beta;
      else energy += beta;
    }
  if (j!=width-1) // east
    {
      if (label == classes[i][j+1]) energy -= beta;
      else energy += beta;
    }
  if (i!=0) // nord
    {
      if (label == classes[i-1][j]) energy -= beta;
      else energy += beta;
    }
  if (j!=0) // west
    {
      if (label == classes[i][j-1]) energy -= beta;
      else energy += beta;
    }
  return energy;
}


/* compute global energy
 */
double ColorMRF::CalculateEnergy()
{
  double singletons = 0.0;
  double doubletons = 0.0;
  int i, j, k;
  for (i=0; i<height; ++i)
    for (j=0; j<width; ++j)
      {
	k = classes[i][j];
	// singleton
	singletons += Singleton(i,j,k);
	// doubleton
	doubletons += Doubleton(i,j,k); // Note: here each doubleton is
					// counted twice ==> divide by
					// 2 at the end!
      }
  return singletons + doubletons/2; 
}



double ColorMRF::LocalEnergy(int i, int j, int label)
{
  return Singleton(i,j,label) + Doubleton(i,j,label);
}


/* Initialize segmentation
 */
void ColorMRF::InitOutImage()
{

  int i, j, k, r;
  double *temp_data;
  double e, e2;	 // store local energy

  classes = new int* [height]; // allocate memory for classes
  for (i=0; i<height; ++i)
    classes[i] = new int[width];
  /* initialize using Maximum Likelihood (~ max. of singleton energy)
   */
  for (i=0; i<height; ++i)
    for (j=0; j<width; ++j)
      {
	e = Singleton(i, j, 0);
	classes[i][j] = 0;
	for (r=1; r<no_regions; ++r)
	  if ((e2=Singleton(i, j, r)) < e)
	    {
	      e = e2;
	      classes[i][j] = r;
	    }
      }
  out_regions = new int[no_regions*3];
  temp_data = new double[3];
  for (r=0; r<no_regions; r++)
    {
      temp_data[0] = mean[0][r];
      temp_data[1] = mean[1][r];
      temp_data[2] = mean[2][r];
      temp_data = LuvToRGB(temp_data);
      for (k=0; k<3; k++)
	{
	  out_regions[r*3+k] = (int)temp_data[k];
	}
    }
}

/* Compute CIE-L*u*v* values and 
 * set in_data, L_data, u_data, v_data
 */
void ColorMRF::SetLuv()
{
  int i, j;
  double *luv_data;
  unsigned char *scaled_luv_data;
  double *xyz_data;
  double u0, v0;

 
  luv_data = (double *)malloc(width*height*3*sizeof(double));
  l_data = (unsigned char *)malloc(width*height*3*sizeof(unsigned char));
  u_data = (unsigned char *)malloc(width*height*3*sizeof(unsigned char));
  v_data = (unsigned char *)malloc(width*height*3*sizeof(unsigned char));
  scaled_luv_data = (unsigned char *)malloc(width*height*3*sizeof(unsigned char));
  xyz_data = (double *)malloc(width*height*3*sizeof(double));

  // Compute u0, v0 (corresponding to white color)
  u0 = 4 * 242.36628 / (242.36628 + 15 * 254.999745 +  3 * 277.63227);
  v0 = 9 * 254.999754 / (242.36628 + 15 * 254.999745 +  3 * 277.63227);

  // Convert into CIE-XYZ color space
  for (i=0; i<height; i++)
    for (j=0; j<width; j++)
      {
	// X component
	xyz_data[(i*width*3)+j*3] = (in_data[i*width*3+j*3] * 0.412453 +
				     in_data[i*width*3+j*3+1] * 0.35758 +
				     in_data[i*width*3+j*3+2] * 0.180423);
	// Y component
	xyz_data[(i*width*3)+j*3+1] = (in_data[i*width*3+j*3] * 0.212671 +
				       in_data[i*width*3+j*3+1] * 0.715160 +
				       in_data[i*width*3+j*3+2] * 0.072169);
	// Z component
	xyz_data[(i*width*3)+j*3+2] = (in_data[i*width*3+j*3] * 0.019334 +
				       in_data[i*width*3+j*3+1] * 0.119193 +
				       in_data[i*width*3+j*3+2] * 0.950227);
      }

  // Convert into CIE-L*u*v* color space
  for (i=0; i<height; i++)
    for (j=0; j<width; j++)
      {
	// L component
	if ((xyz_data[(i*width*3)+j*3+1]/254.999745) > 0.008856)
	  luv_data[(i*width*3) + j*3] = 116*pow((xyz_data[(i*width*3)+j*3+1]/254.999745), (1.0/3.0))-16;
	else
	  luv_data[(i*width*3) + j*3] = 903.3*(xyz_data[(i*width*3)+j*3+1]/254.999745);

	// u component
	if ((xyz_data[(i*width*3)+j*3] + 15 * xyz_data[(i*width*3)+j*3+1] + 3 * xyz_data[(i*width*3)+j*3+2])==0)
	  luv_data[(i*width*3) + j*3 +1] = 13 * luv_data[(i*width*3) + j*3] * (-u0);
	else
	  luv_data[(i*width*3) + j*3 +1] = 13 * luv_data[(i*width*3) + j*3] * ((4 * xyz_data[(i*width*3)+j*3] /
										(xyz_data[(i*width*3)+j*3] + 15 * xyz_data[(i*width*3)+j*3+1] + 3 * xyz_data[(i*width*3)+j*3+2])) - u0);

	// v component
	if ((xyz_data[(i*width*3)+j*3] + 15 * xyz_data[(i*width*3)+j*3+1] + 3 * xyz_data[(i*width*3)+j*3+2])==0)
	  luv_data[(i*width*3) + j*3 +2] = 13 * luv_data[(i*width*3) + j*3] * (-v0);
	else
	  luv_data[(i*width*3) + j*3 +2] = 13 * luv_data[(i*width*3) + j*3] * ((9 * xyz_data[(i*width*3)+j*3+1] /
										(xyz_data[(i*width*3)+j*3] + 15 * xyz_data[(i*width*3)+j*3+1] + 3 * xyz_data[(i*width*3)+j*3+2])) - v0);
      }

  in_image_data = new double** [height]; // allocate memory for in_image_data
  for (i=0; i<height; i++)
    {
      in_image_data[i] = new double*[width];
      for(j=0;j<width;j++)
	in_image_data[i][j] = new double[3];
    }

  for (i=0; i<height; i++)
    for (j=0; j<width; j++)
      {
	in_image_data[i][j][0] = luv_data[(i*width*3)+j*3];	//L
	in_image_data[i][j][1] = luv_data[(i*width*3)+j*3+1];	//u
	in_image_data[i][j][2] = luv_data[(i*width*3)+j*3+2];	//v
      }

  // Scale Luv values into [0,255]
  scaled_luv_data = scale(luv_data);

  // image containing the L component only
  for (i=0; i<height; i++)
    for (j=0; j<width; j++)
      {
	l_data[(i*width*3) + j*3] = scaled_luv_data[(i*width*3) + j*3];
	l_data[(i*width*3) + j*3 +1] = scaled_luv_data[(i*width*3) + j*3];
	l_data[(i*width*3) + j*3 +2] = scaled_luv_data[(i*width*3) + j*3];
      }

  // image containing the u component only
  for (i=0; i<height; i++)
    for (j=0; j<width; j++)
      {
	u_data[(i*width*3) + j*3] = scaled_luv_data[(i*width*3) + j*3 +1];
	u_data[(i*width*3) + j*3 +1] = scaled_luv_data[(i*width*3) + j*3 +1];
	u_data[(i*width*3) + j*3 +2] = scaled_luv_data[(i*width*3) + j*3 +1];
      }

  // image containing the v component only
  for (i=0; i<height; i++)
    for (j=0; j<width; j++)
      {
	v_data[(i*width*3) + j*3] = scaled_luv_data[(i*width*3) + j*3 +2];
	v_data[(i*width*3) + j*3 +1] = scaled_luv_data[(i*width*3) + j*3 +2];
	v_data[(i*width*3) + j*3 +2] = scaled_luv_data[(i*width*3) + j*3 +2];
      }
	free(luv_data);
	free(scaled_luv_data);
	free(xyz_data);

}

unsigned char *ColorMRF::scale(double *luv_vector)
{
  int i, j, k;
  unsigned char *t;
  double max[3] = {luv_vector[0], luv_vector[1], luv_vector[2]};
  double min[3] = {luv_vector[0], luv_vector[1], luv_vector[2]};

  for (i=0; i<height; i++)
    for (j=0; j<width; j++)
      for (k=0; k<3; k++)
	{
	  if (luv_vector[(i*width*3) + j*3 + k] < min[k])
	    min[k] = luv_vector[(i*width*3) + j*3 + k];
	  else if (luv_vector[(i*width*3) + j*3 + k] > max[k])
	    max[k] = luv_vector[(i*width*3) + j*3 + k];
	}

  t = (unsigned char *)malloc(width*height*3*sizeof(unsigned char));
  for (i=0; i<height; i++)
    for (j=0; j<width; j++)
      for (k=0; k<3; k++)
	{
	  t[(i*width*3) + j*3 + k] = 
	    (unsigned char)((luv_vector[(i*width*3) + j*3 + k]-min[k]) 
			    * (min[k]!=max[k]?255/(max[k] - min[k]):0));
	}
  return t;
}
/* convert from CIE-L*u*v* colorspace to RGB colorspace
 */
double *ColorMRF::LuvToRGB(double *luv_pixel)
{
  double *rgb_pixel;
  double *xyz_pixel;
  double u0, v0;
  double uV, vV;	// u', v'
  rgb_pixel = new double[3];
  xyz_pixel = new double[3];

  // CIE-L*u*v* -> CIE-XYZ
  // Compute u0, v0 (corresponding to white color)
  u0 = 4 * 242.36628 / (242.36628 + 15 * 254.999745 +  3 * 277.63227);
  v0 = 9 * 254.999745 / (242.36628 + 15 * 254.999745 +  3 * 277.63227);

  uV = luv_pixel[1]/(13*luv_pixel[0]) + u0;
  vV = luv_pixel[2]/(13*luv_pixel[0]) + v0;

  // Y component
  xyz_pixel[1] = (pow(((double)(luv_pixel[0]+16.0)/116.0), 3.0))*254.999745;

  // X component
  xyz_pixel[0] = (-9*xyz_pixel[1]*uV)/((uV-4)*vV-uV*vV);

  // Z component
  xyz_pixel[2] = (9*xyz_pixel[1]-15*vV*xyz_pixel[1]-vV * xyz_pixel[0])/(3.0*vV);

  // CIE-XYZ to RGB

  // R component
  rgb_pixel[0] = (xyz_pixel[0] * 3.240479 + xyz_pixel[1] * -1.537150 + xyz_pixel[2] * -0.498535);
  // G component
  rgb_pixel[1] = (xyz_pixel[0] * -0.969256 + xyz_pixel[1] * 1.875992 + xyz_pixel[2] * 0.041556);
  // B component
  rgb_pixel[2] = (xyz_pixel[0] * 0.055648 + xyz_pixel[1] * -0.204043 + xyz_pixel[2] * 1.057311);

  return rgb_pixel;
}


/* Create and display the output image based on the current labeling.
 * Executed at each iteration.
 */
void ColorMRF::CreateOutput()
{
  int i, j;

  /* Do not count GUI overhead
  */
  timer.Stop();
  
  if(!out_data)
  {
	  out_data=new unsigned char[width*height*3];
  }
  for( i=0; i<height; i++)
	  for( j=0; j<width; j++) 
	  {			
		  BYTE*buf=((BYTE*)(out_data+ width*i*3))+j*3;
		  
		  *(buf)=
			  (unsigned char)out_regions[classes[i][j]*3];
		  *(buf+1)=
			  (unsigned char)out_regions[classes[i][j]*3+1];
		  *(buf+2)=
			  (unsigned char)out_regions[classes[i][j]*3+2];
	  }
	  
	  timer.Start();
	  
}


/* Metropolis & MMD
 */
void ColorMRF::Metropolis(bool mmd)
{
  InitOutImage();
  int i, j;
  int r;
  double kszi = log(alpha); // This is for MMD. When executing
			    // Metropolis, kszi will be randomly generated.
  double summa_deltaE;
  
  TRandomMersenne rg(clock());  // create instance of random number generator

  K = 0;
  T = T0;
  E_old = CalculateEnergy();

  do
    {
      summa_deltaE = 0.0;
      for (i=0; i<height; ++i)
	for (j=0; j<width; ++j)
	  {
	    /* Generate a new label different from the current one with
	     * uniform distribution.
	     */
	    if (no_regions == 2)
	      r = 1 - classes[i][j];
	    else
	      r = (classes[i][j] +
		   (int)(rg.Random()*(no_regions-1))+1) % no_regions;
	    if (!mmd)  // Metropolis: kszi is a  uniform random number
	      kszi = log(rg.Random()); 
	    /* Accept the new label according to Metropolis dynamics.
	     */
	    if (kszi <= (LocalEnergy(i, j, classes[i][j]) -
			 LocalEnergy(i, j, r)) / T) {
	      summa_deltaE += 
		fabs(LocalEnergy(i, j, r) - LocalEnergy(i, j, classes[i][j]));
	      E_old = E = E_old - 
		LocalEnergy(i, j, classes[i][j]) + LocalEnergy(i, j, r);
	      classes[i][j] = r;
	    }
	  }
      T *= c;         // decrease temperature
      ++K;	      // advance iteration counter
      CreateOutput(); // display current labeling
    } while (summa_deltaE > t); // stop when energy change is small
}


/* ICM
 */
void ColorMRF::ICM()
{
  InitOutImage();
  int i, j;
  int r;
  double summa_deltaE;

  K = 0;
  E_old = CalculateEnergy();

  do
    {
      summa_deltaE = 0.0;
      for (i=0; i<height; ++i)
	for (j=0; j<width; ++j)
	  {
	    for (r=0; r<no_regions; ++r)
	      {
		if (LocalEnergy(i, j, classes[i][j]) > LocalEnergy(i, j, r))
		  {
		    classes[i][j] = r;
		  }
	      }
	  }
      E = CalculateEnergy();
      summa_deltaE += fabs(E_old-E);
      E_old = E;

      ++K;	      // advance iteration counter
      CreateOutput(); // display current labeling
    }while (summa_deltaE > t); // stop when energy change is small
}


/* Gibbs sampler
 */
void ColorMRF::Gibbs()
{
  InitOutImage();
  int i, j;
  double *Ek;		       // array to store local energies
  int s;
  double summa_deltaE;
  double sumE;
  double z;
  double r;

  TRandomMersenne rg(time(0)); // make instance of random number generator

  Ek = new double[no_regions];

  K = 0;
  T = T0;
  E_old = CalculateEnergy();

  do
    {
      summa_deltaE = 0.0;
      for (i=0; i<height; ++i)
	for (j=0; j<width; ++j)
	  {
	    sumE = 0.0;
	    for (s=0; s<no_regions; ++s)
	      {
		Ek[s] = exp(-LocalEnergy(i, j, s)/T);
		sumE += Ek[s];
	      }
	    r = rg.Random();	// r is a uniform random number
	    z = 0.0;
	    for (s=0; s<no_regions; ++s)
	      {
		z += Ek[s]/sumE; 
		if (z > r) // choose new label with probabilty exp(-U/T).
		  {
		    classes[i][j] = s;
		    break;
		  }
	      }
	  }
      E = CalculateEnergy();
      summa_deltaE += fabs(E_old-E);
      E_old = E;

      T *= c;         // decrease temperature
      ++K;	      // advance iteration counter
      CreateOutput(); // display current labeling
    } while (summa_deltaE > t); // stop when energy change is small

  delete Ek;
}

//allocate memory for image data
void ColorMRF::SetImage(int w, int h,unsigned char*data)
{
	width=w;
	height=h;
	in_data=new unsigned char[w*h*3];
	if(data)
		memcpy(in_data,data,sizeof(unsigned char)*w*h);

}
