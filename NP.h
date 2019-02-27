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
	NP(int=-1,int=0);
	virtual ~NP();
	bool operator ==(const NP &R);
	bool operator <(const NP &b);
    int pInd;
	int rInd;

};

#endif // !defined(AFX_NP_H__FF460978_0D64_41EA_937F_E28DDB0AA543__INCLUDED_)
