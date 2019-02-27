// QT.cpp: implementation of the QT class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "image.h"
#include "QT.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QT::QT(int ind,int x,int y,int m,int n,QT*p):index(ind),ulx(x),uly(y),lm(m),un(n),parent(p)
{
	for(int i=0;i<4;i++)
	child[i]=NULL;
}

QT::~QT()
{

}

void QT::Print()
{
//	ofstream out("jiu1.txt",ios::app);

//	out<<index<<"\t"<<ulx<<"\t"<<uly<<"\t"<<lm<<"\t"<<un<<endl;
}

bool QT::IsLeaf()
{
	return child[1]==NULL&&child[2]==NULL&&child[3]==NULL&&child[0]==NULL;
}
