// QT.h: interface for the QT class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QT_H__1F872C06_E5B9_433E_A171_04FC6A906CF2__INCLUDED_)
#define AFX_QT_H__1F872C06_E5B9_433E_A171_04FC6A906CF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class QT  
{
public:
	bool IsLeaf();
	void Print();
	QT(int ind=0,int x=0,int y=0,int m=0,int n=0,QT*p=NULL);
	virtual ~QT();
	QT*parent,*child[4];//0,1,2,3 quadrant order.
	int index;
	int ulx,uly;//upper left coordinate
	int lm,un;//edge length left m,upper n
};

#endif // !defined(AFX_QT_H__1F872C06_E5B9_433E_A171_04FC6A906CF2__INCLUDED_)
