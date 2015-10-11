/**************************************************************************

 *  文件名：TemplateTrans.h

 *

 *  图像模板变换API函数库：

 *

 *  Template()			- 图像模板变换，通过改变模板，可以用它实现

 *						  图像的平滑、锐化、边缘识别等操作。

 *  MedianFilter()		- 图像中值滤波。

 *  GetMedianNum()		- 获取中值。被函数MedianFilter()调用来求中值。

 *  ReplaceColorPal()	- 更换伪彩色编码表。

 *

 *************************************************************************/

#ifndef _INC_TemplateTransAPI
#define _INC_TemplateTransAPI

// 函数原型
BOOL WINAPI Template(LPSTR lpDIBBits, LONG lWidth, LONG lHeight, 
					 int iTempH, int iTempW, 
					 int iTempMX, int iTempMY,
					 FLOAT * fpArray, FLOAT fCoef);
BOOL WINAPI MedianFilter(LPSTR lpDIBBits, LONG lWidth, LONG lHeight, 
						 int iFilterH, int iFilterW, 
						 int iFilterMX, int iFilterMY);
unsigned char WINAPI GetMedianNum(unsigned char * bArray, int iFilterLen);
BOOL WINAPI GradSharp(LPSTR lpDIBBits, LONG lWidth, LONG lHeight, BYTE bThre);
BOOL WINAPI ReplaceColorPal(LPSTR lpDIB, BYTE * bpColorsTable);

#endif //!_INC_TemplateTransAPI
