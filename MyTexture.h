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


// MyTexture.h: interface for the CMyTexture class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYTEXTURE_H__799334D4_CC09_4595_9BD6_2DA2E3705954__INCLUDED_)
#define AFX_MYTEXTURE_H__799334D4_CC09_4595_9BD6_2DA2E3705954__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif //_MSC_VER > 1000
#include "mymath.h"

#define NEIRADIUS 8

typedef struct tagDirectionHis{
	FLOAT sita;
	INT count;
}DirectionHis;

class CMyTexture  
{
public:
	CMyTexture();
	virtual ~CMyTexture();
	FLOAT CalcuEntropy(BYTE* inMatrix, INT width, INT height);//计算输入图像块信息熵；
	BOOL GetDirection(BYTE* inMatrix, INT width, INT height, INT& direction, FLOAT& itensity);//计算输入块的方向, 输出direction表示四个方向之一，itensity表示强度；
	BOOL GetDirectionReal(BYTE* inMatrix, INT width, INT height, FLOAT& direction, FLOAT& itensity, FLOAT& err);//计算输入块的方向, direction方向，itensity强度, err方差；
	FLOAT CalcuDisBetweenDHis(DirectionHis* his1, DirectionHis* his2, INT binCounts);//计算两个方向直方图间的欧氏距离；
	BOOL CalcuDirectionHis(BYTE* inMatrix, INT width, INT height, DirectionHis** outHis, INT& binsInHis);//计算输入块的方向直方图，如有明确方向，则返回TRUE，否则返回FALSE；；

private:
	MyMath myMath;
	
private:
    void CMyTexture::GetGradient(BYTE* image, INT width, INT height
		, FLOAT* deltar, FLOAT* deltasita);//得到输入块梯度；

};

#endif // !defined(AFX_MYTEXTURE_H__799334D4_CC09_4595_9BD6_2DA2E3705954__INCLUDED_)
