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

	int pInd;
	double t;

	_MC(int p=-1,double c=0):pInd(p),t(c){}
	_MC(const _MC&pass):pInd(pass.pInd),t(pass.t){}
	_MC&operator=(const _MC&pass){
		if(this==&pass)
			return*this;
		pInd=pass.pInd;
		t=pass.t;
		return*this;
	}
	virtual ~_MC(){}
	bool operator<(const _MC &b) const{		
		if(t==b.t)
			return pInd<b.pInd;
		else 
			return t<b.t;
	}
};

#endif // !defined(AFX__MC_H__864D4BE1_6A01_4531_BB41_D332A9EF68BB__INCLUDED_)
