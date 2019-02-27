// NP.h: interface for the NP class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NP_H__FF460978_0D64_41EA_937F_E28DDB0AA543__INCLUDED_)
#define AFX_NP_H__FF460978_0D64_41EA_937F_E28DDB0AA543__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class NP  
{
public:

	int rInd;//region index in line with tag[]
    int pInd;//pair index in A
	
	NP(int r=-1,int p=0):rInd(r),pInd(p){}
	NP(const NP&pass):rInd(pass.rInd),pInd(pass.pInd){}
	NP&operator=(const NP&pass){
		if(this==&pass)
			return*this;
		rInd=pass.rInd;
		pInd=pass.pInd;
		return *this;
	}
	virtual ~NP(){}
	bool operator ==(const NP &R)const {return rInd==R.rInd;}
	bool operator <(const NP &R)const {return rInd<R.rInd;}
};

#endif // !defined(AFX_NP_H__FF460978_0D64_41EA_937F_E28DDB0AA543__INCLUDED_)
