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

Region::Region():size(0),perim(0),p(-1),interdif(0),isChecked(false),norbox(NULL)
{	
}

Region::~Region()
{  
	NPList.clear();
}

float Region::InterDiff(int d,float wc,float wp,float range)
{
	if(size==1)
		return interdif=(1-wc)*(1+3*wp);
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
	interdif+=(1-wc)*wp*perim/sqrt(size);
	
	unsigned int wid=norbox->Width(),hei=norbox->Height();
	unsigned int realb=2*(wid+hei);
	
	assert(perim>=realb);

	interdif+=(1-wc)*(1-wp)*perim/realb;
	interdif*=size;
	assert(interdif>=0);
	return interdif;
}
