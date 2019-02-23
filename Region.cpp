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

// Region.cpp: implementation of the Region class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 
#include "Region.h"
#include <assert.h>
#include <math.h>
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Region::~Region()
{  
	NPList.clear();
}

float Region::InterDiff(int d,float wc,float wp,float range)
{
	if(size==1)
		return interdif=(1-wc)*(1+3*wp); // a shorthand to bail out the 
		// following calculations
	float trek=0;
	interdif=0;
	for(int i=0;i<d;i++)
	{
		trek=sSum[i]-(addition[i]/size*addition[i]);
		if(trek<0)
		{
			assert(abs(trek)<1e-4);
			continue;
		}
		interdif+=sqrt(trek/(size-1));
	}
	interdif*=(wc*range/d);
	interdif+=(1-wc)*wp*perim/sqrt((float)size);
	
	unsigned int wid=norbox->Width(),hei=norbox->Height();
	unsigned int realb=2*(wid+hei);
	
	assert(perim>=realb);

	interdif+=(1-wc)*(1-wp)*perim/realb;
	interdif*=size;
	assert(interdif>=0);
	return interdif;
}
