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

Region::Region():size(0),perim(0),p(-1),interdif(0),norbox(NULL)
{	
}

Region::~Region()
{  
	NPList.clear();
	attlist.clear();
}

float Region::InterDiff(int d,float wc,float wp,float range)
{
	if(size==1)
		return interdif=(1-wc)*(1+3*wp);
		float trek=0;
		interdif=0;
		for(int i=0;i<d;i++)
		{
			trek=sSum[i]-(addition[i]*addition[i])/size;
		//	assert(trek>=0);
			if(trek<0)continue;
			interdif+=sqrt(trek/size);
		}
		interdif*=wc;
		interdif+=(1-wc)*wp*perim/sqrt(size)/range;

		unsigned int wid=norbox->Width(),hei=norbox->Height();
		unsigned int realb=2*(wid+hei);

		assert(perim>=realb);
		interdif+=(1-wc)*(1-wp)*perim/realb/range;
		interdif*=size;
		assert(interdif>=0);
		return interdif;
}
