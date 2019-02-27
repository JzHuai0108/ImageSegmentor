#include "stdafx.h"
#include "changedetector.h"
#include "require.h"
#include "ImpExpData.h"
#include "HC.h"//for CvFFillSegment declaration AND Immerse() function
#include "..\\include\\gdal.h"
#include "..\\include\\gdal_priv.h"//for gdaldataset in savechange
#include <iostream>   
#include <fstream>
using namespace std;
int Parcel::dim=0;
ChangeDetector::ChangeDetector():tag1(NULL),tag2(NULL),buf(NULL),sMean1(NULL),sMean2(NULL),
sVar1(NULL),sVar2(NULL){
}

ChangeDetector::~ChangeDetector(){
	if(tag1){
		delete[]tag1;
		tag1=NULL;
		delete[]tag2;
		tag2=NULL;
	}
	if(sMean1){
		delete[]sMean1;
		sMean1=NULL;
		delete[]sMean2;
		sMean2=NULL;
	}
	if(sVar1){
		delete[]sVar1;
		sVar1=NULL;
		delete[]sVar2;
		sVar2=NULL;
	}
	if(buf){
		delete[]buf;
		buf=0;
	}
	regList1.clear();
	regList2.clear();
	refList1.clear();		
	refList2.clear();
}
//for each pixel in segmentation 1, floodfill with matching pixel in segmentation 2
void ChangeDetector::InitPairs(){
	int i;
	int regNum,total;
	int w=width,h=height;
	int L=w*h;
	int *tag=new int[L];

	memset(tag,-1,sizeof(int)*L);
	CRect camp;
	
	CvFFillSegment* buffer = 0;	
	int buffersize = __max( w,h)*2;
	buffer = (CvFFillSegment*)cvAlloc( buffersize*sizeof(buffer[0]));
	CvConnectedComp parcel;
	int area;
	refList1=vector<RefPair>(regList1.size());
	refList2=vector<RefPair>(regList2.size());
	//check out
	for(i=0,total=0,regNum=0;i<L;++i)//for each region indicated by index1, 
	{
		if(tag[i]!=-1)			
			continue;
		
		area=Immerse(tag1,tag2,tag,cvSize(w,h),cvPoint(i%w,i/w),regNum,&parcel,4,buffer,buffersize);
		assert(area>0&&area<=regList1[tag1[i]].size&&area<=regList2[tag2[i]].size);
		++regNum;
		if(refList1[tag1[i]].section<area){
			refList1[tag1[i]].section=area;
			refList1[tag1[i]].bestp=tag2[i];
		}
		if(refList2[tag2[i]].section<area){
			refList2[tag2[i]].section=area;
			refList2[tag2[i]].bestp=tag1[i];
		}
		total+=area;				
	}
	assert(total==L);
	cvFree( (void**)&buffer );
	delete[]tag;
}
void ChangeDetector::RadioDiff(){
	//spectral difference
}
//return number of 4 connected neighboring pixels equal to the pixel's value at pos
inline int CheckNei(int*tag,int w,int h,int pos){
	return pos-1>=0?(tag[pos-1]==tag[pos]?1:0):0+
	pos+1<w*h?(tag[pos+1]==tag[pos]?1:0):0+
	pos-w>=0?(tag[pos-w]==tag[pos]?1:0):0+
	pos+w<w*h?(tag[pos+w]==tag[pos]?1:0):0;
}
//coverage the minimum intersection requirement, gap the maximum distance between pixels 
//to become a candidate matched edge pixel
void ChangeDetector::MorphDiff(float coverage, float gap){
	int i,j,k,amount=refList1.size();
	CvMat Ma, Mb;
	int x0,y0,h0,w0,mx,my,mh,mw;
	int pos,dest,total,cand,bl,nbl;
	CRect meg;
	//compute distance of boundaries in segmentation 1 to that of segmentation 2
	for(i=0;i<amount;++i){
		if((float)refList1[i].section/regList1[i].size<coverage){
				continue;
		}

		cand=refList1[i].bestp;
		meg.UnionRect(&regList1[i].boundBox, &regList2[cand].boundBox);
		h0=meg.Height();
		w0=meg.Width();
		BYTE*patch=new BYTE[h0*w0];
		float *pool=new float[h0*w0];
		memset(patch,1,sizeof(BYTE)*h0*w0);
		cvInitMatHeader( &Ma, h0, w0, CV_8UC1, patch);
		cvInitMatHeader( &Mb, h0, w0, CV_32FC1, pool);

		x0=meg.left;
		y0=meg.top;
		mx=regList2[cand].boundBox.left;
		my=regList2[cand].boundBox.top;
		mw=regList2[cand].boundBox.Width();
		mh=regList2[cand].boundBox.Height();

		pos=my*width+mx;
		total=0;//number of pixel in a region
//		bl=0;//boundary length
		for(j=0;j<mh;++j)
		{			
			for(k=0;k<mw;++k)
			{
				if(tag2[pos]==cand)
				{
					if(CheckNei(tag2,width,height,pos)!=4){
					//	++bl;
						dest=(j+my-y0)*w0+(k+mx-x0);
						patch[dest]=0;
					}					
					++total;
				}
				++pos;
			}
			pos+=width-mw;
		}
		
		assert(total==regList2[cand].size);
	
		cvDistTransform(&Ma,&Mb,CV_DIST_L2,3);
//sum up the pixels of distance less than gap
		mx=regList1[i].boundBox.left;
		my=regList1[i].boundBox.top;
		mw=regList1[i].boundBox.Width();
		mh=regList1[i].boundBox.Height();

		pos=my*width+mx;
		total=0;
		bl=0;
		nbl=0;//matched boundry length
		for(j=0;j<mh;++j)
		{			
			for(k=0;k<mw;++k)
			{
				if(tag1[pos]==i)
				{
					if(CheckNei(tag1,width,height,pos)!=4){
						++bl;
						dest=(j+my-y0)*w0+(k+mx-x0);
						nbl+=pool[dest]<=gap?1:0;
					}
					++total;
				}
				++pos;
			}
			pos+=width-mw;
		}
		assert(nbl<=bl);
		refList1[i].gDist=(float)nbl/bl;
		assert(total==regList1[i].size);
		delete[]patch;
		delete[]pool;
	}
	amount=refList2.size();
	//compute distance of boundaries in segmentation 2 to that of segmentation 1
	for(i=0;i<amount;++i){
		if((float)refList2[i].section/regList2[i].size<coverage)
			continue;
	
		cand=refList2[i].bestp;
		meg.UnionRect(&regList2[i].boundBox, &regList1[cand].boundBox);
		h0=meg.Height();
		w0=meg.Width();
		BYTE*patch=new BYTE[h0*w0];
		float *pool=new float[h0*w0];
		memset(patch,1,sizeof(BYTE)*h0*w0);
		cvInitMatHeader( &Ma, h0, w0, CV_8UC1, patch);
		cvInitMatHeader( &Mb, h0, w0, CV_32FC1, pool);

		x0=meg.left;
		y0=meg.top;
		mx=regList1[cand].boundBox.left;
		my=regList1[cand].boundBox.top;
		mw=regList1[cand].boundBox.Width();
		mh=regList1[cand].boundBox.Height();

		pos=my*width+mx;
		total=0;//number of pixel in a region
//		bl=0;//boundary length
		for(j=0;j<mh;++j)
		{			
			for(k=0;k<mw;++k)
			{
				if(tag1[pos]==cand)
				{
					if(CheckNei(tag1,width,height,pos)!=4){
					//	++bl;
						dest=(j+my-y0)*w0+(k+mx-x0);
						patch[dest]=0;
					}					
					++total;
				}
				++pos;
			}
			pos+=width-mw;
		}
		
		assert(total==regList1[cand].size);
	
		cvDistTransform(&Ma,&Mb,CV_DIST_L2,3);
//sum up the pixels of distance less than gap
		mx=regList2[i].boundBox.left;
		my=regList2[i].boundBox.top;
		mw=regList2[i].boundBox.Width();
		mh=regList2[i].boundBox.Height();

		pos=my*width+mx;
		total=0;
		bl=0;
		nbl=0;//matched boundry length
		for(j=0;j<mh;++j)
		{			
			for(k=0;k<mw;++k)
			{
				if(tag2[pos]==i)
				{
					if(CheckNei(tag2,width,height,pos)!=4){
						++bl;
						dest=(j+my-y0)*w0+(k+mx-x0);
						nbl+=pool[dest]<=gap?1:0;
					}
					++total;
				}
				++pos;
			}
			pos+=width-mw;
		}
		assert(nbl<=bl);
		refList2[i].gDist=(float)nbl/bl;
		assert(total==regList2[i].size);
		delete[]patch;
		delete[]pool;
	}
}
//intersection minimum requirement, gthresh geometrical distance thresh
void ChangeDetector::Detect(float coverage,float gThresh){
	//for each region pair in pairList, decide the no changed one

	int i,amount=refList1.size();
	int w=width,h=height;
	CvFFillSegment* buffer = 0;	
	int buffersize = __max( w,h)*2;
	buffer = (CvFFillSegment*)cvAlloc( buffersize*sizeof(buffer[0]));
	CvConnectedComp parcel;
	int j;
	int *tag=new int[w*h];
	memset(tag,-1,sizeof(int)*w*h);
	for(i=0;i<amount;++i){
		if((float)refList1[i].section/regList1[i].size<coverage||refList1[i].gDist<gThresh)
			continue;
		j=regList1[i].pIndex;
		Immerse(tag1,tag2,tag,cvSize(w,h),cvPoint(j%w,j/w),0,&parcel,4,buffer,buffersize);
	}
	amount=refList2.size();
	for(i=0;i<amount;++i){
		if((float)refList2[i].section/regList2[i].size<coverage||refList2[i].gDist<gThresh)
			continue;
		j=regList2[i].pIndex;
		Immerse(tag1,tag2,tag,cvSize(w,h),cvPoint(j%w,j/w),0, &parcel,4,buffer,buffersize);
	}
	buf=new BYTE[w*h];
	for(i=0;i<w*h;++i)
		buf[i]=tag[i]==0?0:255;
	delete[]tag;
}
//fn is the name of the file that contains the original image data,
//dst is the filename to save the change detection results
void ChangeDetector::SaveChange(const CString&fn, const CString&dst)const {
	assure(buf,"saving void change results!");
	GDALDataset* m_pDataset=(GDALDataset *) GDALOpen(fn,GA_ReadOnly);
	GDALRasterBand  *poBand=NULL;
	GDALDataType dataType=GDT_Byte;

	const char *pszFormat = "GTiff";
	GDALDriver *poDriver;
	
	poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	
	if( poDriver == NULL)
	{
		AfxMessageBox("This format is not able to be created!");
		return;
	}
	//using create
	GDALDataset *poDstDS;       
    char **papszOptions = NULL;

 //   OGRSpatialReference oSRS;
	double adfGeoTransform[6]= { 444720, 30, 0, 3751320, 0, -30 };
	
	

	poDstDS = poDriver->Create( dst,width,height, 1,dataType, 
		papszOptions );
	if(CE_None==m_pDataset->GetGeoTransform( adfGeoTransform ))	
			poDstDS->SetGeoTransform( adfGeoTransform );

	
	const char *pszSRS_WKT=m_pDataset->GetProjectionRef();
	poDstDS->SetProjection( pszSRS_WKT );
	CPLFree( (void*)pszSRS_WKT );

	
	poBand = poDstDS->GetRasterBand(1);	
	if (poBand)
	{		
		if (CE_None!=poBand->RasterIO( GF_Write,0,0, width, height, buf, width,height,GDT_Byte, 0, 0 ))
		{
			AfxMessageBox("error write mpdataset!");
		}
	}	
	GDALClose((GDALDatasetH)m_pDataset);
	GDALClose( (GDALDatasetH) poDstDS );

}

//seg1 is the first file for segmentation 1, contains information for each region
void ChangeDetector::PrepRegList(const CString &seg1, const CString &seg2)
{
	std::ifstream inp(seg1);
	assure(inp,seg1);
	int w,h,len,dimen;
	int i,j,k,tp,bt,lf,rt;
	inp>>w>>h>>len>>dimen;
	width=w;
	height=h;
	dim=dimen;
	regList1=vector<Parcel>(len);
	sMean1=new float[len*dimen];
	sVar1=new float[len*dimen];
	for(i=0;i<len;++i)
	{
		inp>>j>>k>>lf>>tp>>rt>>bt;
		regList1[i].pIndex=j;
		regList1[i].size=k;
		regList1[i].boundBox.left=lf;
		regList1[i].boundBox.top=tp;
		regList1[i].boundBox.right=rt;
		regList1[i].boundBox.bottom=bt;
		regList1[i].sum1=sMean1+dimen*i;
		regList1[i].sum2=sVar1+dimen*i;

		for(int s=0;s<dimen;++s){
			inp>>regList1[i].sum1[s]>>regList1[i].sum2[s];
		}
	}
	inp.close();
	inp.open(seg2);
	assure(inp,seg2);
	inp>>w>>h>>len>>dimen;
	assert(width==w&&height==h&&dim==dimen);
	regList2=vector<Parcel>(len);
	sMean2=new float[len*dimen];
	sVar2=new float[len*dimen];
	for(i=0;i<len;++i)
	{
		inp>>j>>k>>lf>>tp>>rt>>bt;
		regList2[i].pIndex=j;
		regList2[i].size=k;
		regList2[i].boundBox.left=lf;
		regList2[i].boundBox.top=tp;
		regList2[i].boundBox.right=rt;
		regList2[i].boundBox.bottom=bt;
		regList2[i].sum1=sMean2+dimen*i;
		regList2[i].sum2=sVar2+dimen*i;

		for(int s=0;s<dimen;++s){
			inp>>regList2[i].sum1[s]>>regList2[i].sum2[s];
		}
	}
	inp.close();

}

void ChangeDetector::PrepLabel(const CString &tagf1, const CString &tagf2)
{
	int L=width*height,i;
	tag1=new int[L];
	require(ImportData(tagf1,tag1,L)!=0,"error reading tagmat.txt");
	for(i=0;i<regList1.size();++i){
		assert(tag1[regList1[i].pIndex]==i);
	}
	tag2=new int[L];
	require(ImportData(tagf2,tag2,L)!=0,"error reading tagmat.txt");
	for(i=0;i<regList2.size();++i){
		assert(tag2[regList2[i].pIndex]==i);
	}
}
