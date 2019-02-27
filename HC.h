// HC.h: interface for the CHC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HC_H__76B42677_678A_468C_98BC_640578E761BF__INCLUDED_)
#define AFX_HC_H__76B42677_678A_468C_98BC_640578E761BF__INCLUDED_
#pragma warning (disable:4786)
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//chc part begin
#include<vector>
#include<set> 
#include"Region.h"
#include"_MC.h"
#include"QT.h"
#include<assert.h> 
#include<algorithm>
#include <fstream>
#include "misc.h" // for square
#include "..\\include\\gdal.h"
#include "..\\include\\gdal_priv.h"
#include "..\\include\\cpl_string.h"

//#include "cv.h"
//#include "highgui.h"

//remember the format of set declaration where space are indispensable.
//Note that when set is changed to multiset as well as change of operator< for MC, function erase delete the keys with same t
//which cannot be mended by adding function operator== to MC, thereby in Hierclust MCL.erase()
//must be edited. However, even doing so cannot make the entries in MC with same t randomly disposed
//, because it seems the multiset tries to make those items in MC tidy and nice.
#define BYTE_PER_LINE(w, c) ((((w)*(c)+31)/32)*4)

enum Appro
{
	HC=0,
	QTS,
	GBS
};
//COMPACT = Sqrt (4 * AREA / pi) / outer contour length 
//ROUNDNESS = 4 * (AREA) / (pi * MAXAXISLEN^2) 
//SOLIDITY = AREA / area of convex hull 
//(Band4-Band3)/(Band4 + Band3) for quickbird
//NDWI=(G-NIR)/(G+NIR) mask black body
//SSI=ABS(R + B-2G)// distinguish water from shadow
const int	MMSquare=46;	//      正方形
const int	MMRhombus=47;		//		菱形
const int REGSIZE=11;
const int PERIM=12;
const int INTERDIF=13;
const int MAXAL=14;
const int MINAL=15;
const int MEANW=16;
const int MEANL=17;
const int NDVI=18;
const int CMPCT=19;
const int ELONG=20;//ELONGATION MAXAL/MINAL
const int MLWR=21;//MEAN LENGTH width ratio
enum EdgeMode
{
	SUSAN=0,
	SOBEL,
	CANNY
};

typedef set <_MC, less<_MC> > sl;

const float Range=256.f;
class CHC  
{
	int Width,Height;
//	QT *root;//the root for quadtree which is fundamental for quadtree seg,not for quadtree merge
	float * data_;//original data and the place for work in order of bgr for rgb images, in windows format
	float *sData_;//square of data, data_ are regularized by Range
	vector<Region> S;//region set, if S[x].p==x, S[x] is a region
	vector<tPair> A;// neighbor pair list
	sl MCL;//merge candidate list, because MCL is required to be able to find, erase and insert element so set.
	CRect *grid;//bounding box of each region

public:
	int Type;
	int d_;//dimension of data in mlpbits for seg, default 3
	int loop;//number of cycle or number of regs
	//determine when iteration should stop in hierclust it means required region number.
	//in multi-resolution segmentation it means cycles to be performed.
	double eval;//evvaluation 
	double RI;//rand index
	int minsize;//least region size not to be pruned
	int comps;// number of regions 
//	float alpha;//firstly used as ratio between neighbor's internal difference 
	//and combined difference in treesegsub, then employed as edge ratio threshold.
	float K;//scale parameter for graph segmentation
	float maxDelta;//the threshold to merge pixels in quadtree
	BYTE *EM;//edge strength map
	float *ndv;//mdvi values
	int*tag;//tag for each pixel according to S, always keep abreast
	float wc,wp;//weight for color and weight for compactness
	float hdiff;//imitating scale parameter in ecognition
	vector<float> bWArray;//recorded the weight for each band
public:
	void GetPreviewMask(CRect&rect);
	void SaveSeg2(GDALDataset *,CString pathname,int);
	void RegionThresh(float thresh,CRect);
	void GetHistogramData(int*dataChannel,float &rmax,float &rmin);
	void GetBWArray(CString);
	
	void GaussianFilter(float sigma);
	void RoadSeed(BYTE*,char*fisData=NULL);
	void RoadThin(BYTE*,vector<int>&);
	void RoadLink(BYTE*,vector<int>&);
	void RoadExpo(BYTE*,vector<int>&,GDALDataset*);
	CRect GetBoundBox(int miss);
	int GetSize(int label);
	void SaveSeg(GDALDataset* m_pDataset,CString pathname);

	void RegMoment(int label);
	void SetWH(int, int);
	void Morph(int opt);
	void BinMorph(LPBYTE, int ,int, int,BYTE mode, BYTE&nRepeat);
	void qttest(int md=32);

	void LenWidR();
	void RegionProps();
	void BorderRefine();
	void KillMinion(UINT nsize);
	void MRS(int L=0);
	CHC();
	virtual ~CHC();
	void Clear();
	int MiniTag(int*indarr,int len);
	double BenchMark(int *rect,int ind=1);
	BYTE* EdgeSusan(int w,int h,int=20,bool=false);
	void EdgeMag(int w,int h);	
	void EdgePoints(int w,int h,float sig=0.8f, float low=0.4f, float high=0.79f);
	void BuildData(GDALDataset* m_pDataset);
	int InitializeASM(int ln,int com, enum Appro=HC);
	void QTMerge();
	void ElimTree(QT*r);
	void ShowQT(QT*r);
	bool Predicate(int ul,int sz1,int ur,int sz2,int ll=-1,int sz3=0,int lr=-1,int sz4=0);
	void BQT(int sz, QT *parent);
	int HierClust();

	void InitiateRegionSet();
	int SegGraph4();
	void RegionLabel();

	void EvalQs(BYTE*lpBits,int bitCount);
	void InitiateRegions();
	void InitiateRegions8();

	void LamSchd(Region*a,Region*b,unsigned int len,double &ret);
	
	//remember to initiate region a.intertif before bistbenz
	inline void DistBenz(Region&a,Region&b,unsigned int len,float &ret)
	{
	//	ofstream out("fault.txt",ios::app);
		ret=0;
		//compute sigma first
		float ext=a.size+b.size;
		float trek=0;
		for(int i=0;i<d_;i++)
		{
			trek=a.sSum[i]+b.sSum[i]-square(a.addition[i]+b.addition[i])/ext;
			//assert(trek>=0);
			if(trek<0) 
			{
			//out<<"trek/t"<<trek<<endl;
				continue;
			}
		//	assert(trek>=0);
			ret+=sqrt(trek/ext);
		}
		ret*=wc;
		unsigned int perim=a.perim+b.perim-2*len;
		ret+=(1-wc)*wp*perim/sqrt(ext)/Range;
		CRect c;
		c.UnionRect(a.norbox,b.norbox);
		unsigned int wid=c.Width(),hei=c.Height();
		unsigned int realb=2*(wid+hei);
/*		UINT ba=2*(a.norbox->Width()+a.norbox->Height()),bb=2*(b.norbox->Width()+b.norbox->Height());
		UINT pa=a.perim,pb=b.perim;
		if(((1.f*perim/realb)<(1.f*pa/ba))||((1.f*perim/realb)<(1.f*pb/bb)))
out<<"{perim, realb}*{a+b,a,b}"<<perim<<"\t"<<realb<<"\t"<<pa<<"\t"<<ba<<"\t"<<pb<<"\t"<<bb<<"\n";
		if((1.f*perim/sqrt(ext))<(1.f*pa/sqrt(a.size))||(1.f*perim/sqrt(ext))<(1.f*pb/sqrt(b.size)))
out<<"{perim, ext}*{a+b,a,b}"<<perim<<"\t"<<ext<<"\t"<<pa<<"\t"<<a.size<<"\t"<<pb<<"\t"<<b.size<<"\n";
*/
		assert(perim>=realb);
		ret+=(1-wc)*(1-wp)*perim/realb/Range;
		ret*=ext;
		ret-=(a.interdif+b.interdif);
	}

	inline void JhHypo(Region&a,Region&b,unsigned int len,float &ret)
	{//edge weight
		ret=0;
		float as=a.size,bs=b.size;
		//compute sigma first
		for(int i=0;i<d_;++i)
		{
			//here for the quotient of addition sum the fractional part is discarded for small difference
			ret+=square((a.addition[i])/as-(b.addition[i])/bs);
		}
		
		ret*=(as*bs);
		ret/=(as+bs);
		
	//	ret*=phi;
	//	ret+=(1-phi)*ew;
		ret/=len;
	}
	//for the pixels indexing from xori+uly*w downwards until step.
	//inout:xori uly are the x and y coord of the upper left point to check
	//step is the height for the line of pixels,must be power of 2, rID is the tag of region 
	//which is checking neighbors,sernum is the last void A tuple serno.
	inline void NamNeiY(int xori,int uly, int step, int rID,int&sernum)
	{
		int t=tag[uly*Width+xori];
		
		if(step==1)
		{
			NP np(t,sernum);
			S[rID].NPList.push_back(np);
			np.rInd=rID;
			S[t].NPList.push_back(np);
			
			A[sernum].r[0]=rID;
			A[sernum].r[1]=t;
			A[sernum].bl=1;
			++sernum;
			return;
		}
		int hs=step>>1;
		if(t==tag[(uly+hs)*Width+xori])
		{
			NP np(t,sernum);
			S[rID].NPList.push_back(np);
			np.rInd=rID;
			S[t].NPList.push_back(np);
			A[sernum].r[0]=rID;
			A[sernum].r[1]=t;
			A[sernum].bl=step;
			++sernum;
			return;
		}
		else
		{
			NamNeiY(xori,uly,hs,rID,sernum);
			NamNeiY(xori,uly+hs,hs,rID, sernum);
		}
	}
	//improved version namneiy for arbitray positive step this function intended for boundary neighbor initiation
	inline void NamNeiYEx(int xori,int uly, int step, int rID,int&sernum)
	{
		int back=step+uly;
		int k=0;
		while(step)
		{
			if(step&1)
			{
				back-=1<<k;
				NamNeiY(xori,back,1<<k,rID,sernum);
			}
			step>>=1;
			++k;
		}
	}
	//checking pixels from index yori*w+ulx to yori*w+lrx for region rID
	inline void NamNeiX(int yori,int ulx, int step, int rID, int&sernum)
	{
		int t=tag[yori*Width+ulx];
		if(step==1)
		{
			NP np(t,sernum);
			S[rID].NPList.push_back(np);
			np.rInd=rID;
			S[t].NPList.push_back(np);
			A[sernum].r[0]=rID;
			A[sernum].r[1]=t;
			A[sernum].bl=1;
			++sernum;
			return;
		}
		int hs=step>>1;
		if(t==tag[yori*Width+ulx+hs])
		{
			NP np(t,sernum);
			S[rID].NPList.push_back(np);
			np.rInd=rID;
			S[t].NPList.push_back(np);
			A[sernum].r[0]=rID;
			A[sernum].r[1]=t;
			A[sernum].bl=step;
			++sernum;
			return;
		}
		else
		{
			NamNeiX(yori,ulx,hs,rID, sernum);
			NamNeiX(yori,ulx+hs,hs,rID, sernum);
		}
	}
	inline void NamNeiXEx(int yori,int ulx, int step, int rID, int&sernum)
	{
		int back=step+ulx;
		int k=0;
		while(step)
		{
			if(step&1)
			{
				back-=1<<k;
				NamNeiX(yori,back,1<<k,rID,sernum);
			}
			step>>=1;
			++k;
		}
	}
};



short Referee(unsigned int a,unsigned int b, unsigned int c,unsigned int d=-1);

int DitherGen(int len, int k, int&loc);

double RandIndex(int size,int*U,int R,int*V,int C,double &EI);
void FindContours(int Width,int Height, int*tag);
void GetMask(int j, int i,int *g, int Width,int Height,int*n);
void BorderEncode(int *tag,int Width, int Height);
//CvSeq* GetBoundary(int *tag, int Width, int Height,CvMemStorage* storage);
void GetProfile(float line[4],int len,int Width, vector<int>&pontiff);
double Eval(int* ts, int*ind,int numd,int rts,int rind);
double Eval(int* eu,int rts,int rind);
#endif // !defined(AFX_HC_H__76B42677_678A_468C_98BC_640578E761BF__INCLUDED_)
