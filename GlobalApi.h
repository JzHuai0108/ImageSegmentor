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

#ifndef _GLOBAL_API
#define _GLOBAL_API

#include "mycolorspace.h"
#include <math.h>
//#include "ColorTable.h"
//#include <complex>
typedef struct tagMyFeatureVector{
	CString classname;
	DOUBLE colorl;
	DOUBLE colorlw;
	DOUBLE coloru;
    DOUBLE coloruw;
	DOUBLE colorv;
	DOUBLE colorvw;
	DOUBLE minmaxtexl;
	DOUBLE minmaxtexlw;
	DOUBLE minmaxtexu;
	DOUBLE minmaxtexuw;
	DOUBLE minmaxtexv;
	DOUBLE minmaxtexvw;
}MyFeatureVector;	

typedef struct tagMyPtNei{
	BYTE* ptneiarr;
}MyPtNei;

typedef struct tagRgnInfo{
	BOOL  isflag;//备用标记；
	INT   ptcount;//该区所含的点数；
	FLOAT   l;//该区l均值
	FLOAT   u;//该区u均值
	FLOAT   v;//该区v均值
}MyRgnInfo;//分割后各个区的一些统计信息,图像中各点所属区域的信息存放在flag数组中；

//void RoadThin(BYTE*roadseed, int Width,int Height,vector<int>&terminal);

	BYTE* EdgeSusan(BYTE*,int w,int h,int=20,bool=false);
	void Morph(BYTE*,int wid, int heg,int opt);

void GaussianFilter(BYTE*,int Width, int Height,float sigma);

void EdgeMag(BYTE*,int w,int h);	
void EdgePoints(BYTE* ,int w,int h,float sig=0.8f, float low=0.4f, float high=0.79f);
MyLUV*  GetNearPixelsLUV(int xPos, int yPos
	  , MyLUV* inLUVs, int picWidth, int picHeight
	  , int inScale, int& outWidth, int& outHeight);
//得到LUV邻域；


void  GetNearPixelsGreenExt(int xPos, int yPos
	    , BYTE* inPixels, int picWidth, int picHeight
	    , int radius, BYTE** outArr);
//得到邻域像素值(正方形,G通道),输入位置从0开始计数, 边缘处对称延拓；



void  GetNearPixelsExt(int xPos, int yPos
	, BYTE* inPixels, int picWidth, int picHeight
	, int radius, BYTE** outArr);
//得到邻域像素值(正方形),输入位置从0开始计数, 边缘处延拓；



BYTE*  GetNearPixels(int xPos, int yPos, 
      BYTE* inPixels, int picWidth, int picHeight, int inScale, 
	  int& outWidth, int& outHeight);
//得到邻域像素值, 输入位置从0开始计数；
void GetGradient(BYTE* image, int width, int height
		, FLOAT* deltar, FLOAT* deltasita);
void  AddBNeiToANei(int curid, int nearid, CString* neiarr, int* mergearr);
//将nearid的邻域加到curid的邻域中去；



int  FindNearestNei(int curid, CString neistr, MyRgnInfo* rginfoarr, int* mergearr);
//寻找neistr中与curid最接近的区，返回该区id号；


int  FindMergedRgnMaxbias(int idint, int* mergearr, int bias);
//大阈值终止查找合并区，用于coarse watershed, 
//调用者必须保证idint有效，即：mergearr[idint]>0；
//以及mergearr有效，即：mergearr[idint]<idint;


int  FindMergedRgn(int idint, int* mergearr);
//找到idint最终所合并到的区号；


void  AddNeiRgn(int curid, int neiid, CString* neiarr);
//增加neiid为curid的相邻区


void  AddNeiOfCur(int curid, int left, int right, int up, int down, int* flag, CString* neiarr);
//刷新当前点的所有相邻区；

//////////////////////////////////////////////////////////////////////////
// Luc Vincent and Pierre Soille的分水岭分割flood步骤的实现代码， 
// 修改自相应伪代码, 伪代码来自作者论文《Watersheds in Digital Spaces:
// An Efficient Algorithm Based on Immersion Simulations》
// IEEE TRANSACTIONS ON PATTERN ANALYSIS AND MACHINE INTELLIGENCE.
// VOL.13, NO.6, JUNE 1991;
// by dzj, 2004.06.28 
// MyImageGraPt* imiarr - 输入的排序后数组
// int* graddarr -------- 输入的各梯度数组，由此直接存取各H像素点
// int minh，int maxh == 最小最大梯度
// int* flagarr --------- 输出标记数组
// 注意：目前不设分水岭标记，只设每个像素所属区域；
//////////////////////////////////////////////////////////////////////////
void FloodVincent(MyImageGraPt* imiarr, int imageWidth,int imageHeight,int* graddarr, int minh, int maxh, int* flagarr, int& outrgnumber);

//找到idint最终所合并到的区号；
int FindMergedRgnMaxbias(int idint, int* mergearr, int bias);
#define NearMeasureBias 200.0//判定区域颜色相似的阈值；
void  MergeRgs(MyRgnInfo* rginfoarr, int rgnumber, int* flag, int width, int height, int* outmerge, int& rgnum);
//合并相似区域；


void  MergeNearest(int curid, MyRgnInfo* rginfoarr, CString* neiarr, int* mergearr);
//合并相似区域；


void  MergeTwoRgn(int curid, int nearid
	, CString* neiarr, MyRgnInfo* rginfoarr, int* mergearr);
//将nearid合并到curid中去，更新合并后的区信息，并记录该合并；



//----------------------------------------------------------------------
/* DIB处理函数
BOOL DIBShow(CFile* pFile);
BOOL ConvertDDBToDIB(HBITMAP hBitmap, CDIB* pDibDest, HPALETTE hPal);
BOOL CopyDIB(CDIB* pDibSrc, CDIB* pDibDst);
BOOL CopyScreenToDIB(LPRECT lpRect, CDIB* pDibDest);*/
HPALETTE GetSystemPalette() ;
HPALETTE CopyPalette(HPALETTE hPalSrc);
BOOL ReadDIB(CFile* pFile, LPBITMAPINFOHEADER lpBMIH, LPBYTE lpImage);
HPALETTE MakeDIBPalette(LPVOID lpvColorTable,  LPBITMAPINFOHEADER lpBMIH);

//----------------------------------------------------------------------
// 运动检测
BOOL GetBackground(CString strFilePath, int nTotalFrameNum, int nImageWidth, 
		   int nImageHeight, unsigned char* pUnchBackGround);
//BOOL LoadDibSeq(CString strFilePath, int nCurFrameNum, int nTotalFrameNum, CDIB* pDib);
CString GetFileName(CString strFilePathName, int nCurFrameNum);
void BinaFrameDiff(unsigned char *pUnchImg1, unsigned char *pUnchImg2, 
		  int nWidth, int nHeight, unsigned char * pUnchResult,
		  int nThreshold=10);
void ErodeFrameDiff(unsigned char *pUnchImg, int nWidth, int nHeight, int nErodeHalfWin, 
	   int nErodeThreshold, unsigned char *pUnchResult);

//----------------------------------------------------------------------
/* 图象分割
void RegionSegFixThreshold(CDIB * pDib, int nThreshold=100);
void RobertsOperator(CDIB * pDib, double * pdGrad);
void LaplacianOperator(CDIB * pDib, double * pdGrad);
void EdgeTrack(CDIB * pDib, unsigned char * pUnEdgeTrack);
void RegionGrow(CDIB * pDib, unsigned char * pUnRegion, int nThreshold = 10);
void RegionSegAdaptive(CDIB * pDib) ;
void SobelOperator(CDIB * pDib, double * pdGrad);
void PrewittOperator(CDIB * pDib, double * pdGrad);*/
	// canny 算子
void GaussianSmooth(unsigned char *pUnchImg, int nWidth, int nHeight, double sigma, unsigned char * pUnchSmthdImg);
void MakeGauss(double sigma, double **pdKernel, int *pnWindowSize) ; 
void DirGrad(unsigned char *pUnchSmthdImg, int nWidth, int nHeight,
						 int *pnGradX , int *pnGradY);
void GradMagnitude(int *pnGradX, int *pnGradY, int nWidth, int nHeight, int *pnMag) ; 
void NonmaxSuppress(int *pnMag, int *pnGradX, int *pnGradY, int nWidth, int nHeight,	unsigned char *pUnchRst);
void EstimateThreshold(int *pnMag, int nWidth, int nHeight, int *pnThdHigh,int *pnThdLow, 
											 unsigned char * pUnchEdge, double dRatioHigh, double dRationLow) ;
void Canny(unsigned char *pUnchImage, int nWidth, int nHeight, double sigma,
					 double dRatioLow, double dRatioHigh, unsigned char *pUnchEdge) ;
void Hysteresis(int *pnMag, int nWidth, int nHeight, double dRatioLow, 
								double dRatioHigh, unsigned char *pUnchEdge) ;
void TraceEdge (int y, int x, int nLowThd, unsigned char *pUnchEdge, int *pnMag, int nWidth) ;
void Gradient(unsigned char*pImage, int nWidth, int nHeight,float* pEdge);

// 小波变换函数原型
// Daubechies紧致正交小波基
// 不同支撑区间长度下的滤波器系数如下
const double hCoef[10][20] =
{
	{ .707106781187,  .707106781187},

	{ .482962913145,  .836516303738,  .224143868042, -.129409522551 },

	{ .332670552950,  .806891509311,  .459877502118, -.135011020010, -.085441273882,  .035226291882 },

	{ .230377813309,  .714846570553,  .630880767930, -.027983769417,
	 -.187034811719,  .030841381836,  .032883011667, -.010597401785 },

	{ .160102397974,  .603829269797,  .724308528438,  .138428145901, -.242294887066,
	 -.032244869585,  .077571493840, -.006241490213, -.012580751999,  .003335725285 },

	{ .111540743350,  .494623890398,  .751133908021,  .315250351709, -.226264693965,
	 -.129766867567,  .097501605587,  .027522865530, -.031582039318,  .000553842201,
	  .004777257511, -.001077301085 },

	{ .077852054085,  .396539319482,  .729132090846,  .469782287405, -.143906003929,
	 -.224036184994,  .071309219267,  .080612609151, -.038029936935, -.016574541631,
	  .012550998556,  .000429577973, -.001801640704,  .000353713800 },

	{ .054415842243,  .312871590914,  .675630736297,  .585354683654, -.015829105256,
	 -.284015542962,  .000472484574,  .128747426620, -.017369301002, -.044088253931,
	  .013981027917,  .008746094047, -.004870352993, -.000391740373,  .000675449406,
	 -.000117476784 },

	{ .038077947364,  .243834674613,  .604823123690,  .657288078051,  .133197385825,
	 -.293273783279, -.096840783223,  .148540749338,  .030725681479, -.067632829061,
	  .000250947115,  .022361662124, -.004723204758, -.004281503682,  .001847646883,
	  .000230385764, -.000251963189,  .000039347320 },

	{ .026670057901,  .188176800078,  .527201188932,  .688459039454,  .281172343661,
	 -.249846424327, -.195946274377,  .127369340336,  .093057364604, -.071394147166,
	 -.029457536822,  .033212674059,  .003606553567, -.010733175483,  .001395351747,
	  .001992405295, -.000685856695, -.000116466855,  .000093588670, -.000013264203 }
};

BOOL DWT_1D(double* pDbSrc, int nMaxLevel,int nDWTSteps, int nInv, int nStep, int nSupp);
BOOL DWTStep_1D(double* pDbSrc, int nCurLevel,int nInv, int nStep, int nSupp);
BOOL DWT_2D(double* pDbSrc, int nMaxWLevel, int nMaxHLevel, int nDWTSteps, int nInv, int nStep, int nSupp);
BOOL DWTStep_2D(double* pDbSrc, int nCurWLevel, int nCurHLevel,	int nMaxWLevel, int nMaxHLevel, int nInv, int nStep, int nSupp);	
BOOL ImageDWT(LPBYTE lpImage, int nMaxWLevel, int nMaxHLevel,int nDWTSteps, int nInv, int nStep, int nSupp);
int Log2(int n);
BYTE FloatToByte(double f);
char FloatToChar(double f);


//----------------------------------------------------------------------
/* 图象增强函数
BOOL GraySegLinTrans(CDIB* pDib, int nX1, int nY1, 
					 int nX2, int nY2);					// 分段线性变换
BOOL HistogramEqualize(CDIB* pDib);						// 直方图均衡化
BOOL  GeneralTemplate(CDIB* pDib,  int nTempWidth, int nTempHeight,	
					 int nTempCenX, int nTempCenY,
					 double* pdbTemp, double dbCoef);	// 通用模板操作
BOOL MedianFilter(CDIB* pDib,  int nTempWidth, int nTempHeight, 
				 int nTempCenX, int nTempCenY);			// 中值滤波
unsigned char  GetMedianValue(unsigned char * pUnchFltValue, int iFilterLen);	// 获取中值
BOOL  ReplaceDIBColorTable(CDIB* pDib, LPBYTE  pColorsTable);			// 替换DIB颜色表*/

void LowPassFilterEnhance(LPBYTE lpImage, int nWidth, int nHeight, int nRadius) ;// 低通滤波
void HighPassFilterEnhance(LPBYTE lpImage, int nWidth, int nHeight, int nRadius);// 高通滤波
void ButterWorthLowPass(LPBYTE lpImage, int nWidth, int nHeight, int nRadius);	//  ButterWorth低通滤波
void ButterWorthHighPass(LPBYTE lpImage, int nWidth, int nHeight, int nRadius);	//  ButterWorth高通滤波
void LinearSharpen (LPBYTE lpImage, int nWidth, int nHeight);			//  线性锐化增强

//-----------------------------------------------------------------------
// imageview.cpp函数原型
//BOOL LimbPatternBayer(CDIB *pDib);
//BOOL DitherFloydSteinberg(CDIB *pDib);

#endif
