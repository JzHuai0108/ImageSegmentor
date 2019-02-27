// MyColorSpace.cpp: implementation of the MyColorSpace class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "myimagedb.h"
#include "MyColorSpace.h"
#include <MATH.H>
#include "mymath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Coefficient matrix for xyz and rgb spaces
static const int    XYZ[3][3] = { { 4125, 3576, 1804 },
{ 2125, 7154,  721 },
{  193, 1192, 9502 } };
static const double  RGB[3][3] = { 
{ (float)3.2405, (float)-1.5371, (float)-0.4985 },
{(float)-0.9693,  (float)1.8760,  (float)0.0416 },
{ (float)0.0556, (float)-0.2040,  (float)1.0573 } };

// Constants for LUV transformation 
static const float     Xn = (float)0.9505;
static const float     Yn = (float)1.0;
static const float     Zn = (float)1.0888;
static const float     Un_prime = (float)0.1978;
static const float     Vn_prime = (float)0.4683;
static const float     Lt = (float)0.008856;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MyColorSpace::MyColorSpace()
{

}

MyColorSpace::~MyColorSpace()
{

}


BOOL MyColorSpace::Rgb2Hsi(FLOAT r, FLOAT g, FLOAT b, FLOAT& h, FLOAT& s, FLOAT& i)
//冈萨雷斯《数字图像处理》第二版，p235-p237；
{
	//有问题；
/*
	h = r;
	s = g;
	i = b;
*/
	FLOAT temp1 = (FLOAT) ( 0.5 * ( (r-g) + (r-b) ) );
	FLOAT temp2 = (FLOAT) sqrt( (r-g)*(r-g) + (r-g)*(g-b) );
	if (temp1>temp2)
	{
		temp1=temp2=1;//本来不应该，但确实会有这种情况，晕！
	}

	FLOAT sita = (FLOAT) acos( temp1/temp2 );
	if (b<=g)
	{
		h = sita;
	}else
	{
		h = 360 - sita;
	}
	
	if (h>=360)
	{
		h = h - 360;
	}

	s = (FLOAT) ( 1 - 3.0 * myMath.ReturnMinInThree(r, g, b) / (r + g + b) );

	i = (FLOAT) ( (r + g + b) / 3.0 );

	return TRUE;
}

BOOL MyColorSpace::Hsi2Rgb(FLOAT h, FLOAT s, FLOAT i, FLOAT& r, FLOAT& g, FLOAT& b)
//冈萨雷斯《数字图像处理》第二版，p237；
{
/*
	r = h;
	g = s;
	b = i;
*/
	if (h<0||h>=360)
	{
		return FALSE;
	}

	if (h<120)
	{
		b = i * ( 1 - s);
		r = (FLOAT) ( i * ( 1 + s*cos(h) / cos(60.0-h) ) );
		g = 1 - ( r+b );
		return TRUE;
	}

	if (h<240)
	{
		h = h - 120;
		r = i * ( 1 - s);
		g = (FLOAT) ( i * ( 1 + s*cos(h) / cos(60.0-h) ) );
		b = 1 - ( r+g );
		return TRUE;
	}

	if (h<360)
	{
		h = h - 240;
		g = i * ( 1 - s);
		b = (FLOAT) ( i * ( 1 + s*cos(h) / cos(60.0-h) ) );
		r = 1 - ( g+b );
		return TRUE;
	}

	return FALSE;
}

BOOL MyColorSpace::Rgb2Ycbcr(FLOAT r, FLOAT g, FLOAT b, FLOAT& y, FLOAT& cb, FLOAT& cr)
//据《Color and Texture Descriptors》by B.S.Manjunath,eg.
{	
	y = (FLOAT) ( 0.2989*r + 0.5866*g + 0.1145*b );
	cb = (FLOAT) ( -0.1687*r - 0.3312*g + 0.5000*b );
	cr = (FLOAT) ( 0.5000*r - 0.4183*g - 0.0816*b );
	return TRUE;
}

BOOL MyColorSpace::Rgb2Hsv(FLOAT r, FLOAT g, FLOAT b, FLOAT& h, FLOAT& s, FLOAT& v)
//据《Color and Texture Descriptors》by B.S.Manjunath,eg.
{	
	//无定义(非彩色)时，h返回0;
	FLOAT tempmax = (FLOAT) myMath.ReturnMaxInThree(r, g, b);
	FLOAT tempmin = (FLOAT) myMath.ReturnMinInThree(r, g, b);

	v = tempmax;

	if (tempmax == 0)
	{
		s = 0;
	}else
	{
		s = (tempmax - tempmin) / tempmax;
	}

	if (tempmax == tempmin)
	{
		h = 0;//undefined(achromatic color);
	}else
	{
		if (tempmax==r && g>b)
		{
			h = 60*(g-b) / (tempmax - tempmin);
		}else if (tempmax==r &&g<b)
		{
			h = 360 + 60*(g-b) / (tempmax-tempmin);
		}else if (g==tempmax)
		{
			h = (FLOAT) ( 60 * ( 2.0 + (b-r) / (tempmax-tempmin) ) );
		}else
		{
			h = (FLOAT) ( 60 * ( 4.0 + (r-g) / (tempmax-tempmin) ) );
		}
	}

	return TRUE;
}

BOOL MyColorSpace::Rgb2Hsv2(FLOAT r, FLOAT g, FLOAT b, FLOAT& h, FLOAT& s, FLOAT& v)
//姚子程序中的转换，和上面书中的的不一样？？？
{	
	if (r==g && r==b)
	{
		h = 0;
	}else
	{
		h = (FLOAT) acos( (2*r-g-b) / ( 2.0 * sqrt( pow((r-g),2.0) + (r-b)*(g-b) ) ) );
	}
	if (b>g) 
	{
		h = (FLOAT) ( 2*PI - h );
	}

	s = (FLOAT) ( ( __max( r, __max(g,b) ) - __min( r, __min(g,b) ) ) / 255 );
	v = (FLOAT) ( (r+g+b) / (3.0*255) );

	return TRUE;
}


BOOL MyColorSpace::XyztoRgb(FLOAT* xyzDatas, int width, int height, BYTE* rgbDatas)
{
	if (NULL == rgbDatas)
	{
		CString tempstr;
		tempstr.Format("传入内存未分配，在MyColorSpace::RGBtoHSV");
		AfxMessageBox(tempstr,NULL,MB_OK);
		return FALSE;		
	}

	int i,j;
	FLOAT r,g,b;
	FLOAT x, y, z;
	unsigned long pos;

	for(i=0;i<height;i++)
	{
		for (j=0;j<width;j++)
		{
			pos = (i*width+j) * 3;
			x = xyzDatas[pos];
			y = xyzDatas[pos+1];
			z = xyzDatas[pos+2];
			
			Xyz2Rgb(x, y, z, r, g, b);
			rgbDatas[pos] = (BYTE) ( b*255 );
			rgbDatas[pos+1] = (BYTE) ( g*255 );
			rgbDatas[pos+2] = (BYTE) ( r*255 );			
		}
	}
	return TRUE;
}

BOOL MyColorSpace::RgbtoXyz(BYTE* inDatas, int width, int height, FLOAT* xyzbuff)
//将位图图像数据转换为HSV数据；
{
	if (NULL == xyzbuff)
	{
		CString tempstr;
		tempstr.Format("传入内存未分配，在MyColorSpace::RGBtoHSV");
		AfxMessageBox(tempstr,NULL,MB_OK);
		return FALSE;		
	}

	int i, j;
	FLOAT r, g, b;
	FLOAT x, y, z;
	unsigned long pos;

	for(i=0;i<height;i++)
	{
		for (j=0;j<width;j++)
		{
			pos = (i*width+j) * 3;
			r = (FLOAT) ( (FLOAT)inDatas[pos+2] / 255. );
			g = (FLOAT) ( (FLOAT)inDatas[pos+1] / 255. );
			b = (FLOAT) ( (FLOAT)inDatas[pos] / 255. );

			Rgb2Xyz(r, g, b, x, y, z);
			xyzbuff[pos] = x;
			xyzbuff[pos+1] = y;
			xyzbuff[pos+2] = z;			
		}
	}
	return TRUE;
}

BOOL MyColorSpace::RgbtoHsv(BYTE* inDatas, int width, int height, MyHSV* outHSV)
//将位图图像数据转换为HSV数据；
{
	if (NULL == outHSV)
	{
		CString tempstr;
		tempstr.Format("传入内存未分配，在MyColorSpace::RGBtoHSV");
		AfxMessageBox(tempstr,NULL,MB_OK);
		return FALSE;		
	}

	int i,j;
	FLOAT r,g,b;
	unsigned long pos;

	for(i=0;i<height;i++)
	{
		for (j=0;j<width;j++)
		{
			pos = i*width+j;
			r = (FLOAT) ( (FLOAT)inDatas[3*pos+2] / 255. );
			g = (FLOAT) ( (FLOAT)inDatas[3*pos+1] / 255. );
			b = (FLOAT) ( (FLOAT)inDatas[3*pos] / 255. );
			FLOAT h, s, v;
			Rgb2Hsv(r, g, b, h, s, v);
			outHSV[pos].h = h;
			outHSV[pos].s = s;
			outHSV[pos].v = v;			
		}
	}
	return TRUE;
}

DOUBLE MyColorSpace::GetHsvDistance(MyHSV x, MyHSV y)
{
    //距离在0与sqrt(5)之间;
	//来自姚子程序RemoteDemo;
	double m_d, v, sc, ss;
	v = pow(x.v-y.v, 2);
	sc = pow( x.s*cos(x.h)-y.s*cos(y.h), 2 );
	ss = pow( x.s*sin(x.h)-y.s*sin(y.h), 2 );
	m_d = sqrt( v+sc+ss );
	return m_d;
}

/*
BOOL MyColorSpace::Luv2Rgb(FLOAT l, FLOAT u, FLOAT v, FLOAT& r, FLOAT& g, FLOAT& b)
//来自D. Comaniciu, P. Meer，Robust Analysis of Feature Spaces: Color Image Segmentation 的相应代码；
{
	if(v<0.1)
	{
		r=0;
		g=0;
		b=0;
	}
	else
	{
		float my_x, my_y, my_z;
		if(v< 8.0)
			my_y = (float) ( Yn * v / 903.3 );
		else
			my_y = (float) ( Yn * pow((v + 16.0) / 116.0, 3) );
		
		float u_prime = u / (13 * v) + Un_prime;
		float v_prime = l / (13 * v) + Vn_prime;
		
		my_x = 9 * u_prime * my_y / (4 * v_prime);
		my_z = (12 - 3 * u_prime - 20 * v_prime) * my_y / (4 * v_prime);
		
		b = (float) ( int((RGB[0][0]*my_x + RGB[0][1]*my_y + RGB[0][2]*my_z)*255.0) );
		g = (float) ( int((RGB[1][0]*my_x + RGB[1][1]*my_y + RGB[1][2]*my_z)*255.0) );
		r = (float) ( int((RGB[2][0]*my_x + RGB[2][1]*my_y + RGB[2][2]*my_z)*255.0) );
		
		if(b>255) b=255;
		else if(b<0) b=0;
		
		if(g>255) g=255;
		else if(g<0) g=0;
		
		if(r>255) r=255;
		else if(r<0) r=0;
	}

	return TRUE;
}
*/

BOOL MyColorSpace::Rgb2Xyz(FLOAT r, FLOAT g, FLOAT b, FLOAT& x, FLOAT& y, FLOAT& z)
//RGB to CIE XYZitu(D65), 根据David Bourgin, Color space FAQ
{
	x = (FLOAT) ( 0.431*r + 0.342*g + 0.178*b );
	y = (FLOAT) ( 0.222*r + 0.707*g + 0.071*b );
	z = (FLOAT) ( 0.020*r + 0.130*g + 0.939*b );
	return TRUE;
}

BOOL MyColorSpace::Xyz2Rgb(FLOAT x, FLOAT y, FLOAT z, FLOAT& r, FLOAT& g, FLOAT& b)
//CIE XYZitu(D65) to RGB,根据David Bourgin, Color space FAQ
{
	r =  (FLOAT) ( 3.063*x - 1.393*y - 0.476*z );
	g = (FLOAT) ( -0.969*x + 1.876*y + 0.042*z );
	b =  (FLOAT) ( 0.068*x - 0.229*y + 1.069*z );
	return TRUE;
}

BOOL MyColorSpace::Xyz2Luv(FLOAT x, FLOAT y, FLOAT z, FLOAT& l, FLOAT& u, FLOAT& v)
//CIE XYZitu(D65) to CIE Luv,根据David Bourgin, Color space FAQ
//The white point is D65 and have coordinates fixed as (xn;yn)=(0.312713;0.329016). 
//上面这句不懂意思。Xn、Yn和Zn的值来自D. Comaniciu, P. Meer，Robust Analysis of Feature Spaces: Color Image Segmentation 的相应代码
{
	DOUBLE tempb = pow((y/Yn), (1.0/3.0));
	if (y/Yn>0.008856)
	{
		l = (FLOAT) ( 116 * pow((y/Yn), (1.0/3.0)) );
	}else
	{
		l = (FLOAT) ( 903.3 * y/Yn );
	}

	float up = 4 * x / (x+15*y+3*z);
	float vp = 9 * y / (x+15*y+3*z);
    float upn = 4 * Xn / (Xn+15*Yn+3*Zn);
	float vpn = 9 * Yn / (Xn+15*Yn+3*Zn);
	
	u = 13 * (l) * (up - upn);
    v = 13 * (l) * (vp - vpn);

	return TRUE;
}

BOOL MyColorSpace::Ycbcr2Rgb(FLOAT y, FLOAT cb, FLOAT cr, FLOAT& r, FLOAT& g, FLOAT& b)
//Rec 601-1 YCbCr to RGB, 根据David Bourgin, Color space FAQ
{
	r = (FLOAT) ( y + 0.0000*cb + 1.4022*cr );
	g = (FLOAT) ( y - 0.3456*cb - 0.7145*cr );
	b = (FLOAT) ( y + 1.7710*cb + 0.0000*cr );

	return TRUE;
}

FLOAT MyColorSpace::Labf(FLOAT input, FLOAT Y_Yn)
//用于计算Lab色彩；
{
	if (Y_Yn>0.008856)
	{
		return (FLOAT)pow(input, (1.0/3.0));		
	}else
	{
		return (FLOAT) ( 7.787*input + 16./116. );
	}	
}

BOOL MyColorSpace::Xyz2Lab(FLOAT x, FLOAT y, FLOAT z, FLOAT& l, FLOAT& a, FLOAT& b)
//CIE XYZitu(D65) to CIE Lab, 根据David Bourgin, Color space FAQ
{
	FLOAT y_yn = y / Yn;
	if (y_yn>0.008856)
	{
		l = (FLOAT) ( 116. * pow(y_yn, 1./3.) );
	}else
	{
		l = (FLOAT) ( 903.3 * y_yn );
	}
    a = 500 * ( Labf(x/Xn, y_yn) - Labf(y/Yn, y_yn) );
    b = 200 * ( Labf(y/Yn, y_yn) - Labf(z/Zn, y_yn) );

	return TRUE;	
}

BOOL MyColorSpace::Lab2Lch(FLOAT li, FLOAT a, FLOAT b, FLOAT& lo, FLOAT& c, FLOAT& h)
//CIE Lab to CIE LCH, 根据David Bourgin, Color space FAQ
{
    lo = li;
    c = (FLOAT) ( pow( (a*a+b*b), 0.5) );

	INT k= -1;
	if (a>=0 && b>=0)
	{
		k = 0;
	}else if (a>=0 && b<0)
	{
		k = 1;
	}else if (a<0 && b<0)
	{
		k = 2;
	}else if (a<0 && b>=0)
	{
		k = 3;
	}

	if (a==0)
	{
		h = 0;
	}else
	{
		h = (FLOAT) ( (atan(b/a) + k*PI/2.) / (2.*PI) );
		if (h<0)
		{
			h += (FLOAT) ( PI/2. );
		}
	}

	return TRUE;
}

#define MAXV 256
BOOL MyColorSpace::RgbtoLuvPcm(BYTE* inDatas, int width, int height, MyLUV* luvbuff)
//基于表转换，将位图图像数据转换为LUV数据, 修改自D. Comaniciu, P. Meer，
//Robust Analysis of Feature Spaces: Color Image Segmentation 的相应代码
{
	int x, y, z, my_temp;
	
	float l_star, u_star, v_star;
	float u_prime, v_prime;
	//register int temp_col, temp_index, temp_ind;
	register int j;//,k;
	
	int a00=XYZ[0][0], a01=XYZ[0][1], a02=XYZ[0][2];
	int a10=XYZ[1][0], a11=XYZ[1][1], a12=XYZ[1][2];
	int a20=XYZ[2][0], a21=XYZ[2][1], a22=XYZ[2][2];
	
	int *A00 = new int[MAXV]; int *A01 = new int[MAXV]; int *A02 = new int[MAXV];
	int *A10 = new int[MAXV]; int *A11 = new int[MAXV]; int *A12 = new int[MAXV];
	int *A20 = new int[MAXV]; int *A21 = new int[MAXV]; int *A22 = new int[MAXV];
	
	for(j=0; j<MAXV; j++)
    {
		A00[j]=a00*j; A01[j]=a01*j; A02[j]=a02*j;
		A10[j]=a10*j; A11[j]=a11*j; A12[j]=a12*j;
		A20[j]=a20*j; A21[j]=a21*j; A22[j]=a22*j;
    }
	
	float *my_pow = new float[MAXV];
	for(j=0; j<MAXV; j++)
		my_pow[j]= (FLOAT) ( 116.0 * pow(j/255.0, 0.3333333) - 16 );
	
	for ( j = 0; j < width*height; j++)
    {
		INT R = inDatas[j*3+2];
		INT G = inDatas[j*3+1];
		INT B = inDatas[j*3];
	
        x = A00[R] + A01[G] + A02[B];
        y = A10[R] + A11[G] + A12[B];
        z = A20[R] + A21[G] + A22[B];
		
        float  tval = (FLOAT) ( y / 2550000.0 ); //Yn==1
		if ( tval >  Lt)  l_star = my_pow[(int)(tval*255+0.5)];
        else  l_star = (FLOAT) ( 903.3 * tval );
		
        my_temp = x + 15 * y + 3 * z;
		if(my_temp)
		{
			u_prime = (float)(x << 2) / (float)(my_temp);
			v_prime = (float)(9 * y) / (float)(my_temp);
		}
		else
		{
			u_prime = 4.0;
			v_prime = (FLOAT) ( 9.0/15.0 );
		}
		
		tval = 13*l_star;
        u_star = tval * (u_prime - Un_prime); // Un_prime = 0.1978
        v_star = tval * (v_prime - Vn_prime); // Vn_prime = 0.4683
		
		luvbuff[j].l = l_star;
		luvbuff[j].u = u_star;
		luvbuff[j].v = v_star;
    }

	delete [] my_pow;
	delete [] A22; delete [] A21; delete [] A20;
	delete [] A12; delete [] A11; delete [] A10;
	delete [] A02; delete [] A01; delete [] A00;

	return TRUE;
}

BOOL MyColorSpace::RgbtoLuv(BYTE* inDatas, int width, int height, MyLUV* luvbuff)
//将位图图像数据转换为LUV数据；
{
	if (NULL == luvbuff)
	{
		CString tempstr;
		tempstr.Format("传入内存未分配，在MyColorSpace::RgbtoLuv");
		AfxMessageBox(tempstr,NULL,MB_OK);
		return FALSE;		
	}

	int i,j;
	FLOAT r,g,b;
	unsigned long pos;

	for(i=0;i<height;i++)
	{
		for (j=0;j<width;j++)
		{
			pos = i*width+j;
			r = (FLOAT)( (FLOAT)inDatas[3*pos+2] / 255. );
			g = (FLOAT)( (FLOAT)inDatas[3*pos+1] / 255. );
			b = (FLOAT)( (FLOAT)inDatas[3*pos] / 255. );
			FLOAT x, y, z;
			Rgb2Xyz(r, g, b, x, y, z);

			FLOAT l, u, v;
			Xyz2Luv(x, y, z, l, u, v);
			luvbuff[pos].l = l;
			luvbuff[pos].u = u;
			luvbuff[pos].v = v;			
		}
	}
	return TRUE;
}

BOOL MyColorSpace::LuvToRgb(FLOAT* luvData, INT width, INT height, BYTE* rgbData)
{
	if (NULL == rgbData)
	{
		CString tempstr;
		tempstr.Format("传入内存未分配，在MyColorSpace::RGBtoHSV");
		AfxMessageBox(tempstr,NULL,MB_OK);
		return FALSE;		
	}

	int i,j;
	INT r,g,b;
	FLOAT l, u, v;
	unsigned long pos;

	for(i=0;i<height;i++)
	{
		for (j=0;j<width;j++)
		{
			pos = (i*width+j) * 3;
			l = luvData[pos];
			u = luvData[pos+1];
			v = luvData[pos+2];
			
			Luv2Rgb(l, u, v, r, g, b);
			rgbData[pos] = b;
			rgbData[pos+1] = g;
			rgbData[pos+2] = r;			
		}
	}
	return TRUE;
}

BOOL MyColorSpace::Luv2Rgb(FLOAT l, FLOAT u, FLOAT v, INT& r, INT& g, INT& b)
//LUV to RGB, 修改自D. Comaniciu, P. Meer，
//Robust Analysis of Feature Spaces: Color Image Segmentation 的相应代码
{
	if(l<0.1)
	{
		r = g = b =0;
	}
	else
	{
		float my_x, my_y, my_z;
		if(l < 8.0)
			my_y = (FLOAT) ( Yn * l / 903.3 );
		else
			my_y = (FLOAT) ( Yn * pow((l + 16.0) / 116.0, 3) );
		
		float u_prime = u / (13 * l) + Un_prime;
		float v_prime = v / (13 * l) + Vn_prime;
		
		my_x = 9 * u_prime * my_y / (4 * v_prime);
		my_z = (12 - 3 * u_prime - 20 * v_prime) * my_y / (4 * v_prime);
		
		r =int((RGB[0][0]*my_x + RGB[0][1]*my_y + RGB[0][2]*my_z)*255.0);
		g =int((RGB[1][0]*my_x + RGB[1][1]*my_y + RGB[1][2]*my_z)*255.0);
		b =int((RGB[2][0]*my_x + RGB[2][1]*my_y + RGB[2][2]*my_z)*255.0);
		
		if(r>255) r=255;
		else if(r<0) r=0;
		
		if(g>255) g=255;
		else if(g<0) g=0;
		
		if(b>255) b=255;
		else if(b<0) b=0;
	}
	
	return TRUE;
}
