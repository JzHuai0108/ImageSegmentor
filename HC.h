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

#include"Region.h"
#include"_MC.h"
#include"QT.h"

#include "misc.h" // for square

#include "cv.h"
#include "highgui.h"


#include <stdio.h>
#include<assert.h> 
#include<algorithm>
#include<vector>
#include<set> 

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
const int CMPCT=19;//A/MAXAL/MINAL
const int SHADERATIO=20;
const int NEISHADERATIO=21;
const int CHANGERATIO=22;

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
	HCParams():minArea(200.f),maxLWR(5.f),maxArea(2400.f),minCmpct(0.6f),minWid(4.f)
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

typedef std::set <_MC, std::less<_MC> > sl;

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

	std::vector<tPair> A;// neighbor pair list
	std::vector<Region> S;//region set, if S[x].p==x, S[x] is a region,S[x].p is the parent index of region x according to S[]
	//in graphbased seg S has size the partition count produced by graph based seg, in MRS and QuadTree seg, S is of size W*H, 
	//thus, it's expected that the region's pixels' labels are equal to the region's first pixel's label
	std::vector<exRegion> exS;//store properties of comps regions in S, its exRegion.label is equal to the index of this region in S[]	
	
	float Delta;//2^11 for quickbird raw data, 2^8 for gray scale data,
	int d_;//channels of data_ for seg, default 3 channels
	int comps;// number of regions 
	int*tag;//the index of the region to which each pixel belong to in accordance with region's index in S[]
	//s[tag[x]].p==tag[x]
	std::vector<CRect> grid;//bounding box of each region
	int typeProp;//indicate region feature index or what region merging order is used 0 for MRS 1 for hierclust
	int propDim;//property count
	int loop;//number of cycle or number of regs
	//determine when iteration should stop in hierclust it means required region number.
	//in multi-resolution segmentation it means cycles to be performed.
	int minsize;//least region size not to be pruned

//	float alpha;//firstly used as ratio between neighbor's internal difference 
	//and combined difference in treesegsub, then employed as edge ratio threshold.
	float K;//scale parameter for graph segmentation
	float maxDelta;//the threshold to merge pixels in quadtree

	float wc,wp;//weight for color and weight for compactness
	float mindiff;
	bool sortDM;//decide whether to use homogeneity criteria or component number to control region merging
	DistMetric metric;//the distance measure used
private:
	CHC(const CHC&);
	CHC& operator=(const CHC&);
	void DistBenz(Region&a,Region&b,unsigned int len,float &ret);
	
	//fisher distance is used in "multistage graph-based segmentation of thoracoscopic images"
	//this distance is aimed at 1d intensity data
	void DistFisher(Region&a,Region&b,unsigned int len,float &ret);
	
	//shape similarity size1/size2*min(perim1,perim2)/common boundary 
	void DistShape(Region&a,Region&b,unsigned int len,float &ret);
	
	//distance used in "implementation of a fast algorithm for segmenting SAR imagery", 
	//later, I added edge weight but now discarded
	void JhHypo(Region&a,Region&b,unsigned int len,float &ret);
	
	//for the pixels indexing from xori+uly*w downwards until step.
	//inout:xori uly are the x and y coord of the upper left point to check
	//step is the height for the line of pixels,must be power of 2, rID is the tag of region 
	//which is checking neighbors,sernum is the last void A tuple serno.
	void NamNeiY(int xori,int uly, int step, int rID,int&sernum);
	
	//improved version namneiy for arbitray positive step this function intended for boundary neighbor initiation
	void NamNeiYEx(int xori,int uly, int step, int rID,int&sernum);
	
	//checking pixels from index yori*w+ulx to yori*w+lrx for region rID
	void NamNeiX(int yori,int ulx, int step, int rID, int&sernum);
	
	void NamNeiXEx(int yori,int ulx, int step, int rID, int&sernum);
	
public:
	bool PrepPropMemo(int propDim);
	int StoreProp(const char*,std::vector<int>&symb=std::vector<int>(0));
    int CDThresh(float*src,float lim,BYTE*,int);
	void GetEXSLabel(std::vector<int>&);
	void SetPropDim(int );
	int GetRegCount();
	int GetTag(int*,CString="");
	void SetTag(int*);
	void BorderRefine();
	int GetNPLSize(int);
	int GetHeight();
	int GetWidth();

	int StoreSeg(LPCTSTR fn1,LPCTSTR fn2);
    void SaveTags(int*);
	void VisitSeq(int*,int);
	void Polygonize(CString);
	void GetPreviewMask(CRect&rect);
    void GetHistogramData(int*dataChannel,float &rmax,float &rmin);
	int GetSetSize();
	void RoadSeed(BYTE*,char*fisData=NULL);
	void RoadThin(BYTE*,std::vector<int>&);
	void RoadLink(BYTE*,std::vector<int>&);
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
	void RegionProps();
	void CompRegSimi(float*storage);
	int MergeSeg(std::vector<int>&index1,int*tagArray1,std::vector<CRect>&grid1,std::vector<int>&index2,int*tagArray2,std::vector<CRect>&grid2);
	int MergeSeg(int*tagArray1,int*tagArray2);
	
	int DefReg(int*tagMat,int parts);
	int ConfirmGridSp(std::vector<exRegion>&,std::vector<CRect>&);

//	int MahalDist(float*feat1,float*feat2,int count,int dimOn, float* points,int=0);
	int GetNeighList(int order,std::vector<int>&nl);

	void InitiateRegions();
	void InitiateRegions8();

	void LamSchd(Region*a,Region*b,unsigned int len,double &ret);
	
	//remember to initiate region a.intertif before bistbenz

};



short Referee(unsigned int a,unsigned int b, unsigned int c,unsigned int d=-1);

int DitherGen(int k, int loc);

double RandIndex(int size,int*U,int R,int*V,int C,double &EI);
void FindContours(int Width,int Height, int*tag);
void GetMask(int j, int i,int *g, int Width,int Height,int*n);
void BorderEncode(int *tag,int Width, int Height);
void GetProfile(float line[4],int len,int Width, std::vector<int>&pontiff);
double Eval(int* ts, int*ind,int numd,int rts,int rind);
double Eval(int* eu,int rts,int rind);
int VegRegCount();


void JhKMeans(float*,float*);

void GetLevelArray(CString src,std::vector<int>&);
void GetBWArray(CString,std::vector<float>&);
CvSeq* GetBoundary(int *tag, int Width, int Height,CvMemStorage* storage);


void ImportImg(const char*fn,IplImage*img);
int CreateSub(IplImage*src,IplImage* ,int label, int*tagArray,CRect box);
float DeducePCT(IplImage*src,int label, int*tagArray,CRect box,int thresh);
float DeduceMean(IplImage*src,int label, int*tagArray,CRect box);
void ConvertImg(IplImage*src,IplImage*dst);
bool TestHist(CvHistogram*histogram,int area);
int MyKmeans(float*points,int dim, int*label,int count, float** estimates=NULL,int=2);
void VRegSimi(IplImage*band1,IplImage*band2, int step, IplImage*coeff);


static double* _cv_max_element( double* start, double* end );
void AdaptiveFindThreshold(CvMat *dx, CvMat *dy, double &low, double &high,double);


int Flood(int* pImage, CvSize roi, CvPoint seed,
                      int newVal, CvConnectedComp* region, int flags,
                      CvFFillSegment* buffer, int buffer_size);

//  Function cvChangeDetection performs change detection for Foreground detection algorithm
// parameters:
//      prev_frame -
//      curr_frame -
//      change_mask -
int cvChangeDetection( IplImage*  prev_frame,
                   IplImage*  curr_frame,
                   IplImage*  change_mask );
int P2PImgMultiply(IplImage* inp1,IplImage* inp2,IplImage* dst);
int IntegralImage(IplImage*src,IplImage*dst=NULL);
int cvWinMeanVar(IplImage* gray, int l, IplImage* winMean, IplImage* winVar);
int cvWinCorr(IplImage*,IplImage*,int l, IplImage* outcome);
IplImage*LoadGDALToIPL(const char*,int=1,int=32);
static void icvSinCos( int angle, float *cosval, float *sinval );
float JHLineAngle(CvPoint pt1, CvPoint pt2, CvPoint pt3);//calculate the cosine of the angle formed by 
//line pt1 to pt2 and line pt2 to pt3


int BuildingCand(CHC*PHC,HCParams *params,IplImage*portal);
int BuildingCand2(CHC&myHC,HCParams *params,IplImage*portal);
void RegionThresh(CHC&,float thresh,CRect,IplImage*);
void ShadeRatio(CHC&myHC, IplImage*shade,bool);
void VRegSimi(CHC*,IplImage*band1,IplImage*band2);
void HSRegSimi(CHC*,IplImage**bandPtr1,IplImage**bandPtr2,float=35);

void ColorTransform(int*,CString,int number=0);
float EvalQs(const CHC&,const CString&,std::vector<float>&);
void RoadExpo(CHC*,BYTE*,std::vector<int>&,const char*);
void SaveSeg2(CHC&,const CString&,const int);
void SaveSeg(const CHC&,const CString&, const CString&);
int BuildData(CHC&,const CString&,const CString&,std::vector<float>&);	
int BuildData(CHC&,const CString&,std::vector<float>&);
//find the connected area (decided by flags) in pImage of value the same as location seed and store the area marked with newval
//in pMask, the two image both of size roi and one channel
template<class T>
int Immerse(T* pImage1,T* pImage2, T*pMask, CvSize roi, CvPoint seed,
                      T newVal, CvConnectedComp* region, int flags,
                      CvFFillSegment* buffer, int buffer_size)
{
	int step=roi.width;
    T* img1 = pImage1 + step * seed.y;
	T* img2 = pImage2 + step * seed.y;
	T* msk= pMask+ step * seed.y;
    int i, L, R; 
    int area = 0;
    int val0[2]={0,0};

    int XMin, XMax, YMin = seed.y, YMax = seed.y;
    int _8_connectivity = (flags & 255) == 8;
    CvFFillSegment* buffer_end = buffer + buffer_size, *head = buffer, *tail = buffer;
	
    L = R = XMin = XMax = seed.x;
	
	
	val0[0] = img1[L];
	val0[1] = img2[L];

	msk[L] = newVal;
	
	while( ++R < roi.width && img1[R] == val0[0]&& img2[R] == val0[1]&& msk[R]== -1 )
		
		msk[R] = newVal;
	
	
	while( --L >= 0 && img1[L] == val0[0]&& img2[L] == val0[1]&& msk[L]== -1 )
	
	
		msk[L] = newVal;
	 
	
    XMax = --R;
    XMin = ++L;
    ICV_PUSH( seed.y, L, R, R + 1, R, UP );
	
    while( head != tail )
    {
        int k, YC, PL, PR, dir;
        ICV_POP( YC, L, R, PL, PR, dir );
		
        int data[][3] =
        {
            {-dir, L - _8_connectivity, R + _8_connectivity},
            {dir, L - _8_connectivity, PL - 1},
            {dir, PR + 1, R + _8_connectivity}
        };
		
        if( region )
        {
            area += R - L + 1;
			
            if( XMax < R ) XMax = R;
            if( XMin > L ) XMin = L;
            if( YMax < YC ) YMax = YC;
            if( YMin > YC ) YMin = YC;
        }
		
        for( k = 0/*(unsigned)(YC - dir) >= (unsigned)roi.height*/; k < 3; k++ )
        {
            dir = data[k][0];
            img1 = pImage1 + (YC + dir) * step;
            img2 = pImage2 + (YC + dir) * step;
			msk = pMask + (YC + dir) * step;
            int left = data[k][1];
            int right = data[k][2];
			
            if( (unsigned)(YC + dir) >= (unsigned)roi.height )
                continue;
			
            
			for( i = left; i <= right; i++ )
			{
				if( (unsigned)i < (unsigned)roi.width &&img1[i] == val0[0]&& img2[i] == val0[1]&& msk[i]== -1 )
				{
					int j = i;
			
					msk[i] = newVal;
					while( --j >= 0 && img1[j] == val0[0]&& img2[j] == val0[1]&& msk[j]== -1 )
					
						msk[j] = newVal;
										
					while( ++i < roi.width && img1[i] == val0[0]&& img2[i] == val0[1]&& msk[i]== -1 )
				
						msk[i] = newVal;
					
					
					ICV_PUSH( YC + dir, j+1, i-1, L, R, -dir );
				}
			}
			
        }
    }
	
    if( region )
    {
        region->area = area;
        region->rect.x = XMin;
        region->rect.y = YMin;
        region->rect.width = XMax - XMin + 1;
        region->rect.height = YMax - YMin + 1;
     //   region->value = newVal;
    }
	
    return area;
}
#endif // !defined(AFX_HC_H__76B42677_678A_468C_98BC_640578E761BF__INCLUDED_)
