// _MC.cpp: implementation of the _MC class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "image.h"
#include "_MC.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

_MC::_MC(int p,double c):pInd(p),t(c)
{

}

_MC::~_MC()
{

}
bool _MC::operator<(const _MC &b)const
{
	if(t==b.t)
		return pInd<b.pInd;
	else 
		return t<b.t;
}