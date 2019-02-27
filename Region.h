// Region.h: interface for the Region class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGION_H__5C2B5D5D_02AB_4004_99C1_3F7AE3C28297__INCLUDED_)
#define AFX_REGION_H__5C2B5D5D_02AB_4004_99C1_3F7AE3C28297__INCLUDED_
//#pragma warning (disable:4786)
#include<vector> 
#include "NP.h"
using namespace std;
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
typedef vector<NP> NPL;

typedef struct tPair
{
	int r[2];
	float t;
	unsigned int bl;//boundary length between the pair region
//	float es;//edge strength, note that when a pixel belong to P abuts two neighbors which merge to Q later,
	//then the edge strength of this pixel is added twice in the edge strength between region P and Q.
	//since intuitively, the chance the pixel is an edge point is bigger.
	tPair():t(0.f),bl(0){}
}tPair;
enum PROP
{
	maxaxislen=0,
	minaxislen,
	meanwidth,
	meanlen,
	ndvindex
};

class Region  
{
public:

	//void DeleteNeighbor(const int tag);
	UINT size; 
	float *addition;//all the pixel gray value sum
	float *sSum;//average square sum
	NPL NPList;
	int perim;
	int bestp;// best merge candidate pair index
	int p;//parent index
	float interdif;
	CRect *norbox;//regular box bounding the region
	vector<float> attlist;//attribute list according to prop
public:
	float InterDiff(int d=1,float wc=0.9,float=0.5,float=256.f);
	Region();
	~Region();
}; 
//Actually,the result of compute dist is the internaldif(a&b)


#endif // !defined(AFX_REGION_H__5C2B5D5D_02AB_4004_99C1_3F7AE3C28297__INCLUDED_)
