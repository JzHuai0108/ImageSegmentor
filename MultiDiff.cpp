#include"stdafx.h"
#include"MultiDiff.h"
#include"ImpExpData.h"
#include"require.h"

#include"HC.h"
#include "..\\include\\gdal.h"
#include "..\\include\\gdal_priv.h"
using namespace std;

const float Delta=256.f;
//read seg result text file 
//fn1 composed of width, height, reg count, region number regsize and rectangle
//fn2 made up of pixel label matrix 
int ReadSeg(LPCTSTR fn1,LPCTSTR fn2,vector<CRect>&grid,int*tagArray,vector<int>&index)
{
	std::ifstream inp(fn1);
	assure(inp,fn1);
	int w,h,len,dimen;
	int i,j,k,tp,bt,lf,rt;
	float avg,var;
	inp>>w>>h>>len>>dimen;
	bool allocated=index.size()>0;
	for(i=0;i<len;++i)
	{
		inp>>j>>k>>lf>>tp>>rt>>bt;
		if(allocated)
			index[i]=j;
		grid[i].left=lf;
		grid[i].top=tp;
		grid[i].right=rt;
		grid[i].bottom=bt;
		for(int s=0;s<dimen;++s){
			inp>>avg>>var;
		}
	}
	inp.close();

	if(!ImportData(fn2,tagArray,w*h))
	{
		AfxMessageBox("error reading tagmat.txt");
		return 0;
	}
	if(index.size()){
	for(i=0;i<len;++i){
		assert(tagArray[index[i]]==i);
	}
	}
	return len;
}

//compute feature vector for one level of an image including mean and stddev
//bwArray decides whether a band is considered
void MultiDiff::CompFeat(const CString& fn,vector<float>&bWArray,float*storage,int curLev)
{
	GDALDataset*pSet=(GDALDataset *) GDALOpen(fn,GA_ReadOnly);
	if(!pSet){
		AfxMessageBox("cannot open image for MultiDiff!");
		return;
	}	
	int i,j,k,regNum0=index0.size();
	int s,t,x0,y0,sernum1,sernum2;
	
	int curIndex;
	int L=regCount[2*storey+curLev];
	assert(grid.size()==L);
	float*buf=new float[height*width];
	float*storagek=new float[L*2*bandCount];
	
	float diff,temp; 
	float sigma;
	int area,total=0;
	GDALRasterBand  *m_pBand=NULL;	
	
	for(j=1,curIndex=0;j<=bWArray.size();++j)
	{
		if(bWArray[j-1]==0)
			continue;
		m_pBand= pSet->GetRasterBand(j);
		m_pBand->RasterIO( GF_Read,0,0, width,height, buf, width,height, GDT_Float32, 0, 0 );
		total=0;
		for(i=0;i<L;++i)
		{		
			y0=grid[i].bottom-grid[i].top;
			x0=grid[i].right-grid[i].left;
			sernum1=grid[i].top*width+grid[i].left;
			diff=0;
			sigma=0;
			area=0;
			for(s=0;s<y0;++s)
			{
				for(t=0;t<x0;++t)
				{
					if(tag[sernum1]==i)
					{
						temp=buf[sernum1]/Delta;
						diff+=temp;
						sigma+=(temp*temp);
						++area;
					}
					++sernum1;
				}
				sernum1+=(width-x0);
			}
			assert(sernum1==(grid[i].bottom*width+grid[i].left));
			if(area==1)
				sigma=0;
			else
			{
				sigma=(sigma-diff*diff/area)/(area-1);
				sigma=sqrt(abs(sigma));
			}
			total+=area;
			diff/=area;
			area=(i*bandCount+curIndex)*2;
			storagek[area]=diff;
			storagek[area+1]=sigma;
		}
		assert(total==width*height);
		++curIndex;
	}
	assert(curIndex==bandCount);
	for(j=0;j<regNum0;++j)
	{
		for(k=0;k<bandCount;++k)
		{
			sernum1=((j*storey+curLev)*bandCount+k)*2;
			sernum2=tag[index0[j]];//this line works out right with MRS and QTHC but not GSHC
			sernum2=(sernum2*bandCount+k)*2;
			storage[sernum1]=storagek[sernum2];
			storage[sernum1+1]=storagek[sernum2+1];
		}
	}
	delete[]buf;
	delete[]storagek;
	GDALClose((GDALDatasetH)pSet);
}


MultiDiff::MultiDiff(const CString&path,const SynSegWay&way):dirName(path),method(way),tag(NULL),
storey(0),regCount(0),width(0),height(0),bandCount(0),
mindiff(0.f),maxdiff(0.f),buf(NULL){
}
MultiDiff::~MultiDiff(){
	if(index0.size()){
		index0.clear();
		delete[]tag;
		tag=NULL;
		delete[]regCount;
		regCount=0;
		delete[]buf;
		buf=0;
	}
	grid.clear();
	char bale[100]={0};
	char bulk[100]={0};
	float curdiff;
	for(int i=0;i<storey;++i)
	{	
		if(storey<=1) curdiff=mindiff;
		else	curdiff=mindiff+(maxdiff-mindiff)/(storey-1)*i;
		sprintf(bale,"%smerge-rect-%.0f.txt",dirName,curdiff);
		sprintf(bulk,"%smerge-tagmat-%.0f.txt",dirName,curdiff);
		remove(bale);
		remove(bulk);
	}
	sprintf(bale,"%sfeat1.txt",dirName);
	sprintf(bulk,"%sfeat2.txt",dirName);
	remove(bale);
	remove(bulk);
}

//initiate bandCount and bWArray if necessary
//synchronic multilevel image segmnetation, method=0 for intersection merge with connected region mergeseg with tagArray
//method=1 for intersection merge with not connected region by mergeseg(index)
//method=2 for simutaneously segmentation
int MultiDiff::SynMultiSeg(const CString& fn1, const CString& fn2,vector<float>&bWArray)
{
	char bale[100]={0};
	char bulk[100]={0};
	int i;
	CHC m_HC;
	float curdiff;

	if(method==SimuSeg)
	{

		BuildData(m_HC,fn1,fn2,bWArray);//builddata may affect bwarray
		if(bandCount==0)
			for(i=0;i<bWArray.size();++i)
				bandCount+=bWArray[i]>0?1:0;

		width=m_HC.GetWidth();
		height=m_HC.GetHeight();
		m_HC.InitiateRegionSet();
		for(i=0;i<storey;++i)
		{
			if(storey==1) curdiff=mindiff;
			else	curdiff=mindiff+(maxdiff-mindiff)/(storey-1)*i;
			m_HC.MRS(curdiff);
			m_HC.RegionLabel();
			sprintf(bale,"%smerge-rect-%.0f.txt",dirName,curdiff);
			sprintf(bulk,"%smerge-tagmat-%.0f.txt",dirName,curdiff);
			regCount[i]= m_HC.StoreSeg(bale,bulk);
			regCount[i+storey]=regCount[i];
			regCount[i+storey*2]=regCount[i];
		}
		m_HC.Clear();

		return 1;
	}

	BuildData(m_HC,fn1,bWArray);//builddata can affect the values of bwarray
	if(bandCount==0)
		for(i=0;i<bWArray.size();++i)
			bandCount+=bWArray[i]>0?1:0;
	width=m_HC.GetWidth();
	height=m_HC.GetHeight();
	m_HC.InitiateRegionSet();

	for(i=0;i<storey;++i)
	{
		if(storey==1) curdiff=mindiff;
		else	curdiff=mindiff+(maxdiff-mindiff)/(storey-1)*i;
		m_HC.MRS(curdiff);
		m_HC.RegionLabel();
		sprintf(bale,"%st1-rect-%.0f.txt",dirName,curdiff);
		sprintf(bulk,"%st1-tagmat-%.0f.txt",dirName,curdiff);
		regCount[i]= m_HC.StoreSeg(bale,bulk);
	}
	m_HC.Clear();

//segment image t2

	BuildData(m_HC,fn2,bWArray);

	assert(	width==m_HC.GetWidth()&&height==m_HC.GetHeight());
	m_HC.InitiateRegionSet();

	for(i=0;i<storey;++i)
	{
		if(storey==1) curdiff=mindiff;
		else	curdiff=mindiff+(maxdiff-mindiff)/(storey-1)*i;
		m_HC.MRS(curdiff);
		m_HC.RegionLabel();
	
		sprintf(bale,"%st2-rect-%.0f.txt",dirName,curdiff);
		sprintf(bulk,"%st2-tagmat-%.0f.txt",dirName,curdiff);
		regCount[i+storey]= m_HC.StoreSeg(bale,bulk);
	}

//merge segmenation
	int* tagArray1=new int[width*height];
	int* tagArray2=new int[width*height];
	vector<int> index1,index2;
	vector<CRect>grid1,grid2;
	for(i=0;i<storey;++i)
	{	
		m_HC.Clear();
		if(index1.size())
		{
			grid1.clear();
			index1.clear();
		}
		if(storey<=1) curdiff=mindiff;
		else	curdiff=mindiff+(maxdiff-mindiff)/(storey-1)*i;
		grid1=vector<CRect>(regCount[i]);	
		index1=vector<int>(regCount[i]);
		sprintf(bale,"%st1-rect-%.0f.txt",dirName,curdiff);
		sprintf(bulk,"%st1-tagmat-%.0f.txt",dirName,curdiff);
		ReadSeg(bale,bulk,grid1,tagArray1,index1);

		if(index2.size())
		{
			grid2.clear();
			index2.clear();
		}
		grid2=vector<CRect>(regCount[i+storey]);
		index2=vector<int>(regCount[i+storey]);
		sprintf(bale,"%st2-rect-%.0f.txt",dirName,curdiff);
		sprintf(bulk,"%st2-tagmat-%.0f.txt",dirName,curdiff);
		ReadSeg(bale,bulk,grid2,tagArray2,index2);
	
		sprintf(bale,"%smerge-rect-%.0f.txt",dirName,curdiff);
		sprintf(bulk,"%smerge-tagmat-%.0f.txt",dirName,curdiff);
		if(method==DiscreteMerge)
			m_HC.MergeSeg(index1,tagArray1,grid1,index2,tagArray2,grid2);
		else
			m_HC.MergeSeg(tagArray1,tagArray2);
		regCount[i+2*storey]=m_HC.StoreSeg(bale,bulk);
		sprintf(bale,"%st1-rect-%.0f.txt",dirName,curdiff);
		sprintf(bulk,"%st1-tagmat-%.0f.txt",dirName,curdiff);
		remove(bale);
		remove(bulk);
		sprintf(bale,"%st2-rect-%.0f.txt",dirName,curdiff);
		sprintf(bulk,"%st2-tagmat-%.0f.txt",dirName,curdiff);
		remove(bale);
		remove(bulk);
	}
	grid1.clear();
	grid2.clear();
	index1.clear();
	index2.clear();

	delete []tagArray1;
	tagArray1=NULL;
	delete []tagArray2;
	tagArray2=NULL;
	return 1;
}

void MultiDiff::Init(int l, float min, float max){

	storey=l;
	maxdiff=max;
	mindiff=min;
	if(max==min) storey=1;
	if(regCount) delete[]regCount;
	regCount=new int[storey*3];//for 3 seg hierarchy, first, second, merged
	
}
//compute mean and variance for each parcel in a segmentation, a quicker way exist for 
//SimuSeg, wanting to be implemented. note there is a nuance(thousandth) between the variance obtained
//by Storeseg and CompFeat for SimuSeg.
void MultiDiff::EvalFeat(const char *dataset, const char *dest, vector<float>&bWArray)
{
//compute region signitures and similarity 
	//COMPUTE features for dataset1
	//for the first level


	int i,temp;
	int regNum0=regCount[2*storey];
	
	if(index0.size()!=regNum0){
		index0.clear();
		index0=vector<int>(regNum0);
	}
	float*storage=new float[regNum0*storey*bandCount*2];
	char bale[100]={0};
	char bulk[100]={0};
	float curdiff;
	for(i=0;i<storey;++i)
	{	
		if(storey<=1) curdiff=mindiff;
		else	curdiff=mindiff+(maxdiff-mindiff)/(storey-1)*i;
		sprintf(bale,"%smerge-rect-%.0f.txt",dirName,curdiff);
		sprintf(bulk,"%smerge-tagmat-%.0f.txt",dirName,curdiff);
		Reallocate(regCount[2*storey+i]);
		if(i==0)
			temp=ReadSeg(bale,bulk,grid,tag,index0);
		else
			temp=ReadSeg(bale,bulk,grid,tag);
		assert(temp==regCount[2*storey+i]);
		CompFeat(dataset,bWArray,storage,i);
	}

	Export(storage,storey*bandCount*2,regNum0,dest);
	delete[]storage;

}


void MultiDiff::Reallocate(int count)
{
	grid.clear();
	grid=vector<CRect>(count);
	if(!tag)
	{
		tag=new int[width*height];
	}
}

void MultiDiff::CalcDist(vector<int>&levelUse,float*ptArray) {
	char bale[100]={0};
	char bulk[100]={0};
	sprintf(bale,"%s\\merge-rect-%.0f.txt",dirName,mindiff);
	sprintf(bulk,"%s\\merge-tagmat-%.0f.txt",dirName,mindiff);
	int count=regCount[2*storey];
	Reallocate(count);
	int temp=ReadSeg(bale,bulk,grid,tag);
	assert(temp==count);

	int dim0=storey*bandCount*2;
	if(levelUse.size()==0)
	{
		for(int i=0;i<storey;++i)
			levelUse.push_back(i);
	}

	int dim=bandCount*levelUse.size();
	vector<int> dimUse;
	for(int i=0;i<levelUse.size();++i)
		for(int j=0;j<bandCount;++j)
		{
			if(levelUse[i]<storey)
			dimUse.push_back(levelUse[i]*bandCount*2+j*2);
		}
	sprintf(bale,"%sfeat1.txt",dirName);
	sprintf(bulk,"%sfeat2.txt",dirName);
	float*	feat1=new float[dim*count*2];
	float*	feat2=&feat1[dim*count];

	ReadFeat(bale,bulk,feat1,feat2,count,dim0,dimUse);
//	if(method==2)
//		MahalDist(feat1,feat2,count,dim,pts,1);//deprecated
//	else
	EuclidDist(feat1,feat2,count,dim,ptArray);
	delete []feat1;
	dimUse.clear();
}
//create change mask label is the label of value change or unchanged for each region 
//in SF0, produced by exterior classifier, tagC0 is the tag array for each pixel with value of range [0,S0.size-1]
//label is array of change indicator 0 for change, count is size of label array
void MultiDiff::CreateChangeMask(int*label)
{
	int L=grid.size(),ink=0;
	int sernum,x0,y0,i,j,k;
	if(!buf) buf=new BYTE[width*height];
	for(i=0;i<width*height;++i)
		buf[i]=255;	
	for( i=0;i<L;++i)
	{
	//	if(SF0[i].isChecked==false)
	//		continue;
		x0=grid[i].right-grid[i].left;
		y0=grid[i].bottom-grid[i].top;
		sernum=grid[i].top*width+grid[i].left;
		for(j=0;j<y0;++j)
		{
			for(k=0;k<x0;++k)
			{
				if(tag[sernum]==i)
				{				
					buf[sernum]=label[i]?255:0;
					++ink;
				}
				++sernum;
			}
			sernum+=(width-x0);
		}

	}
	assert(ink==width*height);
}
//fn is the name of the file that contains the original image data,
//dst is the filename to save the change detection results
void MultiDiff::SaveChange(const CString&fn, const CString&dst)const {
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
//compute euclidean distance between each vector stored in feat1 and feat2 of count data and dimOn dimension
//output distance is recorded in points of count size, note the momory of feat1 and feat2 are connected
int MultiDiff::EuclidDist(float*feat1,float*feat2,const int count,const int dim, float* points) const
{
	int i;
	CvMat vec1,vec2;
	
	for(i=0;i<count;++i)
	{
		cvInitMatHeader(&vec1,1,dim,CV_32FC1,(feat1+i*dim));
		cvInitMatHeader(&vec2,1,dim,CV_32FC1,(feat2+i*dim));
		points[i]=cvNorm(&vec1,&vec2,CV_L2,NULL);		
	}
	return 1;		
}

//READ FEATURE FROM file ft1 and ft2 to feat1 and feat2 which is connected to feat1, 
//each file has count rows, dim columns, dimUse indicates which column to import
int ReadFeat(const char *ft1, const char *ft2,float*feat1,float*feat2,int count,int dim,vector<int>&dimUse)
{
	//read in feat1 and feat2 and select dimen to use

	float*feat=new float[dim*count];
	if(!ImportData(ft1,feat,dim*count))
	{
		AfxMessageBox("error reading feat1.txt");
		return 0;
	}
	int dimOn=__min(dim,dimUse.size());

	int i,j,seqnum,subspt;

	for(i=0;i<count;++i)
	{
		seqnum=i*dimOn;
		subspt=i*dim;
		for(j=0;j<dimOn;++j)
		{	
			feat1[seqnum+j]=feat[subspt+dimUse[j]];
		}
	}
	
	if(!ImportData(ft2,feat,dim*count))
	{
		AfxMessageBox("error reading feat2.txt");
		return 0;
	}
	for(i=0;i<count;++i)
	{	
		seqnum=i*dimOn;
		subspt=i*dim;
		for(j=0;j<dimOn;++j)
		{		
			feat2[seqnum+j]=feat[subspt+dimUse[j]];
		}
	}
	delete []feat;
	return 1;
}
