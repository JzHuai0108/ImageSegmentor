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
#include <stdio.h>
#include "misc.h" // for square
#include "cv.h"
#include "highgui.h"

#include "..\\include\\gdal.h"
#include "..\\include\\gdal_priv.h"
#include "..\\include\\ogrsf_frmts.h"

#include "..\\include\\cpl_string.h"


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
const int MAXAL=14;//the longer edge of the bounding box
const int MINAL=15;//the shorter edge of the bounding box
const int ELONG=16;//ELONGATION MAXAL/MINAL
const int COVLWR=17;//covariance matrix deduced eigenvalues ratio with the larger one as norminator
const int MEANW=18;//the mean thickness of a region 
const int CMPCT=19;//sqrt(4*area/pi)/perimeter

enum EdgeMode
{
	SUSAN=0,
	SOBEL,
	CANNY
};
enum DistMetric
{
	DISTBENZ=0,//Distance used in ecognition
	DISTLSCHD,//lambda-schedule,distance used in implementation of a fast algorithm for segmenting sar imagery
	DISTFISHER//distance used in segmenting thorascopia images
};
typedef struct HCParams
{
	float minArea;
	float maxLWR;
	float maxArea;

	float minCmpct;
	float minWid;
	HCParams():minArea(20.f),maxLWR(6.f),maxArea(2000.f),minCmpct(0.6f),minWid(4.f)
	{}
}HCParams;
typedef struct CvFFillSegment
{
    ushort y;
    ushort l;
    ushort r;
    ushort prevl;
    ushort prevr;
    short dir;
}
CvFFillSegment;

#define UP 1
#define DOWN -1 
#define ICV_PUSH( Y, L, R, PREV_L, PREV_R, DIR )\
{                                               \
    tail->y = (ushort)(Y);                      \
    tail->l = (ushort)(L);                      \
    tail->r = (ushort)(R);                      \
    tail->prevl = (ushort)(PREV_L);             \
    tail->prevr = (ushort)(PREV_R);             \
    tail->dir = (short)(DIR);                   \
    if( ++tail >= buffer_end )                  \
        tail = buffer;                          \
}


#define ICV_POP( Y, L, R, PREV_L, PREV_R, DIR ) \
{                                               \
    Y = head->y;                                \
    L = head->l;                                \
    R = head->r;                                \
    PREV_L = head->prevl;                       \
    PREV_R = head->prevr;                       \
    DIR = head->dir;                            \
    if( ++head >= buffer_end )                  \
        head = buffer;                          \
}

typedef set <_MC, less<_MC> > sl;

const float Range=(float)(1<<8);

class CHC  
{
	
sl MCL;//merge candidate list, because MCL is required for hierclust to be able to find, erase and insert element so set.

public:
	float *propData;//properties data
    int Width,Height;
//	QT *root;//the root for quadtree which is fundamental for quadtree seg,not for quadtree merge
	float * data_;//original data normalized by Delta to [0,1] and the place for work in order of bgr for rgb images, in windows format
	float *sData_;//square of data

	vector<tPair> A;// neighbor pair list
	vector<Region> S;//region set, if S[x].p==x, S[x] is a region,S[x].p is the parent index of region x according to S[]
	//in graphbased seg S has size the partition count produced by graph based seg, in MRS and QuadTree seg, S is of size W*H, 
	//thus, it's expected that the region's pixels' labels are equal to the region's first pixel's label
	vector<exRegion> exS;//store properties of comps regions in S, its exRegion.label is equal to the index of this region in S[]	
	
	float Delta;//2^11 for quickbird raw data, 2^8 for gray scale data,
	int d_;//channels of data_ for seg, default 3 channels
	int comps;// number of regions 
	int*tag;//the index of the region to which each pixel belong to in accordance with region's index in S[]
	//s[tag[x]].p==tag[x]
	vector<CRect> grid;//bounding box of each region
	int typeProp;//indicate region feature index or what region merging order is used 0 for MRS 1 for hierclust
	int propDim;//property count
	int loop;//number of cycle or number of regs
	//determine when iteration should stop in hierclust it means required region number.
	//in multi-resolution segmentation it means cycles to be performed.
	double eval;//evvaluation 
	int minsize;//least region size not to be pruned

//	float alpha;//firstly used as ratio between neighbor's internal difference 
	//and combined difference in treesegsub, then employed as edge ratio threshold.
	float K;//scale parameter for graph segmentation
	float maxDelta;//the threshold to merge pixels in quadtree

	float wc,wp;//weight for color and weight for compactness
	float mindiff;
	bool sortDM;//decide whether to use homogeneity criteria or component number to control region merging
	DistMetric metric;//the distance measure used
public:
	int StoreProp(const char*,vector<int>&symb=vector<int>(0));
    int CDThresh(float*src,float lim,BYTE*,int);
	void GetEXSLabel(vector<int>&);
	void SetPropDim(int );
	int GetRegCount();
	int GetTag(int*,CString="");
	void SetTag(int*);
	void BorderRefine();
	int GetNPLSize(int);
	int GetHeight();
	int GetWidth();
	int MergeSeg(vector<int>index1,int*tagArray1,vector<CRect>&grid1,vector<int>index2,int*tagArray2,vector<CRect>&grid2);
	int MergeSeg(int*tagArray1,int*tagArray2);
	int StoreSeg(LPCTSTR fn1,LPCTSTR fn2);
    void SaveTags(int*);
	void VisitSeq(int*,int);
	void Polygonize(CString);
	void GetPreviewMask(CRect&rect);
    void GetHistogramData(int*dataChannel,float &rmax,float &rmin);
	int GetSetSize();
	void RoadSeed(BYTE*,char*fisData=NULL);
	void RoadThin(BYTE*,vector<int>&);
	void RoadLink(BYTE*,vector<int>&);
    CRect GetBoundBox(int miss);
	int GetRegSize(int label);
	void RegMoment(int label);
	void SetWH(int, int);
	void BinMorph(LPBYTE, int ,int, int,BYTE mode, BYTE&nRepeat);
	void qttest(int md=32);
    void LenWidR();
	void KillMinion(int);
	void MRS(float hdiff);
	CHC();
	virtual ~CHC();
	void Clear();
	int MiniTag(int*indarr);
	int BenchMark(int *rect,double&,double&);
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

	void RegSimiThresh(float*,float thresh);
	int ReadSeg(LPCTSTR ,LPCTSTR);
	void CompRegSimi(float*storage);
	
	int DefReg(int*tagMat,int parts);
	int ConfirmGridSp(vector<exRegion>&,vector<CRect>&);
	int EuclidDist(float*feat1,float*feat2,int count,int dim, float* points);

	int MahalDist(float*feat1,float*feat2,int count,int dimOn, float* points,int=0);
	int GetNeighList(int order,vector<int>&nl);

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
			trek=a.sSum[i]+b.sSum[i]-square((a.addition[i]+b.addition[i])/ext)*ext;
			if(trek<0)
			{
				assert(abs(trek)<1e-2);
		/*	if(abs(trek)>=0.05f)
				{
					float m=a.sSum[i];
					m=a.addition[i];
					m=b.sSum[i];
					m=b.addition[i];
				}*/
				continue;
			}
			ret+=sqrt(trek/(ext-1));
		}
		ret*=(wc*Delta/d_);//back to normal to follow the invariant color space parctice in eCognition
		unsigned int perim=a.perim+b.perim-2*len;
		ret+=(1-wc)*wp*perim/sqrt(ext);
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
		ret+=(1-wc)*(1-wp)*perim/realb;
		ret*=ext;
		ret-=(a.interdif+b.interdif);
	}
	//fisher distance is used in "multistage graph-based segmentation of thoracoscopic images"
	//this distance is aimed at 1d intensity data
	inline void DistFisher(Region&a,Region&b,unsigned int len,float &ret)
	{
		float as=a.size,bs=b.size;
		float trek=0;
		
		trek=a.sSum[0]/as-square(a.addition[0]/as);
		trek=square(trek*trek);
		ret=b.sSum[0]/bs-square(b.addition[0]/bs);
		ret=square(ret*ret);
		ret+=trek;
		
		ret=sqrt(ret);
		trek=__min(a.perim,b.perim)/(float)len;
		trek*=abs((a.sSum[0])/as-(b.sSum[0])/bs);
		ret*=trek;
		ret*=(Delta*Delta);	//to amplify the result	
	}
	//shape similarity size1/size2*min(perim1,perim2)/common boundary 
	inline void DistShape(Region&a,Region&b,unsigned int len,float &ret)
	{
		float as=a.size,bs=b.size;
		float trek=0;
		if(as>bs)
			ret=bs/as;
		else ret=as/bs;
		ret*=__min(a.perim,b.perim)/(float)len;
	}
	//distance used in "implementation of a fast algorithm for segmenting SAR imagery", 
	//later, I added edge weight but now discarded
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
		ret*=(Delta*Delta);
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

int DitherGen(int k, int loc);

double RandIndex(int size,int*U,int R,int*V,int C,double &EI);
void FindContours(int Width,int Height, int*tag);
void GetMask(int j, int i,int *g, int Width,int Height,int*n);
void BorderEncode(int *tag,int Width, int Height);
//CvSeq* GetBoundary(int *tag, int Width, int Height,CvMemStorage* storage);
void GetProfile(float line[4],int len,int Width, vector<int>&pontiff);
double Eval(int* ts, int*ind,int numd,int rts,int rind);
double Eval(int* eu,int rts,int rind);
int VegRegCount();


void JhKMeans(float*,float*);
template <class T>
int ImportData(const char* pathName,T*label,int count)
{
	ifstream inp(pathName);
	if(!inp.is_open())
	{
		return 0;
	}
	int i,j;
	inp>>i>>j;

	assert(i*j==count);
	for(i=0;i<count;++i)
		inp>>label[i];
	inp.close();
	return 1;
}
//store data to a text file with each datum of dimension dimen, 
//make sure each column is a vector of dimen, return size of exported data
template <class T>
void Export(T *data,int dimen, int count,const char* fn)
{	
	ofstream out(fn,ios::out|ios::trunc);
	out<<dimen<<"\t"<<count<<endl;
	T*pf=data;
	for(int i=0;i<count;++i)
	{
		for(int j=0;j<dimen;++j)
		{
			out<<*pf<<"\t";
			++pf;
		}
		out<<endl;
	}
	out<<endl;
}
void GetLevelArray(CString src,vector<int>&);
void GetBWArray(CString,vector<float>&);

void ColorTransform(GDALDataset*,int*,CString,int number=0);
void ImportImg(const char*fn,IplImage*img);
int CreateSub(IplImage*src,IplImage* ,int label, int*tagArray,CRect box);
float DeducePCT(IplImage*src,int label, int*tagArray,CRect box,int thresh);
float DeduceMean(IplImage*src,int label, int*tagArray,CRect box);
void ConvertImg(IplImage*src,IplImage*dst);
bool TestHist(CvHistogram*histogram,int area);
int ReadFeat(const char *ft1, const char *ft2,float*feat1,float*feat2,int count,int dim,vector<int>&dimUse);
int MyKmeans(float*points,int dim, int*label,int count, float** estimates=NULL,int=2);
int ReadSeg(LPCTSTR fn1,LPCTSTR fn2,vector<int>&index,vector<CRect>&grid,int*tagArray);
void VRegSimi(IplImage*band1,IplImage*band2, int step, IplImage*coeff);


static double* _cv_max_element( double* start, double* end );


int Flood(int* pImage, CvSize roi, CvPoint seed,
                      int newVal, CvConnectedComp* region, int flags,
                      CvFFillSegment* buffer, int buffer_size);

//  Function cvChangeDetection performs change detection for Foreground detection algorithm
// parameters:
//      prev_frame -
//      curr_frame -
//      change_mask -
 int
cvChangeDetection( IplImage*  prev_frame,
                   IplImage*  curr_frame,
                   IplImage*  change_mask );
int P2PImgMultiply(IplImage* inp1,IplImage* inp2,IplImage* dst);
int IntegralImage(IplImage*src,IplImage*dst=NULL);
int cvWinMeanVar(IplImage* gray, int l, IplImage* winMean, IplImage* winVar);
int cvWinCorr(IplImage*,IplImage*,int l, IplImage* outcome);
IplImage*LoadGDALToIPL(const char*,int=1,int=32);
static void icvSinCos( int angle, float *cosval, float *sinval );
int BuildingCand(CHC*,HCParams *,IplImage*,IplImage*mk=NULL);
void RegionThresh(CHC*,float thresh,CRect,IplImage*);
void RegionProps(CHC*,IplImage*mask=NULL);
void VRegSimi(CHC*,IplImage*band1,IplImage*band2);
void HSRegSimi(CHC*,IplImage**bandPtr1,IplImage**bandPtr2,float=35);
void LBPCRegSimi(CHC*,IplImage**bandPtr1,IplImage**bandPtr2,float*perct,float*,float=400,int=1,int=4);

int EvalQs(CHC*,GDALDataset*,vector<float>&);
void RoadExpo(CHC*,BYTE*,vector<int>&,GDALDataset*);
void SaveSeg2(CHC*,GDALDataset *,CString pathname,int);
void SaveSeg(CHC*,GDALDataset* ,CString pathname);
void CreateChangeMask(CHC*,GDALDataset*,int*label);
int BuildData(CHC*,GDALDataset* m_pDataset1, GDALDataset* m_pDataset2, vector<float>&);	
int BuildData(CHC*,GDALDataset* ,vector<float>&);
void CompFeat(CHC*,GDALDataset *pSet,vector<float>&,float *storage,int, int,vector<int>&index);

#endif // !defined(AFX_HC_H__76B42677_678A_468C_98BC_640578E761BF__INCLUDED_)
