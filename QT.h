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

// QT.h: interface for the QT class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QT_H__1F872C06_E5B9_433E_A171_04FC6A906CF2__INCLUDED_)
#define AFX_QT_H__1F872C06_E5B9_433E_A171_04FC6A906CF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class QT  
{
public:
	bool IsLeaf();
	void Print();
	QT(int ind=0,int x=0,int y=0,int m=0,int n=0,QT*p=NULL);
	virtual ~QT();
	QT*parent,*child[4];//0,1,2,3 quadrant order.
	int index;
	int ulx,uly;//upper left coordinate
	int lm,un;//edge length left m,upper n
};

#endif // !defined(AFX_QT_H__1F872C06_E5B9_433E_A171_04FC6A906CF2__INCLUDED_)
