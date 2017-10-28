/**
* This file is part of ImageSegmentor.
*
* Copyright (C) 2012 Jianzhu Huai <huai dot 3 at osu dot edu> (The Ohio State University)
*
* ImageSegmentor is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ImageSegmentor is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ImageSegmentor. If not, see <http://www.gnu.org/licenses/>.
*/

// HC.cpp: implementation of the CHC class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "image.h"

#include "HC.h"
#include "fuzzylogic.h"
#include "require.h"
#include "ImpExpData.h"
#include "segment-image.h"


//#include "mwcomtypes.h" 
//#include "regprops_idl_i.c" 
//#include "regprops_idl.h" 
#include "..\\include\\gdal.h"
#include "..\\include\\gdal_priv.h"
#include "..\\include\\ogrsf_frmts.h"
#include "..\\include\\gdal_alg.h"

using namespace std;
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//Programming Tips
//recommend using vector instead of array, but remember to use reference & when using function to assign values to vector memory allocated previously
//match new and delete, cvCreateXX and cvReleaseXX, 

//Notes: Import data into and export data from CHC, the raster array order remained the same as that obtained by GDALRasterIO()
//Import and Export data in CDIB, the raster matrix order is vertically reversed from that of GDALRasterIO()
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


static int Count=0;

void AdaptiveFindThreshold(CvMat *dx, CvMat *dy, double &low, double &high,double PercentOfPixelsNotEdges)
{
	CvSize size;
	IplImage *imge=0;
	int i,j;  CvHistogram *hist;
	int hist_size = 255;
    float range_0[]={0,256};
    float* ranges[] = { range_0 };
	size = cvGetSize(dx);
	imge = cvCreateImage(size, IPL_DEPTH_32F, 1);
	// 计算边缘的强度, 并存于图像中
	float maxv = 0;
	for(i = 0; i < size.height; i++ )
	{
		const short* _dx = (short*)(dx->data.ptr + dx->step*i);
        const short* _dy = (short*)(dy->data.ptr + dy->step*i);
		float* _image = (float *)(imge->imageData + imge->widthStep*i);
		for(j = 0; j < size.width; j++)
		{
			_image[j] = (float)(abs(_dx[j]) + abs(_dy[j]));
			maxv = maxv < _image[j] ? _image[j]: maxv;
		}
	}
	// 计算直方图
	range_0[1] = maxv;
	hist_size = (int)(hist_size > maxv ? maxv:hist_size);
	hist = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
	cvCalcHist( &imge, hist, 0, NULL );
	int total = (int)(size.height * size.width * PercentOfPixelsNotEdges);
	float sum=0;
	int icount = hist->mat.dim[0].size;
	
	float *h = (float*)cvPtr1D( hist->bins, 0 );
	for(i = 0; i < icount; i++)
	{
		sum += h[i];
		if( sum > total )
			break; 
	} // 计算高低门限
	high = (i+1) * maxv / hist_size ;
	low = high * 0.4;
	cvReleaseImage( &imge );
	cvReleaseHist(&hist);
}
/****************************************************************************************\
*                   Antialiazed Elliptic Arcs via Antialiazed Lines                      *
\****************************************************************************************/

static const float icvSinTable[] =
    { 0.0000000f, 0.0174524f, 0.0348995f, 0.0523360f, 0.0697565f, 0.0871557f,
    0.1045285f, 0.1218693f, 0.1391731f, 0.1564345f, 0.1736482f, 0.1908090f,
    0.2079117f, 0.2249511f, 0.2419219f, 0.2588190f, 0.2756374f, 0.2923717f,
    0.3090170f, 0.3255682f, 0.3420201f, 0.3583679f, 0.3746066f, 0.3907311f,
    0.4067366f, 0.4226183f, 0.4383711f, 0.4539905f, 0.4694716f, 0.4848096f,
    0.5000000f, 0.5150381f, 0.5299193f, 0.5446390f, 0.5591929f, 0.5735764f,
    0.5877853f, 0.6018150f, 0.6156615f, 0.6293204f, 0.6427876f, 0.6560590f,
    0.6691306f, 0.6819984f, 0.6946584f, 0.7071068f, 0.7193398f, 0.7313537f,
    0.7431448f, 0.7547096f, 0.7660444f, 0.7771460f, 0.7880108f, 0.7986355f,
    0.8090170f, 0.8191520f, 0.8290376f, 0.8386706f, 0.8480481f, 0.8571673f,
    0.8660254f, 0.8746197f, 0.8829476f, 0.8910065f, 0.8987940f, 0.9063078f,
    0.9135455f, 0.9205049f, 0.9271839f, 0.9335804f, 0.9396926f, 0.9455186f,
    0.9510565f, 0.9563048f, 0.9612617f, 0.9659258f, 0.9702957f, 0.9743701f,
    0.9781476f, 0.9816272f, 0.9848078f, 0.9876883f, 0.9902681f, 0.9925462f,
    0.9945219f, 0.9961947f, 0.9975641f, 0.9986295f, 0.9993908f, 0.9998477f,
    1.0000000f, 0.9998477f, 0.9993908f, 0.9986295f, 0.9975641f, 0.9961947f,
    0.9945219f, 0.9925462f, 0.9902681f, 0.9876883f, 0.9848078f, 0.9816272f,
    0.9781476f, 0.9743701f, 0.9702957f, 0.9659258f, 0.9612617f, 0.9563048f,
    0.9510565f, 0.9455186f, 0.9396926f, 0.9335804f, 0.9271839f, 0.9205049f,
    0.9135455f, 0.9063078f, 0.8987940f, 0.8910065f, 0.8829476f, 0.8746197f,
    0.8660254f, 0.8571673f, 0.8480481f, 0.8386706f, 0.8290376f, 0.8191520f,
    0.8090170f, 0.7986355f, 0.7880108f, 0.7771460f, 0.7660444f, 0.7547096f,
    0.7431448f, 0.7313537f, 0.7193398f, 0.7071068f, 0.6946584f, 0.6819984f,
    0.6691306f, 0.6560590f, 0.6427876f, 0.6293204f, 0.6156615f, 0.6018150f,
    0.5877853f, 0.5735764f, 0.5591929f, 0.5446390f, 0.5299193f, 0.5150381f,
    0.5000000f, 0.4848096f, 0.4694716f, 0.4539905f, 0.4383711f, 0.4226183f,
    0.4067366f, 0.3907311f, 0.3746066f, 0.3583679f, 0.3420201f, 0.3255682f,
    0.3090170f, 0.2923717f, 0.2756374f, 0.2588190f, 0.2419219f, 0.2249511f,
    0.2079117f, 0.1908090f, 0.1736482f, 0.1564345f, 0.1391731f, 0.1218693f,
    0.1045285f, 0.0871557f, 0.0697565f, 0.0523360f, 0.0348995f, 0.0174524f,
    0.0000000f, -0.0174524f, -0.0348995f, -0.0523360f, -0.0697565f, -0.0871557f,
    -0.1045285f, -0.1218693f, -0.1391731f, -0.1564345f, -0.1736482f, -0.1908090f,
    -0.2079117f, -0.2249511f, -0.2419219f, -0.2588190f, -0.2756374f, -0.2923717f,
    -0.3090170f, -0.3255682f, -0.3420201f, -0.3583679f, -0.3746066f, -0.3907311f,
    -0.4067366f, -0.4226183f, -0.4383711f, -0.4539905f, -0.4694716f, -0.4848096f,
    -0.5000000f, -0.5150381f, -0.5299193f, -0.5446390f, -0.5591929f, -0.5735764f,
    -0.5877853f, -0.6018150f, -0.6156615f, -0.6293204f, -0.6427876f, -0.6560590f,
    -0.6691306f, -0.6819984f, -0.6946584f, -0.7071068f, -0.7193398f, -0.7313537f,
    -0.7431448f, -0.7547096f, -0.7660444f, -0.7771460f, -0.7880108f, -0.7986355f,
    -0.8090170f, -0.8191520f, -0.8290376f, -0.8386706f, -0.8480481f, -0.8571673f,
    -0.8660254f, -0.8746197f, -0.8829476f, -0.8910065f, -0.8987940f, -0.9063078f,
    -0.9135455f, -0.9205049f, -0.9271839f, -0.9335804f, -0.9396926f, -0.9455186f,
    -0.9510565f, -0.9563048f, -0.9612617f, -0.9659258f, -0.9702957f, -0.9743701f,
    -0.9781476f, -0.9816272f, -0.9848078f, -0.9876883f, -0.9902681f, -0.9925462f,
    -0.9945219f, -0.9961947f, -0.9975641f, -0.9986295f, -0.9993908f, -0.9998477f,
    -1.0000000f, -0.9998477f, -0.9993908f, -0.9986295f, -0.9975641f, -0.9961947f,
    -0.9945219f, -0.9925462f, -0.9902681f, -0.9876883f, -0.9848078f, -0.9816272f,
    -0.9781476f, -0.9743701f, -0.9702957f, -0.9659258f, -0.9612617f, -0.9563048f,
    -0.9510565f, -0.9455186f, -0.9396926f, -0.9335804f, -0.9271839f, -0.9205049f,
    -0.9135455f, -0.9063078f, -0.8987940f, -0.8910065f, -0.8829476f, -0.8746197f,
    -0.8660254f, -0.8571673f, -0.8480481f, -0.8386706f, -0.8290376f, -0.8191520f,
    -0.8090170f, -0.7986355f, -0.7880108f, -0.7771460f, -0.7660444f, -0.7547096f,
    -0.7431448f, -0.7313537f, -0.7193398f, -0.7071068f, -0.6946584f, -0.6819984f,
    -0.6691306f, -0.6560590f, -0.6427876f, -0.6293204f, -0.6156615f, -0.6018150f,
    -0.5877853f, -0.5735764f, -0.5591929f, -0.5446390f, -0.5299193f, -0.5150381f,
    -0.5000000f, -0.4848096f, -0.4694716f, -0.4539905f, -0.4383711f, -0.4226183f,
    -0.4067366f, -0.3907311f, -0.3746066f, -0.3583679f, -0.3420201f, -0.3255682f,
    -0.3090170f, -0.2923717f, -0.2756374f, -0.2588190f, -0.2419219f, -0.2249511f,
    -0.2079117f, -0.1908090f, -0.1736482f, -0.1564345f, -0.1391731f, -0.1218693f,
    -0.1045285f, -0.0871557f, -0.0697565f, -0.0523360f, -0.0348995f, -0.0174524f,
    -0.0000000f, 0.0174524f, 0.0348995f, 0.0523360f, 0.0697565f, 0.0871557f,
    0.1045285f, 0.1218693f, 0.1391731f, 0.1564345f, 0.1736482f, 0.1908090f,
    0.2079117f, 0.2249511f, 0.2419219f, 0.2588190f, 0.2756374f, 0.2923717f,
    0.3090170f, 0.3255682f, 0.3420201f, 0.3583679f, 0.3746066f, 0.3907311f,
    0.4067366f, 0.4226183f, 0.4383711f, 0.4539905f, 0.4694716f, 0.4848096f,
    0.5000000f, 0.5150381f, 0.5299193f, 0.5446390f, 0.5591929f, 0.5735764f,
    0.5877853f, 0.6018150f, 0.6156615f, 0.6293204f, 0.6427876f, 0.6560590f,
    0.6691306f, 0.6819984f, 0.6946584f, 0.7071068f, 0.7193398f, 0.7313537f,
    0.7431448f, 0.7547096f, 0.7660444f, 0.7771460f, 0.7880108f, 0.7986355f,
    0.8090170f, 0.8191520f, 0.8290376f, 0.8386706f, 0.8480481f, 0.8571673f,
    0.8660254f, 0.8746197f, 0.8829476f, 0.8910065f, 0.8987940f, 0.9063078f,
    0.9135455f, 0.9205049f, 0.9271839f, 0.9335804f, 0.9396926f, 0.9455186f,
    0.9510565f, 0.9563048f, 0.9612617f, 0.9659258f, 0.9702957f, 0.9743701f,
    0.9781476f, 0.9816272f, 0.9848078f, 0.9876883f, 0.9902681f, 0.9925462f,
    0.9945219f, 0.9961947f, 0.9975641f, 0.9986295f, 0.9993908f, 0.9998477f,
    1.0000000f
};



static void
icvSinCos( int angle, float *cosval, float *sinval )
{
    angle += (angle < 0 ? 360 : 0);
    *sinval = icvSinTable[angle];
    *cosval = icvSinTable[450 - angle];
}
float JHLineAngle(CvPoint pt1, CvPoint pt2, CvPoint pt3)//calculate the cosine of the angle formed by 
//line pt1 to pt2 and line pt2 to pt3
{
	float cross=(pt1.x-pt2.x)*(pt2.x-pt3.x)+(pt1.y-pt2.y)*(pt2.y-pt3.y);
	float ln1=sqrt((float)(pt1.x-pt2.x)*(pt1.x-pt2.x)+(pt1.y-pt2.y)*(pt1.y-pt2.y));
	float ln2=sqrt((float)(pt3.x-pt2.x)*(pt3.x-pt2.x)+(pt3.y-pt2.y)*(pt3.y-pt2.y));
	return cross/(ln1*ln2);
}
//intensity similarity performed on regular grid of step size 
//input step 16 by default, band1 and band2 two gray scale single band images
//output coeff gray scale image
void VRegSimi(IplImage*band1,IplImage*band2, int step, IplImage*coeff)
{
	CvHistogram* histv1;
	{
		int hist_size=256;	
		float v_ranges[] = { 0, 256 };
		float* ranges= v_ranges;
		histv1= cvCreateHist(
			1,
			&hist_size,
			CV_HIST_ARRAY,
			&ranges,
			1
			);
	}
	CvHistogram* histv2;
	{
		int hist_size=256;	
		float v_ranges[] = { 0, 256 };
		float* ranges= v_ranges;
		histv2= cvCreateHist(
			1,
			&hist_size,
			CV_HIST_ARRAY,
			&ranges,
			1
			);
	}

	int i,j,xs,ys;
	int wid=band1->width,heg=band1->height;
	float temp;

	for(j=0;j<heg;j+=step)
	{
		for(i=0;i<wid;i+=step)
		{
			xs=step>(wid-i)?(wid-step):i;
			ys=step>(heg-j)?(heg-step):j;
		cvSetImageROI(band1, cvRect(xs,ys,step,step));
		cvCalcHist( &band1, histv1, 0, 0 ); //Compute histogram		
	
		cvNormalizeHist( histv1, 1.0 ); //Normalize it
			
		cvSetImageROI(band2, cvRect(xs,ys,step,step));
	
		cvCalcHist(&band2, histv2, 0, 0 ); //Compute histogram
		cvNormalizeHist( histv2, 1.0 ); //Normalize it
		cvSetImageROI(coeff, cvRect(xs,ys,step,step));
		temp=(float)cvCompareHist(histv1,histv2,CV_COMP_CORREL);
		temp=(temp*128+127);
		cvSet(coeff,cvRealScalar(temp));
		}
	}
	cvResetImageROI(band1);
	cvResetImageROI(band2);
	cvResetImageROI(coeff);
	cvReleaseHist(&histv1);
	cvReleaseHist(&histv2);
}
//load gdaldataset data to iplimage, bandcount is 1 bydefault
IplImage*LoadGDALToIPL(const char*fN,int bC, int bits)
{
	GDALDataset*pDataset = (GDALDataset *) GDALOpen(fN,GA_ReadOnly);
	if (!pDataset)
	{
		AfxMessageBox("Cannot open file in LoadGDALToIPL!");
		return NULL;
	}
	int w,h,spp,d,x,y,temp;
	w=pDataset->GetRasterXSize(); //影响的高度，宽度
	h=pDataset->GetRasterYSize();
	spp=pDataset->GetRasterCount();//波段的数目
	GDALRasterBand  *m_pBand=NULL;
	IplImage* goal;
	
	switch( bits)
	{
	case 32:
	default:
		{
			bC=bC>spp?spp:bC;
			goal=cvCreateImage(cvSize(w,h),IPL_DEPTH_32F,bC);
			float* buf =new float[w*h];
			for(d=0;d<bC;++d)
			{
				m_pBand= pDataset->GetRasterBand(d+1);		
				if (m_pBand)
				{
					
					if (CE_None==m_pBand->RasterIO( GF_Read,0,0, w,h, buf, w,h, GDT_Float32, 0, 0 ))
					{	
						
						temp=0;
						for(y=0;y<h;++y)
						{
							for (x = 0; x < w; ++x) 
							{
								((float*)(goal->imageData + goal->widthStep*y))[x*bC+d]=buf[temp];	
								
								++temp;
							}				
						}
					}
				}
			}
			delete[]buf;
			break;
		}
	case 8:
		{
			goal=cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,bC);
			BYTE* buf =new BYTE[w*h];
			if(spp==1)
			{
				m_pBand= pDataset->GetRasterBand(1);		
				if (m_pBand)
				{
					
					if (CE_None==m_pBand->RasterIO( GF_Read,0,0, w,h, buf, w,h, GDT_Byte, 0, 0 ))
					{
						if(bC==1)
						{
							temp=0;
							for(y=0;y<h;++y)
							{
								for (x = 0; x < w; ++x) 
								{
									
									((BYTE*)(goal->imageData + goal->widthStep*y))[x]=buf[temp];	
									
									++temp;
								}				
							}
						}
						else if(bC==3)
						{
							temp=0;
							for(y=0;y<h;++y)
							{
								for (x = 0; x < w; ++x) 
								{
									
									((BYTE*)(goal->imageData + goal->widthStep*y))[x*3]=buf[temp];	
									((BYTE*)(goal->imageData + goal->widthStep*y))[x*3+1]=buf[temp];
									((BYTE*)(goal->imageData + goal->widthStep*y))[x*3+2]=buf[temp];
									++temp;
								}				
							}
						}
						else
							AfxMessageBox("Unsupportable format in LoadGDALToIPL!");
						
					}
				}
			}
			else if(spp>=3&&bC<=3)
			{				
				for(d=0;d<bC;++d)
				{
					m_pBand= pDataset->GetRasterBand(d+1);		
					if (m_pBand)
					{
						
						if (CE_None==m_pBand->RasterIO( GF_Read,0,0, w,h, buf, w,h, GDT_Byte, 0, 0 ))
						{				
							temp=0;
							for(y=0;y<h;++y)
							{
								for (x = 0; x < w; ++x) 
								{
									((BYTE*)(goal->imageData + goal->widthStep*y))[x*bC+d]=buf[temp];	
									++temp;
								}				
							}
						}
					}
				}
			}
			else
				AfxMessageBox("Unsupportable format in LoadGDALToIPL!");
			delete[]buf;
			break;
		}
		
		
	}
	GDALClose((GDALDatasetH)pDataset);
	return goal;
}

//compute mean and variance of gray for each pixel based on a window centered on this pixel of side length l
//if l is even, l=l+1, using integral image technique menthioned in 
//"Building Extraction and Change Detection in Multitemporal Aerial and Satellite Images in a Joint Stochastic Approach"
//output winMean and winVar are of float data type, gray is also float based intensity image
int cvWinMeanVar(IplImage* gray, int l, IplImage* winMean, IplImage* winVar)
{

	int i,j;
	int step=l/2; 

	int w=gray->width, h=gray->height;
	float temp;
	//store integral image data
	IplImage* sum=cvCreateImage(cvSize(w,h),IPL_DEPTH_32F,1);
	IplImage* sum2=cvCreateImage(cvSize(w,h),IPL_DEPTH_32F,1);
	for(j=0;j<h;++j)
	{
		for(i=0;i<w;++i)
		{			
			temp=((float*)(gray->imageData + gray->widthStep*j))[i];
			((float*)(sum->imageData + sum->widthStep*j))[i]=temp;
			((float*)(sum2->imageData + sum2->widthStep*j))[i]=temp*temp;
		}
	}
	//cascade accumulate data in sum and sum2
	//x axis accumulate
	for(j=0;j<h;++j)
	{
		for(i=1;i<w;++i)
		{	
			((float*)(sum->imageData + sum->widthStep*j))[i]+=
				((float*)(sum->imageData + sum->widthStep*j))[i-1];
			((float*)(sum2->imageData + sum2->widthStep*j))[i]+=
				((float*)(sum2->imageData + sum2->widthStep*j))[i-1];
		}
	}
	//y axis accumulate
	for(i=0;i<w;++i)
	{
		for(j=1;j<h;++j)
		{
			
			((float*)(sum->imageData + sum->widthStep*j))[i]+=
				((float*)(sum->imageData + sum->widthStep*(j-1)))[i];
			((float*)(sum2->imageData + sum2->widthStep*j))[i]+=
				((float*)(sum2->imageData + sum2->widthStep*(j-1)))[i];
		}
	}
	//compute mean and variance for each pixel
	int lf,rt,tp,bt,sz;
	for(j=0;j<h;++j)
	{
		for(i=0;i<w;++i)
		{
			lf=i-step<0?0:(i-step);
			tp=j-step<0?0:(j-step);
			rt=i+step>=w?w-1:(i+step);
			bt=j+step>=h?h-1:(j+step);
			sz=(bt-tp+1)*(rt-lf+1);
			if(lf==0&&tp==0)
			{
				((float*)(winMean->imageData + winMean->widthStep*j))[i]=
					((float*)(sum->imageData + sum->widthStep*bt))[rt]/sz;
				((float*)(winVar->imageData + winVar->widthStep*j))[i]=
					((float*)(sum2->imageData + sum2->widthStep*bt))[rt]/sz;
				temp=((float*)(winMean->imageData + winMean->widthStep*j))[i];
				((float*)(winVar->imageData + winVar->widthStep*j))[i]-=temp*temp;
					temp=((float*)(winVar->imageData + winVar->widthStep*j))[i];
			assert(temp>=0);
			}
			else if(tp==0&&lf>0)
			{
				temp=((float*)(sum->imageData + sum->widthStep*bt))[rt]-
					((float*)(sum->imageData + sum->widthStep*bt))[lf-1];
				((float*)(winMean->imageData + winMean->widthStep*j))[i]=temp/sz;
				temp=((float*)(sum2->imageData + sum2->widthStep*bt))[rt]-
					((float*)(sum2->imageData + sum2->widthStep*bt))[lf-1];
				((float*)(winVar->imageData + winVar->widthStep*j))[i]=temp/sz;
						temp=((float*)(winMean->imageData + winMean->widthStep*j))[i];
				((float*)(winVar->imageData + winVar->widthStep*j))[i]-=temp*temp;
					temp=((float*)(winVar->imageData + winVar->widthStep*j))[i];
			assert(temp>=0);
			}
			else if(lf==0&&tp>0)
			{
				temp=((float*)(sum->imageData + sum->widthStep*bt))[rt]-
					((float*)(sum->imageData + sum->widthStep*(tp-1)))[rt];
				((float*)(winMean->imageData + winMean->widthStep*j))[i]=temp/sz;
				temp=((float*)(sum2->imageData + sum2->widthStep*bt))[rt]-
					((float*)(sum2->imageData + sum2->widthStep*(tp-1)))[rt];
				((float*)(winVar->imageData + winVar->widthStep*j))[i]=temp/sz;
						temp=((float*)(winMean->imageData + winMean->widthStep*j))[i];
				((float*)(winVar->imageData + winVar->widthStep*j))[i]-=temp*temp;
					temp=((float*)(winVar->imageData + winVar->widthStep*j))[i];
			assert(temp>=0);
			}
			else
			{
				temp=((float*)(sum->imageData + sum->widthStep*bt))[rt]+
					((float*)(sum->imageData + sum->widthStep*(tp-1)))[lf-1]-
					((float*)(sum->imageData + sum->widthStep*(tp-1)))[rt]-
					((float*)(sum->imageData + sum->widthStep*bt))[lf-1];
				((float*)(winMean->imageData + winMean->widthStep*j))[i]=temp/sz;
				temp=((float*)(sum2->imageData + sum2->widthStep*bt))[rt]+
					((float*)(sum2->imageData + sum2->widthStep*(tp-1)))[lf-1]-
					((float*)(sum2->imageData + sum2->widthStep*(tp-1)))[rt]-
					((float*)(sum2->imageData + sum2->widthStep*bt))[lf-1];
				((float*)(winVar->imageData + winVar->widthStep*j))[i]=temp/sz;
					temp=((float*)(winMean->imageData + winMean->widthStep*j))[i];
				((float*)(winVar->imageData + winVar->widthStep*j))[i]-=temp*temp;
					temp=((float*)(winVar->imageData + winVar->widthStep*j))[i];
		
					if(temp<0)
					{
							assert(abs(temp)<1e-1);
							((float*)(winVar->imageData + winVar->widthStep*j))[i]=0;
					}
			}
		}
	}

	cvReleaseImage(&sum);
	cvReleaseImage(&sum2);
	return 1;
}
//point to point multiply image inp1 and inp2 of float type
int P2PImgMultiply(IplImage* inp1,IplImage* inp2,IplImage* dst)
{
	int i,j;
	int w=inp1->width, h=inp1->height;
	for(j=0;j<h;++j)
	{
		for(i=0;i<w;++i)
		{			
			((float*)(dst->imageData + dst->widthStep*j))[i]=
			((float*)(inp1->imageData + inp1->widthStep*j))[i]*
			((float*)(inp2->imageData + inp2->widthStep*j))[i];
		}
	}
	return 1;
}
//integral image of window size l*l, dest can be identical to src
int IntegralImage(IplImage*src,IplImage*dest)
{
	//integral image to compute correlation
	int w=src->width,h=src->height;
	int i,j;
	if(dest)
	{
		cvCopyImage(src,dest);
		//x axis accumulate
		for(j=0;j<h;++j)
		{
			for(i=1;i<w;++i)
			{	
				((float*)(dest->imageData + dest->widthStep*j))[i]+=
					((float*)(dest->imageData + dest->widthStep*j))[i-1];
			}
		}
		//y axis accumulate
		for(i=0;i<w;++i)
		{
			for(j=1;j<h;++j)
			{			
				((float*)(dest->imageData + dest->widthStep*j))[i]+=
					((float*)(dest->imageData + dest->widthStep*(j-1)))[i];
			}
		}
	}
	else
	{
		//x axis accumulate
		for(j=0;j<h;++j)
		{
			for(i=1;i<w;++i)
			{	
				((float*)(src->imageData + src->widthStep*j))[i]+=
					((float*)(src->imageData + src->widthStep*j))[i-1];
			}
		}
		//y axis accumulate
		for(i=0;i<w;++i)
		{
			for(j=1;j<h;++j)
			{			
				((float*)(src->imageData + src->widthStep*j))[i]+=
					((float*)(src->imageData + src->widthStep*(j-1)))[i];
			}
		}
	}
	return 1;
}
//compute correlation image between gray1 and gray2 of unsigned char, l is window size 
//mentioned by Csaba Benedek in "Change Detection in Optical Aerial Images by a
//Multi-Layer Conditional Mixed Markov Model, also 
//Building Extraction and Change Detection in Multitemporal Aerial and
// Satellite Images in a Joint Stochastic Approach" vxv neighborhood v=17, gray1 and gray2 are all float type
//outcome is of 1 channel float,
//note this function can also be implemented using Opencv cvMatchTemplate of method=CV_TM_CCOEFF_NORMED
int cvWinCorr(IplImage*gray1,IplImage* gray2, int l, IplImage* outcome)
{
	int i,j;
	int w=gray1->width, h=gray1->height;
	float temp;
	IplImage* mean1=cvCreateImage(cvSize(w,h),IPL_DEPTH_32F,1);
	IplImage* mean2=cvCreateImage(cvSize(w,h),IPL_DEPTH_32F,1);
	IplImage* var1=cvCreateImage(cvSize(w,h),IPL_DEPTH_32F,1);
	IplImage* var2=cvCreateImage(cvSize(w,h),IPL_DEPTH_32F,1);
	IplImage* fg1=cvCreateImage(cvSize(w,h),IPL_DEPTH_32F,1);
	IplImage* fg2=cvCreateImage(cvSize(w,h),IPL_DEPTH_32F,1);
	//normalize to [0,1]
	//search for largest order
	float scope,max=0;
	for(j=0;j<h;++j)
	{
		for(i=0;i<w;++i)
		{			
			temp=((float*)(gray1->imageData + gray1->widthStep*j))[i];
			if(temp>max)
				max=temp;
		}
	}
	j=(int)ceil(max);
	i=1;
	while(j>1)
	{
		++i;
		j>>=1;
	}
	scope=(float)(1<<i);
	
	for(j=0;j<h;++j)
	{
		for(i=0;i<w;++i)
		{			
			temp=((float*)(gray1->imageData + gray1->widthStep*j))[i]/scope;
			((float*)(fg1->imageData + fg1->widthStep*j))[i]=temp;
			temp=((float*)(gray2->imageData + gray2->widthStep*j))[i]/scope;
			((float*)(fg2->imageData + fg2->widthStep*j))[i]=temp;
		}
	}
	cvWinMeanVar(fg1, l, mean1,var1);
	cvWinMeanVar(fg2, l, mean2,var2);
	
	P2PImgMultiply(fg1,fg2,fg1);
	P2PImgMultiply(mean1,mean2,mean1);
	P2PImgMultiply(var1,var2,var1);
	cvReleaseImage(&var2);
	cvReleaseImage(&mean2);
	cvReleaseImage(&fg2);
	//integral image to compute correlation
	//x axis accumulate
	for(j=0;j<h;++j)
	{
		for(i=1;i<w;++i)
		{	
			((float*)(fg1->imageData + fg1->widthStep*j))[i]+=
				((float*)(fg1->imageData + fg1->widthStep*j))[i-1];
		}
	}
	//y axis accumulate
	for(i=0;i<w;++i)
	{
		for(j=1;j<h;++j)
		{			
			((float*)(fg1->imageData + fg1->widthStep*j))[i]+=
				((float*)(fg1->imageData + fg1->widthStep*(j-1)))[i];
		}
	}
	int lf,rt,tp,bt,sz;
	int step=l/2;
	for(j=0;j<h;++j)
	{
		for(i=0;i<w;++i)
		{
			lf=i-step<0?0:(i-step);
			tp=j-step<0?0:(j-step);
			rt=i+step>=w?w-1:(i+step);
			bt=j+step>=h?h-1:(j+step);
			sz=(bt-tp+1)*(rt-lf+1);
			if(lf==0&&tp==0)
			{
				temp=((float*)(fg1->imageData + fg1->widthStep*bt))[rt]/sz-
					((float*)(mean1->imageData + mean1->widthStep*j))[i];
				if(((float*)(var1->imageData + var1->widthStep*j))[i]==0)
					((float*)(outcome->imageData + outcome->widthStep*j))[i]=1000;
				else
				{
					temp/=sqrt(((float*)(var1->imageData + var1->widthStep*j))[i]);
				((float*)(outcome->imageData + outcome->widthStep*j))[i]=temp;
				}
			}
			else if(tp==0&&lf>0)
			{
				temp=((float*)(fg1->imageData + fg1->widthStep*bt))[rt]-
					((float*)(fg1->imageData + fg1->widthStep*bt))[lf-1];
				temp/=sz;
				temp-=((float*)(mean1->imageData + mean1->widthStep*j))[i];
			
				if(((float*)(var1->imageData + var1->widthStep*j))[i]==0)
					((float*)(outcome->imageData + outcome->widthStep*j))[i]=1000;
				else
				{
					temp/=sqrt(((float*)(var1->imageData + var1->widthStep*j))[i]);
				((float*)(outcome->imageData + outcome->widthStep*j))[i]=temp;
				}
				
				
			}
			else if(lf==0&&tp>0)
			{
				temp=((float*)(fg1->imageData + fg1->widthStep*bt))[rt]-
					((float*)(fg1->imageData + fg1->widthStep*(tp-1)))[rt];
				temp/=sz;
				temp-=((float*)(mean1->imageData + mean1->widthStep*j))[i];
				if(((float*)(var1->imageData + var1->widthStep*j))[i]==0)
					((float*)(outcome->imageData + outcome->widthStep*j))[i]=1000;
				else
				{
					temp/=sqrt(((float*)(var1->imageData + var1->widthStep*j))[i]);
				((float*)(outcome->imageData + outcome->widthStep*j))[i]=temp;
				}
				
			}
			else
			{
				temp=((float*)(fg1->imageData + fg1->widthStep*bt))[rt]+
					((float*)(fg1->imageData + fg1->widthStep*(tp-1)))[lf-1]-
					((float*)(fg1->imageData + fg1->widthStep*(tp-1)))[rt]-
					((float*)(fg1->imageData + fg1->widthStep*bt))[lf-1];
				temp/=sz;
				temp-=((float*)(mean1->imageData + mean1->widthStep*j))[i];
				if(((float*)(var1->imageData + var1->widthStep*j))[i]==0)
					((float*)(outcome->imageData + outcome->widthStep*j))[i]=1000;
				else
				{
					temp/=sqrt(((float*)(var1->imageData + var1->widthStep*j))[i]);
				((float*)(outcome->imageData + outcome->widthStep*j))[i]=temp;
				}
			}
		}
	}
	cvReleaseImage(&var1);
	cvReleaseImage(&mean1);
	cvReleaseImage(&fg1);
	return 1;
	
}
short Referee(unsigned int a,unsigned int b, unsigned int c,unsigned int d)
{	
	if(a==b)
	{
		if(b==c)
		{
			if(d==c)
				return 0;
			else
				return 1;
		}
		else
		{
			if(d==b)
				return 2;
			else if(d==c)
				return 3;
			else
				return 4;
		}
	}
	else
	{
		if(c==a)
		{
			if(d==c)
				return 5;
			else if(d==b)
				return 6;
			else return 7;
		}
		else if(c==b)
		{
			if(d==a)
				return 8;
			else if(d==c)
				return 9;
			else return 10;
		}
		else
		{
			if(d==a)
				return 11;
			else if(d==b)
				return 12;
			else if(d==c)
				return 13;
			else return 14;
		}
	}
	
}

//input: the inf of k sufficing len<=2^(2*k),the cell location of the dither matrix
//output: the element corresponding the location in the dither matrix 2^(k)*2^(k),
//if the element is larger than len, the next element is retrieved.
int DitherGen(int k, int loc)
{
	int yloc=loc%(1<<k);
	int xloc=loc>>k;
	yloc^=xloc;
	int res=0;
	int d2i1=0,d2i=0,i=0;
	for(i=k;i>0;--i)
	{
		d2i1=yloc%2;
		d2i=xloc%2;
		res<<=2;
		res+=d2i1*2+d2i;
		xloc>>=1;
		yloc>>=1;	
	}
	return res;
}

//input: U and V are partitions for size objects, and U[i] denotes the class of object i, 0 based.
//R and C are the number of classes in partition U and V. so the largest expected U[i] is R-1.
//output: rand index RI and its expectation EI
double RandIndex(int size,int*U,int R,int*V,int C,double &EI)
{
	int i=0,j=0;
	float*cT=new float[R*C];//Contigency table
	for(i=0;i<R*C;++i)
		cT[i]=0;
	float *UT=new float[R],*VT=new float[C];
	for(i=0;i<R;++i)
		UT[i]=0;
	for(i=0;i<C;++i)
		VT[i]=0;
	double snij2=0,sni2=0,snj2=0;
	double div=(double)size*(size-1)/2;
	for(i=0;i<size;++i)
	{
		++cT[U[i]*C+V[i]];
		++UT[U[i]];
		++VT[V[i]];
	}
	for(i=0;i<R;++i)
	{
		for(j=0;j<C;++j)
		{
			snij2+=cT[i*C+j]*(cT[i*C+j]-1)/2;
		}
		sni2+=UT[i]*(UT[i]-1)/2;
	}
	for(i=0;i<C;++i)
	{
		snj2+=VT[i]*(VT[i]-1)/2;
	}
	EI=((double)(2*sni2*snj2)/(div*div)-(sni2+snj2))/div+1;
	delete[] cT;
	delete[] UT;
	delete[] VT;
	return 1+(double)(2*snij2-sni2-snj2)/div/2;
}


static double* _cv_max_element( double* start, double* end )
{
    double* p = start++;

    for( ; start != end;  ++start) {

        if (*p < *start)   p = start;
    } 

    return p;
}

//paul rosin thresholding for change detection
//  Function cvChangeDetection performs change detection for Foreground detection algorithm
// parameters:
//      prev_frame -
//      curr_frame -
//      change_mask -
 int
cvChangeDetection( IplImage*  prev_frame,
                   IplImage*  curr_frame,
                   IplImage*  change_mask )
{
    int i, j, b, x, y, thres;
    const int PIXELRANGE=256;

    if( !prev_frame
    ||  !curr_frame
    ||  !change_mask
 //   ||   prev_frame->nChannels  != 3
//    ||   curr_frame->nChannels  != 3
    ||   change_mask->nChannels != 1
    ||   prev_frame->depth  != IPL_DEPTH_8U
    ||   curr_frame->depth  != IPL_DEPTH_8U
    ||   change_mask->depth != IPL_DEPTH_8U
    ||   prev_frame->width  != curr_frame->width
    ||   prev_frame->height != curr_frame->height
    ||   prev_frame->width  != change_mask->width
    ||   prev_frame->height != change_mask->height
    ){
        return 0;
    }

    cvZero ( change_mask );

    // All operations per colour
    for (b=0 ; b<prev_frame->nChannels ; b++) 
	{

        // Create histogram:

        long HISTOGRAM[PIXELRANGE]; 
        for (i=0 ; i<PIXELRANGE; i++) HISTOGRAM[i]=0;
        
        for (y=0 ; y<curr_frame->height ; y++)
        {
            uchar* rowStart1 = (uchar*)curr_frame->imageData + y * curr_frame->widthStep + b;
            uchar* rowStart2 = (uchar*)prev_frame->imageData + y * prev_frame->widthStep + b;
            for (x=0 ; x<curr_frame->width ; x++, rowStart1+=curr_frame->nChannels, rowStart2+=prev_frame->nChannels) {
                int diff = abs( int(*rowStart1) - int(*rowStart2) );
                HISTOGRAM[diff]++;
            }
        }

        double relativeVariance[PIXELRANGE];
        for (i=0 ; i<PIXELRANGE; i++) relativeVariance[i]=0;

        for (thres=PIXELRANGE-2; thres>=0 ; thres--)
        {
            //            fprintf(stderr, "Iter %d\n", thres);
            double sum=0;
            double sqsum=0;
            int count=0;
            //            fprintf(stderr, "Iter %d entering loop\n", thres);
            for (j=thres ; j<PIXELRANGE ; j++) {
                sum   += double(j)*double(HISTOGRAM[j]);
                sqsum += double(j*j)*double(HISTOGRAM[j]);
                count += HISTOGRAM[j];
            }
            count = count == 0 ? 1 : count;
            //            fprintf(stderr, "Iter %d finishing loop\n", thres);
            double my = sum / count;
            double sigma = sqrt( sqsum/count - my*my);
            //            fprintf(stderr, "Iter %d sum=%g sqsum=%g count=%d sigma = %g\n", thres, sum, sqsum, count, sigma);
            //            fprintf(stderr, "Writing to %x\n", &(relativeVariance[thres]));
            relativeVariance[thres] = sigma;
            //            fprintf(stderr, "Iter %d finished\n", thres);
        }

        // Find maximum:
        uchar bestThres = 0;

        double* pBestThres = _cv_max_element(relativeVariance, relativeVariance+PIXELRANGE);
        bestThres = (uchar)(*pBestThres); if (bestThres <10) bestThres=10;

        for (y=0 ; y<prev_frame->height ; y++)
        {
            uchar* rowStart1 = (uchar*)(curr_frame->imageData) + y * curr_frame->widthStep + b;
            uchar* rowStart2 = (uchar*)(prev_frame->imageData) + y * prev_frame->widthStep + b;
            uchar* rowStart3 = (uchar*)(change_mask->imageData) + y * change_mask->widthStep;
            for (x = 0; x < curr_frame->width; x++, rowStart1+=curr_frame->nChannels,
                rowStart2+=prev_frame->nChannels, rowStart3+=change_mask->nChannels) {
                // OR between different color channels
                int diff = abs( int(*rowStart1) - int(*rowStart2) );
                if ( diff > bestThres)
                    *rowStart3 |=255;
            }
        }
    }

    return 1;
}


//compute mahalanobis distance for each parcel between feature vectors of time1 and time2, 
//stored in ft1 and ft2, respectively. dimUse denotes dimension of feature vector 
//for distance calculation, method=0  means directly computing distance between corresponding 
//feature vectors, but method=1(obsolete) means the mahal distance for a parcel is the minimum distance
//between the feature vector for this parcel at time1 and the feature vectors for its neighbood 
//parcels at time2 including this parcel.
//note the momory of feat1 and feat2 are connected
/*
int CHC::MahalDist(float*feat1,float*feat2,int count,int dimOn, float* points,int method)
{
	int i,seqnum,subspt,j;
	CvMat *victor[1];
	
	CvMat *covarMatrix;
	CvMat *invertCovarMatrix;
	CvMat *avgVector;
	
	
	CvMat featVec1;
	cvInitMatHeader(&featVec1,count*2,dimOn,CV_32FC1,feat1);

	
	victor[0]=&featVec1;
	covarMatrix=cvCreateMat(dimOn,dimOn,CV_32FC1);
	invertCovarMatrix=cvCreateMat(dimOn,dimOn,CV_32FC1);
	avgVector=cvCreateMat(1,dimOn,CV_32FC1);
	
	cvCalcCovarMatrix((const CvArr **)victor,count*2,covarMatrix,avgVector,CV_COVAR_SCALE|CV_COVAR_NORMAL|CV_COVAR_ROWS);
	cvInvert(covarMatrix,invertCovarMatrix,CV_SVD_SYM);

	CvMat vec1,vec2;

//	CvMat* vec1=cvCreateMat(1,dimOn,CV_32FC1);
//	CvMat* vec2=cvCreateMat(1,dimOn,CV_32FC1);
//	cvSetData(vec1,(feat1+i*dimOn),vec1->step);
//	cvSetData(vec2,(feat2+i*dimOn),vec2->step);
	if(method)
	{
		//initiate and test neighborlist for each segment
		
		DefReg(tag,count);
		ConfirmGridSp(exS,grid);
		
		vector< int > adjObj;//adjacent objects
		float *weighList=NULL;
		for(i=0;i<count;++i)
		{
			cvInitMatHeader(&vec1,1,dimOn,CV_32FC1,(feat1+i*dimOn));			
			seqnum=GetNPLSize(i);			
			adjObj.clear();
			adjObj=vector<int>(seqnum);
			GetNeighList(i,adjObj);
			if(weighList)
				delete []weighList;
			weighList=new float[seqnum+1];
			cvInitMatHeader(&vec2,1,dimOn,CV_32FC1,(feat2+i*dimOn));
			weighList[0]=cvMahalanobis(&vec1,&vec2,invertCovarMatrix);
			for(j=0;j<seqnum;++j)
			{
				subspt=adjObj[j];
				cvInitMatHeader(&vec2,1,dimOn,CV_32FC1,(feat2+subspt*dimOn));
				weighList[j+1]=cvMahalanobis(&vec1,&vec2,invertCovarMatrix);
			}
			points[i]=1e9f;
			for(j=0;j<=seqnum;++j)
				points[i]=__min(points[i],weighList[j]);
		}
		delete[]weighList;
		adjObj.clear();
	}
	else
	{
		for(i=0;i<count;++i)
		{
			cvInitMatHeader(&vec1,1,dimOn,CV_32FC1,(feat1+i*dimOn));
			cvInitMatHeader(&vec2,1,dimOn,CV_32FC1,(feat2+i*dimOn));
			points[i]=cvMahalanobis(&vec1,&vec2,invertCovarMatrix);		
		}
	}
	
	cvReleaseMat(&covarMatrix);
	cvReleaseMat(&invertCovarMatrix);
	cvReleaseMat(&avgVector);
//	cvReleaseMat(&vec1);
//	cvReleaseMat(&vec2);

	return 1;		
}
*/

//for each dimension, estimates {mean1,var1, mean2, var2}
int MyKmeans(float*points,int dim, int*label,int count, float** estimates, int cls)
{
	int i,j;
	CvMat ptSeq;
	cvInitMatHeader(&ptSeq,count,dim,CV_32FC1,points);
	CvMat clusters;
	cvInitMatHeader(&clusters,count,1,CV_32SC1,label);
	cvKMeans2( &ptSeq, cls, &clusters,
		cvTermCriteria( CV_TERMCRIT_ITER,100,1.0));//CV_TERMCRIT_EPS| 1.0

	float *avg=new float[cls*dim];
	float *sum=new float[cls*dim];
	int *ct=new int[cls*dim];
	memset(avg,0,sizeof(float)*cls*dim);
	memset(sum,0,sizeof(float)*cls*dim);
	memset(ct,0,sizeof(int)*cls*dim);
	for(i=0;i<count;++i)
	{
		for(j=0;j<dim;++j)
		{
		avg[label[i]*dim+j]+=points[i*dim+j];
		sum[label[i]*dim+j]+=points[i*dim+j]*points[i*dim+j];
		++ct[label[i]*dim+j];
		}
	}
	for(i=0;i<cls;++i)
	{
		for(j=0;j<dim;++j)
		{
			estimates[j][2*i+1]=(sum[i*dim+j] - (avg[i*dim+j]*avg[i*dim+j])/ct[i*dim+j])/ct[i*dim+j];
			estimates[j][2*i]=avg[i*dim+j]/ct[i*dim+j];
		}
	}
	if(cls==2)
	{
		if(estimates[0][2]>estimates[0][0])
			for(i=0;i<count;++i)
				label[i]=label[i]==0?1:0;
	}
	delete[]avg;
	delete[]sum;
	delete[]ct;

	return 1;
/*	ofstream out("pointind.txt");
	for(i=0;i<count;++i)
	{		
		out<<(clusters.data.i[i]);	
		out<<endl;
	}
	out<<endl;*/
}
// 构造函数，初始化CHC对象的数据
CHC::CHC():Width(0),Height(0),data_(NULL),sData_(NULL),
propData(NULL),tag(NULL),Delta(0),d_(0),comps(0),
typeProp(0),propDim(0),loop(1),minsize(3),K(20.0f),maxDelta(50.f),
wp(0.5f),wc(0.9f),mindiff(20.f),sortDM(true),metric(DISTBENZ)
{
	++Count;
}

// 析构函数
CHC::~CHC()
{
	Clear();
//	cvDestroyWindow("result");
}

// 清除以前的图像数据，并释放内存
void CHC::Clear()
{
	d_=0;//avoid storeseg output spectral data
	if(data_)
	{
		delete[] data_;
		delete[] sData_;	
	}
	data_=NULL;			
	sData_=NULL;
	
	if(S.size())
	{
		S.clear();
		A.clear();
		grid.clear();
	}
	if(tag)
		delete[] tag;
	tag=NULL;
	if(exS.size())
	{
		exS.clear();
		if(propDim)
			delete []propData;
		propData=NULL;
	}
}


inline bool Prior(const NP&a,const NP&b)
	{return a.rInd<b.rInd;}
	void CHC::DistBenz(Region&a,Region&b,unsigned int len,float &ret)
	{
	//	ofstream out("fault.txt",ios::app);
		ret=0;
		//compute sigma first
		float ext=a.size+b.size;
		float trek=0;
		for(int i=0;i<d_;i++)
		{
			trek=a.sSum[i]+b.sSum[i]-square((a.addition[i]+b.addition[i])/ext)*ext;
			if(trek<0)
			{
				assert(abs(trek)<1e-2);
		/*	if(abs(trek)>=0.05f)
				{
					float m=a.sSum[i];
					m=a.addition[i];
					m=b.sSum[i];
					m=b.addition[i];
				}*/
				continue;
			}
			ret+=sqrt(trek/(ext-1));
		}
		ret*=(wc*Delta/d_);//back to normal to follow the invariant color space parctice in eCognition
		unsigned int perim=a.perim+b.perim-2*len;
		ret+=(1-wc)*wp*perim/sqrt(ext);
		CRect c;
		c.UnionRect(a.norbox,b.norbox);
		unsigned int wid=c.Width(),hei=c.Height();
		unsigned int realb=2*(wid+hei);
/*		UINT ba=2*(a.norbox->Width()+a.norbox->Height()),bb=2*(b.norbox->Width()+b.norbox->Height());
		UINT pa=a.perim,pb=b.perim;
		if(((1.f*perim/realb)<(1.f*pa/ba))||((1.f*perim/realb)<(1.f*pb/bb)))
out<<"{perim, realb}*{a+b,a,b}"<<perim<<"\t"<<realb<<"\t"<<pa<<"\t"<<ba<<"\t"<<pb<<"\t"<<bb<<"\n";
		if((1.f*perim/sqrt(ext))<(1.f*pa/sqrt(a.size))||(1.f*perim/sqrt(ext))<(1.f*pb/sqrt(b.size)))
out<<"{perim, ext}*{a+b,a,b}"<<perim<<"\t"<<ext<<"\t"<<pa<<"\t"<<a.size<<"\t"<<pb<<"\t"<<b.size<<"\n";
*/
		assert(perim>=realb);
		ret+=(1-wc)*(1-wp)*perim/realb;
		ret*=ext;
		ret-=(a.interdif+b.interdif);
	}
	//fisher distance is used in "multistage graph-based segmentation of thoracoscopic images"
	//this distance is aimed at 1d intensity data
	void CHC::DistFisher(Region&a,Region&b,unsigned int len,float &ret)
	{
		float as=a.size,bs=b.size;
		float trek=0;
		
		trek=a.sSum[0]/as-square(a.addition[0]/as);
		trek=square(trek*trek);
		ret=b.sSum[0]/bs-square(b.addition[0]/bs);
		ret=square(ret*ret);
		ret+=trek;
		
		ret=sqrt(ret);
		trek=__min(a.perim,b.perim)/(float)len;
		trek*=abs((a.sSum[0])/as-(b.sSum[0])/bs);
		ret*=trek;
		ret*=(Delta*Delta);	//to amplify the result	
	}
	//shape similarity size1/size2*min(perim1,perim2)/common boundary 
	void CHC::DistShape(Region&a,Region&b,unsigned int len,float &ret)
	{
		float as=a.size,bs=b.size;
		float trek=0;
		if(as>bs)
			ret=bs/as;
		else ret=as/bs;
		ret*=__min(a.perim,b.perim)/(float)len;
	}
	//distance used in "implementation of a fast algorithm for segmenting SAR imagery", 
	//later, I added edge weight but now discarded
	void CHC::JhHypo(Region&a,Region&b,unsigned int len,float &ret)
	{//edge weight
		ret=0;
		float as=a.size,bs=b.size;
		//compute sigma first
		for(int i=0;i<d_;++i)
		{
			//here for the quotient of addition sum the fractional part is discarded for small difference
			ret+=square((a.addition[i])/as-(b.addition[i])/bs);
		}
		ret*=(Delta*Delta);
		ret*=(as*bs);
		ret/=(as+bs);
		
	//	ret*=phi;
	//	ret+=(1-phi)*ew;
		ret/=len;
	}
	//for the pixels indexing from xori+uly*w downwards until step.
	//inout:xori uly are the x and y coord of the upper left point to check
	//step is the height for the line of pixels,must be power of 2, rID is the tag of region 
	//which is checking neighbors,sernum is the last void A tuple serno.
	void CHC::NamNeiY(int xori,int uly, int step, int rID,int&sernum)
	{
		int t=tag[uly*Width+xori];
		
		if(step==1)
		{
			NP np(t,sernum);
			S[rID].NPList.push_back(np);
			np.rInd=rID;
			S[t].NPList.push_back(np);
			
			A[sernum].r[0]=rID;
			A[sernum].r[1]=t;
			A[sernum].bl=1;
			++sernum;
			return;
		}
		int hs=step>>1;
		if(t==tag[(uly+hs)*Width+xori])
		{
			NP np(t,sernum);
			S[rID].NPList.push_back(np);
			np.rInd=rID;
			S[t].NPList.push_back(np);
			A[sernum].r[0]=rID;
			A[sernum].r[1]=t;
			A[sernum].bl=step;
			++sernum;
			return;
		}
		else
		{
			NamNeiY(xori,uly,hs,rID,sernum);
			NamNeiY(xori,uly+hs,hs,rID, sernum);
		}
	}
	//improved version namneiy for arbitray positive step this function intended for boundary neighbor initiation
	void CHC::NamNeiYEx(int xori,int uly, int step, int rID,int&sernum)
	{
		int back=step+uly;
		int k=0;
		while(step)
		{
			if(step&1)
			{
				back-=1<<k;
				NamNeiY(xori,back,1<<k,rID,sernum);
			}
			step>>=1;
			++k;
		}
	}
	//checking pixels from index yori*w+ulx to yori*w+lrx for region rID
	void CHC::NamNeiX(int yori,int ulx, int step, int rID, int&sernum)
	{
		int t=tag[yori*Width+ulx];
		if(step==1)
		{
			NP np(t,sernum);
			S[rID].NPList.push_back(np);
			np.rInd=rID;
			S[t].NPList.push_back(np);
			A[sernum].r[0]=rID;
			A[sernum].r[1]=t;
			A[sernum].bl=1;
			++sernum;
			return;
		}
		int hs=step>>1;
		if(t==tag[yori*Width+ulx+hs])
		{
			NP np(t,sernum);
			S[rID].NPList.push_back(np);
			np.rInd=rID;
			S[t].NPList.push_back(np);
			A[sernum].r[0]=rID;
			A[sernum].r[1]=t;
			A[sernum].bl=step;
			++sernum;
			return;
		}
		else
		{
			NamNeiX(yori,ulx,hs,rID, sernum);
			NamNeiX(yori,ulx+hs,hs,rID, sernum);
		}
	}
	void CHC::NamNeiXEx(int yori,int ulx, int step, int rID, int&sernum)
	{
		int back=step+ulx;
		int k=0;
		while(step)
		{
			if(step&1)
			{
				back-=1<<k;
				NamNeiX(yori,back,1<<k,rID,sernum);
			}
			step>>=1;
			++k;
		}
	}
//initiate S[] based on tagMat and parts,note pre: max(tagMat)==parts-1 and min(tagMat)==0;
//post: S[tagMat[i]].p==tagMat[i]
//tagMat is array of labels for each pixel
//the function initiates CHC::grid and CHC::S, 
int CHC::DefReg(int*tagMat,int parts)
{
	comps=parts;
	S=vector<Region>(parts);
	grid=vector<CRect>(parts);
	int label;
	int h=Height;
	int w=Width;
	int comp,compleft,compup,y,x;
	int L=w*h;
	CRect abox(0,0,0,0);
//	assert(data_);
	for(y=0;y<parts;++y)
	{
		S[y].p=y;
		S[y].norbox=&grid[y];
	//	grid[y]=abox;
	}
	if(data_)
	{
		for(y=0;y<parts;++y)
		{
			S[y].addition=data_+y*d_;
			S[y].sSum=sData_+y*d_;
		}
	}

	for (y = 0,label=0; y < h; ++y)
	{
		for (x = 0; x < w; ++x)
		{
			comp =tagMat[label];
	
			abox.left=x;
			abox.right=x+1;
			abox.top=y;
			abox.bottom=y+1;
			if(S[comp].size==0)			
			{
				grid[comp]=abox;
			}
			else
				grid[comp].UnionRect(abox,grid[comp]);
			++(S[comp].size);
	
			
			//initiate neighbor list and perim 
			//note that in each np stores neighbors index and common bl to save memory
			if(x==0&&y==0)//the head pixel
			{				
				S[comp].perim+=2;
				compleft=comp;			
				++label;
				continue;
			}
			if(x>0&&y==0)
			{						
				if(comp!=compleft)
				{	
					NP nl(compleft,1);
					S[comp].NPList.push_back(nl);
					++(S[comp].perim);
					NP nr(comp,1);
					S[compleft].NPList.push_back(nr);
					++S[compleft].perim;
				}
				++S[comp].perim;
				if(x==w-1)
					++S[comp].perim;
				compleft=comp;
				++label;
				continue;
			}
			if(x==0&&y>0)
			{	
				compup=tagMat[label-w];
				if(compup!=comp)
				{	
					NP nu(compup,1);
					S[comp].NPList.push_back(nu);
					++S[comp].perim;
					NP nd(comp,1);
					S[compup].NPList.push_back(nd);
					++S[compup].perim;
				}			
				++S[comp].perim;
				if(y==h-1)
					++S[comp].perim;
				compleft=comp;			
				++label;
				continue;
			}
			compup=tagMat[label-w];
			if(compup==comp&&compleft==comp)
			{				
			}
			else if(compup==comp&&compleft!=comp)
			{
				NP nr(comp,1);
				S[compleft].NPList.push_back(nr);
				++S[compleft].perim;
				NP nl(compleft,1);
				S[comp].NPList.push_back(nl);
				++S[comp].perim;
			}
			else if(compup!=comp&&compleft==comp)
			{
				NP nu(compup,1);
				S[comp].NPList.push_back(nu);
				++S[comp].perim;
				NP nd(comp,1);
				S[compup].NPList.push_back(nd);
				++S[compup].perim;
			}
			else if(compup!=comp&&compleft==compup)
			{
				NP nl(compleft,2);
				S[comp].NPList.push_back(nl);
				S[comp].perim+=2;
				NP nr(comp,2);
				S[compup].NPList.push_back(nr);
				S[compup].perim+=2;
			}
			else
			{
				NP nu(compup,1);
				S[comp].NPList.push_back(nu);
				++S[comp].perim;
				NP nd(comp,1);
				S[compup].NPList.push_back(nd);
				++S[compup].perim;
				NP nl(compleft,1);
				S[comp].NPList.push_back(nl);
				++S[comp].perim;
				NP nr(comp,1);
				S[compleft].NPList.push_back(nr);
				++S[compleft].perim;
			}
			if((x==w-1)||(y==h-1))
			{
				if(x==w-1&&y==h-1)
					S[comp].perim+=2;
				else if(x==w-1&&y!=h-1)
					++S[comp].perim;
				else// if(x!=w-1&&y==h-1)
					++S[comp].perim;
			}
			compleft=comp;			
			++label;
		}//second loop
	} //first loop

	return 1;
}
//assertation function
int CHC::ConfirmGridSp(vector<exRegion>&stigma,vector<CRect>&gridX)
{
	int y;
	int numb=stigma.size();
	assert(numb==S.size());
	assert(numb==grid.size());
	for(y=0;y<numb;++y)
	{
		assert(grid[y].EqualRect(gridX[y]));
/*		if(!grid[y].EqualRect(gridX[y]))
		{
			int tp=grid[y].top;
			tp=gridX[y].top;
			tp=grid[y].bottom;
			tp=gridX[y].bottom;
			tp=grid[y].left;
			tp=gridX[y].left;
			tp=grid[y].right;
			tp=gridX[y].right;
		}*/
/*		assert(S[y].p==stigma[y].label);
		if(S[y].p!=stigma[y].label)
		{
			int x=S[y].p;

			x=stigma[y].label;
		}*/
	}
	return 1;
}
int CHC::GetNeighList(int order,vector<int>&nl)
{
	int nc=S[order].NPList.size();
	assert(nc==nl.size());
	int i;
	for(i=0;i<nc;++i)
	{
		nl[i]=S[order].NPList[i].rInd;		
	}
	return 1;
}
//input: graphbased or any other method seged image tag[L] when tagMat is null, ohterwise tagMat[L] 
//output: S with length comps current region number, grid stores bounding box of each region
//A is allocated large enough to store edges 
//considering 4 connected neighborhood
void CHC::InitiateRegions()
{
	int h=Height;
	int w=Width;
	int y,x;
	int L=w*h;
	int len=__min((2*L-w-h),4*comps);
	NPL::iterator nplp,nptemp,nptp2;
	A=vector<tPair>(len);

	DefReg(tag,comps);
	//initiate A and S.NP.pair index
	int linknum=1;//start from 1, avoid 0 for linknum
	for(x=0;x<comps;++x)
	{
		if(metric==DISTBENZ)
			S[x].InterDiff(d_,wc,wp,Delta);
		sort(S[x].NPList.begin(),S[x].NPList.end(),Prior);//because for list sort doesnot change
		//the position of elements while for vector sort changes the positions of elements, so
		//it's ok to combine this line with the following section for list<NP> but not feasible with vector<NP>
	}
	for(x=0;x<comps;++x)
	{
		nplp=S[x].NPList.begin();
		while(nplp!=S[x].NPList.end())
		{
			y=nplp->rInd;
			if(nplp->pInd<0)//this pair has been visited, so delete its successors with same rind
				//and go to the next neighbor
			{
				nplp->pInd=-(nplp->pInd);
				nptemp=++nplp;
				while(nplp!=S[x].NPList.end()&&(nplp->rInd)==y)
					++nplp;
				nplp = S[x].NPList.erase(nptemp,nplp);
				
			}
			else
			{
				nptemp=nplp;//first item with rInd
				++nplp;
				while(nplp!=S[x].NPList.end()&&(nplp->rInd)==y)
				{	
					nptemp->pInd+=nplp->pInd;
					++nplp;
				}				
				//partner pair initiation notice
				//how to avoid second initiation for region pairs in S[y].Nplist?
				NP temp(x,0);
				nptp2=find(S[y].NPList.begin(),S[y].NPList.end(),temp);
				assert(nptp2!=S[y].NPList.end());
				nptp2->pInd=-linknum;//avoid redundancy visit trick 
				//if any region has pind negative after the for cycle, it signals error.
				//initiate A(unsigned int)
				A[linknum].bl=nptemp->pInd;//note that initially to save footprint store bl in pind
				nptemp->pInd=linknum;
				A[linknum].r[0]=x;
				A[linknum].r[1]=y;
				++linknum;
				nplp=S[x].NPList.erase(++nptemp,nplp);				
			}
		}
	}

	InitializeASM(linknum,comps,GBS);
}
//initiate regions after graph based partition with 8-connectedness, 
//this function is outdated
//interlen or comps denotes current region number which is equal to size of S 
void CHC::InitiateRegions8()
{
	unsigned int label=0;//the serial number
	unsigned int buoy=0;//count the region number
	int h=Height;
	int w=Width;
	int y=0,x=0,d=0;
	unsigned int comp=0,compup=0,compleft=0,compul=0,compd=0;
	int L=w*h;
	int len=__min((2*L-w-h),8*comps);
//	NPL::iterator nplp,nptemp,nptp2;
	A=vector<tPair>(len);
	S=vector<Region>(comps);
	grid=vector<CRect>(comps);

	CRect abox(0,0,0,0);
//	assert(data_);
	for(y=0;y<comps;++y)
	{
		S[y].p=y;
		S[y].addition=data_+y*d_;
		S[y].sSum=sData_+y*d_;
		S[y].norbox=&grid[y];
		grid[y]=abox;
	}

	for (y = 0; y < h; ++y)//each line
	{
		for (x = 0; x < w; ++x)//each pixel
		{
			comp =tag[label];
			abox.left=x;
			abox.right=x+1;
			abox.top=y;
			abox.bottom=y+1;
			if(S[comp].size==0)
			
				grid[comp]=abox;
			
			else			
			grid[comp].UnionRect(abox,grid[comp]);
			++(S[comp].size);
		
			if(x==0&&y==0)//the head pixel
			{				
				S[comp].perim+=2;
				compleft=comp;			
				++label;
				continue;
			}
			if(x>0&&y==0)
			{						
				if(comp!=compleft)
				{	
					NP nl(compleft,1);
					S[comp].NPList.push_back(nl);
					++(S[comp].perim);
					NP nr(comp,1);
					S[compleft].NPList.push_back(nr);
					++S[compleft].perim;
				}
				++S[comp].perim;
				if(x==w-1)
					++S[comp].perim;
				compleft=comp;
				++label;
				continue;
			}	
			
			if(x==0&&y>0)
			{				
				compup=tag[label-w];
				compd=tag[label-w+1];
				if(compd==compup)
				{
					if(comp!=compd)
					{
						NP nd(compd,1);
						S[comp].NPList.push_back(nd);
						++(S[comp].perim);
						NP nb(comp,1);
						S[compd].NPList.push_back(nb);
						++S[compd].perim;
						
					}
				}
				else
				{
					if(comp==compd)
					{
						NP nu(compup,1);
						S[comp].NPList.push_back(nu);
						++(S[comp].perim);
						NP nb(comp,1);
						S[compup].NPList.push_back(nb);
						++S[compup].perim;
						
					}
					else if(comp!=compup&&comp!=compd)
					{
						NP nu(compup,1);
						S[comp].NPList.push_back(nu);
						++(S[comp].perim);
						NP nd(compd,0);
						S[comp].NPList.push_back(nd);
						NP nk(comp,0);
						S[compd].NPList.push_back(nk);
						NP nb(comp,1);
						S[compup].NPList.push_back(nb);
						++S[compup].perim;
						
					}
				}
				
				++S[comp].perim;
				if(y==h-1)
					++S[comp].perim;
				compleft=comp;
				compul=compup;
				compup=compd;
				++label;
				continue;
			}
			
			if(x==w-1&&y>0)
			{				
				switch(Referee(compleft,compul,compup))
				{
				case 1:
					if(comp!=compup)
					{
						NP nr(comp,2);
						S[compup].NPList.push_back(nr);
						S[compup].perim+=2;
						NP nl(compup,2);
						S[comp].NPList.push_back(nl);
						S[comp].perim+=2;
					}
					break;
				case 4:	
				case 10:
					if(comp!=compup&&comp!=compleft)					
					{
						NP nu(compup,1);
						S[comp].NPList.push_back(nu);
						++S[comp].perim;
						NP nd(comp,1);
						S[compup].NPList.push_back(nd);
						++S[compup].perim;
						NP nl(compleft,1);
						S[comp].NPList.push_back(nl);
						++S[comp].perim;
						NP nr(comp,1);
						S[compleft].NPList.push_back(nr);
						++S[compleft].perim;
						
					}
					else
					{
						NP nd(compleft,1);
						S[compup].NPList.push_back(nd);
						++S[compup].perim;
						NP nr(compup,1);
						S[compleft].NPList.push_back(nr);
						++S[compleft].perim;
						
					}
					break;
				case 7:					
					if(comp!=compup&&comp!=compul)					
					{
						NP nu(compup,2);
						S[comp].NPList.push_back(nu);
						S[comp].perim+=2;
						NP nd(comp,2);
						S[compup].NPList.push_back(nd);
						S[compup].perim+=2;
						NP nl(compul,0);
						S[comp].NPList.push_back(nl);
						NP nr(comp,0);
						S[compul].NPList.push_back(nr);
						
					}
					else if(comp==compul)
					{
						NP nl(compleft,2);
						S[comp].NPList.push_back(nl);
						S[comp].perim+=2;
						NP nr(comp,2);
						S[compleft].NPList.push_back(nr);
						S[compleft].perim+=2;
						
					}
					break;				
				case 14:
					if(comp!=compup&&comp!=compul&&comp!=compleft)				
					{
						NP nu(compup,1);
						S[comp].NPList.push_back(nu);
						++S[comp].perim;
						NP nd(comp,1);
						S[compup].NPList.push_back(nd);
						++S[compup].perim;
						NP nl(compleft,1);
						S[comp].NPList.push_back(nl);
						++S[comp].perim;
						NP nr(comp,1);
						S[compleft].NPList.push_back(nr);
						++S[compleft].perim;
						NP nk(compul,0);
						S[comp].NPList.push_back(nk);
						NP nb(comp,0);
						S[compul].NPList.push_back(nb);
						
					}
					else if(comp==compul)
					{
						NP nu(compup,1);
						S[comp].NPList.push_back(nu);
						++S[comp].perim;
						NP nd(comp,1);
						S[compup].NPList.push_back(nd);
						++S[compup].perim;
						NP nl(compleft,1);
						S[comp].NPList.push_back(nl);
						++S[comp].perim;
						NP nr(comp,1);
						S[compleft].NPList.push_back(nr);
						++S[compleft].perim;
						
					}
					break;
				default:
					AfxMessageBox("Error happens when initiate 8 connected regions!");
					break;
				}
				++S[comp].perim;
				if(y==h-1)
					++S[comp].perim;
				
				++label;
				continue;		
				
			}
			compd=tag[label-w+1];
			switch(Referee(compleft,compul,compup,compd))
			{
			case 0:
				if(comp!=compup)
				{
					NP nr(comp,2);
					S[compup].NPList.push_back(nr);
					S[compup].perim+=2;
					NP nl(compup,2);
					S[comp].NPList.push_back(nl);
					S[comp].perim+=2;
					
				}
				break;
			case 1:			
				if(comp!=compup&&comp!=compd)					
				{
					NP nu(compup,2);
					S[comp].NPList.push_back(nu);
					S[comp].perim+=2;
					NP nd(comp,2);
					S[compup].NPList.push_back(nd);
					S[compup].perim+=2;
					NP nl(compd,0);
					S[comp].NPList.push_back(nl);
					NP nr(comp,0);
					S[compd].NPList.push_back(nr);
					
					
				}
				else if(comp==compd)
				{
					NP nu(compup,2);
					S[comp].NPList.push_back(nu);
					S[comp].perim+=2;
					NP nd(comp,2);
					S[compup].NPList.push_back(nd);
					S[compup].perim+=2;
					
				}
				break;
			case 8:
			case 9:
				if(comp!=compup&&comp!=compleft)					
				{
					NP nu(compup,1);
					S[comp].NPList.push_back(nu);
					++S[comp].perim;
					NP nd(comp,1);
					S[compup].NPList.push_back(nd);
					++S[compup].perim;
					NP nl(compleft,1);
					S[comp].NPList.push_back(nl);
					++S[comp].perim;
					NP nr(comp,1);
					S[compleft].NPList.push_back(nr);
					++S[compleft].perim;
					
					
				}
				else
				{
					NP nd(compleft,1);
					S[compup].NPList.push_back(nd);
					++S[compup].perim;
					NP nr(compup,1);
					S[compleft].NPList.push_back(nr);
					++S[compleft].perim;
					
				}
				break;
			case 2:				
			case 3:
				if(comp!=compup&&comp!=compul)					
				{
					NP nu(compup,1);
					S[comp].NPList.push_back(nu);
					++S[comp].perim;
					NP nd(comp,1);
					S[compup].NPList.push_back(nd);
					++S[compup].perim;
					NP nl(compul,1);
					S[comp].NPList.push_back(nl);
					++S[comp].perim;
					NP nr(comp,1);
					S[compul].NPList.push_back(nr);
					++S[compul].perim;
					
				}
				else
				{
					NP nd(compul,1);
					S[compup].NPList.push_back(nd);
					++S[compup].perim;
					NP nr(compup,1);
					S[compul].NPList.push_back(nr);
					++S[compul].perim;
					
				}
				break;
			case 5:				
			case 6:
				if(comp!=compup&&comp!=compul)					
				{
					
					NP nu(compup,1);
					S[comp].NPList.push_back(nu);
					++S[comp].perim;
					NP nd(comp,1);
					S[compup].NPList.push_back(nd);
					++S[compup].perim;
					NP nl(compul,0);
					S[comp].NPList.push_back(nl);
					NP nr(comp,0);
					S[compul].NPList.push_back(nr);
					
				}
				else if(comp==compul)
				{
					NP nd(compul,2);
					S[compup].NPList.push_back(nd);
					S[compup].perim+=2;
					NP nl(compup,2);
					S[compul].NPList.push_back(nl);
					S[compul].perim+=2;
					
				}
				break;
			case 4:
			case 10:
				if(comp==compleft)				
				{
					NP nu(compd,0);
					S[compleft].NPList.push_back(nu);
					
					NP nd(compleft,0);
					S[compd].NPList.push_back(nd);
					
					NP nl(compup,1);
					S[compleft].NPList.push_back(nl);
					++S[compleft].perim;
					NP nr(compleft,1);
					S[compup].NPList.push_back(nr);
					++S[compup].perim;
					
				}
				else if(comp==compd)
				{
					NP nu(compd,1);
					S[compleft].NPList.push_back(nu);
					++S[compleft].perim;
					NP nd(compleft,1);
					S[compd].NPList.push_back(nd);
					++S[compd].perim;
					NP nl(compup,1);
					S[compd].NPList.push_back(nl);
					++S[compd].perim;
					NP nr(compd,1);
					S[compup].NPList.push_back(nr);
					++S[compup].perim;
			
				}
				else if(comp==compup)
				{
					NP nl(compleft,1);
					S[compup].NPList.push_back(nl);
					++S[compup].perim;
					NP nr(compup,1);
					S[compleft].NPList.push_back(nr);
					++S[compleft].perim;
				
				}
				else				
				{
					NP nu(compup,1);
					S[comp].NPList.push_back(nu);
					++S[comp].perim;
					NP nb(comp,1);
					S[compup].NPList.push_back(nb);
					++S[compup].perim;
					NP nd(compd,0);
					S[comp].NPList.push_back(nd);
				//	++S[comp].perim;
					NP nk(comp,0);
					S[compd].NPList.push_back(nk);
					//++S[compd].perim;
					NP nl(compleft,1);
					S[comp].NPList.push_back(nl);
					++S[comp].perim;
					NP nr(comp,1);
					S[compleft].NPList.push_back(nr);
					++S[compleft].perim;
				
				}
				break;		
			case 7:
				if(comp==compul||comp==compd)				
				{
					NP nu(compd,0);
					S[compul].NPList.push_back(nu);
				//	++S[compul].perim;
					NP nd(compul,0);
					S[compd].NPList.push_back(nd);
				//	++S[compd].perim;
					NP nl(compup,2);
					S[comp].NPList.push_back(nl);
					S[comp].perim+=2;
					NP nr(comp,2);
					S[compup].NPList.push_back(nr);
					S[compup].perim+=2;
					
				}
				else if(comp!=compd&&comp!=compul&&comp!=compup)
				{
					NP nu(compup,2);
					S[comp].NPList.push_back(nu);
					S[comp].perim+=2;
					NP nd(comp,2);
					S[compup].NPList.push_back(nd);
					S[compup].perim+=2;
					NP nk(compd,0);
					S[comp].NPList.push_back(nk);
				
					NP nr(comp,0);
					S[compd].NPList.push_back(nr);
					
					NP nl(compul,0);
					S[comp].NPList.push_back(nl);
				
					NP nb(comp,0);
					S[compul].NPList.push_back(nb);
				
				}				
				break;						
				
			case 11:
			case 12:
			case 13:
			/*	if(comp==compleft||comp==compup)
				{
					Blocks[compup].NPList.push_back(compleft]+=1;					
					Blocks[compleft].NPList.push_back(compup]+=1;		
				}
				else if(comp==compul)
				{
					Blocks[compup].NPList.push_back(compul]+=1;					
					Blocks[compul].NPList.push_back(compup]+=1;
					Blocks[compleft].NPList.push_back(compul]+=1;					
					Blocks[compul].NPList.push_back(compleft]+=1;
				}
				else
				{
					Blocks[comp].NPList.push_back(compup]+=1;
					Blocks[comp].NPList.push_back(compul]+=0;
					Blocks[comp].NPList.push_back(compleft]+=1;					
					Blocks[compleft].NPList.push_back(comp]+=1;					
					Blocks[compul].NPList.push_back(comp]+=0;											
					Blocks[compup].NPList.push_back(comp]+=1;	
				}*/
				break;			
			case 14:
			/*	if(comp==compleft)					
				{
					Blocks[compleft].NPList.push_back(compd]+=0;						
					Blocks[compd].NPList.push_back(compleft]+=0;	
					Blocks[compleft].NPList.push_back(compup]+=1;						
					Blocks[compup].NPList.push_back(compleft]+=1;	
				}
				else if(comp==compul)					
				{
					Blocks[compul].NPList.push_back(compd]+=0;					
					Blocks[compd].NPList.push_back(compul]+=0;
					Blocks[compul].NPList.push_back(compleft]+=1;					
					Blocks[compleft].NPList.push_back(compul]+=1;
					Blocks[compul].NPList.push_back(compup]+=1;					
					Blocks[compup].NPList.push_back(compul]+=1;
				}
				else if(comp==compup)
				{					
					Blocks[compleft].NPList.push_back(compup]+=1;					
					Blocks[compup].NPList.push_back(compleft]+=1;
				}
				else if(comp==compd)					
				{
					Blocks[compleft].NPList.push_back(compd]+=1;					
					Blocks[compul].NPList.push_back(compd]+=0;						
					Blocks[compd].NPList.push_back(compleft]+=1;
					Blocks[compd].NPList.push_back(compul]+=0;
					Blocks[compd].NPList.push_back(compup]+=1;					
					Blocks[compup].NPList.push_back(compd]+=1;
				} 
				else 				
				{
					Blocks[comp].NPList.push_back(compup]+=1;
					Blocks[comp].NPList.push_back(compul]+=0;
					Blocks[comp].NPList.push_back(compleft]+=1;
					Blocks[comp].NPList.push_back(compd]+=0;					
					Blocks[compd].NPList.push_back(comp]+=0;					
					Blocks[compleft].NPList.push_back(comp]+=1;						
					Blocks[compul].NPList.push_back(comp]+=0;					
					Blocks[compup].NPList.push_back(comp]+=1;						
				}	*/			
				break;
			default:
				AfxMessageBox("Error happens when initiate 8 connected regions on the normal inner place!");
				break;
			}
			if(y==h-1)
				++S[comp].perim;
			compleft=comp;
			compul=compup;
			compup=compd;
			label++;
		}//second loop
	} //first loop 

	comps=buoy;
}

//This function is to evaluate segmentation following the method in 
//Quantitative evaluation of color image segmentation results
//EvalQS can only be called after regionLabel and before any change of tag[]
//tag[L] corresponds to S[L] in QT seg
float EvalQs(const CHC&sHC,const CString&fn,vector<float>&bWArray)
{	
	GDALDataset*pDataset=(GDALDataset*)GDALOpen(fn,GA_ReadOnly);
	if(sHC.comps<1)
	{
		AfxMessageBox("Segment image before calling EvalQS!");
		return -1.f;
	}
	int spp=pDataset->GetRasterCount();//波段的数目
	int w=sHC.Width,h=sHC.Height,sernum,trans,ext;

	double pedler;
	int d,cur,temp,x,y;

	assert(bWArray.size()==spp);
	
	float** buf=new float*[sHC.d_];
	for(d=0;d<sHC.d_;++d)
		buf[d]=new float[sHC.Width*sHC.Height];

	GDALRasterBand  *m_pBand=NULL;
	float max=0.f;	
	
	for(d=0,cur=0;d<spp;++d)
	{
		if(bWArray[d]==0)
			continue;

		m_pBand= pDataset->GetRasterBand(d+1);
	
//		m_pBand->GetStatistics( 0,  1,&min, &max,0,0);

		if (m_pBand)
		{	
			if (CE_None==m_pBand->RasterIO( GF_Read,0,0, w, h, buf[cur], w,h, GDT_Float32, 0, 0 ))
			{
				assert(sHC.Delta>0);
				temp=0;
				for(y=0;y<sHC.Height;++y)
				{
					for (x = 0; x < sHC.Width; ++x) 
					{
						buf[cur][temp]/=sHC.Delta;					
						++temp;
					}				
				}
			}
		}
		++cur;
	}
	assert(cur==sHC.d_);
	//compute the average of each region



	int*record=new int[sHC.comps]; 
	int*pi=record;//record region number has the same size with prededing ones
	//		record[y].first=y;
	int len=sHC.S.size();
	float eval=0.f;
		
	double *errsum=new double[len];
	memset(errsum,0,sizeof(double)*len);
	int monitor=0;
	for (y = 0,sernum=0; y < h; y++) 
	{
		for (x = 0; x < w; x++) 
		{
			pedler=0;
			trans=sHC.tag[sernum];
			for(d=0;d<sHC.d_;d++)
			{
				pedler+=square(buf[d][sernum]-sHC.S[trans].addition[d]/sHC.S[trans].size);
			}
			errsum[trans]+=sqrt(pedler);
			++sernum;
		}	
	}
	for(x=0,y=0;x<len;++x)
	{
		if(sHC.S[x].p!=x)continue;
		errsum[x]=(1<<16)*square(errsum[x]);
		++y;
	}
	assert(y==sHC.comps);

	for(x=0;x<len;x++)
	{
		if(sHC.S[x].p==x)//reg
		{
			ext=sHC.S[x].size;	
			*(pi)=ext;
			++pi;
			errsum[x]/=(1+log((float)ext));
			eval+=errsum[x];
			//eval+=square(record[ext]/ext);
		}
	}
	assert(pi-record==sHC.comps);
	pi=record;
	sort(record,record+sHC.comps);
	while((pi-record)<sHC.comps)
	{ 
		y=*pi;
		ext=0;
		while(((pi-record)<sHC.comps)&&*pi==y)
		{
			++ext;
			++pi;
		}
		eval+=(double)ext*ext/(y*y);
	}

	eval=eval*sqrt((float)sHC.comps)/(1e4*w*h);
	delete []errsum;
	delete []record;
	for(d=0;d<sHC.d_;++d)
		delete[] buf[d];
	delete[] buf;
	GDALClose((GDALDatasetH)pDataset);
	return eval;
}


//a bug lies in killminion,after graph2tree4, and immediately usage of killminion may lead to some mysterious failure.

//input: tag2[len] in graph based seg or tag[L] for quadtree based seg recording the immediate father indices for each reg(pixel)
//output: tag[L] recording the ultimate father index for each pixel
void CHC::RegionLabel()
{	
	int width = Width;
	int height=Height;
	int L=width*height;
	int sernum=0,label=0;
	int x,y;
	int rec=0,len=S.size();
	if(len<L)
	{
		int *tag2=new int[len];
		//search father for each region
		for (x = 0; x <len; x++) 
		{
			label=x;
			while(label!=S[label].p)
			{
				label=S[label].p;
			}
			tag2[x]=label;
		}

		//father for each pixel
		sernum=0;
		for (y = 0; y <L; ++y) 
		{
		
				label=tag[sernum];
				tag[sernum]=tag2[label];
				++sernum;		
		}
		delete []tag2;
	}
	else
	{		
		for (y = 0; y <L; ++y) 
		{
			
				label=sernum;
				
				while(label!=S[label].p)
				{
					label=S[label].p;
				}
				tag[sernum]=label;
				++sernum;					
		}	
	}

/*	ofstream out("label.txt",ios::app);
	out<<"comps:"<<comps<<"\nregion label\t\tsize in pixel\n";
	vector<int> trap;
	sernum=0;
	for (y = 0; y < L; ++y) 
	{
			//check the size distribution
			if(tag[sernum]==sernum)
				trap.push_back(sernum);
			++sernum;
	}
	//export size distribution
	for(y=0;y<comps;++y)
	{
		out<<trap[y]<<"\t\t"<<S[trap[y]].size<<"\n";
	}	*/
}
//input: original image
//output:tag[L] recording each father label for each pixel and father labels are minimized
//data_ and SUM_ are also shrinked.
int CHC::SegGraph4()
{
	assert(data_);
	int w=Width,h=Height;
	int L=w*h;
	int x,y,d;

	int sernum=0,rec=0;
	float alter=K/Range;
	GraphSeg4(data_,tag,d_,w,h,alter,minsize,comps);

	//minimize label value for each pixel stored in tag array
	for(sernum=0;sernum<L;sernum++)
	{
		if(tag[sernum]==sernum)
		{
			tag[sernum]=(-rec);//discrimate from no seed pixels
			++rec;
		}
	}
	assert(rec==comps);
	for(sernum=0;sernum<L;sernum++)
	{
		if(tag[sernum]<=0)
			tag[sernum]=(-tag[sernum]);//seed pixels
		else
			tag[sernum]=abs(tag[tag[sernum]]);//no seed points
	}
	/*check label with 4 connectedness
	ofstream output4("regche.txt",ios::app);
	for(y=0;y<L;++y)
	{
		if((tag[y]!=tag[y-1])&&(tag[y]!=tag[y+1])&&(tag[y]!=tag[y-w])&&(tag[y]!=tag[y+w]))
			output4<<y<<"\t"<<tag[y]<<"\n";
	}*/
	//rebuilt data_ and sSum_ since the previous is too large and not in tandem with tag
	float *data2=new float[comps*d_];
		float *sData2=new float[comps*d_];
	memset(data2,0,comps*d_*sizeof(float));
	memset(sData2,0,comps*d_*sizeof(float));
	for(sernum=0;sernum<L;sernum++)
	{
		x=sernum*d_;
		y=tag[sernum]*d_;
		for(d=0;d<d_;d++)
		{
			data2[y+d]+=data_[x+d];
			sData2[y+d]+=sData_[x+d];
		}
	}
	delete []data_;
	data_=data2;
	delete []sData_;
	sData_=sData2;
	return comps;
}
//this function initiate regions list for pure hierclust
void CHC::InitiateRegionSet()
{
	int height=Height;
	int width=Width;
	int L=height*width;
	comps=L;
	//if 3d data,the sequence of color is blue green red
	int i,j,y;
	int r,u,w;
	int sernum=0;
	S=vector<Region>(L);
	A=vector<tPair>(2*L-width-height);
	grid=vector<CRect>(L);
	//initiate S
	for (  y = 0; y < L; y++)
	{
			//comp =*(alphaptr+tag);
			tag[y]=y;
			S[y].p=y;
			S[y].addition=data_+y*d_;
			S[y].sSum=sData_+y*d_;
			S[y].size=1;
			S[y].perim=4;
			S[y].interdif=(1-wc)*(1+3*wp);
			i=y%width;
			j=y/width;
			S[y].norbox=&grid[y];
			grid[y].top=j;
			grid[y].bottom=j+1;
			grid[y].left=i;
			grid[y].right=i+1;
			if(i==0)
			{
				if(j==0)
				{
					NP nr(y+1,0);			
					S[y].NPList.push_back(nr);
					w=L-height;				
					NP nb(y+width,w);			
					S[y].NPList.push_back(nb);
				}
				else if(j==height-1)
				{
					r=j*(width-1)+i;
					NP nr(y+1,r);			
					S[y].NPList.push_back(nr);
					u=(height+j-1)*width-height+i;
					NP nt(y-width,u);			
					S[y].NPList.push_back(nt);
				}
				else
				{
					NP nr(y+1,j*(width-1)+i);			
					S[y].NPList.push_back(nr);
					NP nt(y-width,(height+j-1)*width-height+i);			
					S[y].NPList.push_back(nt);
					NP nb(y+width,(height+j)*width-height+i);			
					S[y].NPList.push_back(nb);
				}
			}
			else if(i==width-1)
			{
				if(j==0)
				{
					NP nl(y-1,j*(width-1)+i-1);			
					S[y].NPList.push_back(nl);
					NP nb(y+width,(height+j)*width-height+i);			
					S[y].NPList.push_back(nb);
				}
				else if(j==height-1)
				{
					NP nl(y-1,j*(width-1)+i-1);			
					S[y].NPList.push_back(nl);
					NP nt(y-width,(height+j-1)*width-height+i);			
					S[y].NPList.push_back(nt);
				}
				else
				{
					NP nl(y-1,j*(width-1)+i-1);			
					S[y].NPList.push_back(nl);
					NP nt(y-width,(height+j-1)*width-height+i);			
					S[y].NPList.push_back(nt);
					NP nb(y+width,(height+j)*width-height+i);			
					S[y].NPList.push_back(nb);
				}
			}
			else
			{
				if(j==0)
				{
					NP nl(y-1,j*(width-1)+i-1);			
					S[y].NPList.push_back(nl);
					NP nr(y+1,j*(width-1)+i);			
					S[y].NPList.push_back(nr);
					NP nb(y+width,(height+j)*width-height+i);			
					S[y].NPList.push_back(nb);
				}
				else if(j==height-1)
				{
					NP nl(y-1,j*(width-1)+i-1);			
					S[y].NPList.push_back(nl);
					NP nr(y+1,j*(width-1)+i);			
					S[y].NPList.push_back(nr);
					NP nt(y-width,(height+j-1)*width-height+i);			
					S[y].NPList.push_back(nt);
				}
				else
				{
					NP nl(y-1,j*(width-1)+i-1);			
					S[y].NPList.push_back(nl);
					NP nr(y+1,j*(width-1)+i);			
					S[y].NPList.push_back(nr);
					NP nt(y-width,(height+j-1)*width-height+i);			
					S[y].NPList.push_back(nt);
					NP nb(y+width,(height+j)*width-height+i);			
					S[y].NPList.push_back(nb);
				}
			}			
	}
	//initiate A
	for(y=0;y<L;y++)
	{
		i=y%width;
		j=y/width;
		if(i==width-1)
		{
			if(j==height-1)
			{			
			}
			else
			{				
				w=(height+j)*width-height+i;
				A[w].r[0]=y;
				A[w].r[1]=y+width;
				A[w].bl=1;	
				
			}
		}
		else
		{
			if(j==height-1)
			{				
				r=j*(width-1)+i;
				A[r].r[0]=y;
				A[r].r[1]=y+1;
				A[r].bl=1;
			}
			else
			{
				r=j*(width-1)+i;
				A[r].r[0]=y;
				A[r].r[1]=y+1;
				A[r].bl=1;
				
				w=(height+j)*width-height+i;
				A[w].r[0]=y;
				A[w].r[1]=y+width;
				A[w].bl=1;	
			}			
		}		
	}
	y=2*width*height-width-height;
	InitializeASM(y,L,HC);
}
//input: S[comps],tag[comps]
//output:S[comps] with only loop regions which father regs bear the same label as index,
//tag[comps] new recording labels for each comp
int CHC::HierClust()
{
	sl::iterator iter;
	NPL::iterator lit,ltemp;
	int sernum=0;
	int ql,pl,temp;
	int x,y,i,u;//w,j
	double mini=0;
	NP np;
	_MC mc;
	while(comps>loop)
	{
		//start merge
//		int charlie=MCL.size();
		assert(MCL.size());
		iter=MCL.begin();
		sernum=iter->pInd;
		pl=A[sernum].r[0];
		ql=A[sernum].r[1];
		
//		assert(A[sernum].t>-1);//when using distbenz A.t is not sure to be positive

		if(pl>ql)
		{
			temp=pl;
			pl=ql;
			ql=temp;
		}
		S[pl].size+=S[ql].size;
		for(x=0;x<d_;++x)
		{
			S[pl].addition[x]+=S[ql].addition[x];
			S[pl].sSum[x]+=S[ql].sSum[x];
		}
		S[pl].perim+=S[ql].perim-2*A[sernum].bl;
		S[ql].p=pl;

		S[pl].norbox->UnionRect(S[ql].norbox,S[pl].norbox);
		S[pl].interdif+=(A[sernum].t+S[ql].interdif);
		//take care the neighbors affected by merge pl and ql;
		
		//take care neighbors of ql
		lit=S[ql].NPList.begin();
		while(lit!=S[ql].NPList.end())
		{
			if(lit->rInd==pl)
			{
				lit=S[ql].NPList.erase(lit);
				continue;
			}
			if((ltemp=find(S[pl].NPList.begin(),S[pl].NPList.end(),*lit))!=S[pl].NPList.end())
			{
				A[ltemp->pInd].bl+=A[lit->pInd].bl;
//				A[ltemp->pInd].es+=A[lit->pInd].es;//this line for edge strength
				np.rInd=ql;
				S[lit->rInd].NPList.erase(remove(S[lit->rInd].NPList.begin(),S[lit->rInd].NPList.end(),np),S[lit->rInd].NPList.end());
			}
			else
			{
				if(A[lit->pInd].r[0]==ql)
					A[lit->pInd].r[0]=pl;
				else //if(A[lit->pInd].r[1]==ql)
					A[lit->pInd].r[1]=pl;
				//else
				//	assert(0);
				S[pl].NPList.push_back(*lit);
				np.rInd=ql;
				ltemp=find(S[lit->rInd].NPList.begin(),S[lit->rInd].NPList.end(),np);
				ltemp->rInd=pl;
				assert(ltemp->pInd==lit->pInd);
			}			
			lit++;
		}
		//take care neighbors of pl and MCL
		lit=S[pl].NPList.begin();
		while(lit!=S[pl].NPList.end())
		{
			if(lit->rInd==ql)
			{				
				lit = S[pl].NPList.erase(lit);
				continue;
			}
			assert((A[lit->pInd].r[0]==pl&&A[lit->pInd].r[1]==lit->rInd)||(A[lit->pInd].r[0]==lit->rInd&&A[lit->pInd].r[1]==pl));
			switch (metric)
			{
			case DISTBENZ:
			default:
				DistBenz(S[pl],S[lit->rInd],A[lit->pInd].bl,A[lit->pInd].t);
				break;
			case DISTLSCHD:
				JhHypo(S[pl],S[lit->rInd],A[lit->pInd].bl,A[lit->pInd].t);
				break;
			case DISTFISHER:
				DistFisher(S[pl],S[lit->rInd],A[lit->pInd].bl,A[lit->pInd].t);
				break;
			}
			x=S[lit->rInd].bestp;
			if(A[x].r[0]==pl||A[x].r[1]==pl||A[x].r[0]==ql||A[x].r[1]==ql)
			{
				mini=1e20;
				for(ltemp=S[lit->rInd].NPList.begin();ltemp!=S[lit->rInd].NPList.end();++ltemp)
				{
					if(mini>A[ltemp->pInd].t)
					{
						mini=A[ltemp->pInd].t;
						S[lit->rInd].bestp=ltemp->pInd;
					}
					else if(mini==A[ltemp->pInd].t)
					{
						i=S[lit->rInd].bestp;
						//j=A[i].r[0]==lit->rInd?A[i].r[1]:A[i].r[0];
						u=ltemp->pInd;
						//w=A[u].r[0]==lit->rInd?A[u].r[1]:A[u].r[0];
						if(i>u)//(j>w)
							S[lit->rInd].bestp=u;//lit->pInd;
					}
				}
			}
			else
			{
				if(A[x].t>A[lit->pInd].t)					
				{
					//it's possible that the pair does not exist in merge candidate list
					//but it's safe to delete a _MC not exist!
					mc.pInd=x;
					mc.t=A[x].t;
					MCL.erase(mc);
					S[lit->rInd].bestp=lit->pInd;
				}
				else if(A[x].t==A[lit->pInd].t)
				{
					
					//j=A[x].r[0]==lit->rInd?A[x].r[1]:A[x].r[0];
					u=lit->pInd;
					//w=A[u].r[0]==lit->rInd?A[u].r[1]:A[u].r[0];
					if(x>u)//(j>w)
					{
						mc.pInd=x;
						mc.t=A[x].t;
						MCL.erase(mc);
						S[lit->rInd].bestp=u;//lit->pInd;
					}
				}
			}
			++lit;
		}
		//compute bestp for pl
		mini=1e20;
		for(ltemp=S[pl].NPList.begin();ltemp!=S[pl].NPList.end();ltemp++)
		{
			if(mini>A[ltemp->pInd].t)
			{
				mini=A[ltemp->pInd].t;
				S[pl].bestp=ltemp->pInd;
			}
			else if(mini==A[ltemp->pInd].t)
			{
				i=S[pl].bestp;
				//j=A[i].r[0]==pl?A[i].r[1]:A[i].r[0];
				u=ltemp->pInd;
				//w=A[u].r[0]==pl?A[u].r[1]:A[u].r[0];
				if(i>u)//(j>w)
					S[pl].bestp=u;//lit->pInd;
			}
		}
		//update MCL this procedure might be implemented more efficient but more tricky.
		//for all pl's neighbors AND PL decide the mutual best pairs
		x=S[pl].bestp;
		y=A[x].r[0]==pl?A[x].r[1]:A[x].r[0];
		if(S[y].bestp==x)
		{
			mc.pInd=x;
			mc.t=A[x].t;
			MCL.insert(mc);
		}
		lit=S[pl].NPList.begin();
		while(lit!=S[pl].NPList.end())
		{
			x=S[lit->rInd].bestp;
			y=A[x].r[0]==lit->rInd?A[x].r[1]:A[x].r[0];
			if(S[y].bestp==x)
			{
				mc.pInd=x;
				mc.t=A[x].t;
				MCL.insert(mc);
			}			
			lit++;
		}
		//test
	/*		ofstream output("C://reg9pixel.txt",ios::app);
	if(!output){
		AfxMessageBox("Can't open reg.txt!");
		return 1;
	}
	output<<"label\t"<<"size\t"<<"perim\t"<<"bestp\t"<<"interdif\t"<<"neigh(rInd,pInd)\t\t\t\t"<<"bl\t"<<endl;
	for(x=0;x<Height*Width;x++)
		S[x].Print("C://reg9pixel.txt");
			ofstream output2("C://pairs.txt",ios::app);
	output2<<"reg1\t"<<"reg2\t"<<"bl\t"<<"t\t"<<endl;
	for(x=0;x<(2*Width*Height-(Width+Height));x++)
	{
		output2<<A[x].r[0]<<"	"<<A[x].r[1]<<"	"<<A[x].bl<<"	"<<A[x].t<<endl;
	}*/
	//test
		MCL.erase(iter);
		--comps;
	}

	return 0;
}
//build quad tree for arbitrary size image
void CHC::BQT(int s, QT *parent)
{
	if(s==1)
	{
		++comps;
		return;
	}
	int cx=parent->ulx;
	int cy=parent->uly;
	int m=parent->lm;
	int n=parent->un;

	if(Predicate(m,n,cx,cy))
	{
		++comps;
		return;//predicate
	}
	int shift=0,temp=0;
	int pi=parent->index;
	if(pi==1)
	{
		shift=(int)ceil(log((double)m)/log(2.0));
		temp=(int)ceil(log((double)n)/log(2.0));
		shift=shift>temp?shift:temp;
		s=1<<shift;
	}
	int hs=s>>1;//half s
	int i=0;
	if(hs>=m&&hs<n)
	{
		QT*nw=new QT;
		nw->index=pi*4+1;
		nw->ulx=cx;
		nw->uly=cy;
		nw->lm=m;
		nw->un=hs;
		nw->parent=parent;
		for(i=0;i<4;i++)
		nw->child[i]=NULL;
		parent->child[1]=nw;
		BQT(hs,nw);
		QT*ne=new QT;		
		ne->index=pi*4;
		ne->ulx=cx+hs;
		ne->uly=cy;
		ne->lm=m;
		ne->un=n-hs;
		ne->parent=parent;
		for(i=0;i<4;i++)
		ne->child[i]=NULL;
		parent->child[0]=ne;
		BQT(hs,ne);
		return;
	}
	else if(hs<m&&hs>=n)
	{
		QT*nw=new QT;
		nw->index=pi*4+1;
		nw->ulx=cx;
		nw->uly=cy;
		nw->lm=hs;
		nw->un=n;
		nw->parent=parent;
		for(i=0;i<4;i++)
		nw->child[i]=NULL;
		parent->child[1]=nw;
		BQT(hs,nw);
		QT*sw=new QT;
		sw->index=pi*4+2;
		sw->ulx=cx;
		sw->uly=cy+hs;
		sw->lm=m-hs;
		sw->un=n;
		sw->parent=parent;
		for(i=0;i<4;i++)
		sw->child[i]=NULL;
		parent->child[2]=sw;
		BQT(hs,sw);
		return;
	}
	else if(hs>=m&&hs>=n)
	{
		QT*nw=new QT;
		nw->index=pi*4+1;
		nw->ulx=cx;
		nw->uly=cy;
		nw->lm=m;
		nw->un=n;
		nw->parent=parent;
		for(i=0;i<4;i++)
		nw->child[i]=NULL;
		parent->child[1]=nw;
		BQT(hs,nw);
		return;
	}
	else
	{
		QT*nw=new QT[4];
		nw->index=pi*4+1;
		nw->ulx=cx;
		nw->uly=cy;
		nw->lm=hs;
		nw->un=hs;
		nw->parent=parent;
		for(i=0;i<4;i++)
		nw->child[i]=NULL;
		parent->child[1]=nw;
		BQT(hs,nw);
		QT*sw=new QT;
		sw->index=pi*4+2;
		sw->ulx=cx;
		sw->uly=cy+hs;
		sw->lm=m-hs;
		sw->un=hs;
		sw->parent=parent;
		for(i=0;i<4;i++)
		sw->child[i]=NULL;
		parent->child[2]=sw;
		BQT(hs,sw);
		QT*se=new QT;
		se->index=pi*4+3;
		se->ulx=cx+hs;
		se->uly=cy+hs;
		se->lm=m-hs;
		se->un=n-hs;
		se->parent=parent;
		for(i=0;i<4;i++)
		se->child[i]=NULL;
		parent->child[3]=se;
		BQT(hs,se);
		QT*ne=new QT;
		ne->index=pi*4;
		ne->ulx=cx+hs;
		ne->uly=cy;
		ne->lm=hs;
		ne->un=n-hs;
		ne->parent=parent;
		for(i=0;i<4;i++)
		ne->child[i]=NULL;
		parent->child[0]=ne;
		BQT(hs,ne);
		return;
	}
}
//if sum of maxdelta is less than episilon, return true
//input: valid corners if only two, then ll and lr are -1, if four, name denotes the locations.
//szi denotes the size of four regions lying in the four quadrants
bool CHC::Predicate(int ul,int sz1,int ur,int sz2,int ll,int sz3,int lr,int sz4)
{
	int d=0;//counter
	float res=0;
	if(ll==-1)
	{
		ul*=d_;
		ur*=d_;
		
		float* avr=new float[2];
		
		for(d=0;d<d_;d++)
		{
			avr[0]=(float)data_[ul+d]/sz1;
			avr[1]=(float)data_[ur+d]/sz2;
			res+=2*abs(avr[1]-avr[0]);
		}
		delete []avr;
		return res<=maxDelta/Range;
	}
	ul*=d_;
	ur*=d_;
	ll*=d_;
	lr*=d_;
	
	float* avr=new float[4];
	for(d=0;d<d_;d++)
	{
		avr[0]=(float)data_[ul+d]/sz1;
		avr[1]=(float)data_[ur+d]/sz2;
		avr[2]=(float)data_[ll+d]/sz3;
		avr[3]=(float)data_[lr+d]/sz4;
		
		sort(avr,avr+4);
		res+=avr[3]+avr[2]-avr[1]-avr[0];
	}
	delete []avr;
	return res<=maxDelta/Range;
}

void CHC::ShowQT(QT*r)
{
	if(r==NULL)return;
	r->Print();
	int i=0;
	for(i=0;i<4;i++)
	{
		if(r->child[i])
		ShowQT(r->child[i]);
	}
}

void CHC::ElimTree(QT *r)
{	
	for(int i=0;i<4;i++)
	{
		if(r->child[i])
			ElimTree(r->child[i]);
	}
	delete r;
}
//input: original image 
//note now this function is paralysed CRect grid[] should be refilled when time allowed!
//output:quadtree segmented image updated comps,data_, sSum_,A initialized
// S and tag with size L for each pixel,A start with 0,tag array stores the father pixel index

void CHC::QTMerge()
{
	int dim=d_;
	int h=Height,w=Width;
	int L=h*w;
	comps=L;
	int x,y,d;
	int sernum=0;
	S=vector<Region>(L);
	A=vector<tPair>(2*L-w-h);
	grid=vector<CRect>(L);//store boundary information for each primitive region
	memset(tag,-1,L*sizeof(int));
	int i=0,j=0,sz=0;
	int rec=0;//current number of segments
	int step=0,hs=0,hs2=0;//edge length of blocks checked in each loop and half step
	
	int flag=1,temp=0;
	int limx,limy;
	int ul,ur,ll,lr;//four upper left corner point index in four quadrant
	static int proxy=0;
	//quadtree merge main section and Regions initiation
	while(flag>0)//some blocks have not yet been assigned to region
	{
		flag=0;
		++temp;
		step=1<<temp;
		hs=step>>1;
		hs2=hs*hs;
		limy=(int)ceil((double)h/step)*step;
		limx=(int)ceil((double)w/step)*step;
		for(y=0;y<limy-step;y+=step)
		{
			for(x=0;x<limx-step;x+=step)
			{
				//	CheckBlock(x,y,step);
				ul=y*w+x;
				ur=y*w+x+hs;
				ll=(y+hs)*w+x;
				lr=(y+hs)*w+x+hs;
				//if this block is not contaminated, it has chance to be merged
				if(tag[ul]==-1&&tag[ur]==-1&&tag[ll]==-1&&tag[lr]==-1&&Predicate(ul,hs2,ur,hs2,ll,hs2,lr,hs2))
				{
					//predicate is met
					for(d=0;d<dim;++d)
					{
						data_[ul*dim+d]+=(data_[ur*dim+d]+data_[ll*dim+d]+data_[lr*dim+d]);
						sData_[ul*dim+d]+=(sData_[ur*dim+d]+sData_[ll*dim+d]+sData_[lr*dim+d]);
					}
					++flag;
				}
				//this block contains some component has upper left corner pixel labelled or not pure
				else
				{
					//if tA is -1 which means the quadrant is not yet assigned to region,so do it
					if(tag[ul]==-1)
					{
						S[ul].addition=data_+ul*dim;
						S[ul].sSum=sData_+ul*dim;
						S[ul].perim=2*step;
						grid[ul].top=y;
						grid[ul].bottom=y+hs;
						grid[ul].left=x;
						grid[ul].right=x+hs;
						S[ul].norbox=&grid[ul];
						S[ul].size=hs2;
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
						S[ur].addition=data_+ur*dim;
						S[ur].sSum=sData_+ur*dim;
						S[ur].perim=2*step;
						grid[ur].top=y;
						grid[ur].bottom=y+hs;
						grid[ur].left=x+hs;
						grid[ur].right=x+step;
						S[ur].norbox=&grid[ur];
						S[ur].size=hs2;
						proxy=ur;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						S[ll].addition=data_+ll*dim;
						S[ll].sSum=sData_+ll*dim;
						S[ll].perim=2*step;
						grid[ll].top=y+hs;
						grid[ll].bottom=y+step;
						grid[ll].left=x;
						grid[ll].right=x+hs;
						S[ll].norbox=&grid[ll];
						S[ll].size=hs2;
						proxy=ll;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					if(tag[lr]==-1)
					{
						S[lr].addition=data_+lr*dim;
						S[lr].sSum=sData_+lr*dim;
						S[lr].perim=2*step;
						grid[lr].top=y+hs;
						grid[lr].bottom=y+step;
						grid[lr].left=x+hs;
						grid[lr].right=x+step;
						S[lr].norbox=&grid[lr];
						S[lr].size=hs2;
						proxy=lr;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=lr;
							proxy+=w;
						}
						++rec;
					}	
				}
			}
		}
		assert(y==limy-step);

		//for blocks lying on the boundary first check the downside boundary and use y 
		//with the same value gotten from above
		for(x=0;x<limx-step;x+=step)
		{
			//if only two component exist
			if(y+hs>=h)
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				//if two small blocks are pure, it is possible a new pure block can be founded
				if(tag[ul]==-1&&tag[ur]==-1&&Predicate(ul,(h-y)*hs,ur,(h-y)*hs))
				{
					for(d=0;d<dim;++d)
					{
						data_[ul*dim+d]+=data_[ur*dim+d];
						sData_[ul*dim+d]+=sData_[ur*dim+d];
					}
					++flag;
					
				}
				//if two blocks contains labeled pixel
				else
				{
					if(tag[ul]==-1)
					{
						S[ul].addition=data_+ul*dim;
							S[ul].sSum=sData_+ul*dim;
						S[ul].perim=2*(h-y)+step;
						grid[ul].top=y;
						grid[ul].bottom=h;
						grid[ul].left=x;
						grid[ul].right=x+hs;
						S[ul].norbox=&grid[ul];
						S[ul].size=(h-y)*hs;
						proxy=ul;
						for(j=0;j<h-y;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
						S[ur].addition=data_+ur*dim;
							S[ur].sSum=sData_+ur*dim;
						S[ur].perim=2*(h-y)+step;
						grid[ur].top=y;
						grid[ur].bottom=h;
						grid[ur].left=x+hs;
						grid[ur].right=x+step;
						S[ur].norbox=&grid[ur];
						S[ur].size=hs*(h-y);
						proxy=ur;
						for(j=0;j<h-y;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					
				}
			}
			else// if y+hs<h, four blocks should be considered just as the beignning
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				ll=(y+hs)*w+x;
				lr=(y+hs)*w+x+hs;
				//if this block is not contaminated, it has chance to be merged
				if(tag[ul]==-1&&tag[ur]==-1&&tag[ll]==-1&&tag[lr]==-1&&Predicate(ul,hs2,ur,hs2,ll,hs*(h-hs-y),lr,hs*(h-hs-y)))
				{
					//predicate is met
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=(data_[ur*dim+d]+data_[ll*dim+d]+data_[lr*dim+d]);
						sData_[ul*dim+d]+=(sData_[ur*dim+d]+sData_[ll*dim+d]+sData_[lr*dim+d]);
					}
					++flag;
				}
				else//this block contains some component has upper left corner pixel labelled 
				{
					//if tA is -1 which means the quadrant is not yet assigned to region,so do it
					if(tag[ul]==-1)
					{
						S[ul].addition=data_+ul*dim;
							S[ul].sSum=sData_+ul*dim;
						S[ul].perim=2*step;
						grid[ul].top=y;
						grid[ul].bottom=y+hs;
						grid[ul].left=x;
						grid[ul].right=x+hs;
						S[ul].norbox=&grid[ul];
						S[ul].size=hs2;
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
						S[ur].addition=data_+ur*dim;
							S[ur].sSum=sData_+ur*dim;
						S[ur].perim=2*step;
						grid[ur].top=y;
						grid[ur].bottom=y+hs;
						grid[ur].left=x+hs;
						grid[ur].right=x+step;
						S[ur].norbox=&grid[ur];
						S[ur].size=hs2;
						proxy=ur;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						S[ll].addition=data_+ll*dim;
							S[ll].sSum=sData_+ll*dim;
						S[ll].perim=2*(h-y-hs)+step;
						grid[ll].top=y+hs;
						grid[ll].bottom=h;
						grid[ll].left=x;
						grid[ll].right=x+hs;
						S[ll].norbox=&grid[ll];
						S[ll].size=hs*(h-y-hs);
						proxy=ll;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					if(tag[lr]==-1)
					{
						S[lr].addition=data_+lr*dim;
							S[lr].sSum=sData_+lr*dim;
						S[lr].perim=2*(h-y-hs)+step;
						grid[lr].top=y+hs;
						grid[lr].bottom=h;
						grid[lr].left=x+hs;
						grid[lr].right=x+step;
						S[lr].norbox=&grid[lr];
						S[lr].size=hs*(h-y-hs);
						proxy=lr;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=lr;
							proxy+=w;
						}
						++rec;
					}	
				}//end else four block are assigned region label
			}//end else for y+hs<h
		}//end for each block lying on the bottom from left to right
		assert(x==limx-step);
		//check blocks lying to right side of the boundary and the bottom one is not included just as above
		for(y=0;y<limy-step;y+=step)
		{
			if(x+hs>=w)
			{
				ul=y*w+x;
				ll=(y+hs)*w+x;
				//if two small blocks are pure, it is possible a new pure block can be founded
				if(tag[ul]==-1&&tag[ll]==-1&&Predicate(ul,hs*(w-x),ll,hs*(w-x)))
				{					
					for(d=0;d<dim;++d)
					{
						data_[ul*dim+d]+=data_[ll*dim+d];
						sData_[ul*dim+d]+=sData_[ll*dim+d];
					}
					++flag;
					
				}
				//if two blocks contains labeled pixel or heterogeneous
				else
				{
					if(tag[ul]==-1)
					{
						S[ul].addition=data_+ul*dim;
							S[ul].sSum=sData_+ul*dim;
						S[ul].perim=step+2*(w-x);
						grid[ul].top=y;
						grid[ul].bottom=y+hs;
						grid[ul].left=x;
						grid[ul].right=w;
						S[ul].norbox=&grid[ul];
						S[ul].size=hs*(w-x);
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						S[ll].addition=data_+ll*dim;
							S[ll].sSum=sData_+ll*dim;
						S[ll].perim=2*(w-x)+step;
						grid[ll].top=y+hs;
						grid[ll].bottom=y+step;
						grid[ll].left=x;
						grid[ll].right=w;
						S[ll].norbox=&grid[ll];
						S[ll].size=hs*(w-x);
						proxy=ll;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					
				}
			}
			else// if y+hs<h, four blocks should be considered just as the beignning
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				ll=(y+hs)*w+x;
				lr=(y+hs)*w+x+hs;
				//if this block is not contaminated, it has chance to be merged
				if(tag[ul]==-1&&tag[ur]==-1&&tag[ll]==-1&&tag[lr]==-1&&Predicate(ul,hs2,ur,hs*(w-hs-x),ll,hs2,lr,hs*(w-hs-x)))
				{
					//predicate is met
					
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=(data_[ur*dim+d]+data_[ll*dim+d]+data_[lr*dim+d]);
						sData_[ul*dim+d]+=(sData_[ur*dim+d]+sData_[ll*dim+d]+sData_[lr*dim+d]);
					}
					++flag;
					
					
				}
				//this block contains some component has upper left corner pixel labelled 
				else
				{
					//if tA is -1 which means the quadrant is not yet assigned to region,so do it
					if(tag[ul]==-1)
					{
						S[ul].addition=data_+ul*dim;
							S[ul].sSum=sData_+ul*dim;
						S[ul].perim=2*step;
						grid[ul].top=y;
						grid[ul].bottom=y+hs;
						grid[ul].left=x;
						grid[ul].right=x+hs;
						S[ul].norbox=&grid[ul];
						S[ul].size=hs2;
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
						S[ur].addition=data_+ur*dim;
							S[ur].sSum=sData_+ur*dim;
						S[ur].perim=2*(w-x-hs)+step;
						grid[ur].top=y;
						grid[ur].bottom=y+hs;
						grid[ur].left=x+hs;
						grid[ur].right=w;
						S[ur].norbox=&grid[ur];
						S[ur].size=hs*(w-x-hs);
						proxy=ur;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						S[ll].addition=data_+ll*dim;
							S[ll].sSum=sData_+ll*dim;
						S[ll].perim=2*step;
						grid[ll].top=y+hs;
						grid[ll].bottom=y+step;
						grid[ll].left=x;
						grid[ll].right=x+hs;
						S[ll].norbox=&grid[ll];
						S[ll].size=hs2;
						proxy=ll;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					if(tag[lr]==-1)
					{
						S[lr].addition=data_+lr*dim;
							S[lr].sSum=sData_+lr*dim;
						S[lr].perim=2*(w-x-hs)+step;
						grid[lr].top=y+hs;
						grid[lr].bottom=y+step;
						grid[lr].left=x+hs;
						grid[lr].right=w;
						S[lr].norbox=&grid[lr];
						S[lr].size=hs*(w-x-hs);
						proxy=lr;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=lr;
							proxy+=w;
						}
						++rec;
					}	
				}
				
			}
			
		}
		assert(x==limx-step);
		assert(y==limy-step);

		if(x+hs>=w)
		{
			if(y+hs<h)//two blocks are considered
			{
				ul=y*w+x;
				ll=(y+hs)*w+x;
				//if two small blocks are pure, it is possible a new pure block can be founded
				if(tag[ul]==-1&&tag[ll]==-1&&Predicate(ul,hs*(w-x),ll,(w-x)*(h-hs-y)))
				{
					
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=data_[ll*dim+d];
						sData_[ul*dim+d]+=sData_[ll*dim+d];
					}
					++flag;
					
				}
				//if two blocks contains labeled pixel
				else
				{
					if(tag[ul]==-1)
					{
						S[ul].addition=data_+ul*dim;
							S[ul].sSum=sData_+ul*dim;
						S[ul].perim=step+2*(w-x);
						grid[ul].top=y;
						grid[ul].bottom=y+hs;
						grid[ul].left=x;
						grid[ul].right=w;
						S[ul].norbox=&grid[ul];
						S[ul].size=hs*(w-x);
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						
						S[ll].addition=data_+ll*dim;
							S[ll].sSum=sData_+ll*dim;
						S[ll].perim=2*(w-x)+2*(h-y-hs);
						grid[ll].top=y+hs;
						grid[ll].bottom=h;
						grid[ll].left=x;
						grid[ll].right=w;
						S[ll].norbox=&grid[ll];
						S[ll].size=(h-y-hs)*(w-x);
						proxy=ll;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<w-x;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					
				}
			}
			else//only one component exists this block must have been check before by predicate
			{//no addition and square sum is needed as both are computed already
				ul=y*w+x;
				if(tag[ul]==-1)
				{
					++flag;
				}
			}
				
		}		
		else//if x+hs<w
		{
			if(y+hs<h)//four blocks case
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				ll=(y+hs)*w+x;
				lr=(y+hs)*w+x+hs;
				//if this block is not contaminated, it has chance to be merged
				if(tag[ul]==-1&&tag[ur]==-1&&tag[ll]==-1&&tag[lr]==-1&&Predicate(ul,hs2,ur,hs*(w-x-hs),ll,(h-y-hs)*(hs),lr,(h-y-hs)*(w-x-hs)))
				{
					//predicate is met
					
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=(data_[ur*dim+d]+data_[ll*dim+d]+data_[lr*dim+d]);
						sData_[ul*dim+d]+=(sData_[ur*dim+d]+sData_[ll*dim+d]+sData_[lr*dim+d]);
					}
					++flag;
				}
				//this block contains some component has upper left corner pixel labelled 
				else
				{
					//if tA is -1 which means the quadrant is not yet assigned to region,so do it
					if(tag[ul]==-1)
					{
						S[ul].addition=data_+ul*dim;
							S[ul].sSum=sData_+ul*dim;
						S[ul].perim=2*step;
						grid[ul].top=y;
						grid[ul].bottom=y+hs;
						grid[ul].left=x;
						grid[ul].right=x+hs;
						S[ul].norbox=&grid[ul];
						S[ul].size=hs2;
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
						S[ur].addition=data_+ur*dim;
							S[ur].sSum=sData_+ur*dim;
						S[ur].perim=2*(w-x-hs)+step;
						grid[ur].top=y;
						grid[ur].bottom=y+hs;
						grid[ur].left=x+hs;
						grid[ur].right=w;
						S[ur].norbox=&grid[ur];
						S[ur].size=hs*(w-x-hs);
						proxy=ur;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						S[ll].addition=data_+ll*dim;
							S[ll].sSum=sData_+ll*dim;
						S[ll].perim=2*(h-y-hs)+step;
						grid[ll].top=y+hs;
						grid[ll].bottom=h;
						grid[ll].left=x;
						grid[ll].right=x+hs;
						S[ll].norbox=&grid[ll];
						S[ll].size=hs*(h-y-hs);
						proxy=ll;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					if(tag[lr]==-1)
					{
						S[lr].addition=data_+lr*dim;
							S[lr].sSum=sData_+lr*dim;
						S[lr].perim=2*(w-x-hs)+2*(h-y-hs);
						grid[lr].top=y+hs;
						grid[lr].bottom=h;
						grid[lr].left=x+hs;
						grid[lr].right=w;
						S[lr].norbox=&grid[lr];
						S[lr].size=(h-y-hs)*(w-x-hs);
						proxy=lr;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=lr;
							proxy+=w;
						}
						++rec;
					}	
				}				
				
			}
			else//y+hs>h
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				//if two small blocks are pure, it is possible a new pure block can be founded
				if(tag[ul]==-1&&tag[ur]==-1&&Predicate(ul,hs*(h-y),ur,(h-y)*(w-x-hs)))
				{
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=data_[ur*dim+d];
						sData_[ul*dim+d]+=sData_[ur*dim+d];
					}
					++flag;
					
				}
				else	//if two blocks contains labeled pixel
				{
					if(tag[ul]==-1)
					{
						S[ul].addition=data_+ul*dim;
							S[ul].sSum=sData_+ul*dim;
						S[ul].perim=2*(h-y)+step;
						grid[ul].top=y;
						grid[ul].bottom=h;
						grid[ul].left=x;
						grid[ul].right=x+hs;
						S[ul].norbox=&grid[ul];
						S[ul].size=hs*(h-y);
						proxy=ul;
						for(j=0;j<h-y;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
						
					}					
					if(tag[ur]==-1)
					{
						S[ur].addition=data_+ur*dim;
							S[ur].sSum=sData_+ur*dim;
						S[ur].perim=2*(w-x-hs)+2*(h-y);
						grid[ur].top=y;
						grid[ur].bottom=h;
						grid[ur].left=x+hs;
						grid[ur].right=w;
						S[ur].norbox=&grid[ur];
						S[ur].size=(w-x-hs)*(h-y);
						proxy=ur;
						for(j=0;j<h-y;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					
					
				}
		
			}
	
		}
	
	}//while loop ended
	
	comps=rec;
	//test 
	/*ofstream bench("mark.txt",ios::app);
	bench<<"region total  number:"<<rec<<endl;
	bench<<"index\tfather\t\tsize\t\tperim"<<endl;
	for(x=0;x<L;x++)
	{
	bench<<x<<"\t"<<tag[x]<<"\t"<<S[tag[x]].size<<"\t"<<S[tag[x]].perim<<endl;
	}*/
	//test ended
	//initiate S's NL and interdif
	proxy=0;
//	double ret=0;
	for(x=0;x<L;++x)
	{

		assert(tag[x]!=-1);
		S[x].p=tag[x];
		if(tag[x]!=x)//belongs to a father and not a region
			continue;
		if(metric==DISTBENZ)
		S[x].InterDiff(d_,wc,wp,Delta);
		//initiate NPList
		int lrx=grid[x].right,lry=grid[x].bottom ,ulx=grid[x].left ,uly=grid[x].top;
	//	assert(lrx<=w&&lry<=h);	
		if(lrx==w||lry==h)
		{
			if(lrx==w&&lry!=h)
			{
				NamNeiXEx(lry,ulx,lrx-ulx,x,proxy);
			}
			else if(lrx!=w&&lry==h)
			{	
				NamNeiYEx(lrx,uly,lry-uly,x,proxy);
			}
			else//lrx==w&&lry==h
				continue;
		}
		else
		{
			NamNeiY(lrx,uly,lry-uly,x,proxy);
			NamNeiX(lry,ulx,lrx-ulx,x,proxy);
		}		
	}

	//test
/*	ofstream lever("regtag.txt",ios::app);
	lever<<"region tags are below:\n";
	for(x=0;x<h;++x)
	{
		for(y=0;y<w;++y)
			lever<<"\t"<<tag[x*w+y];
	lever<<endl;
	}
	lever<<endl;
/*	lever<<"region total  number:"<<rec<<endl;
	lever<<"index\tsize\tperim\t\tneighbor list:(rind,pind)"<<endl;
	for(x=0;x<L;x++)
	{
		if(tag[x]!=x)
			continue;
		int chrt=-1;
		lever<<x<<"\t"<<S[x].size<<"\t"<<S[x].perim<<"\t\t";
		for(NPL::iterator dido=S[x].NPList.begin();dido!=S[x].NPList.end();++dido)
		{
				if(chrt==dido->rInd)
				{afxDump<<"region no.\t"<<x<<"\n";
				AfxMessageBox("duplicate neighbor in regions neighbor list!");}
				chrt=dido->rInd;
			//	NP temp(x,0);
			//	NPL::iterator	nplp=find(S[chrt].NPList.begin(),S[chrt].NPList.end(),temp);
			//	assert(nplp!=S[chrt].NPList.end());
			lever<<"("<<dido->rInd<<","<<dido->pInd<<")\t";
		}
		//	for(i=0;i<dim;i++)
		lever<<endl;
	}*/

	//	assert(sernum==h*w-1);
	//delete []grid;
	InitializeASM(proxy,L,QTS);
}
//A's size ,comps current,flag=0 for hc 1 for qt,2 for gbs
//function: initate A.es,A.t,S.bestp,MCL if sortDM is false
int CHC::InitializeASM(int ln,int com, enum Appro flag)
{
	int x,y,label,cand,sernum=0;
	int h=Height,w=Width;
	int L=w*h;
	NPL::iterator lit;//,nplp;
	double mini=0;

	//initiate A.es one whole raster scan is performed. check the right and down pixel
	/*if(EM)
	{
		for(y=0;y<h-1;++y)
		{
			sernum=y*w;
			for(x=0;x<w-1;++x)
			{
				label=tag[sernum];
				cand=tag[sernum+1];
				if(label!=cand)
				{
					NP temp(cand,0);
					nplp=find(S[label].NPList.begin(),S[label].NPList.end(),temp);
				//	assert(nplp!=S[label].NPList.end()); // not sure about why sometimes nplp==end while still valid so changed to next line
					assert(nplp->rInd==cand);
					A[nplp->pInd].es+=(EM[sernum]+EM[sernum+1]);
				}
				cand=tag[sernum+w];
				if(label!=cand)
				{
					NP temp(cand,0);
					nplp=find(S[label].NPList.begin(),S[label].NPList.end(),temp);
					assert(nplp!=S[label].NPList.end());
					A[nplp->pInd].es+=(EM[sernum]+EM[sernum+w]);
				}
				++sernum;
			}
			label=tag[sernum];
				cand=tag[sernum+w];
			if(label!=cand)
			{
				NP temp(cand,0);
				nplp=find(S[label].NPList.begin(),S[label].NPList.end(),temp);
				assert(nplp!=S[label].NPList.end());
				A[nplp->pInd].es+=(EM[sernum]+EM[sernum+w]);
			}
			++sernum;
		}
		for(x=0;x<w-1;++x)
		{
			label=tag[sernum];
				cand=tag[sernum+1];
			if(label!=cand)
			{
				NP temp(cand,0);
				nplp=find(S[label].NPList.begin(),S[label].NPList.end(),temp);
				assert(nplp!=S[label].NPList.end());
				A[nplp->pInd].es+=(EM[sernum]+EM[sernum+1]);
			}
			++sernum;
		}
		//	assert(sernum==h*w-1);
	}
	else phi=1;*/
	if(flag==GBS)
		x=1;
	else
		x=0;
	switch (metric)
	{ 
	case DISTBENZ:
	default:
		for(;x<ln;x++)
		{
			label=A[x].r[0];
			cand=A[x].r[1];
			DistBenz(S[label],S[cand],A[x].bl,A[x].t);
			//		JhHypo(S[label],S[cand],A[x].bl,A[x].t);
		}
		break;
	case DISTLSCHD:
		for(;x<ln;x++)
		{
			label=A[x].r[0];
			cand=A[x].r[1];
			JhHypo(S[label],S[cand],A[x].bl,A[x].t);
		}
		break;
	case DISTFISHER:
		for(;x<ln;x++)
		{
			label=A[x].r[0];
			cand=A[x].r[1];
			DistFisher(S[label],S[cand],A[x].bl,A[x].t);
		}
		break;
	}
	switch (flag)
	{//for S is continuous with regards to index
	case GBS:
	case HC:
		{
			//initiate S[x].bestp
			for(x=0;x<com;x++)
			{
				//find the best merge candidate for each region bestp for S[x]
				lit=S[x].NPList.begin();
				mini=1e20;
				while(lit!=S[x].NPList.end())
				{
					assert(lit->pInd>=0);
					if(mini>A[lit->pInd].t)
					{
						mini=A[lit->pInd].t;
						S[x].bestp=lit->pInd;//record the best pair for this region
					}
					
					else if(mini==A[lit->pInd].t)
					{
						label=S[x].bestp;
						//j=A[i].r[0]==x?A[i].r[1]:A[i].r[0];
						cand=lit->pInd;
						//w=A[u].r[0]==x?A[u].r[1]:A[u].r[0];
						if(label>cand)//(j>w)
							S[x].bestp=cand;//lit->pInd;
					}
				
					++lit;
				}
			}
			//initiate MCL
			//if the friendship is mutual, store this pair in MCL
			if(!sortDM)
			{
				for(y=0;y<com;y++)
				{
					label=S[y].bestp;//pair index for this pair
					cand=A[label].r[0]==y?A[label].r[1]:A[label].r[0];//region index of the other region in the pair
					if(S[cand].bestp==label)
					{
						_MC mc(label,A[label].t);
						MCL.insert(mc);//ambiguity is avoided as the key is unique
					}
				}
			}
		}
		break;
		
	case QTS:
		{	
			//initiate S[x].bestp
			for(x=0;x<L;x++)
			{
				//find the best merge candidate for each region bestp for S[x]
				if(tag[x]!=x)//belongs to a father and not a region
					continue;
				lit=S[x].NPList.begin();
				mini=1e20;
				while(lit!=S[x].NPList.end())
				{
					if(mini>A[lit->pInd].t)
					{
						mini=A[lit->pInd].t;
						S[x].bestp=lit->pInd;//record the best pair for this region
					}
					else if(mini==A[lit->pInd].t)
					{
						label=S[x].bestp;
						//j=A[i].r[0]==x?A[i].r[1]:A[i].r[0];
						cand=lit->pInd;
						//w=A[u].r[0]==x?A[u].r[1]:A[u].r[0];
						if(label>cand)//(j>w)
							S[x].bestp=cand;//lit->pInd;
					}
					++lit;
				}
			}
			//initiate MCL
			//if the friendship is mutual, store this pair in MCL
			if(!sortDM)
			{
				for(y=0;y<L;y++)
				{
					if(tag[y]!=y)//belongs to a father and not a region
						continue;
					label=S[y].bestp;//pair index for this pair
					cand=A[label].r[0]==y?A[label].r[1]:A[label].r[0];//region index of the other region in the pair
					if(S[cand].bestp==label)
					{
						_MC mc(label,A[label].t);
						MCL.insert(mc);//ambiguity is avoided as the key is unique
					}
				}
			}
		}
		break;
		
	default:
		AfxMessageBox("Error with initiate ASM!");
		break;
	}

	return flag;
}

//if origin data format is byte, regularization i.e. division by 256 is used to keep precise
int BuildData(CHC&sHC, const CString&path, vector<float>&bWArray)
{
	GDALDataset* pDataset=(GDALDataset *) GDALOpen(path,GA_ReadOnly);
	assure(pDataset,path);
	int spp=pDataset->GetRasterCount();//波段的数目
	int d;
	if(bWArray.size()>0)
	{
		if(bWArray.size()!=spp)
		{
			AfxMessageBox("weight of band size incompatible with band number in Dataset!");
			return 0;
		}
		for(d=0,sHC.d_=0;d<spp;++d)
			sHC.d_+=(bWArray[d]>0?1:0);
	}
	else
	{
		for(d=0;d<spp;++d)
			bWArray.push_back(1.f);
		sHC.d_=spp;
	}

	sHC.Width=pDataset->GetRasterXSize();
	sHC.Height=pDataset->GetRasterYSize();
	int L=sHC.Height*sHC.Width;
	sHC.comps=L;
	if(sHC.tag)
		delete[]sHC.tag;
	sHC.tag=new int[L];	
	if (sHC.data_)
	{
		delete []sHC.data_;
		delete []sHC.sData_;
	}	
	sHC.data_=new float[L*sHC.d_];
	sHC.sData_=new float[L*sHC.d_];
	if(sHC.A.size())
	{
		sHC.A.clear();
		sHC.S.clear();
	}
	int x,y,sernum,temp,cur=0;//cur for current data channel index
	int nByteWidth=sHC.d_*sHC.Width;
	float* buf =new float[sHC.Width*sHC.Height];

	GDALRasterBand  *m_pBand=NULL;
	float max=0, min=0;		
	for(d=0;d<spp;++d)
	{
		if(bWArray[d]==0)
		{		
			continue;
		}

		m_pBand= pDataset->GetRasterBand(d+1);
	
//		m_pBand->GetStatistics( 0,  1,&min, &max,0,0);

		if (m_pBand)
		{	
			if (CE_None==m_pBand->RasterIO( GF_Read,0,0, sHC.Width, sHC.Height, buf,sHC.Width,sHC.Height, GDT_Float32, 0, 0 ))
			{
				if(sHC.Delta==0)
				{
					temp=sHC.Height*sHC.Width;
					for(y=0;y<temp;++y)
					{
						max=max>buf[y]?max:buf[y];
					}
					sernum=(int)ceil(max);
					x=1;
					while(sernum>1)
					{
						++x;
						sernum>>=1;
					}
					sHC.Delta=(float)(1<<x);
				}
				
				sernum=cur;
				temp=0;
				for(y=0;y<sHC.Height;++y)
				{
					for (x = 0; x < sHC.Width; ++x) 
					{
						sHC.data_[sernum]=buf[temp]/sHC.Delta;							
						sHC.sData_[sernum]=sHC.data_[sernum]*sHC.data_[sernum];
						sernum+=sHC.d_;
						++temp;
					}				
				}
			}
		}
		++cur;
	}
	assert(cur==sHC.d_);
	delete[]buf;
	GDALClose( (GDALDatasetH) pDataset);
	return 1;
}


void CHC::LamSchd(Region*a,Region*b,unsigned int len,double &ret)
{
	ret=0;
	//compute sigma first
	for(int i=0;i<d_;i++)
	{//here for the quotient of addition sum the fractional part is discarded for small difference
		ret+=square((a->addition[i])/(double)(a->size)-(b->addition[i])/(double)(b->size));
	}
	ret*=(a->size)*(b->size);
	ret/=(a->size+b->size);
	ret/=len;
}
//function: save edge raster file into bmp file 8bit



//usage: compare the segmentation/ tag in CHC to truth partition/rect following method in
//[18]	A Method For Multi-Spectral Image Segmentation Evaluation Based On Synthetic Images
//the class label stored in rect is 0 based and of size Width*Height
int CHC::BenchMark(int *rect,double&ri,double&cr)
{
	int jude;//number of regions minus 1 in truth partition
	jude=MiniTag(rect);
	int L=Width*Height;
	int*tagTemp=new int[L];
	memcpy(tagTemp,tag,sizeof(int)*L);
	int count=MiniTag(tagTemp);
	assert(count==comps);
	double ei;
	ri=RandIndex(L,rect,jude,tagTemp,comps,ei);
	cr=(ri-ei)/(1-ei);
	delete[]tagTemp;
	return 1;
}
//usage:floodfill T array pImage of size roi with newVal starting from seed and
//save the floodfill region info in region

int Flood(int* pImage, CvSize roi, CvPoint seed,
                      int newVal, CvConnectedComp* region, int flags,
                      CvFFillSegment* buffer, int buffer_size)
{
	int step=roi.width;
    int* img= pImage + step * seed.y;
    int i, L, R; 
    int area = 0;
    int val0=0;

    int XMin, XMax, YMin = seed.y, YMax = seed.y;
    int _8_connectivity = (flags & 255) == 8;
    CvFFillSegment* buffer_end = buffer + buffer_size, *head = buffer, *tail = buffer;
	
    L = R = XMin = XMax = seed.x;
	
	
	val0= img[L];
	img[L] = newVal;
	
	while( ++R < roi.width && img[R] == val0 )		
		img[R] = newVal;	
	while( --L >= 0 && img[L] == val0 )	
		img[L] = newVal; 
	
    XMax = --R;
    XMin = ++L;
    ICV_PUSH( seed.y, L, R, R + 1, R, UP );
	
    while( head != tail )
    {
        int k, YC, PL, PR, dir;
        ICV_POP( YC, L, R, PL, PR, dir );
		
        int data[][3] =
        {
            {-dir, L - _8_connectivity, R + _8_connectivity},
            {dir, L - _8_connectivity, PL - 1},
            {dir, PR + 1, R + _8_connectivity}
        };
		
        if( region )
        {
            area += R - L + 1;			
            if( XMax < R ) XMax = R;
            if( XMin > L ) XMin = L;
            if( YMax < YC ) YMax = YC;
            if( YMin > YC ) YMin = YC;
        }
		
        for( k = 0/*(unsigned)(YC - dir) >= (unsigned)roi.height*/; k < 3; k++ )
        {
            dir = data[k][0];
            img= pImage + (YC + dir) * step;
            int left = data[k][1];
            int right = data[k][2];
			
            if( (unsigned)(YC + dir) >= (unsigned)roi.height )
                continue;
			
            
			for( i = left; i <= right; i++ )
			{
				if( (unsigned)i < (unsigned)roi.width &&img[i] == val0)
				{
					int j = i;
			
					img[i] = newVal;
					while( --j >= 0 && img[j] == val0)					
						img[j] = newVal;										
					while( ++i < roi.width && img[i] == val0)				
						img[i] = newVal;					
					ICV_PUSH( YC + dir, j+1, i-1, L, R, -dir );
				}
			}
			
        }
    }
	
    if( region )
    {
        region->area = area;
        region->rect.x = XMin;
        region->rect.y = YMin;
        region->rect.width = XMax - XMin + 1;
        region->rect.height = YMax - YMin + 1;
     //   region->value = newVal;
    }
	
    return 1;
}
//input: indarr is a matrix of labels(non-negative) of size roi indicating each pixels' region
//each region is 4 connected. 
//output:label is for checking whether labels number is safe.
int CHC::MiniTag(int*indArr)
{

	int label,sernum,total;
	int len=Width*Height;
	CvFFillSegment* buffer = 0;	
	int buffersize = __max( Width, Height )*2;
	buffer = (CvFFillSegment*)cvAlloc( buffersize*sizeof(buffer[0]));
	CvConnectedComp parcel;
	for (label=0,sernum = 0,total=0; sernum< len; ++sernum) 
	{
		if(indArr[sernum]>-1)
		{
			Flood(indArr,cvSize(Width,Height),cvPoint(sernum%Width,sernum/Width),-1-label,&parcel,4,buffer,buffersize);
			++label;
			total+=parcel.area;
		}
	}
	assert(total==len);
	for (sernum= 0; sernum< len;++sernum) 
	{
		indArr[sernum]=abs(indArr[sernum])-1;
	}	
	return label;
}

//routine multiresolution segmentation using dither matrix to distribute region merging
//and locally homogeneity lest growth. merging order created for segment based on 
//its index instead of its size(i.e. pixel based order)

//tagBe stores the visit order(i.e. seed index) for regions in S produced by Bayer dithering, loop maximum iterations
//note for QSHC S.size=comps<H*W, for QT-HC S.size==H*W>comps,for HC S.size=H*W=comps; tag[].size==H*W;
//if pl's bestp's distance equal ql's bestp's distance, then merge them, even ql's bestp is not pl, this 
//avoids A to B to C to A loop trap, 
//for each seed pl, look for its mutual best candidate(if pl's bestp's weight==ql's bestp's weight), 
//if pl's bestp's weight>ql's bestp's weight, pl=ql, continue, until one pair is found or ql is checked
//in each do while, one region is visited/checked only once, if it has a partner checked, then it is marked
//checked without consideration of merging and go on to another seed.
void CHC::MRS(float hdiff)
{
	int i=0,j=0;int cycle,comps0=comps;
	float curdiff=hdiff;
	int *RegLabel=new int[comps];
	int	*tagBe=new int[comps];
	int L=S.size();

//	bool pixelOrder=0;
	for(cycle=1;cycle<=loop;++cycle)
	{		
		curdiff=square(hdiff*cycle/loop);
		
		sl::iterator iter;
		NPL::iterator lit,ltemp;
		int ql,pl,temp,sernum;
		int x,u,v;//w,
		double mini=0;
		NP np;
		do{//loop while anything chnages
			comps0=comps;
			//initiate RegLabel for those segments, subsegments remain isChecked,
			for(i=0,j=0;i<L;++i)
			{
				if(S[i].p==i)
				{
					S[i].isChecked=false;
					RegLabel[j]=i;
					++j;
				}
			}
			assert(j==comps0);
			VisitSeq(tagBe,comps0);
			//j to record the comps that has been visited
			for(i=0,j=0;i<comps0,j<comps0-1;++i)
			{
				//assert(tagBe[i]>=0);//checked before 
				pl=RegLabel[tagBe[i]];
				if(S[pl].isChecked==true) continue;	
				assert(pl==S[pl].p);
				sernum=S[pl].bestp;
				ql=A[sernum].r[0]==pl?A[sernum].r[1]:A[sernum].r[0];
				if(S[ql].isChecked==true) 
				{
					S[pl].isChecked=true;
					++j;
					continue;	
				}
				bool flag=false;
				float a=A[S[ql].bestp].t,b=A[sernum].t;
			
				assert(a<=b);				
				//search for couples, a<b means ql's bestp is not pl, b>curdiff means heterogeneity
				while((j<comps0-1)&&(a<b))				
				{
					S[pl].isChecked=true;
					++j;//pl is checked
					pl=ql;
					sernum=S[ql].bestp;
					ql=A[sernum].r[0]==pl?A[sernum].r[1]:A[sernum].r[0];
					if(S[ql].isChecked==true)
					{
						S[pl].isChecked=true;
						++j;
						flag=true;
						break;
					}
					b=a;
					a=A[S[ql].bestp].t;
				}
				if(flag)continue;			
				assert(a==b);
				
				assert(j<comps0-1);
				
				if(b>curdiff) 
				{
					S[pl].isChecked=true;
					S[ql].isChecked=true;
					j+=2;
					continue;
				}			
			
				//merge the two regions
				--comps;
				if(pl>ql)
				{
					temp=pl;
					pl=ql;
					ql=temp;
				}
				S[pl].isChecked=true;
				S[ql].isChecked=true;
				j+=2;
				S[pl].size+=S[ql].size;
				for(x=0;x<d_;++x)
				{
					S[pl].addition[x]+=S[ql].addition[x];
					S[pl].sSum[x]+=S[ql].sSum[x];
				}
				S[pl].perim+=S[ql].perim-2*A[sernum].bl;
				S[pl].norbox->UnionRect(S[ql].norbox,S[pl].norbox);
				S[ql].p=pl;
				S[pl].interdif+=(A[sernum].t+S[ql].interdif);
				//take care the neighbors affected by merge pl and ql;
				
				//take care neighbors of ql
				lit=S[ql].NPList.begin();
				while(lit!=S[ql].NPList.end())
				{
					if(lit->rInd==pl)
					{
						lit = S[ql].NPList.erase(lit);
						continue;
					}
					if((ltemp=find(S[pl].NPList.begin(),S[pl].NPList.end(),*lit))!=S[pl].NPList.end())
					{
						A[ltemp->pInd].bl+=A[lit->pInd].bl;
						//						A[ltemp->pInd].es+=A[lit->pInd].es;//this line for edge strength
						np.rInd=ql;
						S[lit->rInd].NPList.erase(remove(S[lit->rInd].NPList.begin(),S[lit->rInd].NPList.end(),np),S[lit->rInd].NPList.end());
					}
					else
					{
						if(A[lit->pInd].r[0]==ql)
							A[lit->pInd].r[0]=pl;
						else //if(A[lit->pInd].r[1]==ql)
							A[lit->pInd].r[1]=pl;
						
						S[pl].NPList.push_back(*lit);
						np.rInd=ql;
						ltemp=find(S[lit->rInd].NPList.begin(),S[lit->rInd].NPList.end(),np);
						ltemp->rInd=pl;
						assert(ltemp->pInd==lit->pInd);
					}			
					lit++;
				}
				//take care neighbors of pl
				lit=S[pl].NPList.begin();
				while(lit!=S[pl].NPList.end())
				{
					if(lit->rInd==ql)
					{						
						lit = S[pl].NPList.erase(lit);
						continue;
					}
					assert((A[lit->pInd].r[0]==pl&&A[lit->pInd].r[1]==lit->rInd)||(A[lit->pInd].r[0]==lit->rInd&&A[lit->pInd].r[1]==pl));
					switch (metric)
					{
					case DISTBENZ:
					default:
						DistBenz(S[pl],S[lit->rInd],A[lit->pInd].bl,A[lit->pInd].t);
						break;
					case DISTLSCHD:
						JhHypo(S[pl],S[lit->rInd],A[lit->pInd].bl,A[lit->pInd].t);
						break;
					case DISTFISHER:
						DistFisher(S[pl],S[lit->rInd],A[lit->pInd].bl,A[lit->pInd].t);
						break;
					}
					x=S[lit->rInd].bestp;
					if(A[x].r[0]==pl||A[x].r[1]==pl||A[x].r[0]==ql||A[x].r[1]==ql)
					{
						mini=1e20;
						for(ltemp=S[lit->rInd].NPList.begin();ltemp!=S[lit->rInd].NPList.end();++ltemp)
						{
							if(mini>A[ltemp->pInd].t)
							{
								mini=A[ltemp->pInd].t;
								S[lit->rInd].bestp=ltemp->pInd;
							}
							else if(mini==A[ltemp->pInd].t)
							{
								v=S[lit->rInd].bestp;
								//j=A[v].r[0]==lit->rInd?A[v].r[1]:A[v].r[0];
								u=ltemp->pInd;
								//w=A[u].r[0]==lit->rInd?A[u].r[1]:A[u].r[0];
								if(v>u)//(j>w)
									S[lit->rInd].bestp=u;//lit->pInd;
							}
						}
					}
					else
					{
						if(A[x].t>A[lit->pInd].t)					
						{
							//it's possible that the pair does not exist in merge candidate list
							//but it's safe to delete a _MC not exist!
							
							S[lit->rInd].bestp=lit->pInd;
						}
						else if(A[x].t==A[lit->pInd].t)
						{
							
							//j=A[x].r[0]==lit->rInd?A[x].r[1]:A[x].r[0];
							u=lit->pInd;
							//w=A[u].r[0]==lit->rInd?A[u].r[1]:A[u].r[0];
							if(x>u)//(j>w)
							{
								
								S[lit->rInd].bestp=u;//lit->pInd;
							}
						}
					}
					++lit;
				}
				//compute bestp for pl
				mini=1e20;
				for(ltemp=S[pl].NPList.begin();ltemp!=S[pl].NPList.end();ltemp++)
				{
					if(mini>A[ltemp->pInd].t)
					{
						mini=A[ltemp->pInd].t;
						S[pl].bestp=ltemp->pInd;
					}
					else if(mini==A[ltemp->pInd].t)
					{
						v=S[pl].bestp;
						//j=A[v].r[0]==pl?A[v].r[1]:A[v].r[0];
						u=ltemp->pInd;
						//w=A[u].r[0]==pl?A[u].r[1]:A[u].r[0];
						if(v>u)//(j>w)
							S[pl].bestp=u;//lit->pInd;
					}
				}				
			}//each segment
		}while(comps<comps0);
	}//each cycle
	delete []RegLabel;
	delete []tagBe;
}


//for each region in S, if size no greater than nsize, 
//merged to the nearest color neighboring region
void CHC::KillMinion(int nsize)
{
	if(S.size()==0)
	{
		AfxMessageBox("Region set has not yet been created!");
		return;
	}
	int i,L=S.size();
	for(i=0;i<L;++i)
	{
		if(S[i].p!=i)
			continue;
		if(S[i].size<nsize)
		{
			if(S[i].NPList.size()==1)
			{
				S[i].p=S[S[i].NPList[0].rInd].p;
				
			}
			--comps;
		}
	}
}
//use three strategies to refine borders, ben wuest's border refinement, NIU xutong's region grow 
//frayed boundary removal, and susan edge map confidence decision
void CHC::BorderRefine()
{
}
// the function draws all the squares in the image
void drawSquares( CvSize sz, CvSeq* squares,int*accum,int len )
{
	IplImage* img = cvCreateImage( sz, 8, 3 );
	cvZero(img);
     int i=0,j=0,k=0;
    
 
    //int totum=squares->total;
 
    // read 4 sequence elements at a time (all vertices of a square)
    while(j<len)// <totum)
    {        
        int count = accum[j];
		CvPoint pt0= *CV_GET_SEQ_ELEM(CvPoint, squares, k+count-1);
		for( i = 0; i <count; i++ )
        {
            CvPoint pt =*CV_GET_SEQ_ELEM( CvPoint,squares,k+i) ;
            cvLine( img, pt0, pt, CV_RGB( 0, 255, 0 ), 1, CV_AA, 0 );
            pt0 = pt;
        }
		k+=count;    
		++j;		
    }
     // show the resultant image
    cvShowImage( "result", img );
    cvReleaseImage( &img );
}
//retrieve points on the boundary of an object with random sequence
CvSeq* GetBoundary(int *tag, int Width, int Height,CvMemStorage* storage)
{	
	CvSeq* ptseq;
	CvSeq*hull,*cur;
	CvPoint pt0;
	vector<CvPoint> stk;
	int k,n[9],label,sernum;
	int i,j,s,t,immed,curtag;
	int i1=0,j1=0,cont=0;
	int *g=new int[Width*Height];
	sernum=0;
	for(i=0;i<Height;++i)
	{
		for(j=0;j<Width;++j)
		{
			label=tag[sernum];
			if(j>0&&i>0&&(j<Width-1)&&(i<Height-1))
			{
				if((label==tag[sernum+1])&&(label==tag[sernum-1])&&(label==tag[sernum-Width])&&(label==tag[sernum+Width]))
				{
					g[sernum]=-1;
				}
				else
					g[sernum]=tag[sernum];
			}
			else
					g[sernum]=tag[sernum];
			++sernum;
		}
	}
	for(i=0;i<Height;i++)
	{
		for(j=0;j<Width;j++)
		{
			
			immed=i*Width;
			if(g[immed+j]==-1)continue;
			stk.clear();
			pt0.x=j;
			pt0.y=i;
			stk.push_back(pt0);
			curtag=g[immed+j];
			g[immed+j]=-1;
			
			hull=cvCreateSeq( CV_SEQ_KIND_GENERIC|CV_32SC2, sizeof(CvContour),
				sizeof(CvPoint), storage);
			//create simple polygon curve represented by points array
			//cvCreateSeq( CV_SEQ_ELTYPE_POINT|CV_SEQ_KIND_CURVE|CV_SEQ_FLAG_SIMPLE, sizeof(CvContour),
              //                       sizeof(CvPoint), storage );
			if(cont==0)
			{
				ptseq=hull;
				cur=hull;
			}
			else
			{		
				cur->h_next=hull;
				cur=hull;
			}
			++cont;
			cvSeqPush( cur, &pt0 );
		//	printf("%d,%d\t",j,i);
			
			while(!stk.empty())
			{
				s=stk.rbegin()->y;
				t=stk.rbegin()->x;
				i1=s;j1=t;
				
				stk.pop_back();
				GetMask(t,s,g,Width,Height,n);
				for(k=1;k<=8;++k)
				{
					
					if(n[k]==curtag)
					{
						if(k==8)k=0;
						switch(k)
						{
						case 1:s--;t++;break;
						case 2:s--;break;
						case 3:s--;t--;break;
						case 4:t--;break;
						case 5:s++;t--;break;
						case 6:s++;break;
						case 7:s++;t++;break;
						case 0:t++;k=8;break;
						}
						pt0.x=t;
						pt0.y=s;
						stk.push_back(pt0);
						g[s*Width+t]=-1;
						cvSeqPush( cur, &pt0 );
					//	printf("%d,%d\t",t,s);
						s=i1;t=j1;
					}
					
				}
			}
		//	printf("\n");
		
		}
	}
	delete []g;
	return ptseq;
}
//compute shade or nochange ratio of one region
//input segmentation stored in myHC, shade or nochange pixels with label 0 stored in shade
//output: exS[each region].attList[shadeRatio], or attList[changeRatio] are computed.
void ShadeRatio(CHC&myHC, IplImage*shade,bool shadow)
{
	if(!myHC.S.size())
	{
		AfxMessageBox("Segment image before computing region properties!");
		return;
	}
	int len=myHC.S.size();
	exRegion turg;//temp exregion
	vector<exRegion>::iterator xiter=myHC.exS.begin(),xtemp;	
	
	int x,mW,mH,mx,my,i,j,pos,dest,total;
	for (x = 0; x <len; x++) 
	{		
		if(x!=myHC.S[x].p)			
			continue;
		assert(xiter->label==x);
		mH=myHC.S[x].norbox->Height();
		mW=myHC.S[x].norbox->Width();
		mx=myHC.S[x].norbox->left;
		my=myHC.S[x].norbox->top;
		pos=my*myHC.Width+mx;
		total=0;
		dest=0;
		for(j=0;j<mH;++j)
		{			
			for(i=0;i<mW;++i)
			{
				if(myHC.tag[pos]==x)
				{
					if(((BYTE*)(shade->imageData + shade->widthStep*(my+j)))[mx+i]==0)
						++dest;
					++total;
				}
				++pos;			
			}
			pos+=myHC.Width-mW;				
		}
		
		assert(total==myHC.S[x].size);
		float temp=float(dest)/myHC.S[x].size;
		NPL::iterator lit=myHC.S[x].NPList.begin();
		exRegion turg;
		if(shadow)
		{
			xiter->attList[shadeRatio]=temp;
			//for each neighbor update the neighbor's maximum neiShadereatio
			while(lit!=myHC.S[x].NPList.end())
			{
				turg.label=lit->rInd;
				xtemp=lower_bound(myHC.exS.begin(),myHC.exS.end(),turg);
				if(xtemp->attList[neiShadeRatio]<temp)
					xtemp->attList[neiShadeRatio]=temp;		
				++lit;
			}
		}
		else			
			xiter->attList[changeRatio]=1.0f-temp;
		++xiter;
	}
}

//compute  the region features, NDVI compactness length/width(elongation) roundness 
void CHC::RegionProps()
{
	if(!S.size())
	{
		AfxMessageBox("Segment image before computing region properties!");
		return;
	}
	CvMemStorage* storage= cvCreateMemStorage(0);
	CvSeq*contours;
	CvPoint pt;
	CvBox2D rect;
	int sernum,hullcount;
	
	int x,mW,mH,mx,my,i,j,pos,dest,total;

	CvMat Ma, Mb;
	CvMoments mom;
	CvHuMoments huMom;
	double min,max;
	int len=S.size();
	exRegion turg;//temp exregion
	vector<exRegion>::iterator xiter;
	
	contours=GetBoundary(tag,Width,Height,storage);
	hullcount=0;
	while( contours)
	{		
		rect=cvMinAreaRect2(contours);
//		cvBoxPoints(rect,ptf);
		pt =*CV_GET_SEQ_ELEM( CvPoint, contours, 0 );
		sernum=pt.y*Width+pt.x;
		turg.label=tag[sernum];	
		assert(S[tag[sernum]].p==tag[sernum]);
		xiter=lower_bound(exS.begin(),exS.end(),turg);
		if(xiter->isVisited==true)//if one object is enclosed by another object, revisit may occur!
		{
			contours = contours->h_next;
			continue;
		}
		xiter->attList[centerX]=rect.center.x;
		xiter->attList[centerY]=rect.center.y;
		xiter->attList[minRectWid]=rect.size.width+1;
		xiter->attList[minRectHeg]=rect.size.height+1;
	//	float temp=(rect.size.width+1.0f)*(rect.size.height+1.0f)+1.f;
	//	int quirk=S[tag[sernum]].size;
		assert(((rect.size.width+1.0f)*(rect.size.height+1.0f)+1.f)>=S[tag[sernum]].size);
	/*if(temp<quirk)
		{
		
		int j=tag[sernum];
		int m=grid[j].bottom;
		m=grid[j].top;
		m=grid[j].left;
		m=grid[j].right;
	
		}*/
		xiter->attList[boxAngle]=rect.angle;
		++hullcount;
		xiter->isVisited=true;
	//	hull = cvConvexHull2(contours, 0, CV_CLOCKWISE, 1 );
	//	area=abs(cvContourArea(hull,CV_WHOLE_SEQ));

		// take the next contour
		contours = contours->h_next;
	}
	assert(hullcount==comps);

	cvClearMemStorage( storage);

	//average width and length

	//search father for each region
//	xiter=exS.begin();
	int k=0;
	for (x = 0; x <len; x++) 
	{		
		if(x!=S[x].p)			
			continue;
		assert(exS[k].label==x);
		mH=S[x].norbox->Height();
		mW=S[x].norbox->Width();
		BYTE*patch=new BYTE[(mH+2)*(mW+2)];
		float *pool=new float[(mH+2)*(mW+2)];
		memset(patch,0,sizeof(BYTE)*(mH+2)*(mW+2));
		cvInitMatHeader( &Ma, mH+2, mW+2, CV_8UC1, patch);
		cvInitMatHeader( &Mb, mH+2, mW+2, CV_32FC1, pool);

		mx=S[x].norbox->left;
		my=S[x].norbox->top;
		pos=my*Width+mx;
		dest=(mW+2)+1;
		total=0;
		for(j=0;j<mH;++j)
		{			
			for(i=0;i<mW;++i)
			{
				if(tag[pos]==x)
				{
					patch[dest]=1;	
					++total;
				}
				++pos;
				++dest;
			}
			pos+=Width-mW;
			dest+=2;
		}
		
		assert(total==S[x].size);
	
		cvDistTransform(&Ma,&Mb,CV_DIST_L2,3);
		cvMinMaxLoc(&Mb,&min,&max);
	/*	for(i=1;i<mH+1;++i)
		{
			cvGetRow(&Mb,&Mc,i);
			cvMinMaxLoc(&Mc,&min,&max);
			sum+=max;
		}
		sum=sum/mH*2;*/	
		max*=2;
//		turg.label=x;
//		xiter=lower_bound(exS.begin(),exS.end(),turg);
		exS[k].attList[meanThick]=max;	
	
		cvMoments(&Ma, &mom, 1 );
		cvGetHuMoments(&mom,&huMom);
		min=huMom.hu1;
		max=huMom.hu2;
		exS[k].attList[eigRatio]=sqrt((min+sqrt(max))/(min-sqrt(max)));	
		delete[]patch;
		delete[]pool;
		++k;
	}
	assert(k==comps);
}
//press ONLY f5 produces only 1 contour enveloping the whole image, press ctrl+f5 produces contours
//more than 1 except the frame contour,  debug error memory damage is often admonished, with no idea
//this funct   ion binarize array tag,find contours, polygonize and draw out!

void FindContours(int Width,int Height, int*tag,int comps)
{
	int r,c,sernum=0,label,i,j=0;
	CvSize sz = cvSize( Width, Height);
    IplImage* gray = cvCreateImage( sz, 8, 1 ); 

//	const char* wndname = "result";
	for(r=0;r<Height;++r)
	{
		for(c=0;c<Width;++c)
		{
			label=tag[sernum];
			if(r>0&&c>0&&(c<Width-1)&&(r<Height-1))
			{
				if(label!=tag[sernum+1]||label!=tag[sernum-1]||label!=tag[sernum-Width]||label!=tag[sernum+Width])
				{
					((uchar*)(gray->imageData + gray->widthStep*r))[c]=255;//must be 255, 0 produces unimaginable thing
				}
			}
			else ((uchar*)(gray->imageData + gray->widthStep*r))[c]=255;
			++sernum;
		}
	}//image upside down
//	cvNamedWindow( wndname, 1 );
//	cvShowImage( "result", gray );

	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contours,*result;
	CvSeq* squares = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvPoint), storage );
	int* accum = new int[comps*2];

	   // find contours and store them all as a list
	cvFindContours( gray, storage, &contours, sizeof(CvContour),
		   CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
	   // test each contour
	while( contours )
	{		
		CvBox2D rect=cvMinAreaRect2(contours);
		
		if((rect.size.height/rect.size.width>2.f)||(rect.size.width/rect.size.height>2.f))
		{
			// approximate contour with accuracy proportional
			// to the contour perimeter
			result = cvApproxPoly( contours, sizeof(CvContour), storage,
				CV_POLY_APPROX_DP, cvContourPerimeter(contours)*0.02, 0 );
			
			//write polygon vertices to resultant sequence 
			accum[j]=result->total;
			++j;
			for( i = 0; i < result->total; i++)
				cvSeqPush( squares,(CvPoint*)cvGetSeqElem( result, i ));
		}		 
		// take the next contour
		contours = contours->h_next;
	}
//	CString hun;hun.Format("contours:%d\n",j);
//	AfxMessageBox(hun);
	drawSquares(sz,squares,accum,j);
	cvReleaseImage( &gray );
	delete []accum;
	cvClearMemStorage( storage);
}
//j coordinate in width i in height, g tag array n storage for mask
void GetMask(int j, int i,int *g, int Width,int Height,int*n)
{
			int	s=i,t=j,immed=i*Width;
			if(i==0)
			{
				if(j==0)
				{
					n[3]=-1;n[2]=-1;n[1]=-1;
					n[4]=-1;n[8]=g[immed+t+1];
					n[5]=-1;n[6]=g[immed+Width+t];n[7]=g[immed+Width+t+1];
				}
				else if(j==Width-1)
				{
					n[3]=-1;n[2]=-1;n[1]=-1;
					n[4]=g[immed+t-1];n[8]=-1;
					n[5]=g[immed+Width+t-1];n[6]=g[immed+Width+t];n[7]=-1;
				}
				else
				{
					n[3]=-1;n[2]=-1;n[1]=-1;
					n[4]=g[immed+t-1];n[8]=g[immed+t+1];
					n[5]=g[immed+Width+t-1];n[6]=g[immed+Width+t];n[7]=g[immed+Width+t+1];
				}
			}
			else if(i==Height-1)
			{
				if(j==0)
				{
					n[3]=-1;n[2]=g[immed-Width+t];n[1]=g[immed-Width+t+1];
					n[4]=-1;n[8]=g[immed+t+1];
					n[5]=-1;n[6]=-1;n[7]=-1;
				}
				else if(j==Width-1)
				{
					n[3]=g[immed-Width+t-1];n[2]=g[immed-Width+t];n[1]=-1;
					n[4]=g[immed+t-1];n[8]=-1;
					n[5]=-1;n[6]=-1;n[7]=-1;			
				}
				else
				{
					n[3]=g[immed-Width+t-1];n[2]=g[immed-Width+t];n[1]=g[immed-Width+t+1];
					n[4]=g[immed+t-1];n[8]=g[immed+t+1];
					n[5]=-1;n[6]=-1;n[7]=-1;
				}
			}
			else
			{
				if(j==0)
				{
					n[3]=-1;n[2]=g[immed-Width+t];n[1]=g[immed-Width+t+1];
					n[4]=-1;n[8]=g[immed+t+1];
					n[5]=-1;n[6]=g[immed+Width+t];n[7]=g[immed+Width+t+1];
				}
				else if(j==Width-1)
				{
					n[3]=g[immed-Width+t-1];n[2]=g[immed-Width+t];n[1]=-1;
					n[4]=g[immed+t-1];n[8]=-1;
					n[5]=g[immed+Width+t-1];n[6]=g[immed+Width+t];n[7]=-1;
				}
				else
				{
					n[3]=g[immed-Width+t-1];n[2]=g[immed-Width+t];n[1]=g[immed-Width+t+1];
					n[4]=g[immed+t-1];n[8]=g[immed+t+1];
					n[5]=g[immed+Width+t-1];n[6]=g[immed+Width+t];n[7]=g[immed+Width+t+1];
				}
			}
}
//my function	3 2 1 j width
//				4 * 8
//		height	5 6	7
//				i 
//freeman coding primitive, for a image tag width*height with distinct label for each region boundary
//and the interior of regions are assigned -1, finding a start pixel and search its 1-8 directions
//for candidate edge pixel
void BorderEncode(int *tag,int Width, int Height)
{
    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* ptseq = cvCreateSeq( CV_SEQ_KIND_GENERIC|CV_32SC2, sizeof(CvContour),
		sizeof(CvPoint), storage );
	CvSeq*hull,*cur=ptseq;
	CvPoint pt0;
	int k,nrn,n[9],label,sernum;
//	long int lend;
//	int thresh=160,thresh2=180;
	int i,j,s,t,immed,curtag,cc;
	long int l=0,l0; 
//	int l2,i1=0,j1=0;
	int *g=tag;
	/*检测出发点*/
	sernum=0;
	for(i=0;i<Height;++i)
	{
		for(j=0;j<Width;++j)
		{
			label=g[sernum];
			if(j>0&&i>0&&(j<Width-1)&&(i<Height-1))
			{
				if((label==tag[sernum+1])&&(label==tag[sernum-1])&&(label==tag[sernum-Width])&&(label==tag[sernum+Width]))
				{
					g[sernum]=-1;
				}
			}
			++sernum;
		}
	}
	
	for(i=0;i<Height;i++)
	{
		for(j=0;j<Width;j++)
		{
			immed=i*Width;
			if(g[immed+j]==-1)continue;
			
			s=i;t=j;
			GetMask(t,s,g,Width,Height,n);
			nrn=0;
			for(k=1;k<=8;++k)
				nrn+=n[k];
			/*孤立点检测*/
			if(nrn==-8)
			{				
				pt0.x =j;
				pt0.y =i;
				cvSeqPush( cur, &pt0 );
				hull=cvCreateSeq( CV_SEQ_KIND_GENERIC|CV_32SC2, sizeof(CvContour),
					sizeof(CvPoint), storage );
				cur->h_next=hull;
				cur=hull;
				g[immed+j]=-1;
				continue;
			}
			/*iteration check chains检测*/
			//l length of the loop, cc previous link relative position
			l=1;
			curtag=g[immed+t];
			
			do{
				pt0.x =t;
				pt0.y =s;
				cvSeqPush( cur, &pt0 );
				printf("%d,%d\t",t,s);
				immed=s*Width;
				g[immed+t]=-1;
			
				l0=l;
				for(k=1;k<=8;k++)
				{
					if(n[k]!=curtag)continue;
					if(k==8)k=0;
					//判断是否是前一个点？否继续移向相邻点
					if(l!=1){if(abs(k-cc)==4)continue;}
					cc=k;
					++l;
					/* 将3×3窗口移向相邻点*/
					switch(k)
					{
					case 1:s--;t++;break;
					case 2:s--;break;
					case 3:s--;t--;break;
					case 4:t--;break;
					case 5:s++;t--;break;
					case 6:s++;break;
					case 7:s++;t++;break;
					case 0:t++;break;
					}
					//if((s<1)||(s>height-2)||(t<1)||(t>width-2))break;
					if(l0<l)break;					
					
				}/*转向下一个出发点*/
				if(k==9)break;
				GetMask(t,s,g,Width,Height,n);	
			}while(s!=i||t!=j);
			printf("\n");
			hull=cvCreateSeq( CV_SEQ_KIND_GENERIC|CV_32SC2, sizeof(CvContour),
				sizeof(CvPoint), storage );
			
			cur->h_next=hull;
			cur=hull;
		}
	}
	//last iteration to check whether exist nonnegative pixels
	sernum=0;
	for(i=0;i<Height;i++)
	{
		for(j=0;j<Width;j++)
		{
			if(g[sernum]!=-1)
			//	AfxMessageBox("error!");
			printf("error");
			++sernum;
		}
	}
}

void CHC::LenWidR()
{
	CvMemStorage* storage= cvCreateMemStorage(0);
	CvSeq*contours,*hull;
	CvPoint pt0;//,pt;
//	CvPoint2D32f ptf[4];
	CvSize sz=cvSize(Width,Height);
//	CvBox2D rect;
	int sernum=0,i=0,j=0,hullcount=0;
    IplImage* gray = cvCreateImage( sz, 8, 3 ); 
	
	const char* wndname = "result";
	cvNamedWindow( wndname, 1 );
	
	contours=GetBoundary(tag,Width,Height,storage);
	
	while( contours)
	{	
		//		int  dam=contours->total; 
//		rect=cvMinAreaRect2(contours);
//		cvBoxPoints(rect,ptf);
	//	if((rect.size.height/rect.size.width>2.f)||(rect.size.width/rect.size.height>2.f))
		{
			//draw the rect in img representation approach 1
		/*	pt0=cvPointFrom32f(ptf[3]);			
			for( i = 0; i < 4; i++ )
			{
				CvPoint pt =cvPointFrom32f(ptf[i]); 
				cvLine( gray, pt0, pt, CV_RGB( 0, 255, 0 ), 3, CV_AA, 0 );
				pt0 = pt;
			}*/
			//highlight the contours of objects representation approach 2
		/*	hullcount=contours->total;
			for( i = 0; i < hullcount; i++ )
			{
				
				pt =*CV_GET_SEQ_ELEM( CvPoint, contours, i );
				uchar* temp_ptr = &((uchar*)(gray->imageData + gray->widthStep*pt.y))[pt.x*3];
				temp_ptr[0]=0;
				temp_ptr[1]=255;
				temp_ptr[2]=0;
		}*/
			//draw the convex hull result display approach 3
			hull = cvConvexHull2( contours, 0, CV_CLOCKWISE, 0 );
			hullcount = hull->total;
			float thresh=cos(80/180*3.1415926);
			for( i = 0; i < hullcount; i++ )
			{
		//	JHLineAngle(pt0,pt,pt);
			}
			pt0 = **CV_GET_SEQ_ELEM( CvPoint*, hull, hullcount - 1 );
			for( i = 0; i < hullcount; i++ )
			{
				CvPoint pt = **CV_GET_SEQ_ELEM( CvPoint*, hull, i );
				cvLine(gray, pt0, pt, CV_RGB( 0, 255, 0 ));
				pt0 = pt;
			}

		}		 
		++j;
		// take the next contour
		contours = contours->h_next;
	}
//		CString hun;hun.Format("contours:%d\n",j);
//		AfxMessageBox(hun);
    

	cvShowImage( wndname,gray ); 
	cvReleaseImage( &gray );
	cvClearMemStorage( storage);	
}
//road pixel to store road map, fisdata for parameter setting default null
//when saving roads one flip operation performed, so before assign roadseed, a counteract flip is performed
void CHC::RoadSeed(BYTE*roadpixel,char *fisData)
{	
	int cpt, avrW,elgAL, elgWL, mnv;//compactness*100,AVR WIDTH,
	//	int i,L=Height*Width;
	if(fisData)
	{
		//ifstream guild(fisData);	
		//guild>>cpt>>avrW>>elgAL>>mnv;
		
		IplImage* img = cvLoadImage(fisData);
		if(img!=0)
		{
			if((Height!=img->height)||(Width!=img->width))
			{
				AfxMessageBox("Image size incongruent, fake road seed map encounted!");
				return;
			}
			
			int i,j,sernum=0,L=Height*Width;
			int alpha=img->nChannels,beta=img->widthStep;
		
			for(i=0;i<Height;++i)
			{
				for(j=0;j<Width;++j)
				{
					roadpixel[sernum]=(((uchar*)(img->imageData + beta*i))[alpha*j]>0?1:0);
					++sernum;
				}				
			}			
			cvReleaseImage( &img);
		}
		return;
	}
	else
	{
		cpt=15;
		avrW=15;
		elgAL=4;
		mnv=15;
		elgWL=3;
	}
	int i,L=Height*Width;
	set<int> cand;//store the road seed region tags
	exRegion turg;//temp exregion
	vector<exRegion>::iterator xiter;
	int ratlen=S.size();
	//thresholding
	xiter=exS.begin();
	while(xiter!=exS.end())
	{
		i=xiter->label;
		if((xiter->attList[meanThick]<avrW)
			&&(__max(xiter->attList[minRectWid],xiter->attList[minRectHeg])>80)
			&&((float)S[i].perim/(grid[i].Height()+grid[i].Width())<3.5f))//&&S[i].MaxAL>10&&(S[i].cmpct<0.15)
		cand.insert(i);
		++xiter;
	}
	memset(roadpixel,0,sizeof(BYTE)*Height*Width);
	for(i=0;i<L;++i)
	{
		if(cand.find(tag[i])!=cand.end())
			roadpixel[i]=1;
	}
	cand.clear();

	int x,y;//,perim;
	IplImage*portal;
	CvSize bound=cvSize(Width,Height);
	portal=cvCreateImage(bound, IPL_DEPTH_8U , 1);
	cvNamedWindow("Portal", 0);	
	cvZero(portal);
	for (i= 0; i <L; ++i) 
	{
		if(roadpixel[i])
		{
			x=i%Width;
			y=i/Width;
			((uchar*)(portal->imageData + portal->widthStep*y))[x]=255;			
		}	
	}

	cvShowImage("Portal", portal);
	cvSaveImage("roads.bmp",portal);
	cvWaitKey(0);
	cvDestroyWindow("Portal");
	cvReleaseImage(&portal);

	//get end points and initial direction
	//template extraction
	//road tracking using profile matching, curve fitting, template matching. 
	//curve fitting extrapolate the  next point, profile matching to adjust the point or weighting between predicted point and profile matching point
}
//output quadtree seg result for paper writing
void CHC::qttest(int md)
{
	int dim=d_;
	int h=Height,w=Width;
	int L=h*w;
	comps=L;
	int x,y,d;
	int sernum=0;

	memset(tag,-1,L*sizeof(int));
	int i=0,j=0,sz=0;
	int rec=0;//current number of segments
	int step=0,hs=0,hs2=0;//edge length of blocks checked in each loop and half step
	
	int flag=1,temp=0;
	int limx,limy;
	int ul,ur,ll,lr;//four upper left corner point index in four quadrant
	static int proxy=0;
	//quadtree merge main section and Regions initiation
	while(step<md)//(flag>0)//some blocks have not yet been assigned to region
	{
		flag=0;
		++temp;
		step=1<<temp;
		hs=step>>1;
		hs2=hs*hs;
		limy=(int)ceil((double)h/step)*step;
		limx=(int)ceil((double)w/step)*step;
		for(y=0;y<limy-step;y+=step)
		{
			for(x=0;x<limx-step;x+=step)
			{
				//	CheckBlock(x,y,step);
				ul=y*w+x;
				ur=y*w+x+hs;
				ll=(y+hs)*w+x;
				lr=(y+hs)*w+x+hs;
				//if this block is not contaminated, it has chance to be merged
				if(tag[ul]==-1&&tag[ur]==-1&&tag[ll]==-1&&tag[lr]==-1&&Predicate(ul,hs2,ur,hs2,ll,hs2,lr,hs2))
				{
					//predicate is met
					for(d=0;d<dim;++d)
					{
						data_[ul*dim+d]+=(data_[ur*dim+d]+data_[ll*dim+d]+data_[lr*dim+d]);
						sData_[ul*dim+d]+=(sData_[ur*dim+d]+sData_[ll*dim+d]+sData_[lr*dim+d]);
					}
					++flag;
				}
				//this block contains some component has upper left corner pixel labelled or not pure
				else
				{
					//if tA is -1 which means the quadrant is not yet assigned to region,so do it
					if(tag[ul]==-1)
					{
					
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
					
						proxy=ur;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
					
						proxy=ll;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					if(tag[lr]==-1)
					{
					
						proxy=lr;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=lr;
							proxy+=w;
						}
						++rec;
					}	
				}
			}
		}
		assert(y==limy-step);
		//for blocks lying on the boundary first check the downside boundary and use y 
		//with the same value gotten from above
		for(x=0;x<limx-step;x+=step)
		{
			//if only two component exist
			if(y+hs>=h)
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				//if two small blocks are pure, it is possible a new pure block can be founded
				if(tag[ul]==-1&&tag[ur]==-1&&Predicate(ul,(h-y)*hs,ur,(h-y)*hs))
				{
				for(d=0;d<dim;++d)
					{
						data_[ul*dim+d]+=data_[ur*dim+d];
						sData_[ul*dim+d]+=sData_[ur*dim+d];
					}
					++flag;
					
				}
				//if two blocks contains labeled pixel
				else
				{
					if(tag[ul]==-1)
					{
					
						proxy=ul;
						for(j=0;j<h-y;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
					
						proxy=ur;
						for(j=0;j<h-y;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					
				}
			}
			else// if y+hs<h, four blocks should be considered just as the beignning
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				ll=(y+hs)*w+x;
				lr=(y+hs)*w+x+hs;
				//if this block is not contaminated, it has chance to be merged
				if(tag[ul]==-1&&tag[ur]==-1&&tag[ll]==-1&&tag[lr]==-1&&Predicate(ul,hs2,ur,hs2,ll,hs*(h-hs-y),lr,hs*(h-hs-y)))
				{
					//predicate is met
					//predicate is met
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=(data_[ur*dim+d]+data_[ll*dim+d]+data_[lr*dim+d]);
						sData_[ul*dim+d]+=(sData_[ur*dim+d]+sData_[ll*dim+d]+sData_[lr*dim+d]);
					}
					++flag;
				}
				else//this block contains some component has upper left corner pixel labelled 
				{
					//if tA is -1 which means the quadrant is not yet assigned to region,so do it
					if(tag[ul]==-1)
					{
					
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
					
						proxy=ur;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
					
						proxy=ll;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					if(tag[lr]==-1)
					{
						
						proxy=lr;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=lr;
							proxy+=w;
						}
						++rec;
					}	
				}//end else four block are assigned region label
			}//end else for y+hs<h
		}//end for each block lying on the bottom from left to right
		assert(x==limx-step);
		//check blocks lying to right side of the boundary and the bottom one is not included just as above
		for(y=0;y<limy-step;y+=step)
		{
			if(x+hs>=w)
			{
				ul=y*w+x;
				ll=(y+hs)*w+x;
				//if two small blocks are pure, it is possible a new pure block can be founded
				if(tag[ul]==-1&&tag[ll]==-1&&Predicate(ul,hs*(w-x),ll,hs*(w-x)))
				{					
					for(d=0;d<dim;++d)
					{
						data_[ul*dim+d]+=data_[ll*dim+d];
						sData_[ul*dim+d]+=sData_[ll*dim+d];
					}
					++flag;					
				}
				//if two blocks contains labeled pixel or heterogeneous
				else
				{
					if(tag[ul]==-1)
					{
						
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						
						proxy=ll;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					
				}
			}
			else// if y+hs<h, four blocks should be considered just as the beignning
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				ll=(y+hs)*w+x;
				lr=(y+hs)*w+x+hs;
				//if this block is not contaminated, it has chance to be merged
				if(tag[ul]==-1&&tag[ur]==-1&&tag[ll]==-1&&tag[lr]==-1&&Predicate(ul,hs2,ur,hs*(w-hs-x),ll,hs2,lr,hs*(w-hs-x)))
				{
					//predicate is met
					
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=(data_[ur*dim+d]+data_[ll*dim+d]+data_[lr*dim+d]);
						sData_[ul*dim+d]+=(sData_[ur*dim+d]+sData_[ll*dim+d]+sData_[lr*dim+d]);
					}
					++flag;
					
					
				}
				//this block contains some component has upper left corner pixel labelled 
				else
				{
					//if tA is -1 which means the quadrant is not yet assigned to region,so do it
					if(tag[ul]==-1)
					{
					
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
						
						proxy=ur;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						
						proxy=ll;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					if(tag[lr]==-1)
					{
						
						proxy=lr;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=lr;
							proxy+=w;
						}
						++rec;
					}	
				}
				
			}
			
		}
		assert(x==limx-step);
		assert(y==limy-step);
		if(x+hs>=w)
		{
			if(y+hs<h)//two blocks are considered
			{
				ul=y*w+x;
				ll=(y+hs)*w+x;
				//if two small blocks are pure, it is possible a new pure block can be founded
				if(tag[ul]==-1&&tag[ll]==-1&&Predicate(ul,hs*(w-x),ll,(w-x)*(h-hs-y)))
				{
					
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=data_[ll*dim+d];
						sData_[ul*dim+d]+=sData_[ll*dim+d];
					}
					++flag;
					
				}
				//if two blocks contains labeled pixel
				else
				{
					if(tag[ul]==-1)
					{
						
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						
						
						proxy=ll;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<w-x;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					
				}
			}
			else//only one component exists this block must have been check before by predicate
			{//no addition and square sum is needed as both are computed already
				ul=y*w+x;
				if(tag[ul]==-1)
				{
					++flag;
				}
			}
		}		
		else//if x+hs<w
		{
			if(y+hs<h)//four blocks case
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				ll=(y+hs)*w+x;
				lr=(y+hs)*w+x+hs;
				//if this block is not contaminated, it has chance to be merged
				if(tag[ul]==-1&&tag[ur]==-1&&tag[ll]==-1&&tag[lr]==-1&&Predicate(ul,hs2,ur,hs*(w-x-hs),ll,(h-y-hs)*(hs),lr,(h-y-hs)*(w-x-hs)))
				{
					//predicate is met
					
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=(data_[ur*dim+d]+data_[ll*dim+d]+data_[lr*dim+d]);
						sData_[ul*dim+d]+=(sData_[ur*dim+d]+sData_[ll*dim+d]+sData_[lr*dim+d]);
					}
					++flag;
				}
				//this block contains some component has upper left corner pixel labelled 
				else
				{
					//if tA is -1 which means the quadrant is not yet assigned to region,so do it
					if(tag[ul]==-1)
					{
						
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
						
						proxy=ur;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						proxy=ll;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					if(tag[lr]==-1)
					{
					
						proxy=lr;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=lr;
							proxy+=w;
						}
						++rec;
					}	
				}
				
			}
			else//y+hs>h
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				//if two small blocks are pure, it is possible a new pure block can be founded
				if(tag[ul]==-1&&tag[ur]==-1&&Predicate(ul,hs*(h-y),ur,(h-y)*(w-x-hs)))
				{
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=data_[ur*dim+d];
						sData_[ul*dim+d]+=sData_[ur*dim+d];
					}
					++flag;
					
				}
				else	//if two blocks contains labeled pixel
				{
					if(tag[ul]==-1)
					{
					
						proxy=ul;
						for(j=0;j<h-y;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
					
						proxy=ur;
						for(j=0;j<h-y;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					
				}
				
			}
		}
	}
}


void CHC::SetWH(int w, int h)
{
	Width=w;
	Height=h;
}
//this two function is defined for binmorph
BYTE myMax(BYTE dimm[], BYTE num)
{
	BYTE bmax = dimm[0];
	for( BYTE ii = 1; ii <num; ii++)
		if( bmax < dimm[ ii ] )
			bmax = dimm[ ii ];
	return bmax;
}


BYTE myMin(BYTE dimm[], BYTE num)
{
	BYTE bmin = dimm[0];
	for( BYTE ii = 1; ii <num; ii++)
		if( bmin > dimm[ ii ] )
			bmin = dimm[ ii ];
	return bmin;
}
//binmorph at first is intended to compute the average width of an binary object which is supplanted by distance 
//transform in opencv,. whatever, this function can erode objects boundaries iteratively
//lpos the pointer to binary caricature with size (mw)*(mh),mind padding boundary of width 2
// erode white object black background,
//mw width of bounding box mh height of bounding box, total object pixels number,
//mode 46 and 46 for square and rhombus erosion structure element, nrepeat the time of erosion operation

void CHC::BinMorph(LPBYTE	lpos,int mW,int mH, int total,BYTE mode, BYTE&nRepeat)
{
	long	row,col, pos;	

	BYTE	bResult;
	BYTE pb[9];

	BYTE* pool=new BYTE[mW*mH];
	memcpy(pool,lpos,mW*mH*sizeof(BYTE));
	nRepeat=0;
	while(total>0)
	{	
		for(row=1; row<mH-1; row++)
		{
			pos=1+row*mW;
			for(col=1; col<mW-1; col++)
			{						
				switch( mode )
				{
				case MMSquare:
					{
						pb[0]=lpos[pos -mW- 1];
						pb[1]=lpos[pos - mW ];
						pb[2]=lpos[pos - mW + 1];
						pb[3]=lpos[pos - 1];
						pb[4]=lpos[pos + 1];
						pb[5]=lpos[pos + mW - 1];
						pb[6]=lpos[pos + mW ];
						pb[7]=lpos[pos + mW + 1];
						pb[8]=lpos[pos ];
						bResult = myMin(pb , 9);
						break;
					}
				case MMRhombus:
					{
						pb[0]=lpos[pos - mW ];
						pb[1]=lpos[pos - 1];
						pb[2]=lpos[pos ];
						pb[3]=lpos[pos + 1];
						pb[4]=lpos[pos + mW ];
						bResult = myMin(pb , 5);
						break;
					}
				default:
					break;
				}
				if( lpos[pos]==1&&bResult==0)
					--total;
				if(total==0)
					break;	
				pool[pos]= bResult;
				pos++;							
			}
			if(total==0)
					break;
		}
		memcpy(lpos,pool,mW*mH*sizeof(BYTE));
		++nRepeat;
	}
	delete []pool;
}



//at first, regmoment tries to compute the moments of a particular region; 
//later, it displays a specific region in a window on left button down
//note error crashes when try to open two opencv windows.

void CHC::RegMoment(int label)
{
	assert(grid.size()>0);
	int mH=S[label].norbox->Height(),mW=S[label].norbox->Width();
	int i,j,mx,my,pos,total;
	const char* wndname = "Region";
	CvMemStorage* storage= cvCreateMemStorage(0);
	CvSeq*contours,*result;
//	CvPoint* pt0,*pt;
	CvSize sz = cvSize(mW+2,mH+2);
    IplImage* gray = cvCreateImage( sz, 8,1 );
	cvZero(gray);
	BYTE*patch=(BYTE*)(gray->imageData);
	mx=S[label].norbox->left;
	my=S[label].norbox->top;
	pos=my*Width+mx;
	total=0;
	for(j=1;j<mH+1;++j)
	{			
		for(i=1;i<mW+1;++i)
		{
			if(tag[pos]==label)
			{
				//	((uchar*)(patch + gray->widthStep*j))[i]=1;
				(patch+ gray->widthStep*j)[i]=255;
				++total;
			}				
			++pos;			
		}
		pos+=Width-mW;		
	}
	assert(total==S[label].size);
	cvFindContours( gray, storage, &contours, sizeof(CvContour),
		CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
	float area=(float)cvContourArea(contours, CV_WHOLE_SEQ);
	float	beam=cvArcLength(contours,CV_WHOLE_SEQ,1);
//	CString modem;
//	modem.Format("area of contours:%.2f and arclength %.2f \n",area,beam);	  
//	AfxMessageBox(modem);
	result = cvApproxPoly( contours, sizeof(CvContour), storage,
			  CV_POLY_APPROX_DP, cvContourPerimeter(contours)*0.02, 0 );
//	assert(!contours->h_next);//note when region has holes, there is more than 1 contour


/*	pt=(CvPoint*)cvGetSeqElem( result, result->total-1 );
	for( i = 0; i < result->total; i ++)
	{
		pt0=pt;
		pt=(CvPoint*)cvGetSeqElem( result, i );
		// draw the square as a closed polyline 
		cvLine( cpy, *pt0, *pt, CV_RGB(0,255,0), 3, 8 );
    }*/
	IplImage* cpy = cvCreateImage( sz, 8,3 ); 
	cvZero(cpy);
	if( contours)
		cvDrawContours(cpy,contours,CV_RGB(0,255,0),CV_RGB(0,255,0),100);
	cvReleaseImage( &gray );
	cvClearMemStorage(storage);
	cvNamedWindow(wndname, 1 );

	cvShowImage( wndname, cpy );
	cvWaitKey(0);
    cvReleaseImage( &cpy );
	cvDestroyWindow( wndname);
}
//save object mean and contour of segmentation of the image with pathname to dest

void SaveSeg(const CHC&sHC,const CString& dest,const CString& pathname)
{
	int L=sHC.Height*sHC.Width;

	int x,y,d,sernum,label;
	int nByteWidth=sHC.d_*sHC.Width;
	float* buf =new float[sHC.Width*sHC.Height];
	GDALDataset *m_pDataset=(GDALDataset *) GDALOpen(pathname,GA_ReadOnly);
	GDALRasterBand  *m_pBand=NULL;
	for(d=1;d<sHC.d_+1;++d)
	{
		m_pBand= m_pDataset->GetRasterBand(d);
		if (m_pBand)
		{	
			sernum=0;
			
			for(y=0;y<sHC.Height;++y)
			{
				for (x = 0; x <sHC. Width; ++x) 
				{					
					label=sHC.tag[sernum];
					buf[sernum]=(sHC.S[label].addition[d-1])/(sHC.S[label].size)*Range;							
					if(x>0&&y>0&&(x<sHC.Width-1)&&(y<sHC.Height-1))
						if(label!=sHC.tag[sernum+1]||label!=sHC.tag[sernum+sHC.Width])//one pixel width boundary
							buf[sernum]=0;
						++sernum;
				}				
			}
			if (CE_None!=m_pBand->RasterIO( GF_Write,0,0, sHC.Width,sHC.Height, buf, sHC.Width,sHC.Height,GDT_Float32, 0, 0 ))
			{
				AfxMessageBox("error write mpdataset!");
			}
		}
	}
	delete[]buf;
	const char *pszFormat = "GTiff";
	GDALDriver *poDriver;
	char **papszMetadata;
	GDALDataset* poDstDS;


	poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	
	if( poDriver == NULL)
		exit( 1 );
	
	papszMetadata = poDriver->GetMetadata();
/*	if( CSLFetchBoolean( papszMetadata, GDAL_DCAP_CREATE, FALSE ) )
	{	herald.Format( "Driver %s supports Create() method.\n", pszFormat );
	AfxMessageBox(herald);}
	if( CSLFetchBoolean( papszMetadata, GDAL_DCAP_CREATECOPY, FALSE ) )
	{	herald.Format( "Driver %s supports CreateCopy() method.\n", pszFormat );
	AfxMessageBox(herald);}*/
	if(poDstDS=poDriver->CreateCopy( dest, m_pDataset, FALSE, NULL, NULL, NULL))
		GDALClose( (GDALDatasetH) poDstDS);
	GDALClose( (GDALDatasetH) m_pDataset);
}

int CHC::GetRegSize(int label)
{
	return S[label].size;
}
int CHC::GetSetSize()
{
	return S.size();
}

CRect CHC::GetBoundBox(int miss)
{
	return *(S[miss].norbox);
}

//group road ce3nterlines, if end points are within distance of 5pixels, centerlines connected,
//if end points are away from each other by 9 pixels,but about in the same direction, connect them
//in roadmap, 1 for road, 0 for background, in terminal the order index of end point
//in roadmap 2 for end point in this function 
void CHC::RoadLink(BYTE*roadmap,vector<int>&terminal)
{
	//nearest end point search
	//initiate road end pixel 2
	int tsize=terminal.size(),i,j,k,x,y;//x,y coordinate i,j for index incremental
	const int radius=5;//search radius
	int left,right,top,bottom;// search window
	int pos,count;

	CvSize bound=cvSize(Width,Height);
	IplImage*portal=cvCreateImage(bound, IPL_DEPTH_8U , 3);
	cvNamedWindow("Portal", 0);		
	cvZero(portal);

	for(i=0;i<tsize;++i)
	{
		assert(roadmap[terminal[i]]);
		++roadmap[terminal[i]];//assign 2
	}
	for(i=0;i<tsize;++i)
	{
		if(!roadmap[terminal[i]])
			continue;
		x=terminal[i]%Width;
		y=terminal[i]/Width;
		left=0>(x-radius)?0:(x-radius);
		right=(x+radius)>(Width-1)?(Width-1):(x+radius);
		top=0>(y-radius)?0:(y-radius);
		bottom=(y+radius)>(Height-1)?(Height-1):(y+radius);
		pos=top*Width+left;
		for(j=top;j<=bottom;++j)
		{
			for(k=left;k<=right;++k)
			{
				if(roadmap[pos]==2&&pos!=terminal[i])//link the two nodes
				{
					roadmap[pos]=0;
					roadmap[terminal[i]]=0;
					cvLine( portal, cvPoint(x,y), cvPoint(k,j), CV_RGB(0,255,0), 3, CV_AA, 0 );
				}
				++pos;
			}
			pos=pos+Width-(right-left+1);
		}
	}
	for(i=0;i<Height;++i)
	{
		for(j=0;j<Width;++j)
		{				
			if(roadmap[i*Width+j]>0)//(img2[j][i]==1)
			{	uchar* temp_ptr = &((uchar*)(portal->imageData + portal->widthStep*i))[j*3];
			temp_ptr[0]=0;
			temp_ptr[1]=255;
			temp_ptr[2]=0;
			}		
		}   
	}
	count=terminal.size();
	for(k=0;k<count;++k)
	{
		i=terminal[k]/Width;
		j=terminal[k]%Width;
		CvPoint center=cvPoint(j,i);		
		cvCircle( portal, center, 2, CV_RGB (255, 0, 0 ),1);
	}

	cvShowImage("Portal", portal);
	cvWaitKey(0);
	cvDestroyWindow("Portal");
	cvReleaseImage(&portal);
	//edge extrapolation
}
//input line[0] and line[1] line extending direction, line[2][3] x,y coordinate
//len extending length, width of image line section range [-(len-1)/2,(len)/2]
void GetProfile(float line[4],int len,int Width, vector<int>&pontiff)
{
	pontiff.clear();
	int relay,j;
	
	CvPoint pt1;
	
	for(j=-(len-1)/2;j<len/2+1;++j)
	{		
		pt1.x = cvRound(line[2]+line[0]*j);
		pt1.y = cvRound(line[3]+line[1]*j);
		relay=pt1.y*Width+pt1.x;							
		pontiff.push_back(relay);
		
	}
}
void GetBand(GDALDataset* m_pSrc,float*buf,int d)
{
	GDALRasterBand  *m_pBand=NULL;
	int Width=m_pSrc->GetRasterXSize(); //影响的高度，宽度
	int	Height=m_pSrc->GetRasterYSize();
	float *tempbuf=new float[Width*Height];
	int x,y,sernum,temp;
		m_pBand= m_pSrc->GetRasterBand(d);
		if (m_pBand)
		{	
			if (CE_None==m_pBand->RasterIO( GF_Read,0,0, Width, Height, tempbuf, Width,Height, GDT_Float32, 0, 0 ))
			{
				sernum=0;
				temp=Height*Width-Width;
				for(y=0;y<Height;++y)
				{
					for (x = 0; x < Width; ++x) 
					{
						buf[sernum]=tempbuf[temp];						
						++sernum;
						++temp;
					}
					temp-=2*Width;
				}
			}
		}
		delete[]tempbuf;
}
//compute kappa coefficient 
double Eval(int* ts, int*ind,int numd,int rts,int rind)
{//test result, indicator after clustering, number data, range of test sample counting from 1
	//rind range of indicator counting from 1
	if(rts>rind)
	{
		int temp=rind;
		rind=rts;
		rts=temp;
		int *port=ind;
		ind=ts;
		ts=port;
	}
	int i=0,j,k;
	int s=0,t=0;
	int *eu=new int[rts*rind];//count the occurence of each entry
	int *rl=new  int[rts];//record the index for the maximum in each row
	int *maxm=new int[rts];//record maxima for each row
	int *rms=new int[rts];//row sum except the maxima
	int *cms=new int[rind];//column sum except the maxima
	memset(eu,0,sizeof(int)*rts*rind);
	memset(rl,0,sizeof(int)*rts);
	memset(maxm,0,sizeof(int)*rts);
	memset(rms,0,sizeof(int)*rts);	
	memset(cms,0,sizeof(int)*rind);
	int *ptr;
	int num=0;double po=0,pe=0;
	for(i=0;i<numd;i++)
	{
		j=ts[i];
		k=ind[i];
		eu[j*rind+k]++;
	}
	//compute po 
	for(i=0;i<rts;i++)
	{
		ptr=max_element((eu+i*rind),(eu+(i+1)*rind));
		rl[i]=ptr-(eu+i*rind);
		maxm[i]=*ptr;
	}
	//compute pe
	for(i=0;i<rts;i++)
	{
		s=rl[i];
		for(j=0;j<rts;j++)
		{
			t=rl[j];
			cms[s]+=eu[j*rind+s];
			rms[i]+=eu[i*rind+t];
		}
	}
	for(i=0;i<rts;i++)
	{
		s=rl[i];
		pe+=(rms[i]-maxm[i])*(cms[s]-maxm[i]);
		num+=rms[i];
		po+=maxm[i];
	}
	pe/=(num*num);
	po/=num;
	delete []rl;delete []eu;
	delete []maxm;delete []rms;delete []cms;
	return (po-pe)/(1-pe);
}
double Eval(int* eu,int rts,int rind)
{//test result, indicator after clustering, number data, range of test sample counting from 1
	//rind range of indicator counting from 1
	//some modification's needed when rts>rind 
	int i=0,j,k;
	if(rts>rind)
	{
		int temp=rind;
		rind=rts;
		rts=temp;
		int *eut=new int[rts*rind];
		for(j=0;j<rts;j++)
			for(k=0;k<rind;k++)
				eut[j*rind+k]=eu[k*rind+j];
		for(j=0;j<rts*rind;j++)
			eu[j]=eut[j];
		delete []eut;
	}

	int s=0,t=0;
	int *rl=new  int[rts];//record the index for the maximum in each row
	int *maxm=new int[rts];//record maxima for each row
	int *rms=new int[rts];//row sum except the maxima
	int *cms=new int[rind];//column sum except the maxima

	memset(rl,0,sizeof(int)*rts);
	memset(maxm,0,sizeof(int)*rts);
	memset(rms,0,sizeof(int)*rts);	
	memset(cms,0,sizeof(int)*rind);
	int *ptr;
	int num=0;double po=0,pe=0;
	//compute po 
	for(i=0;i<rts;i++)
	{
		ptr=max_element((eu+i*rind),(eu+(i+1)*rind));
		rl[i]=ptr-(eu+i*rind);
		maxm[i]=*ptr;
	}
	//compute pe
	for(i=0;i<rts;i++)
	{
		s=rl[i];
		for(j=0;j<rts;j++)
		{
			t=rl[j];
			cms[s]+=eu[j*rind+s];
			rms[i]+=eu[i*rind+t];
		}
	}
	for(i=0;i<rts;i++)
	{
		s=rl[i];
	
		pe+=(rms[i]-maxm[i])*(cms[s]-maxm[i]);
		num+=rms[i];
		po+=maxm[i];
	}
	pe/=(num*num);
	po/=num;
	delete []rl;
	delete []maxm;delete []rms;delete []cms;
	return (po-pe)/(1-pe);
}
//frc has been normalized, bench contains profile pixels, test contains test profile pixels
float Correlate(float *src, vector<int>&bench, vector<int>&test)
{
	assert(bench.size()==test.size());
	int bs=bench.size(),i;
	float simil=0;
	for(i=0;i<bs;++i)	
		simil+=square(src[bench[i]]-src[test[i]]);	
	return simil;
}
//road tracking starting from valid endpoints stored in terminal
//in roadmap, 0 for background,1 for road, 2 for road centerline,3 for terminal points.
// in terminal the order index of end point
void RoadExpo(CHC*PHC,BYTE*roadmap,vector<int>&terminal,const char*fName)
{
	int tsize=terminal.size(),i,j,k;//x,y coordinate i,j for index incremental
	vector<int> edgecode;
	int normin[8]={0},dest[8]={0};
	int sernum,count,runlen,next;
	int x,y,hsize;//x,y coordinate for pixel, tsize the size of stack storing terminals, 
	//hsize terminals deleted during pruning
	const int dangle=5;
	bool flag,lof;//flag for detecting small spurs, lof linked or failed for while loop in road tracking	
	CvSize bound=cvSize(PHC->Width,PHC->Height);
	IplImage*portal=cvCreateImage(bound, IPL_DEPTH_8U , 3);
	cvNamedWindow("Portal", 0);		
	cvZero(portal);
	
	float *vegind=new float[PHC->Width*PHC->Height];
	GDALDataset* m_pSrc=(GDALDataset *) GDALOpen(fName,GA_ReadOnly);
	GetBand(m_pSrc,vegind,1);
	GDALClose((GDALDatasetH)m_pSrc);
	for(i=0;i<PHC->Height;++i)
	{
		for(j=0;j<PHC->Width;++j)
		{				
			if(roadmap[i*PHC->Width+j]==1)//(img2[j][i]==1)
			{
				uchar* temp_ptr = &((uchar*)(portal->imageData + portal->widthStep*i))[j*3];
			temp_ptr[0]=0;
			temp_ptr[1]=255;
			temp_ptr[2]=0;
			}
			if(roadmap[i*PHC->Width+j]==2)//(img2[j][i]==1)
			{
				uchar* temp_ptr = &((uchar*)(portal->imageData + portal->widthStep*i))[j*3];
			temp_ptr[0]=255;
			temp_ptr[1]=0;
			temp_ptr[2]=0;
			}
		}   
	}
	hsize=0;
	for(i=0;i<tsize;++i)
	{	
		sernum=terminal[i];//position in image array
		if(roadmap[sernum]<2)
			continue;
		runlen=0;
		flag=true;//true for proceeding, false for halt while loop
		edgecode.clear();
		//get the last five pixels connecting the terminal and fit line
		while(runlen<dangle)
		{
			x=sernum%PHC->Width;
			y=sernum/PHC->Width;
			for(j=0;j<8;++j)
				normin[j]=0;
			if(x==0)
			{
				if(y==0)
				{					
					normin[0]=roadmap[sernum+1];
					normin[6]=roadmap[sernum+PHC->Width];
					normin[7]=roadmap[sernum+1+PHC->Width];
				}
				else if(y==PHC->Height-1)
				{
					normin[0]=roadmap[sernum+1];
					normin[1]=roadmap[sernum+1-PHC->Width];
					normin[2]=roadmap[sernum-PHC->Width];
				}
				else
				{
					normin[0]=roadmap[sernum+1];
					normin[1]=roadmap[sernum+1-PHC->Width];
					normin[2]=roadmap[sernum-PHC->Width];
					normin[6]=roadmap[sernum+PHC->Width];
					normin[7]=roadmap[sernum+1+PHC->Width];					
				}
			}
			else if(x==PHC->Width-1)
			{
				if(y==0)
				{
					normin[4]=roadmap[sernum-1];
					normin[5]=roadmap[sernum-1+PHC->Width];
					normin[6]=roadmap[sernum+PHC->Width];
				}
				else if(y==PHC->Height-1)
				{
					normin[2]=roadmap[sernum-PHC->Width];
					normin[3]=roadmap[sernum-1-PHC->Width];
					normin[4]=roadmap[sernum-1];
				}
				else
				{
					normin[2]=roadmap[sernum-PHC->Width];
					normin[3]=roadmap[sernum-1-PHC->Width];
					normin[4]=roadmap[sernum-1];
					normin[5]=roadmap[sernum-1+PHC->Width];
					normin[6]=roadmap[sernum+PHC->Width];					
				}
			}
			else
			{
				if(y==0)
				{
					normin[0]=roadmap[sernum+1];
					normin[4]=roadmap[sernum-1];
					normin[5]=roadmap[sernum-1+PHC->Width];
					normin[6]=roadmap[sernum+PHC->Width];
					normin[7]=roadmap[sernum+1+PHC->Width];
				}
				else if(y==PHC->Height-1)
				{
					normin[0]=roadmap[sernum+1];
					normin[1]=roadmap[sernum+1-PHC->Width];
					normin[2]=roadmap[sernum-PHC->Width];
					normin[3]=roadmap[sernum-1-PHC->Width];
					normin[4]=roadmap[sernum-1];
				}
				else
				{
					normin[0]=roadmap[sernum+1];
					normin[1]=roadmap[sernum+1-PHC->Width];
					normin[2]=roadmap[sernum-PHC->Width];
					normin[3]=roadmap[sernum-1-PHC->Width];
					normin[4]=roadmap[sernum-1];
					normin[5]=roadmap[sernum-1+PHC->Width];
					normin[6]=roadmap[sernum+PHC->Width];
					normin[7]=roadmap[sernum+1+PHC->Width];
					
				}
			}					
			count=0;
			for(k=0;k<8;++k)
			{
				if(normin[k]==2)
				{
					dest[count]=k;
					++count;
				}
			}
			switch(count)
			{				
			case 1:
				if(runlen>0)					
				{
					AfxMessageBox("small edge with length less than 7 exist!");
					flag=false;
				}
				next=dest[0];
				edgecode.push_back(sernum);
				++runlen;
				break;
			case 2:
				assert(runlen>0);
				next=next>3?next-4:next+4;
				next=dest[0]+dest[1]-next;
				
				edgecode.push_back(sernum);
				++runlen;
				break;
			case 3:
				flag=false;
				break;
			default: 
				AfxMessageBox("neighbor count illegal!");
				break;
				
			}
			if(flag==false)
				break;
			switch(next)
			{
			case 0:
				++sernum;
				break;
			case 1:
				sernum=sernum-PHC->Width+1;
				break;
			case 2:
				sernum=sernum-PHC->Width;
				break;
			case 3:
				sernum=sernum-PHC->Width-1;
				break;
			case 4:
				sernum=sernum-1;
				break;
			case 5:
				sernum=sernum+PHC->Width-1;
				break;
			case 6:
				sernum=sernum+PHC->Width;
				break;
			case 7:
				sernum=sernum+PHC->Width+1;
				break;
			default:
				AfxMessageBox("next neighbor index exceeds bound!");
			}			
			
		}//end while loop for tail trapping
		assert(runlen==edgecode.size());
		if(runlen<dangle)//pruning  pixels in edgecode 
		{
			for(k=0;k<runlen;++k)			
				--roadmap[edgecode[k]];
			++hsize;
			continue;
		}
		//extrapolating using profile matching
		/*	if()//search edgecode[0] radius=3 neighborhood
		{
		continue;
		}*/
		//get template profile perpendicular to the line in the further end to avoid spurs
		
		float line[4],tmpl[4];
		int end[2]={0},t,start,left,right;
		int touch=0,relay,dstep;//decide whether both direction touched the border
		vector<int> profile,pontiff;//template profile and test profile
		float temp,mini;
		CvPoint pt1;
		assert(dangle==edgecode.size());
		CvPoint* points = (CvPoint*)malloc( dangle * sizeof(points[0]));
		CvMat pointMat = cvMat( 1, dangle, CV_32SC2, points );
		for( k=0;k<dangle;++k)
		{
			points[k].x = edgecode[k]%PHC->Width;
			points[k].y = edgecode[k]/PHC->Width;
		}
		cvFitLine( &pointMat, CV_DIST_L1, 1, 0.001, 0.001, line );
		relay=edgecode[dangle-1];
		
		t=0;
		while(roadmap[relay]>0)
		{
			++t;
			pt1.x = cvRound(points[dangle-1].x+line[1]*t);
			pt1.y = cvRound(points[dangle-1].y- line[0]*t);
			
			relay=pt1.y*PHC->Width+pt1.x;
		}
		end[0]=1-t;
		t=0;
		relay=edgecode[dangle-1];
		while(roadmap[relay]>0)
		{
			++t;
			pt1.x = cvRound(points[dangle-1].x - line[1]*t);
			pt1.y = cvRound(points[dangle-1].y + line[0]*t);
			
			relay=pt1.y*PHC->Width+pt1.x;
		}
		end[1]=t-1;
		if(end[1]-end[0]>8)//road anormaly fat reject terminal
			continue;
		profile.clear();
		for(j=end[0];j<end[1]+1;++j)
		{
			pt1.x = cvRound(points[dangle-1].x-line[1]*j);
			pt1.y = cvRound(points[dangle-1].y+line[0]*j);
			relay=pt1.y*PHC->Width+pt1.x;
			profile.push_back(relay);
		}
		for(j=0;j<profile.size();++j)
		{				
			x=profile[j]%PHC->Width;
			y=profile[j]/PHC->Width;
			uchar* temp_ptr = &((uchar*)(portal->imageData + portal->widthStep*y))[x*3];
			temp_ptr[0]=0;
			temp_ptr[1]=0;
			temp_ptr[2]=255;
		}
		lof=false;
		start=edgecode[0];
		while(!lof)			
		{			
			//predict the next pixel and retrieve teh profile based on the line direction with increment of x=3
			//points[0] is the terminal point and the line direction is from point[0] to point[5] so minus
			k=profile.size();
			dstep=(dangle+1)/2;
			//7.f equals to move ahead with step 3
			if((points[0].x-points[dangle-1].x>0)||((points[0].x-points[dangle-1].x==0)&&(points[0].y-points[dangle-1].y>0)))
			{
				x=start%PHC->Width;
					y=start/PHC->Width;
					tmpl[2]= cvRound(x+line[0]*dstep);
					tmpl[3]= cvRound(y+line[1]*dstep);
				tmpl[0]=-line[1];
				tmpl[1]=line[0];
					relay=tmpl[3]*PHC->Width+tmpl[2];
					GetProfile(tmpl,k,PHC->Width,pontiff);
					temp=Correlate(vegind,profile,pontiff);
					mini=temp;
					tmpl[2]= cvRound(x+line[1]);
					tmpl[3] = cvRound(y-line[0]);
					GetProfile(tmpl,k,PHC->Width,pontiff);
						temp=Correlate(vegind,profile,pontiff);
						if(temp<mini)
						{	mini=temp;
						relay=tmpl[3]*PHC->Width+tmpl[2];
						}
					tmpl[2]= cvRound(x-line[1]);
					tmpl[3] = cvRound(y+line[0]);
				GetProfile(tmpl,k,PHC->Width,pontiff);
					Correlate(vegind,profile,pontiff);
						if(temp<mini)
						{	mini=temp;
						relay=tmpl[3]*PHC->Width+tmpl[2];
						}
			}
			else
			{
				x=start%PHC->Width;
					y=start/PHC->Width;
					tmpl[2]= cvRound(x-line[0]*dstep);
					tmpl[3] = cvRound(y-line[1]*dstep);
					tmpl[0]=-line[1];
				tmpl[1]=line[0];
					relay=y*PHC->Width+x;
					GetProfile(tmpl,k,PHC->Width,pontiff);
					tmpl[2]= cvRound(x+line[1]);
					tmpl[3] = cvRound(y-line[0]);
				left=y*PHC->Width+x;
				GetProfile(tmpl,k,PHC->Width,pontiff);
					tmpl[2]= cvRound(x-line[1]);
					tmpl[3]= cvRound(y+line[0]);
				right=y*PHC->Width+x;
				GetProfile(tmpl,k,PHC->Width,pontiff);
			}
			
			for(j=0;j<pontiff.size();++j)
			{				
				x=pontiff[j]%PHC->Width;
				y=pontiff[j]/PHC->Width;
				uchar* temp_ptr = &((uchar*)(portal->imageData + portal->widthStep*y))[x*3];
				temp_ptr[0]=0;
				temp_ptr[1]=0xee;
				temp_ptr[2]=0xee;	
			}		
			lof=true;
			//categorize the line direction and decide the neighboring two candidate pixels
			//profile correlation to determine the next pixel
			//if profile matching is less than a threshold, then go to next terminal
			//if next position is decided, search its head side half of 7*7 window for nodes,if exist, the iteration 
			//is go to next terminal
		}
	}
	edgecode.clear();
	delete[]vegind;
	/*	count=terminal.size();
	for(k=0;k<count;++k)
	{
		i=terminal[k]/Width;
		j=terminal[k]%Width;
		CvPoint center=cvPoint(j,i);		
		cvCircle( portal, center, 2, CV_RGB (255, 0, 0 ),1);
	}*/

	cvShowImage("Portal", portal);
	cvWaitKey(0);
	cvDestroyWindow("Portal");
	cvReleaseImage(&portal);	
}

void GetLevelArray(CString src,vector<int>&use)
{
	use.clear();
	if(src=="")
		return;
	const char* str=(const char*)src;
	int dp,j=0;
	/* Input various data from tokenstring: */
	while(str[j]!='\0')
	{
		sscanf(str+j,"%d",&dp);
		use.push_back(dp);
		while(str[j]!=' '&&str[j]!=','&&str[j]!='\0')
			++j;
		if(str[j]=='\0')
			break;
		else
			++j;		
	}
}

void GetBWArray(CString src, vector<float>&bWArray)
{
	bWArray.clear();
	if(src=="")
	{
		return;
	}
	const char* str=(const char*)src;
	float fp;
	int j=0;
	/* Input various data from tokenstring: */
	while(str[j]!='\0')
	{
		sscanf(str+j,"%f",&fp);
		bWArray.push_back(fp);
		while(str[j]!=' '&&str[j]!=','&&str[j]!='\0')
			++j;
		if(str[j]=='\0')
			break;
		else
			++j;		
	}
}
//index=0,1,2 for spectral band 
//index=11, 12,13 for shape feature
void CHC::GetHistogramData(int *dataBin, float &rmax,float &rmin)
{
	if(exS.size()==0)
	{
		AfxMessageBox("Compute Region Property before preview!");
		return;
	}
	memset(dataBin,0,sizeof(int)*256);
	int i, length, binnum;
	int index=typeProp;
	float step,curd;//range max,range min,current data
	exRegion turg;//temp exregion
	vector<exRegion>::iterator xiter;
	length=S.size();
	if(index<10)//use S.addition
	{
		if(index>d_-1)
			index=d_-1;
		
	
		rmax=0;
		rmin=255;
		for(i=0;i<length;++i)
		{
			if(S[i].p!=i)
				continue;
			curd=S[i].addition[index]/S[i].size*Range;
			rmax=__max(rmax,curd);
			rmin=__min(rmin,curd);		
		}
		step=(rmax-rmin)/255.f;
		for(i=0;i<length;++i)
		{
			if(S[i].p!=i)
				continue;
			curd=S[i].addition[index]/S[i].size*Range;
			binnum=(int)floor((curd-rmin)/step);
			++dataBin[binnum];				
		}	
	}
	else
	{		
		switch(index)
		{
		case REGSIZE:
			rmax=0;
			rmin=1e10;
		xiter=exS.begin();
			while(xiter!=exS.end())
			{
				curd=S[xiter->label].size;
				rmax=__max(rmax,curd);
				rmin=__min(rmin,curd);
				++xiter;
			}
			step=(rmax-rmin)/255.f;
				xiter=exS.begin();
			while(xiter!=exS.end())
			{
				curd=S[xiter->label].size;		
				binnum=(int)floor((curd-rmin)/step);
				++dataBin[binnum];	
				++xiter;
			}
			break;
		case PERIM:
			rmax=0;
			rmin=1e10;
			xiter=exS.begin();
			while(xiter!=exS.end())
			{
				curd=S[xiter->label].perim;
				rmax=__max(rmax,curd);
				rmin=__min(rmin,curd);
					++xiter;
			}
			step=(rmax-rmin)/255.f;
			xiter=exS.begin();
			while(xiter!=exS.end())
			{
				curd=S[xiter->label].perim;			
				binnum=(int)floor((curd-rmin)/step);
				++dataBin[binnum];
					++xiter;
			}
			break;
		case CMPCT:
			rmax=0;
			rmin=1;
			xiter=exS.begin();
			while(xiter!=exS.end())
			{
				curd=S[xiter->label].size/(xiter->attList[minRectHeg]*xiter->attList[minRectWid]);
				rmax=__max(rmax,curd);
				rmin=__min(rmin,curd);	
				++xiter;
			}
			step=(rmax-rmin)/255.f;
			xiter=exS.begin();
			while(xiter!=exS.end())
			{	
				curd=S[xiter->label].size/(xiter->attList[minRectHeg]*xiter->attList[minRectWid]);
				binnum=(int)floor((curd-rmin)/step);
				++dataBin[binnum];
				++xiter;
			}
			break;
		case MAXAL:
			rmax=0;
			rmin=1e5;
			xiter=exS.begin();
			while(xiter!=exS.end())
			{
				curd=__max(xiter->attList[minRectHeg],xiter->attList[minRectWid]);
				rmax=__max(rmax,curd);
				rmin=__min(rmin,curd);	
				++xiter;
			}
			step=(rmax-rmin)/255.f;
			xiter=exS.begin();
			while(xiter!=exS.end())
			{	
				curd=__max(xiter->attList[minRectHeg],xiter->attList[minRectWid]);
				binnum=(int)floor((curd-rmin)/step);
				++dataBin[binnum];
				++xiter;
			}
			break;
		case MEANW:
			rmax=0;
			rmin=1e5;
			xiter=exS.begin();
			while(xiter!=exS.end())
			{	
				curd=xiter->attList[meanThick];
				rmax=__max(rmax,curd);
				rmin=__min(rmin,curd);
				++xiter;
			}
			step=(rmax-rmin)/255.f;
			xiter=exS.begin();
			while(xiter!=exS.end())
			{	
				curd=xiter->attList[meanThick];		
				binnum=(int)floor((curd-rmin)/step);
				++dataBin[binnum];	
				++xiter;
			}
			break;
		case ELONG:
			rmax=0;
			rmin=1e5;
			xiter=exS.begin();
			while(xiter!=exS.end())
			{	
				curd=__max(xiter->attList[minRectHeg],xiter->attList[minRectWid])/__min(xiter->attList[minRectHeg],xiter->attList[minRectWid]);
				
				curd=__min(xiter->attList[eigRatio],curd);
				rmax=__max(rmax,curd);
				rmin=__min(rmin,curd);
				++xiter;
			}
			step=(rmax-rmin)/255.f;
			xiter=exS.begin();
			while(xiter!=exS.end())
			{	
				curd=__max(xiter->attList[minRectHeg],xiter->attList[minRectWid])/__min(xiter->attList[minRectHeg],xiter->attList[minRectWid]);
				
				curd=__min(xiter->attList[eigRatio],curd);	
				binnum=(int)floor((curd-rmin)/step);
				++dataBin[binnum];	
				++xiter;
			}
			break;
		case SHADERATIO:
			rmax=0;
			rmin=1;
			xiter=exS.begin();
			while(xiter!=exS.end())
			{				
				curd=xiter->attList[shadeRatio];
				rmax=__max(rmax,curd);
				rmin=__min(rmin,curd);
				++xiter;
			}
			step=(rmax-rmin)/255.f;
			xiter=exS.begin();
			while(xiter!=exS.end())
			{				
				curd=xiter->attList[shadeRatio];
				binnum=(int)floor((curd-rmin)/step);
				++dataBin[binnum];	
				++xiter;
			}
			break;
		case NEISHADERATIO:
			rmax=0;
			rmin=1;
			xiter=exS.begin();
			while(xiter!=exS.end())
			{				
				curd=xiter->attList[neiShadeRatio];
				rmax=__max(rmax,curd);
				rmin=__min(rmin,curd);
				++xiter;
			}
			step=(rmax-rmin)/255.f;
			xiter=exS.begin();
			while(xiter!=exS.end())
			{	
				
				curd=xiter->attList[neiShadeRatio];
				binnum=(int)floor((curd-rmin)/step);
				++dataBin[binnum];	
				++xiter;
			}
			break;
		case CHANGERATIO:
			rmax=0;
			rmin=1;
			xiter=exS.begin();
			while(xiter!=exS.end())
			{				
				curd=xiter->attList[changeRatio];
				rmax=__max(rmax,curd);
				rmin=__min(rmin,curd);
				++xiter;
			}
			step=(rmax-rmin)/255.f;
			xiter=exS.begin();
			while(xiter!=exS.end())
			{	
				
				curd=xiter->attList[changeRatio];
				binnum=(int)floor((curd-rmin)/step);
				++dataBin[binnum];	
				++xiter;
			}
			break;
		default:			
			AfxMessageBox("This index is illegible!");
		}
	}
}
//distinguish regions with the type indexed property value larger than thresh 
//those regions locate in wind defined area, pixels concerned forms (left,right-1)x(top,bottom-1)
//use opencv image to display the result
void RegionThresh(CHC&myHC,float thresh,CRect wind,IplImage*cpy)
{
	if(myHC.S.size()==0)
	{
		AfxMessageBox("Data not available!");
		return;
	}
	int i,j,sernum,label;
	float temp;
	Region*chariot;
	if(wind.top<0)
		wind.top=0;
	if(wind.bottom>myHC.Height)
		wind.bottom=myHC.Height;
	if(wind.left<0)
		wind.left=0;
	if(wind.right>myHC.Width)
		wind.right=myHC.Width;
	int mH=wind.bottom-wind.top;
	int mW=wind.right-wind.left;


	exRegion turg;//temp exregion
	vector<exRegion>::iterator xiter;

	cvZero(cpy);
	if(myHC.typeProp<10)//use S.addition
	{
		if(myHC.typeProp>myHC.d_-1)
			myHC.typeProp=myHC.d_-1;
		sernum=wind.top*myHC.Width+wind.left;
		for(i=0;i<mH;++i)
		{
			for(j=0;j<mW;++j)
			{			
				chariot=&myHC.S[myHC.tag[sernum]];
				assert(chariot->p==myHC.tag[sernum]);
				if((chariot->addition[myHC.typeProp]/(chariot->size)*Range)>=thresh)
				{
				uchar* temp_ptr = &((uchar*)(cpy->imageData + cpy->widthStep*i))[j*3];
				temp_ptr[0]=0;
				temp_ptr[1]=255;
				temp_ptr[2]=0;
				}			
				++sernum;
			}
			sernum+=(myHC.Width-mW);
		}
	}
	else
	{
		switch(myHC.typeProp)
		{
		case REGSIZE:
			sernum=wind.top*myHC.Width+wind.left;
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					chariot=&myHC.S[myHC.tag[sernum]];
					assert(chariot->p==myHC.tag[sernum]);
					if(chariot->size>=thresh)
					{
						uchar* temp_ptr = &((uchar*)(cpy->imageData + cpy->widthStep*i))[j*3];
						temp_ptr[0]=0;
						temp_ptr[1]=255;
						temp_ptr[2]=0;
					}			
					++sernum;
				}
				sernum+=(myHC.Width-mW);
			}
			break;
		case PERIM:
			sernum=wind.top*myHC.Width+wind.left;
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					chariot=&myHC.S[myHC.tag[sernum]];
					assert(chariot->p==myHC.tag[sernum]);
					if(chariot->perim>=thresh)
					{
						uchar* temp_ptr = &((uchar*)(cpy->imageData + cpy->widthStep*i))[j*3];
						temp_ptr[0]=0;
						temp_ptr[1]=255;
						temp_ptr[2]=0;
					}			
					++sernum;
				}
				sernum+=(myHC.Width-mW);
			}
			break;
		case CMPCT:
			sernum=wind.top*myHC.Width+wind.left;
			
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					label=myHC.tag[sernum];
					assert(myHC.S[label].p==label);
					
					turg.label=label;
					xiter=lower_bound(myHC.exS.begin(),myHC.exS.end(),turg);
					temp=myHC.S[label].size/(xiter->attList[minRectHeg]*xiter->attList[minRectWid]);
					
					if(temp>=thresh)
					{
						uchar* temp_ptr = &((uchar*)(cpy->imageData + cpy->widthStep*i))[j*3];
						temp_ptr[0]=0;
						temp_ptr[1]=255;
						temp_ptr[2]=0;
					}			
					++sernum;
				}
				sernum+=(myHC.Width-mW);
			}
			break;
		case MAXAL:
			sernum=wind.top*myHC.Width+wind.left;
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{				
					turg.label=myHC.tag[sernum];
					xiter=lower_bound(myHC.exS.begin(),myHC.exS.end(),turg);
					
					if(__max(xiter->attList[minRectHeg],xiter->attList[minRectWid])>=thresh)
					{
						uchar* temp_ptr = &((uchar*)(cpy->imageData + cpy->widthStep*i))[j*3];
						temp_ptr[0]=0;
						temp_ptr[1]=255;
						temp_ptr[2]=0;
					}
					++sernum;
				}
				sernum+=(myHC.Width-mW);
			}
			break;
		case MEANW:
			sernum=wind.top*myHC.Width+wind.left;
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					turg.label=myHC.tag[sernum];
					xiter=lower_bound(myHC.exS.begin(),myHC.exS.end(),turg);
					if(xiter->attList[meanThick]>=thresh)
					{
						uchar* temp_ptr = &((uchar*)(cpy->imageData + cpy->widthStep*i))[j*3];
						temp_ptr[0]=0;
						temp_ptr[1]=255;
						temp_ptr[2]=0;
					}
					++sernum;
				}
				sernum+=(myHC.Width-mW);
			}
			break;
		case ELONG:
			sernum=wind.top*myHC.Width+wind.left;
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					turg.label=myHC.tag[sernum];
					xiter=lower_bound(myHC.exS.begin(),myHC.exS.end(),turg);
					temp=__max(xiter->attList[minRectHeg],xiter->attList[minRectWid])/__min(xiter->attList[minRectHeg],xiter->attList[minRectWid]);
					temp=__min(xiter->attList[eigRatio],temp);
					
					if(temp>=thresh)
					{
						uchar* temp_ptr = &((uchar*)(cpy->imageData + cpy->widthStep*i))[j*3];
						temp_ptr[0]=0;
						temp_ptr[1]=255;
						temp_ptr[2]=0;
					}
					++sernum;
				}
				sernum+=(myHC.Width-mW);
			}
			break;
		case SHADERATIO:
			sernum=wind.top*myHC.Width+wind.left;
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					turg.label=myHC.tag[sernum];
					xiter=lower_bound(myHC.exS.begin(),myHC.exS.end(),turg);
					temp=xiter->attList[shadeRatio];
					
					if(temp>=thresh)
					{
						uchar* temp_ptr = &((uchar*)(cpy->imageData + cpy->widthStep*i))[j*3];
						temp_ptr[0]=0;
						temp_ptr[1]=255;
						temp_ptr[2]=0;
					}
					++sernum;
				}
				sernum+=(myHC.Width-mW);
			}
			break;
		case NEISHADERATIO:
			sernum=wind.top*myHC.Width+wind.left;
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					turg.label=myHC.tag[sernum];
					xiter=lower_bound(myHC.exS.begin(),myHC.exS.end(),turg);
					temp=xiter->attList[neiShadeRatio];
					
					if(temp>=thresh)
					{
						uchar* temp_ptr = &((uchar*)(cpy->imageData + cpy->widthStep*i))[j*3];
						temp_ptr[0]=0;
						temp_ptr[1]=255;
						temp_ptr[2]=0;
					}
					++sernum;
				}
				sernum+=(myHC.Width-mW);
			}
			break;
		case CHANGERATIO:
			sernum=wind.top*myHC.Width+wind.left;
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					turg.label=myHC.tag[sernum];
					xiter=lower_bound(myHC.exS.begin(),myHC.exS.end(),turg);
					temp=xiter->attList[changeRatio];
					
					if(temp>=thresh)
					{
						uchar* temp_ptr = &((uchar*)(cpy->imageData + cpy->widthStep*i))[j*3];
						temp_ptr[0]=0;
						temp_ptr[1]=255;
						temp_ptr[2]=0;
					}
					++sernum;
				}
				sernum+=(myHC.Width-mW);
			}
			break;
		default:
			AfxMessageBox("this type information not available!");
			return;
		}
	}

}
//process large dataset, when image dataset is as large as 1300x1300, createcopy to save seg result
//does not work. then I turn to writeblock using origin dataset, always writing faults, for writeblock
//using new dataset, blocksizes are not compatible. at last, rasterio with create is applied.
/*GDALDataType { 
  GDT_Unknown = 0, GDT_Byte = 1, GDT_UInt16 = 2, GDT_Int16 = 3, 
  GDT_UInt32 = 4, GDT_Int32 = 5, GDT_Float32 = 6, GDT_Float64 = 7, 
  GDT_CInt16 = 8, GDT_CInt32 = 9, GDT_CFloat32 = 10, GDT_CFloat64 = 11, 
  GDT_TypeCount = 12 
}*/
//input  BN band number indicates which band to save, 0 corresponds to 1st band in gdaldataset, and so on.
//if bwarray[i]==0 which means band i+1 in gdaldataset is not processed in S,save operation will be ignored
//11- corresponds to features such as perimeter, bn==-1 to save all the processed mean object layers
//pathname is the full path for the original image data
void SaveSeg2(CHC&sHC,const CString&pathname,const int bn)
{
	if(bn<-1)
	{
		AfxMessageBox("Bad Band Number!");
		return;
	}
	GDALDataset *m_pDataset=(GDALDataset*) GDALOpen(pathname,GA_ReadOnly);
	exRegion turg;//temp exregion
	vector<exRegion>::iterator xiter;
	GDALRasterBand  *poBand=NULL;
	poBand= m_pDataset->GetRasterBand(1);
	//GDALDataType dataType=poBand->GetRasterDataType();//数据类型
	GDALDataType dataType=GDT_Float32;
	int x,y,i,j;
	int d,sernum,label;
	const char *pszFormat = "GTiff";
	GDALDriver *poDriver;
	
	poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	int pos=pathname.ReverseFind('\\');
	CString dir=pathname.Left(pos+1);
	if( poDriver == NULL)
	{
		AfxMessageBox("This format is not able to be created!");
		exit( 1 );	
	}
	//using create
	GDALDataset *poDstDS;       
    char **papszOptions = NULL;
	const char*pszDstFilename;
    OGRSpatialReference oSRS;
 
	double adfGeoTransform[6],backup[6] = { 444720, 30, 0, 3751320, 0, -30 };
	float* buf =new float[sHC.Width*sHC.Height];
	if(bn==-1)
	{
		CString name=dir+"panbands.tif";
		pszDstFilename=(const char*)name;
		poDstDS = poDriver->Create( pszDstFilename,sHC.Width,sHC.Height, sHC.d_,dataType, 
			papszOptions );
		
		if(CE_None==m_pDataset->GetGeoTransform( adfGeoTransform ))	
			poDstDS->SetGeoTransform( adfGeoTransform );
		//else
		//	poDstDS->SetGeoTransform(backup); 
		/*
			oSRS.SetUTM( 11, TRUE );
			oSRS.SetWellKnownGeogCS( "NAD27" );
			oSRS.exportToWkt( &pszSRS_WKT );
			poDstDS->SetProjection( pszSRS_WKT );
		*/
		
		const char*pszSRS_WKT=m_pDataset->GetProjectionRef();
		poDstDS->SetProjection( pszSRS_WKT );

		for(d=1;d<sHC.d_+1;++d)
		{
			
			poBand = poDstDS->GetRasterBand(d);	
			if (poBand)
			{	
				sernum=0;
				
				for(y=0;y<sHC.Height;++y)
				{
					for (x = 0; x < sHC.Width; ++x) 
					{
						label=sHC.tag[sernum];
						buf[sernum]=(sHC.S[label].addition[d-1])/(sHC.S[label].size)*Range;							
						
						++sernum;
					}
					
				}
				if (CE_None!=poBand->RasterIO( GF_Write,0,0, sHC.Width, sHC.Height, buf, sHC.Width,sHC.Height,GDT_Float32, 0, 0 ))
				{
					AfxMessageBox("error write mpdataset!");
				}
			}
		}
		delete[]buf; 
		GDALClose( (GDALDatasetH) poDstDS );
		return;
	}
	
	CString name;
	name.Format("Band%d.tif",bn);
	name=dir+name;
	pszDstFilename=(const char*)name;
    poDstDS = poDriver->Create( pszDstFilename,sHC.Width,sHC.Height, 1,dataType, 
		papszOptions );

	if(CE_None==m_pDataset->GetGeoTransform( adfGeoTransform ))	
		poDstDS->SetGeoTransform( adfGeoTransform );
	else
		poDstDS->SetGeoTransform(backup); 
	
	const char*pszSRS_WKT=m_pDataset->GetProjectionRef();
    poDstDS->SetProjection( pszSRS_WKT );
	GDALClose((GDALDatasetH)m_pDataset);
	poBand = poDstDS->GetRasterBand(1);	
	if(!poBand)
	{
		AfxMessageBox("Error fetching the 1st band in poDstDS!");
		return;
	}
	if(bn<10)
	{		
		sernum=0;
		
		for(y=0;y<sHC.Height;++y)
		{
			for (x = 0; x <sHC.Width; ++x) 
			{
				label=sHC.tag[sernum];
				buf[sernum]=(sHC.S[label].addition[bn])/(sHC.S[label].size)*Range;							
				
				++sernum;
			}
			
		}
		if (CE_None!=poBand->RasterIO( GF_Write,0,0, sHC.Width, sHC.Height, buf, sHC.Width,sHC.Height,GDT_Float32, 0, 0 ))
		{
			AfxMessageBox("error write mpdataset!");
		}		
	}
	else
	{
		if(sHC.S.size()==0)
		{		
			AfxMessageBox("partition image before saving!");
			return;		
		}
		if(sHC.exS.size()==0&&(bn>13))
		{		
			AfxMessageBox("Compute region attributes before saving!");
			return;		
		}
		turg.label=sHC.tag[0];
		xiter=lower_bound(sHC.exS.begin(),sHC.exS.end(),turg);
		int mH=sHC.Height,mW=sHC.Width;
		
		sernum=0;
		switch(bn)
		{
		case REGSIZE:
			
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{	
					
					label=sHC.tag[sernum];
					assert(sHC.S[label].p==label);
					buf[sernum]=sHC.S[label].size;	
					++sernum;
				}
				
			}
			break;
		case PERIM:
			
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					label=sHC.tag[sernum];
					assert(sHC.S[label].p==label);
					buf[sernum]=sHC.S[label].perim;	
					++sernum;
				}
				
			}
			break;
		case CMPCT:
			
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					label=sHC.tag[sernum];
					assert(sHC.S[label].p==label);
					buf[sernum]=sqrt(sHC.S[label].size*4/3.1416)/sHC.S[label].perim;			
					++sernum;
				}
			}
			break;
		case MAXAL:
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
				
					turg.label=sHC.tag[sernum];
					xiter=lower_bound(sHC.exS.begin(),sHC.exS.end(),turg);
				
					buf[sernum]=__max(xiter->attList[minRectWid],xiter->attList[minRectHeg]);	
					++sernum;
				}
			}
			break;
		case MEANW:
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					turg.label=sHC.tag[sernum];
					xiter=lower_bound(sHC.exS.begin(),sHC.exS.end(),turg);
				
					buf[sernum]=xiter->attList[meanThick];	
					++sernum;
				}
			}
			break;
		case MINAL:	
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
				turg.label=sHC.tag[sernum];
					xiter=lower_bound(sHC.exS.begin(),sHC.exS.end(),turg);
				
					buf[sernum]=__min(xiter->attList[minRectWid],xiter->attList[minRectHeg]);	
					++sernum;
				}
			}
			break;
		case COVLWR:
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					turg.label=sHC.tag[sernum];
					xiter=lower_bound(sHC.exS.begin(),sHC.exS.end(),turg);
				
					buf[sernum]=xiter->attList[eigRatio];	
					++sernum;
				}
			}
			break;
		case ELONG:
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
				turg.label=sHC.tag[sernum];
					xiter=lower_bound(sHC.exS.begin(),sHC.exS.end(),turg);
				
					buf[sernum]=xiter->attList[minRectHeg]/xiter->attList[minRectWid];
					if(buf[sernum]<1)
						buf[sernum]=1.f/buf[sernum];
					++sernum;
				}
			}
			break;
	
		default:
			{
				AfxMessageBox("this type information not available!");
				remove(name);
				break;
			}
		}
		
		if (CE_None!=poBand->RasterIO( GF_Write,0,0, sHC.Width, sHC.Height, buf, sHC.Width,sHC.Height,GDT_Float32, 0, 0 ))
		{
			AfxMessageBox("error write mpdataset!");
		}
		
	}
	delete[]buf; 
    GDALClose( (GDALDatasetH) poDstDS );	
}

void CHC::GetPreviewMask(CRect &rect)
{
	if(Height==0)
	{
		AfxMessageBox("CHC not yet initiated!");return;
	}
	if(Height<=256||Width<=256)
	{
		rect.top=0;
		rect.bottom=Height;
		rect.left=0;
		rect.right=Width;
		return;
	}
	rect.left=(Width-256)/2+1;
	rect.right=rect.left+256;
	rect.top=(Height-256)/2+1;
	rect.bottom=rect.top+256;
}

void CHC::Polygonize(CString path)
{
	int label, sernum;
	int d,x,y;
	float* buf =new float[Width*Height];
	memset(buf,0,sizeof(float)*Width*Height);
	const char *pszFormat = "GTiff";
    GDALDriver *pGdalDriver;

	int pos=path.ReverseFind('\\');
	path=path.Left(pos+1);
	CString name=path+"temp.tif";
	const char*	pszDstFilename=(const char*)name;

    pGdalDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);

    if( pGdalDriver == NULL )
        exit( 1 );
	GDALRasterBand  *poBand=NULL;
	GDALDataset *poDstDS;       
    char **papszOptions = NULL;
    
    papszOptions = CSLSetNameValue( papszOptions, "TILED", "YES" );
    papszOptions = CSLSetNameValue( papszOptions, "COMPRESS", "PACKBITS" );
    poDstDS = pGdalDriver->Create( pszDstFilename,Width, Height, 1, GDT_Float32, 
                                papszOptions );
	CSLDestroy( papszOptions );
	double adfGeoTransform[6] = { 444720, 30, 0, 3751320, 0, -30 };
    OGRSpatialReference oSRS;
    char *pszSRS_WKT = NULL;
    poDstDS->SetGeoTransform( adfGeoTransform );
	
	oSRS.SetGeogCS( "My geographic coordinate system",
		"WGS_1984", 
		"My WGS84 Spheroid", 
		SRS_WGS84_SEMIMAJOR, SRS_WGS84_INVFLATTENING, 
		"Greenwich", 0.0, 
		"degree", atof(SRS_UA_DEGREE_CONV));	
//GEOGCS["GCS_Tokyo",DATUM["D_Tokyo",SPHEROID["Bessel_1841",6377397.155,299.1528128]],PRIMEM["Greenwich",0.0],UNIT["Degree",0.0174532925199433]]
    oSRS.exportToWkt( &pszSRS_WKT );
    poDstDS->SetProjection( pszSRS_WKT );
    CPLFree( pszSRS_WKT );

    poBand = poDstDS->GetRasterBand(1);
	for(d=1;d<d_+1;++d)
	{	
		sernum=0;		
		for(y=0;y<Height;++y)
		{
			for (x = 0; x < Width; ++x) 
			{
				label=tag[sernum];
				buf[sernum]+=(uchar)((S[label].addition[d-1])/(S[label].size)*Range);
				++sernum;
			}			
		}		
	}
	if (CE_None!=poBand->RasterIO( GF_Write,0,0, Width, Height, buf, Width,Height,GDT_Float32, 0, 0 ))
	{
		AfxMessageBox("error write mpdataset!");
	}
	const char *pszDriverName = "ESRI Shapefile";
	
	OGRSFDriver *poDriver;
	
	poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(pszDriverName);
	name=path+"Polygon";
	OGRDataSource* ds = poDriver->CreateDataSource((const char*)name, NULL ); //create a folder,if polygon is replaced by 
	//polygon.shp only one file which contains one layer can be created
	
    if( ds == NULL )
    {
        printf( "Creation of output file failed.\n" );
        exit( 1 );
    }

	OGRLayer* poLayer = ds->CreateLayer("polygon",&oSRS,wkbMultiPolygon,NULL);
	
	CPLErr er =	GDALPolygonize(poBand , NULL, (OGRLayerH)poLayer ,1,NULL,NULL,NULL);
	if( er!=CE_None)
		AfxMessageBox("error write vector layer!");
	poLayer->SyncToDisk();
	OGRDataSource::DestroyDataSource(ds);	
	delete[]buf; 
	name=path+"temp.tif";
	remove((const char*)name);
//	CPLFree( (void*)pszSRS_WKT );
    GDALClose( (GDALDatasetH) poDstDS );
}

void CHC::VisitSeq( int*tagBe,int L)
{
	int indk=1,supl=0,i=0;
	int l=L;
	int tagp=0;
	int res=0;
	while(l>3)
	{
		l>>=2;
		++indk;
	}
	supl=1<<(2*indk);

	for(i=0;i<supl;++i)
	{
		res=DitherGen(indk,i);
		if(res>=L)
			continue;
		else
		{
			tagBe[tagp]=res;
			++tagp;
		}
	}
	assert(tagp==L);
}


//change v to intensity, namely avg of RGB
void RGB2HSV_Cylinder(int r, int g, int b, float &h, float &s, float &v)
{
	int max,min;
	float triangle;
	
	min=(r<g)?r:g;
	min=(min<b)?min:b;
	max=(r>g)?r:g;
	max=(max>b)?max:b;
	
	v=(r+g+b)/float(3.0);
	s=max-min;
	if(max==min)
	{
		h=0.0f;
		s=0.0f;
	}
	else
	{
		triangle=float(acos((double(0.5*((r-g)+(r-b))))/(sqrt(double((r-g)*(r-g)+(r-b)*(g-b))))));
		if(g>=b)
			h=triangle;
		else
			h=float(2*3.14159265-triangle);
	}
	
	h=float(h*180.0/3.14159265);
}

//transform R,G,B into average hue, saturation, and intensity as defined in Vincent Tao's integrating intensity, texture and color
//input: pDataset contains all the bands, options of size 3 are RGB index in pDataset, bandPtr pointer to 
//3 matrice, hue, saturation, and intensity in order.
void ColorTransform(int*options,CString dir,int number)
{
	GDALDataset*pDataset=(GDALDataset*)GDALOpen(dir,GA_ReadOnly);
	assure(pDataset,dir);
	GDALRasterBand  *m_pBand=NULL;
	float max=0, min=0;	
	char fname[100];
	memset(fname,0,sizeof(char)*100);
	float*orig[3],*proj[3];
	int w,h,d;
	int i,j;

	w=pDataset->GetRasterXSize();
	h=pDataset->GetRasterYSize();
	for(d=0;d<3;++d)
	{
		orig[d]=new float[w*h];
		proj[d]=new float[w*h];
	}
	
	for(d=0;d<3;++d)
	{	
		m_pBand= pDataset->GetRasterBand(options[d]);

		if (m_pBand)
		{	
			if (CE_None!=m_pBand->RasterIO( GF_Read,0,0, w, h, orig[d], w,h, GDT_Float32, 0, 0 ))
			{
				AfxMessageBox("Error READING RGB dataset!");
				return;
			}
		}
	}
	j=w*h;
	for(i=0;i<j;++i)
	RGB2HSV_Cylinder(orig[0][i],orig[1][i],orig[2][i],proj[0][i],proj[1][i],proj[2][i]);

	//save result in text file
	int pos=dir.ReverseFind('\\');
	dir=dir.Left(pos);

	sprintf(fname,"%s\\hue%d.txt",dir,number);

	Export(proj[0],w,h,fname);

	sprintf(fname,"%s\\sat%d.txt",dir,number);
	Export(proj[1],w,h,fname);
	sprintf(fname,"%s\\intensity%d.txt",dir,number);
	Export(proj[2],w,h,fname);
	for(d=0;d<3;++d)
	{
		delete []orig[d];
		delete []proj[d];
	}
	GDALClose((GDALDatasetH)pDataset);
}
//import one dimensional matrix data into memory in img
void ImportImg(const char*fn,IplImage*img)
{
	std::ifstream inp(fn);
	int i,j;
	float dest=0;
	inp>>i>>j;
	assert(i==img->width&&j==img->height);
	for(j=0;j<img->height;++j)
		for(i=0;i<img->width;++i)
		{
			inp>>dest;
			((float*)(img->imageData + img->widthStep*j))[i]=dest;
		}
	inp.close();
}
int CreateSub(IplImage*src,IplImage*sub,int label, int*tagArray,CRect box)
{
	int w=src->width,h=src->height;
	int i,j;
	int tp,bt,lf,rt,w1,h1,sernum,area;

	tp=box.top;
	bt=box.bottom;
	lf=box.left;
	rt=box.right;
	w1=rt-lf;
	h1=bt-tp;	
	sernum=tp*w+lf;
	area=0;
	//for each pixel in region Ai
	for(i=0;i<h1;++i)//each row
	{
		for(j=0;j<w1;++j)//each column
		{
			if(tagArray[sernum]==label)
			{
				((float*)(sub->imageData + sub->widthStep*i))[j]=
				((float*)(src->imageData + src->widthStep*(i+tp)))[j+lf];
				++area;
			}
			else
				((float*)(sub->imageData + sub->widthStep*i))[j]=-1;
			++sernum;				
		}
		sernum+=(w-w1);
	}
	assert(sernum==(bt*w+lf));
	return area;
}
//compute PC or PT in vincent tao's paper
float DeducePCT(IplImage*src,int label, int*tagArray,CRect box,int thresh)
{
	int w=src->width,h=src->height;
	int i,j;
	int tp,bt,lf,rt,w1,h1,sernum,area,numerator;

	tp=box.top;
	bt=box.bottom;
	lf=box.left;
	rt=box.right;
	w1=rt-lf;
	h1=bt-tp;

	sernum=tp*w+lf;
	area=0;
	numerator=0;
	//for each pixel in region Ai
	for(i=0;i<h1;++i)//each row
	{
		for(j=0;j<w1;++j)//each column
		{
			if(tagArray[sernum]==label)
			{
				if(((float*)(src->imageData + src->widthStep*(i+tp)))[j+lf]>thresh)
					++numerator;
				++area;
			}
			++sernum;
		}
		sernum+=(w-w1);
	}
	assert(sernum==(bt*w+lf));
	assert(area>0);
	if(area<1)
	{
		int x=numerator;
		return 0;
	}
	return (float)numerator/area;
}
//Get regional mean intensity
float DeduceMean(IplImage*src,int label, int*tagArray,CRect box)
{
	int w=src->width,h=src->height;
	int i,j;
	int tp,bt,lf,rt,w1,h1;

	tp=box.top;
	bt=box.bottom;
	lf=box.left;
	rt=box.right;
	w1=rt-lf;
	h1=bt-tp;
	
	int sernum=tp*w+lf;
	int area=0;
	float numerator=0;
	//for each pixel in region Ai
	for(i=0;i<h1;++i)//each row
	{
		for(j=0;j<w1;++j)//each column
		{
			if(tagArray[sernum]==label)
			{
				numerator+=((float*)(src->imageData + src->widthStep*(i+tp)))[j+lf];
				++area;
			}
			++sernum;
		}
		sernum+=(w-w1);
	}
	assert(sernum==(bt*w+lf));
	assert(area>0);
	if(area<1)
	{
		int x=numerator;
		return 0;
	}
	return numerator/area;
}

//convert Byte 1 channel image to float type
void ConvertImg(IplImage*src,IplImage*dst)
{
	int i,j,w=src->width,h=src->height;
	assert(w==dst->width);
	assert(h==dst->height);
	
	for(i=0;i<h;i++) 
		for(j=0;j<w;j++) 
			((float*)(dst->imageData + dst->widthStep*i))[j]=
			(float)((uchar*)(src->imageData + src->widthStep*i))[j];
		

}
//intensity similarity
void VRegSimi(CHC*PHC,IplImage*band1,IplImage*band2)
{
	CvHistogram* histv1;
	{
		int hist_size=256;	
		float v_ranges[] = { 0, 256 };
		float* ranges= v_ranges;
		histv1= cvCreateHist(
			1,
			&hist_size,
			CV_HIST_ARRAY,
			&ranges,
			1
			);
	}
	CvHistogram* histv2;
	{
		int hist_size=256;	
		float v_ranges[] = { 0, 256 };
		float* ranges= v_ranges;
		histv2= cvCreateHist(
			1,
			&hist_size,
			CV_HIST_ARRAY,
			&ranges,
			1
			);
	}
	int len=PHC->exS.size();
	int i,area,total=0;
	
	IplImage*roi;
	CvSize sz;
	for(i=0;i<len;++i)
	{	
		assert(i==PHC->tag[PHC->exS[i].label]);
		PHC->exS[i].attList[GMEAN1]=DeduceMean(band1,i, PHC->tag,PHC->grid[i]);
		PHC->exS[i].attList[GMEAN2]=DeduceMean(band2,i, PHC->tag,PHC->grid[i]);
		
		
		sz=cvSize(PHC->grid[i].Width(),PHC->grid[i].Height());
		
		roi=cvCreateImage(sz,IPL_DEPTH_32F,1);
		area=CreateSub(band1,roi,i,PHC->tag,PHC->grid[i]);
		total+=area;
		cvCalcHist( &roi, histv1, 0, 0 ); //Compute histogram		
		assert(TestHist(histv1,area));	

		cvNormalizeHist( histv1, 1.0 ); //Normalize it
			
		CreateSub(band2,roi,i,PHC->tag,PHC->grid[i]);
		cvCalcHist(&roi, histv2, 0, 0 ); //Compute histogram
		cvNormalizeHist( histv2, 1.0 ); //Normalize it
		PHC->exS[i].attList[ROUI]=cvCompareHist(histv1,histv2,CV_COMP_CORREL);
		cvReleaseImage(&roi);		
	}
	cvReleaseHist(&histv1);
	cvReleaseHist(&histv2);
	assert(total==(band1->width)*(band1->height));
}

bool TestHist(CvHistogram*histogram,int area)
{
	int n_dim= histogram->mat.dims;
	int *dimbins=new int[n_dim];
	int i,j;
	bool flag=false;
	float accum=0;
	for(i=0;i<n_dim;++i)
	{
		dimbins[i]=histogram->mat.dim[ i ].size;
	}
	switch(n_dim)
	{
	case 1:
		accum=0;
		for(i=0;i<dimbins[0];++i)
			accum+=*cvGetHistValue_1D(histogram,i);
		flag=(accum==(float)area);
		break;
	case 2:
		accum=0;
		for(i=0;i<dimbins[0];++i)
			for(j=0;j<dimbins[1];++j)
				accum+=*cvGetHistValue_2D(histogram,i,j);
			flag=(accum==(float)area);
			break;
	default:
		flag=false;
		break;		
	}
	delete []dimbins;
	return flag;
}
//HUE AND SAT BASED REGION SIMILARITY
void HSRegSimi(CHC*PHC,IplImage**bandPtr1,IplImage**bandPtr2,float satThresh)			   
{
	// Build the histogram of hue and satuartion and compute its contents.
	
	int h_bins = 60, s_bins = 16;
	CvHistogram* hisths1;
	{
		int hist_size[] = { h_bins, s_bins };
		float h_ranges[] = { 0, 360 }; // hue is [0,180]
		float s_ranges[] = { 0, 255 };
		float* ranges[] = { h_ranges, s_ranges };
		hisths1 = cvCreateHist(
			2,
			hist_size,
			CV_HIST_ARRAY,
			ranges,
			1
			);
	}
	CvHistogram* hisths2;
	{
		int hist_size[] = { h_bins, s_bins };
		float h_ranges[] = { 0, 360 }; // hue is [0,180]
		float s_ranges[] = { 0, 255 };
		float* ranges[] = { h_ranges, s_ranges };
		hisths2 = cvCreateHist(
			2,
			hist_size,
			CV_HIST_ARRAY,
			ranges,
			1
			);
	}
	int len=PHC->exS.size();
	int i,area,total=0;
	IplImage*roi1,*roi2;
	CvSize sz;
	for(i=0;i<len;++i)
	{	
		assert(i==PHC->tag[PHC->exS[i].label]);
		PHC->exS[i].attList[PC1]=DeducePCT(bandPtr1[1],i, PHC->tag,PHC->grid[i],satThresh);
		PHC->exS[i].attList[PC2]=DeducePCT(bandPtr2[1],i, PHC->tag,PHC->grid[i],satThresh);
		
		
		sz=cvSize(PHC->grid[i].Width(),PHC->grid[i].Height());
		
		roi1=cvCreateImage(sz,IPL_DEPTH_32F,1);
		roi2=cvCreateImage(sz,IPL_DEPTH_32F,1);
		CreateSub(bandPtr1[0],roi1,i,PHC->tag,PHC->grid[i]);
		area=CreateSub(bandPtr1[1],roi2,i,PHC->tag,PHC->grid[i]);
		total+=area;
		IplImage*planes[2]={roi1,roi2};
		cvCalcHist( planes, hisths1, 0, 0 ); //Compute histogram
		assert(TestHist(hisths1,area));	

		cvNormalizeHist( hisths1, 1.0 ); //Normalize it
		
		
		CreateSub(bandPtr2[0],roi1,i,PHC->tag,PHC->grid[i]);
		CreateSub(bandPtr2[1],roi2,i,PHC->tag,PHC->grid[i]);
		
		cvCalcHist( planes, hisths2, 0, 0 ); //Compute histogram
		cvNormalizeHist( hisths2, 1.0 ); //Normalize it
		PHC->exS[i].attList[ROUC]=cvCompareHist(hisths1,hisths2,CV_COMP_CORREL);
		cvReleaseImage(&roi1);
		cvReleaseImage(&roi2);
		
	}
	int w=bandPtr1[0]->width,h=bandPtr1[0]->height;

	assert(total==w*h);
	cvReleaseHist(&hisths1);
	cvReleaseHist(&hisths2);	
}
void CHC::CompRegSimi(float*storage)
{
	int i=0,len=exS.size();
	float vt,vi,vc,sum;
	float ct,ci,cc;
	for(;i<len;++i)
	{
		vt=__max(exS[i].attList[PT1],exS[i].attList[PT2])/0.2;
		vt*=(1.0-exS[i].attList[ROUT]);
		vi=1-__min(exS[i].attList[PT1],exS[i].attList[PT2]);
		vi*=(1-exS[i].attList[ROUI]);
		vi*=(fabs(exS[i].attList[GMEAN1]-exS[i].attList[GMEAN2])/128.0);
		vc=__max(exS[i].attList[PC1],exS[i].attList[PC2]);
		vc*=(1.0-exS[i].attList[ROUC]);
		vc*=(1-vt);
		vc*=(1-vi);
		sum=vt+vi+vc;
		assert(sum>=0);
		if(sum==0)
		{		
			ct=0.333f;
			ci=0.333f;
			cc=0.333f;			
		}	
		else
		{
			ct=vt/sum;
			ci=vi/sum;
			cc=vc/sum;
		}
		storage[i]=ct*exS[i].attList[ROUT]+
			ci*exS[i].attList[ROUI]+
			cc*exS[i].attList[ROUC];
	}	
}
void CHC::SaveTags(int*tagC)
{
	int L=Height*Width;
	int x,y,sernum,label;
	int w=Width, h=Height;
	BYTE*buf=new BYTE[L];
	GDALRasterBand  *poBand=NULL;

	GDALDataType dataType=GDT_Byte;

	const char *pszFormat = "GTiff";
	GDALDriver *poDriver;
	
	poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	
	if( poDriver == NULL)
	{
		AfxMessageBox("This format is not able to be created!");
		return;
	}
	//using create
	GDALDataset *poDstDS;       
    char **papszOptions = NULL;
	const char*pszDstFilename;
    OGRSpatialReference oSRS;
    char *pszSRS_WKT = NULL;
	double adfGeoTransform[6]= { 444720, 30, 0, 3751320, 0, -30 };
		oSRS.SetUTM( 11, TRUE );
		oSRS.SetWellKnownGeogCS( "NAD27" );
		oSRS.exportToWkt( &pszSRS_WKT );
	
	if(tagC!=NULL)
	{		
		pszDstFilename="tagComposite.tif";
		poDstDS = poDriver->Create( pszDstFilename,w,h, 2,dataType, 
			papszOptions );
		
		poDstDS->SetGeoTransform(adfGeoTransform); 
		
		poDstDS->SetProjection( pszSRS_WKT );
		
		
		poBand = poDstDS->GetRasterBand(1);	
		if (poBand)
		{
			
			sernum=0;
			
			for(y=0;y<Height;++y)
			{
				for (x = 0; x < Width; ++x) 
				{					
					label=tagC[sernum];
					buf[sernum]=255;
					if(x>0&&y>0&&(x<Width-1)&&(y<Height-1))
					{
						if(label!=tagC[sernum+1]||label!=tagC[sernum+Width])//one pixel width boundary
							buf[sernum]=0;
					}
					++sernum;
				}				
			}
			if (CE_None!=poBand->RasterIO( GF_Write,0,0, w, h, buf, w,h,GDT_Byte, 0, 0 ))
			{
				AfxMessageBox("error write mpdataset!");
			}
		}
		poBand = poDstDS->GetRasterBand(2);	
		if (poBand)
		{
			
			sernum=0;
			
			for(y=0;y<Height;++y)
			{
				for (x = 0; x < Width; ++x) 
				{					
					label=tag[sernum];
					buf[sernum]=255;
					if(x>0&&y>0&&(x<Width-1)&&(y<Height-1))
					{
						if(label!=tag[sernum+1]||label!=tag[sernum+Width])//one pixel width boundary
							buf[sernum]=0;
					}
					++sernum;
				}				
			}
			if (CE_None!=poBand->RasterIO( GF_Write,0,0, w, h, buf, w,h,GDT_Byte, 0, 0 ))
			{
				AfxMessageBox("error write mpdataset!");
			}
		}	
		
		
	}
	else if(tag!=NULL)
	{
		pszDstFilename="tagUni.tif";
		poDstDS = poDriver->Create( pszDstFilename,w,h, 1,dataType, 
			papszOptions );
		
		poDstDS->SetGeoTransform(adfGeoTransform); 
		
		poDstDS->SetProjection( pszSRS_WKT );
		
		
		poBand = poDstDS->GetRasterBand(1);	
		if (poBand)
		{			
			sernum=0;
			
			for(y=0;y<Height;++y)
			{
				for (x = 0; x < Width; ++x) 
				{					
					label=tag[sernum];
					buf[sernum]=255;
					if(x>0&&y>0&&(x<Width-1)&&(y<Height-1))
					{
						if(label!=tag[sernum+1]||label!=tag[sernum+Width])//one pixel width boundary
							buf[sernum]=0;
					}
					++sernum;
				}				
			}
			if (CE_None!=poBand->RasterIO( GF_Write,0,0, w, h, buf, w,h,GDT_Byte, 0, 0 ))
			{
				AfxMessageBox("error write mpdataset!");
			}
		}	
		
	}
	CPLFree( (void*)pszSRS_WKT );
	delete[]buf;
	GDALClose( (GDALDatasetH) poDstDS );	
}



//save segmentation in txt file as following
//in fn1
//Width,height,count, dimension count*2, exS properties dimension
//regno. as in S, reg size, reg rectangle, l,t,r,b, mean and variance for each dimension, region properties stored in exS
//in fn2
//pixel label minimized matrix [0, comps-1], so that in readseg tag[exS[i].p]==i for qthc and mrs

//method: first minimize/compact region label to range [0-regCount], store reg label and
// reg bounding box in file fn1, save tag for each pixel in fn2. both are in txt format
int CHC::StoreSeg(LPCTSTR fn1,LPCTSTR fn2)
{	
	int L=S.size();
	int i=0,j=0,k=0,x0,y0,tp,bt,lf,rt,regCount;
	int sernum,area,total=0;
	int*tagMat=new int[Width*Height];
	memset(tagMat,-1,sizeof(int)*Width*Height);
	ofstream outlet(fn1,ios::out|ios::trunc);

	//write header of fn1
	outlet<<Width<<"\t"<<Height<<"\t"<<comps<<"\t"<<d_<<endl;

	regCount=0;

	for(i=0;i<L;++i)
	{
		if(comps<L)//segmentation produced by MRS, QT, GS, HC 
			//maintains that S[i].p==i means i is a region. 
			//But for segmentation produced by merging, comps==L holds as regions are push_back into S
			if(S[i].p!=i)
			continue;	
		tp=grid[i].top;
		bt=grid[i].bottom;
		lf=grid[i].left;
		rt=grid[i].right;
		area=S[i].size;
		outlet<<S[i].p<<"\t"<<S[i].size<<"\t"<<lf<<"\t"<<tp<<"\t"<<rt<<"\t"<<bt;
		
		for(j=0;j<d_;++j){
			float mean=S[i].addition[j],sigma;

			if(area==1)
				sigma=0;
			else
			{
				sigma=(S[i].sSum[j]-mean*mean/area)/(area-1);
				sigma=sqrt(abs(sigma));
			}
			mean=mean/area;
			outlet<<"\t"<<mean<<"\t"<<sigma;
		}
		
		outlet<<endl;
		x0=rt-lf;
		y0=bt-tp;
		sernum=tp*Width+lf;
		area=0;
	//	assert(S[i].p>=sernum);	
		for(j=0;j<y0;++j)//each row
		{
			for(k=0;k<x0;++k)//each column
			{
				if(tag[sernum]==i)
				{
					++area;
					tagMat[sernum]=regCount;
				}
				++sernum;
			}
			sernum+=(Width-x0);
		}
	//	assert(S[i].p<sernum);	
		assert(sernum==(bt*Width+lf));
		assert(area==S[i].size);
		total+=area;
		++regCount;
	}
	assert(regCount==comps);
	assert(total==Width*Height);
	outlet.close();
	Export(tagMat,Width,Height,fn2);
	delete []tagMat;
	return regCount;
}




int CHC::GetWidth()
{
	return Width;
}

int CHC::GetHeight()
{
	return Height;
}

int CHC::GetNPLSize(int sernum)
{
	return S[sernum].NPList.size();
}

int BuildData(CHC&sHC,const CString& fn1,const CString&fn2, vector<float>&bWArray)
{
	GDALDataset* pDataset1=(GDALDataset *) GDALOpen(fn1,GA_ReadOnly);
	assure(pDataset1,fn1);
	GDALDataset* pDataset2=(GDALDataset *) GDALOpen(fn2,GA_ReadOnly);
	assure(pDataset2,fn2);
	int spp1=pDataset1->GetRasterCount();//波段的数目
	int spp2=pDataset2->GetRasterCount();//波段的数目
	int d;
	if(bWArray.size()>0)
	{
		if(bWArray.size()!=spp1||spp1!=spp2)
		{
			AfxMessageBox("weight of band size incompatible with band number in Dataset!");
			return 0;
		}
		for(d=0,sHC.d_=0;d<spp1;++d)
			sHC.d_+=(bWArray[d]>0?1:0);
		sHC.d_*=2;
	}
	else
	{
		if(spp1!=spp2)
		{
			AfxMessageBox("Incompatible band number in two Dataset!");
			return 0;
		}
		for(d=0;d<spp1;++d)
			bWArray.push_back(1.f);
		sHC.d_=spp1*2;
	}
	sHC.Width=pDataset1->GetRasterXSize();
	sHC.Height=pDataset1->GetRasterYSize();
	assert(sHC.Width==pDataset2->GetRasterXSize());
	assert(	sHC.Height==pDataset2->GetRasterYSize());
	int L=sHC.Height*sHC.Width;
	sHC.comps=L;
	if(sHC.tag)
		delete[]sHC.tag;
	sHC.tag=new int[L];	
	if (sHC.data_)
	{
		delete []sHC.data_;
		delete []sHC.sData_;
	}	
	sHC.data_=new float[L*sHC.d_];
	sHC.sData_=new float[L*sHC.d_];
	if(sHC.A.size())
	{
		sHC.A.clear();
		sHC.S.clear();
	}
	int x,y,sernum,temp,cur;//cur for current band index
//	int nByteWidth=d_*Width;
	float* buf =new float[sHC.Width*sHC.Height];

	GDALRasterBand  *m_pBand=NULL;
	float max=0, min=0;		
	for(d=0,cur=0;d<spp1;++d)
	{
		if(bWArray[d]==0)
			continue;	
	
		m_pBand= pDataset1->GetRasterBand(d+1);
	
//		m_pBand->GetStatistics( 0,  1,&min, &max,0,0);

		if (m_pBand)
		{	
			if (CE_None==m_pBand->RasterIO( GF_Read,0,0, sHC.Width,sHC.Height, buf, sHC.Width,sHC.Height, GDT_Float32, 0, 0 ))
			{
				if(sHC.Delta==0)
				{
					temp=sHC.Height*sHC.Width;
					for(y=0;y<temp;++y)
					{
						max=max>buf[y]?max:buf[y];
					}
					sernum=(int)ceil(max);
					x=1;
					while(sernum>1)
					{
						++x;
						sernum>>=1;
					}
					sHC.Delta=(float)(1<<x);
				}
				
				sernum=cur;
				temp=0;
				for(y=0;y<sHC.Height;++y)
				{
					for (x = 0; x < sHC.Width; ++x) 
					{
						sHC.data_[sernum]=buf[temp]/sHC.Delta;							
						sHC.sData_[sernum]=sHC.data_[sernum]*sHC.data_[sernum];
						sernum+=sHC.d_;
						++temp;
					}				
				}
			}
		}
		++cur;
	}

	for(d=0;d<spp1;++d)
	{
		if(bWArray[d]==0)
		{
			continue;
		}
		m_pBand= pDataset2->GetRasterBand(d+1);
	
//		m_pBand->GetStatistics( 0,  1,&min, &max,0,0);

		if (m_pBand)
		{	
			if (CE_None==m_pBand->RasterIO( GF_Read,0,0, sHC.Width, sHC.Height, buf, sHC.Width,sHC.Height, GDT_Float32, 0, 0 ))
			{
				if(sHC.Delta==0)
				{
					temp=sHC.Height*sHC.Width;
					for(y=0;y<temp;++y)
					{
						max=max>buf[y]?max:buf[y];
					}
					sernum=(int)ceil(max);
					x=1;
					while(sernum>1)
					{
						++x;
						sernum>>=1;
					}
					sHC.Delta=(float)(1<<x);
				}
				
				sernum=cur;
				temp=0;
				for(y=0;y<sHC.Height;++y)
				{
					for (x = 0; x <sHC. Width; ++x) 
					{
						sHC.data_[sernum]=buf[temp]/sHC.Delta;							
					    sHC.sData_[sernum]=sHC.data_[sernum]*sHC.data_[sernum];
						sernum+=sHC.d_;
						++temp;
					}				
				}
			}
		}
		++cur;
	}
	assert(cur==sHC.d_);
	delete[]buf;
	GDALClose( (GDALDatasetH) pDataset1);
	GDALClose( (GDALDatasetH) pDataset2);
	return 1;
}
//set tag in CHC
void CHC::SetTag(int*labels)
{
	if(tag)
		delete[]tag;
	int length=Width*Height;
	tag=new int[length];
	memcpy(tag,labels,sizeof(int)*length);
}
//input:tag must be allocated outside such as chc new int [L]
//output:tag initiated with values of benchmark image denoting classes
int CHC::GetTag(int * tagArr,CString fn)
{
	if(fn=="")
	{
		memcpy(tagArr,tag,sizeof(int)*Width*Height);
		return 0;
	}
	GDALDataset* pDataset = (GDALDataset *) GDALOpen(fn,GA_ReadOnly);
	if (!pDataset)	
	{
		AfxMessageBox("Cannot open standard partition image!");
		return -1;
	}
	if((pDataset->GetRasterCount()!=1)||
		(Height!=pDataset->GetRasterYSize())||
		(Width!=pDataset->GetRasterXSize()))
	{
		AfxMessageBox("Incompatible image extent or wrong band count in benchmark image!");
		return -1;
	}

	int L=Height*Width;
	GDALRasterBand  *m_pBand=NULL;
	m_pBand = pDataset->GetRasterBand(1);

	GInt32* buf =tagArr;
	if (m_pBand)
	{
		if (CE_None!=m_pBand->RasterIO( GF_Read, 0,0,Width, Height, buf, Width,Height, GDT_Int32, 0, 0 ))
		{
			AfxMessageBox("error importing class tag for pixels!");
		}
	}
	GDALClose((GDALDatasetH)pDataset);
	return 1;
}


int CHC::GetRegCount()
{
	return comps;
}

void CHC::SetPropDim(int a)
{
	propDim=a;
}

void CHC::GetEXSLabel(vector<int>&index)
{
	assert(index.size()==exS.size());
	int count=exS.size();
	for(int i=0;i<count;++i)
		index[i]=exS[i].label;
}

int CHC::CDThresh(float *src,float lim,BYTE*lpBits,int spp)
{
	int i=0,j,k,x0,y0,sernum,ink=0,s;
	int w=Width, h=Height;
	BYTE*temp,t;
	sernum=w*h;
	int byteLine=(spp*8*w + 31) / 32 * 4;
	for(i=0;i<comps;++i)
	{
	//	if(SF0[i].isChecked==false)
	//		continue;
		x0=grid[i].right;
		y0=grid[i].bottom;
		sernum=grid[i].top*w+grid[i].left;
		for(j=grid[i].top;j<y0;++j)
		{
			for(k=grid[i].left;k<x0;++k)
			{
				if(tag[sernum]==i)
				{				
					temp=&((BYTE*)(lpBits+ byteLine*(h-1-j)))[k*spp];
					t=src[i]<lim?255:0;
					for(s=0;s<spp;++s)
						temp[s]=t;					
					++ink;
				}
				++sernum;
			}
			sernum+=(w-x0+grid[i].left);
		}
	}
	assert(ink==w*h);

	return 1;
}

int BuildingCand2(CHC&myHC,HCParams *params,IplImage*portal)
{
	int i,j,angle;
	exRegion turg;//temp exregion
	vector<exRegion>::iterator xiter;
//	int ratlen;
	assert(myHC.comps==myHC.exS.size());
	float temp1,temp2,cosine,sine,prob;
	CvPoint pts[4];
//	CvBox2D box2;
	CvScalar color={0,255,0};

	//thresholding
	xiter=myHC.exS.begin();
	while(xiter!=myHC.exS.end())
	{
		i=xiter->label;

		temp1=xiter->attList[minRectWid]/xiter->attList[minRectHeg];
		if(temp1<1.f)
			temp1=1.f/temp1;
		temp1=__min(temp1,xiter->attList[eigRatio]);
		temp2=xiter->attList[minRectWid]*xiter->attList[minRectHeg];
		temp2=myHC.S[i].size/temp2;
	
		prob=LinearIB(100,150,200,2000,2300,2500,myHC.S[i].size);
		prob=__min(LinearLT(5,5.5,6,temp1),prob);

		prob=__min(LinearGT(0.5,0.6,0.65,temp2),prob);

		prob=__min(LinearGT(3,5,7,xiter->attList[meanThick]),prob);
	
		prob=__min(LinearLT(0.1,0.2,0.3,xiter->attList[shadeRatio]),prob);
		prob=__min(LinearGT(0.6,0.7,0.8,xiter->attList[neiShadeRatio]),prob);
		prob=__min(LinearGT(0.55,0.6,0.7,xiter->attList[changeRatio]),prob);

		if(prob>0.8f)
		{
			angle=cvRound(xiter->attList[boxAngle]);
			while( angle < 0 )
				angle += 360;
			while( angle > 360 )
				angle -= 360;
			icvSinCos( angle, &cosine, &sine);
			temp1=xiter->attList[minRectHeg]*cosine-xiter->attList[minRectWid]*sine;
			temp1/=2;
			temp2=-xiter->attList[minRectHeg]*sine-xiter->attList[minRectWid]*cosine;
			temp2/=2;
			pts[0]=cvPointFrom32f(cvPoint2D32f((xiter->attList[centerX]+temp1),(xiter->attList[centerY]+temp2)));
			pts[2]=cvPointFrom32f(cvPoint2D32f((xiter->attList[centerX]-temp1),(xiter->attList[centerY]-temp2)));
			temp1=-xiter->attList[minRectHeg]*cosine-xiter->attList[minRectWid]*sine;
			temp1/=2;
			temp2=xiter->attList[minRectHeg]*sine-xiter->attList[minRectWid]*cosine;
			temp2/=2;
			pts[1]=cvPointFrom32f(cvPoint2D32f((xiter->attList[centerX]+temp1),(xiter->attList[centerY]+temp2)));
			pts[3]=cvPointFrom32f(cvPoint2D32f((xiter->attList[centerX]-temp1),(xiter->attList[centerY]-temp2)));
			for(j=0;j<3;++j)		
				cvLine(portal, pts[j], pts[j+1], color,1, 8 );
			cvLine(portal, pts[3], pts[0], color,1, 8 );
		
			int tp=myHC.grid[i].top;
			
		int bt=myHC.grid[i].bottom;
		int lf=myHC.grid[i].left;
		int rt=myHC.grid[i].right;
		int area=0,k;	
	
		int sernum=tp*myHC.Width+lf;
	
		
		for(j=tp;j<bt;++j)//each row
		{
			for(k=lf;k<rt;++k)//each column
			{
				if(myHC.tag[sernum]==i)
				{
					++area;
					BYTE*temptr=((uchar*)(portal->imageData + portal->widthStep*j))+k*3;
					*temptr=0;
					*(temptr+1)=0;
					*(temptr+2)=255;	
				}
				++sernum;
			}
			sernum+=(myHC.Width-rt+lf);
		}
		assert(sernum==(bt*myHC.Width+lf));
		assert(area==myHC.S[i].size);
	/*		box2.angle=(xiter->attList[boxAngle]);
			box2.center.x=(xiter->attList[centerX]);
			box2.center.y=(xiter->attList[centerY]);
			box2.size.width=(xiter->attList[minRectWid]);
			box2.size.height=(xiter->attList[minRectHeg]);
			
			cvEllipseBox(portal,box2,color,1);*/
		}
		++xiter;
	}
	//display the result
/*	ratlen=cand.size();

	int k,tp,bt,lf,rt,area,sernum,s;
	BYTE*temptr;
	for(s=0;s<ratlen;++s)
	{
		i=cand[s];
		tp=grid[i].top;
		bt=grid[i].bottom;
		lf=grid[i].left;
		rt=grid[i].right;
		area=0;	
	
		sernum=tp*Width+lf;
	
		
		for(j=tp;j<bt;++j)//each row
		{
			for(k=lf;k<rt;++k)//each column
			{
				if(tag[sernum]==i)
				{
					++area;
					temptr=((uchar*)(portal->imageData + portal->widthStep*j))+k*3;
					*temptr=255;
					*(temptr+1)=0;
					*(temptr+2)=0;	
				}
				++sernum;
			}
			sernum+=(Width-rt+lf);
		}
		assert(sernum==(bt*Width+lf));
		assert(area==S[i].size);

	}*/
	return 1;
}
int BuildingCand(CHC*PHC,HCParams *params,IplImage*portal)
{
	int i,j,angle;
	exRegion turg;//temp exregion
	vector<exRegion>::iterator xiter;
//	int ratlen;
	assert(PHC->comps==PHC->exS.size());
	float temp1,temp2,cosine,sine;
	CvPoint pts[4];
//	CvBox2D box2;
	CvScalar color={0,255,0};

	//thresholding
	xiter=PHC->exS.begin();
	while(xiter!=PHC->exS.end())
	{
		i=xiter->label;
		temp1=xiter->attList[minRectWid]/xiter->attList[minRectHeg];
		if(temp1<1.f)
			temp1=1.f/temp1;
		temp1=__min(temp1,xiter->attList[eigRatio]);
		temp2=xiter->attList[minRectWid]*xiter->attList[minRectHeg];
		temp2=PHC->S[i].size/temp2;
		//
		if((xiter->attList[meanThick]>params->minWid)&&(PHC->S[i].size>params->minArea)
			&&(temp1<params->maxLWR)&&(PHC->S[i].size<params->maxArea)&&(temp2>params->minCmpct))
		{
			angle=cvRound(xiter->attList[boxAngle]);
			while( angle < 0 )
				angle += 360;
			while( angle > 360 )
				angle -= 360;
			icvSinCos( angle, &cosine, &sine);
			temp1=xiter->attList[minRectHeg]*cosine-xiter->attList[minRectWid]*sine;
			temp1/=2;
			temp2=-xiter->attList[minRectHeg]*sine-xiter->attList[minRectWid]*cosine;
			temp2/=2;
			pts[0]=cvPointFrom32f(cvPoint2D32f((xiter->attList[centerX]+temp1),(xiter->attList[centerY]+temp2)));
			pts[2]=cvPointFrom32f(cvPoint2D32f((xiter->attList[centerX]-temp1),(xiter->attList[centerY]-temp2)));
			temp1=-xiter->attList[minRectHeg]*cosine-xiter->attList[minRectWid]*sine;
			temp1/=2;
			temp2=xiter->attList[minRectHeg]*sine-xiter->attList[minRectWid]*cosine;
			temp2/=2;
			pts[1]=cvPointFrom32f(cvPoint2D32f((xiter->attList[centerX]+temp1),(xiter->attList[centerY]+temp2)));
			pts[3]=cvPointFrom32f(cvPoint2D32f((xiter->attList[centerX]-temp1),(xiter->attList[centerY]-temp2)));
			for(j=0;j<3;++j)		
				cvLine(portal, pts[j], pts[j+1], color,1, 8 );
			cvLine(portal, pts[3], pts[0], color,1, 8 );
	/*		box2.angle=(xiter->attList[boxAngle]);
			box2.center.x=(xiter->attList[centerX]);
			box2.center.y=(xiter->attList[centerY]);
			box2.size.width=(xiter->attList[minRectWid]);
			box2.size.height=(xiter->attList[minRectHeg]);
			
			cvEllipseBox(portal,box2,color,1);*/
		}
		++xiter;
	}
	//display the result
/*	ratlen=cand.size();

	int k,tp,bt,lf,rt,area,sernum,s;
	BYTE*temptr;
	for(s=0;s<ratlen;++s)
	{
		i=cand[s];
		tp=grid[i].top;
		bt=grid[i].bottom;
		lf=grid[i].left;
		rt=grid[i].right;
		area=0;	
	
		sernum=tp*Width+lf;
	
		
		for(j=tp;j<bt;++j)//each row
		{
			for(k=lf;k<rt;++k)//each column
			{
				if(tag[sernum]==i)
				{
					++area;
					temptr=((uchar*)(portal->imageData + portal->widthStep*j))+k*3;
					*temptr=255;
					*(temptr+1)=0;
					*(temptr+2)=0;	
				}
				++sernum;
			}
			sernum+=(Width-rt+lf);
		}
		assert(sernum==(bt*Width+lf));
		assert(area==S[i].size);

	}*/
	return 1;
}
//store properties for regions in S or indicated by signal, if signal is not null, signal[i] indicates the index of region in S[]
//the first line width, height, count of region to be saved
//information including parent label, size, all spectral mean and variance interweaved of all bands,
// length/width, eigenvalue ratio major/minor, area/width/length, meanthickness
int CHC::StoreProp(const char*fn1,vector<int>&signal)
{
	int L=S.size();
	if(exS.size()==0)
	{
		AfxMessageBox("Calculate region properties before storing!");
		return 0;
	}
	if(data_==NULL)
	{
		AfxMessageBox("No spectral information before storing!");
		return 0;
	}
	int i,j,k,regCount=signal.size();
	assert(L>regCount);
	assert(comps==exS.size());
	float temp1;
	exRegion turg;
	vector<exRegion>::const_iterator xiter;
	FILE *fp1=fopen(fn1,"w");
	if(regCount>0)
	{	//write header of fn1
		fprintf(fp1,"%d\t%d\t%d\n",Width,Height,regCount);
		for(i=0;i<regCount;++i)
		{
			j=signal[i];
			fprintf(fp1,"%d\t%d\t",S[j].p,S[j].size);
			//spectral features
			for(k=0;k<d_;++k)
			{
				fprintf(fp1,"%f\t%f\t",S[j].addition[k],S[j].sSum[k]);
			}
			//spatial
			turg.label=j;//as j==S[j].p for a region
			xiter=lower_bound(exS.begin(),exS.end(),turg);
			temp1=xiter->attList[minRectWid]/xiter->attList[minRectHeg];
			if(temp1<1.f)
				temp1=1.f/temp1;
			fprintf(fp1,"%f\t",temp1);
			fprintf(fp1,"%f\t",xiter->attList[eigRatio]);
			temp1=xiter->attList[minRectWid]*xiter->attList[minRectHeg];
			temp1=S[j].size/temp1;
			fprintf(fp1,"%f\t",temp1);
			fprintf(fp1,"%f\n",xiter->attList[meanThick]);
			//textural is to be added
		}
		fclose(fp1);
		return regCount;
	}
	regCount=0;
	for(i=0;i<L;++i)
	{
		if(S[i].p!=i)
				continue;	
		fprintf(fp1,"%d\t%d\t",S[i].p,S[i].size);
			
		for(k=0;k<d_;++k)
		{
			fprintf(fp1,"%f\t%f\t",S[i].addition[k],S[i].sSum[k]);
		}
		//spatial
		
		xiter=exS.begin()+regCount;
		temp1=xiter->attList[minRectWid]/xiter->attList[minRectHeg];
		if(temp1<1.f)
			temp1=1.f/temp1;
		fprintf(fp1,"%f\t",temp1);
		fprintf(fp1,"%f\t",xiter->attList[eigRatio]);
		temp1=xiter->attList[minRectWid]*xiter->attList[minRectHeg];
		temp1=S[i].size/temp1;
		fprintf(fp1,"%f\t",temp1);
		fprintf(fp1,"%f\n",xiter->attList[meanThick]);
		++regCount;
	}
	assert(regCount==exS.size());
	fclose(fp1);
	return regCount;
}


bool CHC::PrepPropMemo(int pDim)
{

	if(exS.size()==comps)
	return false;
	else
	exS.clear();
	
	propDim=pDim;//add mask prop
	propData=new float[propDim*comps];
	memset(propData,0,sizeof(float)*propDim*comps);
	exS=vector<exRegion>(comps);
		//initialize region list storing features
	int len=S.size(),i,x;
	for (x = 0,i=0; x <len; ++x) 
	{		
		if(x!=S[x].p)			
			continue;
		exS[i].label=x;
		exS[i].attList=propData+i*propDim;
		++i;
	}
	return true;
}

//merge two segmentations, and store the label of pixels in tag[], bounding box in grid, parent index in S
//index1[i] denotes the father pixel index for region i, thus, tagArray1[index[i]]==i, 
//grid1[i] corresponds to index1[i], regCount is the number of regions in index1
//note this method creates intersection partition that may not be connected and it cause error when use this 
//method with mahalkmeans change detection
int CHC::MergeSeg(vector<int>&index1,int*tagArray1,vector<CRect>&grid1,vector<int>&index2,int*tagArray2,vector<CRect>&grid2)
{
	int i=0,j=0,k=0,s=0,t=0,x0,y0,tp,bt,lf,rt,dx;
	int regNum=0,area=0,total=0;
	int sernum, sernum2,dice;
	tag=new int[Width*Height];
	int regCount=index1.size();
	memset(tag,-1,sizeof(int)*Width*Height);
	CRect camp;
	Region ball;
	//check out
	for(i=0;i<regCount;++i)//for each region indicated by index1, 
	{
		dice=index1[i];
		assert(tagArray1[dice]==i);
	
		tp=grid1[i].top;
		bt=grid1[i].bottom;
		lf=grid1[i].left;
		rt=grid1[i].right;
		x0=rt-lf;
		y0=bt-tp;
		sernum=tp*Width+lf;
		//for each pixel in region Ai
		for(j=0;j<y0;++j)//each row
		{
			for(k=0;k<x0;++k)//each column
			{
				if((tagArray1[sernum]==i)&&(tag[sernum]==-1))
				{
					//create a region 				
					camp.IntersectRect(grid1[i],grid2[tagArray2[sernum]]);
					grid.push_back(camp);					
					ball.p=sernum;									
					
					sernum2=camp.top*Width+camp.left;
					dx=camp.right-camp.left;
					area=0;
					assert(sernum<(camp.bottom*Width-Width+camp.right));
					assert(sernum>=(camp.top*Width+camp.left));
				
					//label area of intersection
					for(s=camp.top;s<camp.bottom;++s)
					{
						for(t=0;t<dx;++t)
						{
							if((tagArray1[sernum2]==i)&&(tagArray2[sernum2]==tagArray2[sernum]))
							{
								assert(tag[sernum2]==-1);
								tag[sernum2]=regNum;
								++area;
							}
							++sernum2;
						}
						sernum2+=(Width-dx);
					}
					assert(sernum2==(camp.bottom*Width+camp.left));
					ball.size=area;
					total+=area;
					S.push_back(ball);	
				//	SF[regNum].norbox=&gridF[regNum];
					++regNum;				
				}
				++sernum;				
			}
			sernum+=(Width-x0);
		}
		assert(sernum==(bt*Width+lf));
	}
	comps=regNum;
	assert(total==Width*Height);
	return regNum;
}
int CHC::MergeSeg(int *tagArray1,int *tagArray2)
{
	int i;
	int regNum,total;
	int w=Width,h=Height;
	int L=w*h;
	tag=new int[L];

	memset(tag,-1,sizeof(int)*Width*Height);
	CRect camp;
	Region ball;
	
	CvFFillSegment* buffer = 0;	
	int buffersize = __max( Width, Height )*2;
	buffer = (CvFFillSegment*)cvAlloc( buffersize*sizeof(buffer[0]));
	CvConnectedComp parcel;
	
	//check out
	for(i=0,total=0,regNum=0;i<L;++i)//for each region indicated by index1, 
	{
		if(tag[i]!=-1)			
			continue;
		
		Immerse(tagArray1,tagArray2,tag,cvSize(w,h),cvPoint(i%w,i/w),regNum,&parcel,4,buffer,buffersize);
		++regNum;
		camp.SetRect(parcel.rect.x,parcel.rect.y,parcel.rect.x+parcel.rect.width,parcel.rect.y+parcel.rect.height);
		grid.push_back(camp);					
		ball.p=i;	
		ball.size=parcel.area;
		assert(parcel.area>0);
		S.push_back(ball);	
		total+=parcel.area;						
	}
	comps=regNum;	
	assert(total==Width*Height);

	cvFree( (void**)&buffer );
	return regNum;
}
