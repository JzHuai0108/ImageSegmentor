// MyTexture.cpp: implementation of the CMyTexture class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "myimagedb.h"
#include "MyTexture.h"
#include <MATH.H>
#include "mymath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//#define NEIRADIUS 16//邻域半径，必须与DOC中的同样定义一致；
#define DHISNORM ( sqrt( 4. * (pow(((pow(((NEIRADIUS*2+1)-1), 2. ) )/4.), 2.)) ) )//可能的最大距离，用于归一化方向直方图距离；

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMyTexture::CMyTexture()
{

}

CMyTexture::~CMyTexture()
{

}

FLOAT CMyTexture::CalcuEntropy(BYTE* inMatrix, INT width, INT height)
//计算输入图像块信息熵；
{
	//计算输入图像总的灰度值；
	INT ptcount = width * height;
	DOUBLE maxen = -1;
	DOUBLE allitensity = 0;
	for (INT i=0; i<ptcount; i++)
	{
		allitensity += (DOUBLE) ( inMatrix[i] );
	}

	if (allitensity<1)
	{
		return 0;
	}

	//计算信息熵；
	FLOAT entropy = 0;
	for (i=0; i<ptcount; i++)
	{
		DOUBLE fe = inMatrix[i] / allitensity;
		DOUBLE templog = log(fe);
		entropy -= (FLOAT) ( fe * templog );
	}
	
	return entropy;
}

FLOAT CMyTexture::CalcuDisBetweenDHis(DirectionHis* his1, DirectionHis* his2, INT binCounts)
//计算两个方向直方图间的欧氏距离，用NEIRADIUS归一化到[0, 1]之间；
{
	FLOAT added = 0;
	for (INT i=0; i<binCounts; i++)
	{
		INT tempd = (his1[i].count - his1[i].count);
		FLOAT tempf = (FLOAT) ( pow((DOUBLE)tempd, 2.) );
		added += tempf;
	}

	FLOAT dis = (FLOAT) sqrt(added);
	dis = (FLOAT) ( dis / DHISNORM );//归一化

	return dis;
}

BOOL CMyTexture::GetDirectionReal(BYTE* inMatrix, INT width, INT height, FLOAT& direction, FLOAT& itensity, FLOAT& err)
//计算输入块的方向, direction方向，itensity强度, err方差；
{
	//首先计算各像素的deltaX和deltaY；
	//然后计算各像素点处的梯度方向以及模；
	//计算梯度平均并求大致方向，计算模的平均求强度
	//输出方差为方向方差；

	//下面计算各像素在水平和垂直方向上的梯度,边缘点不计算；
	FLOAT* deltasita;
	FLOAT* deltar;
	INT deltacount = width * height;
	deltasita = new FLOAT[deltacount];
	deltar = new FLOAT[deltacount];
    GetGradient(inMatrix, width, height, deltar, deltasita);

	//求方向均值与方差，以及模均值与方差；
	FLOAT dmean, derr, rmean, rerr;
	dmean = derr = rmean = rerr = 0;

	myMath.ClacuMeanPositive(deltasita, deltacount
		, dmean, derr);//方向均值与方差；
	myMath.ClacuMeanPositive(deltar, deltacount
		, rmean, rerr);//模均值与方差；

	direction = dmean;//角度；
	itensity = rmean;//方向强度；
	err = derr;//方向方差；

	delete [] deltar; deltar = NULL;
	delete [] deltasita; deltasita = NULL;

	return TRUE;	
}

BOOL CMyTexture::GetDirection(BYTE* inMatrix, INT width, INT height, INT& direction, FLOAT& itensity)
//计算输入块的方向, 输出direction表示四个方向之一，itensity表示强度；
{
	DirectionHis* dhisarr = NULL;
	INT binsinhis = -1;
	//计算输入块的方向直方图；
	BOOL tempb = CalcuDirectionHis(inMatrix, width, height, &dhisarr, binsinhis);
	if (binsinhis<0)
	{
		return FALSE;
	}

	if (tempb)
	{
		//无显著方向；
		direction = -1;
		itensity = -1;
	}

	//取最大count对应BIN为该块方向；
	DOUBLE* countarr = new DOUBLE[binsinhis];//临时数组存放各BIN counts;
	INT tempmax = -1;
	for (INT i=0; i<binsinhis; i++)
	{
		countarr[i] = dhisarr[i].count;
		if (dhisarr[i].count>tempmax)
		{
			tempmax = dhisarr[i].count;
			direction = i;
		}		
	}

	delete [] dhisarr; dhisarr = NULL;

	//以下计算均值和方差，以方差的倒数作为强度；
	FLOAT tempmean, temperr;
	myMath.ClacuMean(countarr, binsinhis, tempmean, temperr);
	itensity = (FLOAT) ( 1. / temperr );

	delete [] countarr; countarr = NULL;

	return TRUE;	
}

BOOL CMyTexture::CalcuDirectionHis(BYTE* inMatrix, INT width, INT height, DirectionHis** outHis, INT& binsInHis)
//计算输入块的方向直方图,如有明确方向，则返回TRUE，否则返回FALSE；
{
	//首先计算各像素的deltaX和deltaY；
	//然后计算各像素点处的梯度方向以及模；
	//将它们聚类到指定的四个方向上去得到方向直方图；
	//以下计算梯度方向与模；
	FLOAT* deltasita;
	FLOAT* deltar;
	INT deltacount = width * height;
	deltasita = new FLOAT[deltacount];
	deltar = new FLOAT[deltacount];
    GetGradient(inMatrix, width, height, deltar, deltasita);

	DirectionHis* dhis = new DirectionHis[5];//方向直方图,最后一个为无效方向；
	//赋初值；
	for (INT i=0; i<4; i++)
	{
		dhis[i].count = 0;
		dhis[i].sita = (FLOAT) ( PI/4 * i );//每隔45度取一个方向；
	}
	dhis[4].sita = -1;
	dhis[4].count = 0;

	//以下计算方向直方图；
	FLOAT bias = (FLOAT) (0.1);//模阈值；
	for (INT y=0; y<height; y++)
	{
		for (INT x=0; x<width; x++)
		{
			INT temppos = y*(width) + x;
			//找与其最近的protype;
			INT j = (INT) ( deltasita[temppos] / (PI/4.) );
			if (j<0 || j>3 || deltar[temppos]==0)
			{
				j = 4;
			}
			dhis[j].count++;
		}
	}

	delete [] deltar; deltar = NULL;
	delete [] deltasita; deltasita = NULL;

	*outHis = dhis;
	binsInHis = 4;

    if ( dhis[4].count>(deltacount - width) )
	{
		return FALSE;
	}
	return TRUE;
}

void CMyTexture::GetGradient(BYTE* image, INT width, INT height
		, FLOAT* deltar, FLOAT* deltasita)
//得到输入图像的梯度；
{
	//下面计算各像素在水平和垂直方向上的梯度,边缘点梯度计为0；
	INT* deltaxarr;
	INT* deltayarr;
	INT grawidth = width;
	INT graheight = height;
	INT deltacount = grawidth * graheight;
	deltaxarr = new INT[deltacount];
	deltayarr = new INT[deltacount];
	for (INT y=0; y<graheight; y++)
	{
		INT x1 = 0;
		INT pos1 = y*grawidth + x1;
		deltaxarr[pos1] = 0;
		deltayarr[pos1] = 0;
		INT x2 = grawidth-1;
		INT pos2 = y*grawidth + x2;
		deltaxarr[pos2] = 0;
		deltayarr[pos2] = 0;
	}
	for (INT x=0; x<grawidth; x++)
	{
		INT y1 = 0;
		INT pos1 = y1*grawidth + x;
		deltaxarr[pos1] = 0;
		deltayarr[pos1] = 0;
		INT y2 = graheight-1;
		INT pos2 = y2*grawidth + x;
		deltaxarr[pos2] = 0;
		deltayarr[pos2] = 0;
	}

	for (y=1; y<graheight-1; y++)
	{
		for (int x=1; x<grawidth-1; x++)
		{
			INT inarrpos = ((y)*width + (x)) + 1;//在输入块中的位置；
			INT deltaarrpos = y*grawidth + x;//在梯度数组中的位置；
			//卷积计算；
			deltaxarr[deltaarrpos] = (INT) ( (
				image[((y-1)*width + (x+1))] //右上
				+ image[((y)*width + (x+1))] //右
				+ image[((y+1)*width + (x+1))] //右下
				- image[((y-1)*width + (x-1))] //左上
				- image[((y)*width + (x-1))] //左
				- image[((y+1)*width + (x-1))] ) / 3 );//左下
			deltayarr[deltaarrpos] = (INT) ( ( 
				image[((y-1)*width + (x+1))] //右上
				+ image[((y-1)*width + (x))] //上
				+ image[((y-1)*width + (x-1))] //左上
				- image[((y+1)*width + (x-1))] //左下
				- image[((y+1)*width + (x))] //下
				- image[((y+1)*width + (x+1))]) / 3 );//右下
		}
	}

	for ( y=0; y<graheight; y++)
	{
		for (int x=0; x<grawidth; x++)
		{
			INT temppos = y*grawidth + x;
			if ( (deltaxarr[temppos])==0 )
			{
				if (deltayarr[temppos]!=0)
				{
					deltasita[temppos] = 0;//水平方向;
					deltar[temppos] = (FLOAT) abs(deltayarr[temppos]);
				}else
				{
					deltasita[temppos] = -1;//无确定方向;
					deltar[temppos] = (FLOAT) abs(deltayarr[temppos]);
				}
				continue;
			}
			
			FLOAT tempf = (FLOAT) atan( (FLOAT)deltayarr[temppos]
				/ (FLOAT)deltaxarr[temppos] );
			deltasita[temppos] = (FLOAT) ( tempf + PI/2. );
			deltar[temppos] = (FLOAT) sqrt((DOUBLE) 
				( deltayarr[temppos]*deltayarr[temppos]
				+ deltaxarr[temppos]*deltaxarr[temppos] ) );
		}
	}

	delete [] deltaxarr; deltaxarr = NULL; //删除水平和垂直梯度数组；
	delete [] deltayarr; deltayarr = NULL;
}
