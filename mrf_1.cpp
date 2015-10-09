
#include "stdafx.h"
#include "mrf_1.h"
using namespace std;


/* --------------------------------------------------------- */
ImageOperations::ImageOperations()
{
  in_image = out_image = 0;
 
  width=height=0;

  no_regions = -1; // -1 ==> num. of regions has not been specified yet!
  beta = 0.9;// 0.9;
  t = 0.01;//0.05
  T0 = 4; // 初始的温度值
  c = 0.98; // 每次迭代，温度降低2%
  K = 0;
  E = E_old = 0;
  T = 0;
  mean = variance = 0;
  work_buf1 = 0;
  alpha = 0.1;
  classes = 0;
  in_image_data = 0;
  local_evidence = 0;
}

void ImageOperations::SetNoRegions(int n) 
{ 
  no_regions = n;
  if (n == -1)
    {
      delete [] mean;
      delete [] variance;
	  delete [] work_buf1;
      mean = variance = NULL;
    }
  else 
    {
      mean = new double[n]; 
      variance = new double[n];
	  work_buf1 = new double[n];
      for (int i=0; i<n; ++i) mean[i] = variance[i] = work_buf1[i] = -1;
    }
}

	
/* Compute mean and variance for a given region
 */
void ImageOperations::CalculateMeanAndVariance(int region,int x, int y, int w, int h)
{
  if (in_image != NULL)
    {
	int i, j;

	double sum = 0, sum2=0;
	unsigned char *in_data = (unsigned char *)(in_image->imageData);
	for (i=y; i<y+h; ++i)
		for (j=x; j<x+w; ++j)
		{
			unsigned char t = in_data[i*in_image->widthStep+j];
			sum += t;
			sum2 += t*t;
		}
	mean[region] = sum/(w*h);
	variance[region] = (sum2 - (sum*sum)/(w*h))/(w*h-1);
	if (variance[region] == 0)
		variance[region] = 1e-10;
	work_buf1[region] = log(sqrt(2.0*3.141592653589793*variance[region]));
	// print parameters in gaussians textfield
	cout << region+1 << "\t" << mean[region] << "\t\t" << variance[region] << "\n";
	}
}
void ImageOperations::SetMeanAndVariance(int region,double avg, double var)
{
  if (in_image != NULL)
    {

	mean[region] = avg;
	variance[region] = var;
	if (variance[region] == 0)
		variance[region] = 1e-10;
	work_buf1[region] = log(sqrt(2.0*3.141592653589793*variance[region]));
	// print parameters in gaussians textfield
//	cout << region+1 << "\t" << mean[region] << "\t\t" << variance[region] << "\n";
	}
}


void ImageOperations::InitOutImage()
{
	if(in_image==0)
		return;

	int i, j, r;
	double e, e2;

	//(1) 将iplimage的图像数据转换为内部的图像数据in_image_data
	unsigned char *in_data = (unsigned char *)(in_image->imageData);
	if(in_image_data==0)
	{
		in_image_data = new int* [height];
		for (i=0; i<height; ++i)
			in_image_data[i] = new int[width];
	}
	for (i=0; i<height; ++i)
		for (j=0; j<width; ++j)
			in_image_data[i][j] = in_data[(i*in_image->widthStep)+j];

	//(2) 标签（类）数据
	if(classes==0)
	{
		classes = new int* [height];
		for (i=0; i<height; ++i)
			classes[i] = new int[width];
	}

	//(3) 用单团能量，来初始化类标签(~ max. of singleton energy)
	for (i=0; i<height; ++i)
		for (j=0; j<width; ++j)
		{
			e = _Singleton(i, j, 0);
			classes[i][j] = 0;
			for (r=1; r<no_regions; ++r)
				if ((e2=_Singleton(i, j, r)) < e)
				{
					e = e2;
					classes[i][j] = r;
				}
		}

	//(4) 初始化单团观测证据
	if(local_evidence==0)
	{
		local_evidence = new float* [height];
		for (i=0; i<height; ++i)
		{
			local_evidence[i] = new float[width*no_regions];
			for (j=0; j<width; ++j)
				for (r=0; r<no_regions; r++)
				{
					local_evidence[i][j*no_regions+r]=_Singleton(i, j, r);
				}
		}
	}
//	CreateOutput();
}


void ImageOperations::CreateOutput()
{
  int i, j;
  unsigned char *out_data;

  if(out_image)
  {
	  out_data = (unsigned char *)out_image->imageData;
	
  // cretae output image
  for (i=0; i<height; ++i)
    for (j=0; j<width; ++j)
      {
		  out_data[(i*out_image->widthStep) + j] = (unsigned char)(classes[i][j]*255/no_regions);
      }
  }
}


void ImageOperations::OnIterationOver(void)
{
	;
}

// 1阶团
double ImageOperations::_Singleton(int i, int j, int label)
{
	// work_buf1保存的是查找表，用于性能优化。
	return  work_buf1[label] + pow((double)in_image_data[i][j]-mean[label],2)/(2.0*variance[label]);
}

double ImageOperations::Singleton(int i, int j, int label)
{
	// local_evidence保存的是概率分布，用于性能优化。（idea from "matlab MRF toy examples"）
	return local_evidence[i][j*no_regions+label];
}

// 2阶团
double ImageOperations::Doubleton(int i, int j, int label)
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

// 根据当前标签，计算全局能量
double ImageOperations::CalculateEnergy()
{
	double singletons = 0.0;
	double doubletons = 0.0;
	int i, j, k;
	for (i=0; i<height; ++i)
		for (j=0; j<width; ++j)
		{
			k = classes[i][j];
			singletons += Singleton(i,j,k);
			doubletons += Doubleton(i,j,k); 
		}
	return singletons + doubletons/2; 
}

// 局部能量
double ImageOperations::LocalEnergy(int i, int j, int label)
{
  return Singleton(i,j,label) + Doubleton(i,j,label);
}


void ImageOperations::Gibbs()
{
	int i, j;
	double *Ek;//保存局部能量的数组
	int s;
	double summa_deltaE;
	double sumE;
	double z;
	double r;

	CvRNG rg = cvRNG(time(0)); // make instance of random number generator

	Ek = new double[no_regions];

	K = 0;
	T = T0;
	E_old = CalculateEnergy();

	do
	{
		for (i=0; i<height; ++i)
			for (j=0; j<width; ++j)
			{
				sumE = 0.0;
				for (s=0; s<no_regions; ++s)
				{
					Ek[s] = exp(-LocalEnergy(i, j, s)/T);
					sumE += Ek[s];
				}
				r = cvRandReal(&rg);
				z = 0.0;
				for (s=0; s<no_regions; ++s)
				{
					z += Ek[s]/sumE; 
					if (z > r)
					{
						classes[i][j] = s;
						break;
					}
				}
			}

		E = CalculateEnergy();
		summa_deltaE = fabs(E_old-E);
		E_old = E;
		
		T *= c;
		++K;
		OnIterationOver();
	} while (summa_deltaE > t);

	delete Ek;
}

void ImageOperations::Metropolis(bool mmd)
{
	int i, j;
	double kszi = log(alpha);  // This is for MMD. When executing
	// Metropolis, kszi will be randomly generated.
	double summa_deltaE;

	CvRNG rg = cvRNG(time(0));  // create instance of random number generator

	K = 0;
	T = T0;
	E_old = CalculateEnergy();

	do
	{
		summa_deltaE = 0.0;
		for (i=0; i<height; ++i)
			for (j=0; j<width; ++j)
			{
				int new_lable;

				if (no_regions == 2)
					new_lable = 1 - classes[i][j];
				else
					new_lable = (classes[i][j] + (cvRandInt(&rg) % no_regions)+1 ) % no_regions;

				if (!mmd) kszi = log(cvRandReal(&rg));

				const double old_lable_E = LocalEnergy(i, j, classes[i][j]);
				const double new_label_E = LocalEnergy(i, j, new_lable);
				if (kszi <=  (old_lable_E - new_label_E) / T)
				{
					summa_deltaE += fabs(new_label_E - old_lable_E);
					E_old = E = E_old - old_lable_E + new_label_E;
					classes[i][j] = new_lable;
				}
			}
			T *= c;
			++K;
			OnIterationOver();
	} while (summa_deltaE > t); // stop when energy change is small
}


void ImageOperations::ICM()
{
	int i, j;
	int r;
	double summa_deltaE;

	K = 0;
	E_old = CalculateEnergy();

	do
	{
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
		summa_deltaE = fabs(E_old-E);
		E_old = E;

		++K;
		OnIterationOver();
	}while (summa_deltaE > t);
}

