// NP.cpp: implementation of the NP class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "image.h"
#include "NP.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

NP::NP(int r,int p):rInd(r),pInd(p)
{

}

NP::~NP()
{

}

bool NP::operator ==(const NP &R)
{
	return rInd==R.rInd;
}
//to unique items in region s's nplist.
bool NP::operator <(const NP &b)
{
	return rInd<b.rInd;
}
