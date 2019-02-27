// _MC.h: interface for the _MC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX__MC_H__864D4BE1_6A01_4531_BB41_D332A9EF68BB__INCLUDED_)
#define AFX__MC_H__864D4BE1_6A01_4531_BB41_D332A9EF68BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class _MC  
{
public:
	_MC(int=-1,double=0);
	virtual ~_MC();
	bool operator<(const _MC &b) const;
	int pInd;
	double t;
};

#endif // !defined(AFX__MC_H__864D4BE1_6A01_4531_BB41_D332A9EF68BB__INCLUDED_)
