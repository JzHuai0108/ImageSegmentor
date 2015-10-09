// MyGabor.h: interface for the CMyGabor class.
/*

修改自姚子Gabor纹理代码，
参考文献：
    Texture Features for Browsing and Retrieval of Image Data, by B.S.Manjunath and W.Y.Ma
以及，姚子论文-基于纹理特征的遥感图像检索技术

               04.06.03
*/
/////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYGABOR_H__6E9D2397_C60E_4079_8277_D7912494E533__INCLUDED_)
#define AFX_MYGABOR_H__6E9D2397_C60E_4079_8277_D7912494E533__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMyGabor  
{
public:
	CMyGabor();
	virtual ~CMyGabor();
    void GetBlockGaborCoeff(FLOAT* inData, INT m, INT n, FLOAT& outCoeff);//得到输入数据块在指定方向和尺度下Gabor的变换系数, 调用者要保证输入数据块的大小符合要求；
    void GetImageGaborCoeff(FLOAT* imageData, INT imagWidth, INT imageHeight, INT m, INT n, FLOAT** outData);//得到输入图像在指定方向和尺度下的Gabor变换, 输出内存由调用者分配;

private:
	void InitPara(INT m, INT n);//初始化各项参数, m为尺度，n为方向；
	void GetCoeffAt(INT m, INT n, INT x, INT y, DOUBLE& coeffr, DOUBLE& coeffi);//得到一个滤波器系数；
	void CalcuCoeffWhen(INT m, INT n);//计算此尺度和方向时的滤波器系数, m为尺度，n为方向；

private:
	CString curMN;//当前已计算尺度和方向的滤波器；
	INT myM;//尺度个数；
	INT myN;//方向个数；
	INT myL;//滤波器系数个数；
    DOUBLE curWmn;//调制频率；
	DOUBLE myUl;//upper center frequency;
	DOUBLE myUh;//lower center frequency;
	DOUBLE curAlpha;
	LONG gaborSize;

	DOUBLE* coeffR;//系数实部；
	DOUBLE* coeffI;//系数虚部；
	DOUBLE  deltaXmn;//方差Xmn；
	DOUBLE  deltaYmn;//方差Ymn；
	DOUBLE  sita;//角度；
};

#endif // !defined(AFX_MYGABOR_H__6E9D2397_C60E_4079_8277_D7912494E533__INCLUDED_)
