// MyGabor.cpp: implementation of the CMyGabor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "myimagedb.h"
#include "MyGabor.h"
#include <MATH.H>
#include "myMath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyGabor::CMyGabor()
{
	curMN = "";
	myM = 4;//尺度个数；
	myN = 6;//方向个数；
	myL = 31;//滤波器参数数目；
	myUl = 0.05;
	myUh = 0.4;
	curAlpha = pow( (myUh/myUl), 1.0/(myM-1) );
	gaborSize = myL * myL;

	coeffR = coeffI = NULL;
}

CMyGabor::~CMyGabor()
{
	if (coeffR != NULL)
	{
		delete [] coeffR;
		coeffR = NULL;
	}

	if (coeffI != NULL)
	{
		delete [] coeffI;
		coeffI = NULL;
	}

}

void CMyGabor::InitPara(INT m, INT n)
//初始化各项参数, m为尺度，n为方向；
{
    curWmn = myUl * pow( curAlpha, m );

	DOUBLE tempd1, tempd2, tempd3;
    tempd1 = (curAlpha+1) * ( sqrt( 2*log(2.f) ) );
	tempd2 = 2 * PI * pow(curAlpha, m) * (curAlpha-1) * myUl;
	deltaXmn = tempd1 / tempd2;

	tempd1 = 2*PI*tan( PI/(2*myN) );
	tempd2 = pow(myUh, 2.) / ( 2.*log(2.) );
	tempd3 = pow( 1/(2.*PI*deltaXmn), 2. );
	deltaYmn =  1 / ( tempd1 * sqrt(tempd2-tempd3) );

	sita = ( (FLOAT)n * PI ) / (FLOAT)myN;	

	if (coeffR!=NULL)
	{
		delete coeffR;
		coeffR = NULL;
	}
	if (coeffI!=NULL)
	{
		delete coeffI;
		coeffI = NULL;
	}

	coeffR = new DOUBLE[myL*myL];
	coeffI = new DOUBLE[myL*myL];
}

void CMyGabor::GetCoeffAt(INT m, INT n, INT x, INT y, DOUBLE& coeffr, DOUBLE& coeffi)
//得到一个滤波器系数；
{
	double dX=0.0, dY=0.0 ;

	dX = x*cos(sita) + y*sin(sita);
	dY = y*cos(sita) - x*sin(sita);
	dX = pow(curAlpha, (0-m) ) * dX;
	dY = pow(curAlpha, (0-m) ) * dY;

	double dExponent = pow(dX/deltaXmn, 2) + pow(dY/deltaYmn, 2);
	dExponent = 0 - (0.5*dExponent) ;

	double dReCoefficient = cos(2*PI*curWmn*dX);
	double dImCoefficient = sin(2*PI*curWmn*dX);

	double dpreCoefficient = 1 / (2*PI*deltaXmn*deltaYmn);
	dpreCoefficient = pow(curAlpha, (0-m) ) * dpreCoefficient;

	coeffr = dpreCoefficient*exp(dExponent) * dReCoefficient;
	coeffi = dpreCoefficient*exp(dExponent) * dImCoefficient;
}

void CMyGabor::CalcuCoeffWhen(INT m, INT n)
//计算此尺度和方向时的滤波器系数, m为尺度，n为方向；
{
	CString needmn;//需要的滤波器；
	needmn.Format("%d%d", m, n);
	if (curMN==needmn)
	{
		return;//已计算过了；
	}

	InitPara(m, n);//初始化参数；

	int radius = (myL-1) / 2;

	int x=0, y=0;

	for(y=(-radius); y<=radius; y++)
	{
		for(x=(-radius); x<=radius; x++)
		{
			int tempi = y*myL + x; 
			int pos = (y+radius)*myL + (x+radius);

			GetCoeffAt(m, n, x, y, coeffR[pos], coeffI[pos]);
		}	
	}

	curMN.Format("%d%d", m, n);//当前计算的滤波器标志；
}

void CMyGabor::GetBlockGaborCoeff(FLOAT* inData, INT m, INT n, FLOAT& outCoeff)
//得到输入数据块在指定方向和尺度下Gabor的变换系数, 调用者要保证输入数据块的大小符合要求；
{
	CalcuCoeffWhen(m, n);//计算所需的滤波器
	
	DOUBLE energyr, energyi;
	energyr = energyi = 0.;

	INT radius = myL/2;
	
	for (INT j=-radius; j<=radius; j++)
	{
		for (INT i=-radius; i<=radius; i++)
		{
			INT coefpos = (j+radius)*myL + i+radius;
			INT pixpos = (myL/2+1-j)*myL + (myL/2+1-i);
			FLOAT pixel = inData[pixpos];
			double cre = coeffR[coefpos];
			double cim = coeffI[coefpos];
			
			energyr += pixel*cre;
			energyi += pixel*cim;			
		}
	}
	
	outCoeff = (FLOAT) ( pow(energyr, 2) + pow(energyi, 2) );
	outCoeff = (FLOAT) sqrt(outCoeff);
}

void CMyGabor::GetImageGaborCoeff(FLOAT* imageData, INT imageWidth, INT imageHeight, INT m, INT n, FLOAT** outData)
//得到输入图像在指定方向和尺度下的Gabor变换, 输出内存由调用者分配;
{
	FLOAT* temparr = new FLOAT[imageWidth*imageHeight];

	for (INT y=0; y<imageHeight; y++)
	{
		for (INT x=0; x<imageWidth; x++)
		{
			//得到指定点周围大小为myL*myL的块，并计算该块的Gabor变换；
			//得到块在图像中的左右上下位置；
			INT minx, maxx, miny, maxy;//四角坐标；
			minx = x - myL/2;
			maxx = x + myL/2;
			miny = y - myL/2;
			maxy = y + myL/2;
			
			INT posx, posy;
			posx = posy = 0;//记录数组位置；

			INT useby, usebx;

			//下面的循环得到块；
			for (INT by=miny; by<=maxy; by++)
			{
				if (by<0)//进行拓宽
				{
					useby = - (by-y) + y;
				}else if (by>imageHeight-1)
				{
					useby = y - (by-y);
				}else
				{
					useby = by;
				}
				
				for (INT bx=minx; bx<=maxx; bx++)
				{					
					if (bx<0)//进行拓宽
					{
						usebx = - (bx-x) + x;
					}else if (bx>imageWidth-1)
					{
						usebx = x - (bx-x);
					}else
					{
						usebx = bx;
					}

					INT arrpos = posy * myL + posx;
					INT imagepos = useby*imageWidth + usebx;
					temparr[arrpos] = imageData[imagepos];
					posx++;					
				}
				posx = 0;
				posy++;
			}

			//以下计算所得块的Gabor变换；
			INT coefpos = y*imageWidth + x;
			GetBlockGaborCoeff(temparr, m, n, (*outData)[coefpos]);
		}		
	}
}
