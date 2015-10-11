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

// QT.cpp: implementation of the QT class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "image.h"
#include "QT.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QT::QT(int ind,int x,int y,int m,int n,QT*p):index(ind),ulx(x),uly(y),lm(m),un(n),parent(p)
{
	for(int i=0;i<4;i++)
	child[i]=NULL;
}

QT::~QT()
{

}

void QT::Print()
{
//	ofstream out("jiu1.txt",ios::app);

//	out<<index<<"\t"<<ulx<<"\t"<<uly<<"\t"<<lm<<"\t"<<un<<endl;
}

bool QT::IsLeaf()
{
	return child[1]==NULL&&child[2]==NULL&&child[3]==NULL&&child[0]==NULL;
}
