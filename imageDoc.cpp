/**
* This file is part of ImageSegmentor.
*
* Copyright (C) 2012 Jianzhu Huai <huai dot 3 at osu dot edu> (The Ohio State University)
*
* ImageSegmentor is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ImageSegmentor is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ImageSegmentor. If not, see <http://www.gnu.org/licenses/>.
*/

// imageDoc.cpp : implementation of the CImageDoc class
//

#include "stdafx.h"
#include "image.h"


#include "timeshow.h"
#include "InitiateDlg.h"
#include "BandSelDlg.h"
#include "SUSANDlg.h"
#include "CannyPara.h"
#include "imageDoc.h"
#include "ResultDlg.h"
//#include "Shlwapi.h"
#include "PreviewDlg.h"
#include "SelPropDlg.h"
#include "MFeatDlg.h"
#include "MorphDlg.h"
#include "SetPropDlg.h"
#include "mygabor.h"
#include "THRESHDlg.h"

#include "mytexture.h"
#include "MultiDiff.h"
#include "ChangeDetector.h"
#include "GlobalApi.h"
#include "mrf_1.h"
#include "ColorMRF.h"
#include "MRFOptimDlg.h"//mean shift算法结果图像
#include "ImpExpData.h"
#include "..\\include\\gdal.h"
#include "..\\include\\gdal_priv.h"//
#include "..\\include\\ogrsf_frmts.h"//ogrregisterall()
using namespace std;
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
extern char szFilter[];
/////////////////////////////////////////////////////////////////////////////
// CImageDoc

IMPLEMENT_DYNCREATE(CImageDoc, CDocument)

BEGIN_MESSAGE_MAP(CImageDoc, CDocument)
	//{{AFX_MSG_MAP(CImageDoc)
	ON_COMMAND(ID_PROCESS_HISTOEQUAL, OnProcessHistoequal)
	ON_COMMAND(ID_SEG_INITIATE, OnSegInitiate)
	ON_COMMAND(ID_VIEW_OUTCOME, OnViewOutcome)
	ON_COMMAND(ID_PROCESS_GUASS, OnProcessGuass)
	ON_COMMAND(ID_VIEW_ZOOMIN, OnViewZoomin)
	ON_COMMAND(ID_VIEW_ORIGIN, OnViewOrigin)
	ON_COMMAND(ID_VIEW_ZOOMOUT, OnViewZoomout)
	ON_COMMAND(ID_EVAL_QS, OnEvalQs)
	ON_COMMAND(ID_SEG_QTHC, OnSegQthc)
	ON_COMMAND(ID_SEG_GSHC, OnSegGshc)
	ON_COMMAND(ID_SEG_GS, OnSegGraph)
	ON_COMMAND(ID_CLASS_ROADCLUMP, OnClassRoadclump)
	ON_COMMAND(ID_PREP_SR, OnPrepSr)
	ON_COMMAND(ID_CLASS_ERODE, OnClassErode)
	ON_COMMAND(ID_SEG_SAVE, OnSegSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(ID_SEG_QT, OnSegQuadTree)
	ON_COMMAND(ID_PREP_SOBEL, OnPrepSobel)
	ON_COMMAND(ID_PREP_CANNY, OnPrepCanny)
	ON_COMMAND(ID_PREP_SUSAN, OnPrepSusan)
	ON_COMMAND(ID_PREP_MORPH, OnPrepMorph)
	ON_COMMAND(ID_SEG_REGMEG, OnSegRegMeg)
	ON_COMMAND(ID_CLASS_PREVIEW, OnClassPreview)
	ON_COMMAND(ID_SEG_SAVETOUR, OnSegSavetour)
	ON_COMMAND(ID_SEG_EXPORTSHP, OnSegExportshp)
	ON_COMMAND(ID_CLASS_MULTIFEATURE, OnMultiFeatureDiff)
	ON_COMMAND(ID_PREP_TEXTIMG, OnPrepTextimg)
	ON_COMMAND(ID_SEG_RANDINDEX, OnSegRandIndex)
	ON_COMMAND(ID_PREP_SATHUE, OnPrepSatHue)
	ON_COMMAND(ID_CLASS_BUILDING, OnClassBuilding)
	ON_COMMAND(ID_PREP_GABORTRANS, OnPrepGaborTransform)
	ON_COMMAND(ID_SEG_MEANSHIFTDZJ, OnSegMeanShiftDzj)
	ON_COMMAND(ID_PREP_ENTROPY, OnPrepEntropy)
	ON_COMMAND(ID_SEG_WATERSHEDV, OnSegWatershedVincent)
	ON_COMMAND(ID_SEG_WATERSHEDG, OnSegWatershedG)
	ON_COMMAND(ID_SEG_WATERSHEDSEQ, OnSegWatershedInver)
	ON_COMMAND(ID_SEG_PYRMEANSHIFT, OnSegPyrMeanShift)
	ON_COMMAND(ID_CLASS_MRFCD, OnClassMRFCD)
	ON_COMMAND(ID_CLASS_COLORMRF, OnClassColorMRF)
	ON_COMMAND(ID_CLASS_KMEANS, OnClassKmeans)
	ON_COMMAND(ID_TEXTURE_ROSINCD, OnTextureRosinCD)
	ON_COMMAND(ID_TEXTURE_CORRBINARY, OnTextureCorrBinary)
	ON_COMMAND(ID_TEXTURE_GRADCORR, OnTextureGradCorr)
	ON_COMMAND(ID_PREP_OPENING, OnPrepOpening)
	ON_COMMAND(ID_TEXTURE_HISTOSTATCD, OnTextureHistoStatCD)
	ON_COMMAND(ID_CLASS_BUILDINGISODATA, OnClassBuildingIsodata)
	ON_COMMAND(ID_CLASS_MULTIBUILD, OnClassMultiBuild)
	ON_COMMAND(ID_TEXTURE_GEOMETRICCD, OnTextureGeometricCD)
	//}}AFX_MSG_MAP

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageDoc construction/destruction

CImageDoc::CImageDoc():m_dRoom(1),quantum(0),
curRegion(-1),edgeMap(NULL),storey(0)
{
	// TODO: add one-time construction code here
/*	while(!m_strokeList.IsEmpty())
	{
		delete m_strokeList.RemoveHead();
	}*/
//	current.x=0;
//	current.y=0;
//	m_penCur.CreatePen(PS_SOLID,m_nPenWidth,RGB(0,0,0));
	
//	viewRegion=(m_HC.Blocks.size()!=0);
	GDALAllRegister();
	OGRRegisterAll();
	resultDlg.Create(IDD_DIALOG_RESULTIMAGE);
}

CImageDoc::~CImageDoc()
{
//	AfxMessageBox("IN imagedoc!");

	if(edgeMap)
	{
		delete []edgeMap;
		edgeMap=NULL;
	}
	bWArray.clear();
	levelUse.clear();

}

/////////////////////////////////////////////////////////////////////////////
// CImageDoc serialization

void CImageDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
	//	CFile* cfptr=ar.GetFile();
	

	}
	else
	{
		// TODO: add loading code here
		CFile* cfptr=ar.GetFile();
		CString FileName=cfptr->GetFilePath();
		BOOL res=false;
		
			int r=1,g=2,b=3;
			CBandSelDlg IDlg; 
			if(!ImageProps(IDlg.m_Band,IDlg.m_DataType,FileName))
				return;
			IDlg.m_R=1;
			IDlg.m_G=2;
			IDlg.m_B=3;
	//		IDlg.m_NDVI=4;
	//		IDlg.m_EB=4;
			if(IDlg.DoModal()==IDOK) 
			{	
				r=IDlg.m_R;
				g=IDlg.m_G;
				b=IDlg.m_B;
			//	v=IDlg.m_NDVI;
				edgeBand=IDlg.m_EB;
			}
			else return;
			res=m_DIB.CreateDIB(r,g,b,FileName);		
	}
}

/////////////////////////////////////////////////////////////////////////////
// CImageDoc diagnostics

#ifdef _DEBUG
void CImageDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CImageDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CImageDoc commands 


BOOL CImageDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;	
	return TRUE;
}



void CImageDoc::OnProcessHistoequal() 
{
	// TODO: Add your command handler code here	
	// 调用InteEqualize()函数进行直方图均衡
	m_DIB.InteEqualize();		
	  SetModifiedFlag(FALSE);
	  UpdateAllViews(NULL);
}

void CImageDoc::OnSegInitiate() 
{
	// TODO: Add your command handler code here
	CInitiateDlg IDlg; 
	IDlg.m_nMinsize=m_HC.minsize;
	IDlg.m_nKL=m_HC.K;
	IDlg.m_SpeCoeff=m_HC.wc;
	IDlg.m_PacCoeff=m_HC.wp;
//	IDlg.m_n3by3=0;
	IDlg.m_nLamda=m_HC.maxDelta;
	IDlg.m_nLoop=m_HC.loop;
	IDlg.m_Scale=m_HC.mindiff;

	if(IDlg.DoModal()==IDOK) 
	{
		m_HC.sortDM=!IDlg.m_HM;
		m_HC.mindiff=IDlg.m_Scale;
		m_HC.K=IDlg.m_nKL;
		m_HC.minsize=IDlg.m_nMinsize;
		m_HC.wc=IDlg.m_SpeCoeff;
		m_HC.wp=IDlg.m_PacCoeff;
		//	m_HC.matrix3by3=IDlg.m_n3by3;
		m_HC.maxDelta=IDlg.m_nLamda;
		m_HC.loop=IDlg.m_nLoop;
		m_HC.metric=(DistMetric)IDlg.iPos;
		GetBWArray(IDlg.m_BWArray,bWArray);

	}
}
void CImageDoc::OnViewOutcome() 
{
	// TODO: Add your command handler code here
	CResultDlg outDlg;
	outDlg.m_nComps=m_HC.GetRegCount();
	outDlg.m_nTime=quantum;
//	uint32 en=m_HC.countedge();
	outDlg.m_nQS=eval;
	outDlg.DoModal();	 
}


void CImageDoc::OnProcessGuass() 
{
	// TODO: Add your command handler code here
	StartTimer(); 
	int L=m_DIB.GetLength();
	BYTE*EM=new BYTE[L];
	GetEM(GetPathName(),EM,1);
	int w=m_DIB.GetWidth(),h=m_DIB.GetHeight();
//	m_HC.SetWH(w,h);

	GaussianFilter(EM,m_DIB.GetWidth(),m_DIB.GetHeight(),0.8f);
	m_DIB.CreateDIBFromBits(w,h,EM,8);
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();
	pFrame->pImageView->Invalidate(FALSE);	
//	m_DIB.SaveEdge("GaussianSmooth.bmp",EM);
	delete []EM;
	quantum=ElapsedTime();  	
}


void CImageDoc::OnViewZoomin() 
{
	// TODO: Add your command handler code here
	m_dRoom*=1.2;
	SetModifiedFlag(FALSE);
	UpdateAllViews(NULL);
}

void CImageDoc::OnViewOrigin() 
{
	// TODO: Add your command handler code here
	m_dRoom=1;
/*	if(IsKindOf(&CObject::classCObject))
		afxDump<<GetRuntimeClass()->m_lpszClassName<<"is CObject!";
	else
	AfxMessageBox("Wrong!");*/

	m_DIB.Origin();
	SetModifiedFlag(FALSE);
	UpdateAllViews(NULL);
}

void CImageDoc::OnViewZoomout() 
{
	// TODO: Add your command handler code here
	m_dRoom*=0.8;
	SetModifiedFlag(FALSE);
	UpdateAllViews(NULL);
}

void CImageDoc::OnEvalQs() 
{
	// TODO: Add your command handler code here
	eval=EvalQs(m_HC,GetPathName(),bWArray);
}

void CImageDoc::OnSegRegMeg() 
{
/*	int argc=6;	
	char *xx[10]={"mfams","80","80","500","-j","4"};

	int x=m_HC.MFAMS(argc,xx);	
	if(x==1)
		AfxMessageBox("sucks!");*/
	StartTimer(); 	
	SetEM();//add edge map
	BuildData(m_HC,GetPathName(),bWArray);
	m_HC.InitiateRegionSet();

	if(m_HC.sortDM)
	{
		m_HC.MRS(m_HC.mindiff);
	}
	else
	m_HC.HierClust();
	quantum=ElapsedTime();  	
	m_HC.RegionLabel();

	eval=EvalQs(m_HC,GetPathName(),bWArray);

	int len=m_HC.GetWidth()*m_HC.GetHeight();
	int*tagArr=new int[len];
	m_HC.GetTag(tagArr);
	m_DIB.Origin();
	m_DIB.LookRegions(tagArr);
	SetModifiedFlag(FALSE);
	UpdateAllViews(NULL);
	delete[]tagArr;
}

void CImageDoc::OnSegQthc() 
{
	// TODO: Add your command handler code here
	StartTimer(); 
	SetEM();//add edge map

	BuildData(m_HC,GetPathName(),bWArray);
	m_HC.QTMerge();

	if(m_HC.sortDM)
	{
		m_HC.MRS(m_HC.mindiff);
	}
	else
	m_HC.HierClust();

	quantum=ElapsedTime();  
	m_HC.RegionLabel();
	eval=EvalQs(m_HC,GetPathName(),bWArray);

	int len=m_HC.GetWidth()*m_HC.GetHeight();
	int*tagArr=new int[len];
	m_HC.GetTag(tagArr);
	m_DIB.Origin();
	m_DIB.LookRegions(tagArr);
	SetModifiedFlag(FALSE);
	UpdateAllViews(NULL);
	delete[] tagArr;
}

void CImageDoc::SetEM()
{
	POSITION p=m_pDocTemplate->GetFirstDocPosition();
	CString cur="";//current active document title
	CImageDoc*pD=(CImageDoc*)m_pDocTemplate->GetNextDoc(p);
	CString a=pD->GetTitle();//the first document in the list
	cur=a;
	int i=0;
	while(a.Find("EDGE")==-1&&p!=NULL)
	{
		pD=(CImageDoc*)m_pDocTemplate->GetNextDoc(p);
		a=pD->GetTitle();//the second document in the list
		++i;
		if(a==cur)
			return;
	}
	if(a==GetTitle())
		return;
	edgeMap=pD->edgeMap;
	pD->edgeMap=NULL;
}

void CImageDoc::OnSegGshc() 
{
	// TODO: Add your command handler code here
	StartTimer(); 
	SetEM();//add edge map
	BuildData(m_HC,GetPathName(),bWArray);

	int cur=m_HC.SegGraph4();
	m_HC.InitiateRegions();
	if(m_HC.sortDM)
	{
		m_HC.MRS(m_HC.mindiff);
	}
	else
		m_HC.HierClust();
	quantum=ElapsedTime();  
	m_HC.RegionLabel();
	eval=EvalQs(m_HC,GetPathName(),bWArray);

	int len=m_HC.GetWidth()*m_HC.GetHeight();
	int*tagArr=new int[len];
	m_HC.GetTag(tagArr);
	m_DIB.Origin();
	m_DIB.LookRegions(tagArr);

	SetModifiedFlag(FALSE);
	UpdateAllViews(NULL);
	delete[]tagArr;

}

void CImageDoc::OnSegGraph() 
{
	// TODO: Add your command handler code here
	StartTimer(); 
	SetEM();//add edge map
	BuildData(m_HC,GetPathName(),bWArray);

	int cur=m_HC.SegGraph4();
	m_HC.InitiateRegions();

	quantum=ElapsedTime();  
	m_HC.RegionLabel();
	eval=EvalQs(m_HC,GetPathName(),bWArray);
	int len=m_HC.GetWidth()*m_HC.GetHeight();
	int*tagArr=new int[len];
	m_HC.GetTag(tagArr);
	m_DIB.Origin();
	m_DIB.LookRegions(tagArr);
	SetModifiedFlag(FALSE);
	UpdateAllViews(NULL);
	delete[] tagArr;
}



void CImageDoc::OnClassRoadclump() 
{
	if(m_HC.PrepPropMemo(7))
	m_HC.RegionProps();
	// TODO: Add your command handler code here
	BYTE*roadseed=new BYTE[m_DIB.m_nHeight*m_DIB.m_nWidth];
	m_HC.SetWH(m_DIB.m_nWidth,m_DIB.m_nHeight);
	vector<int> endpixel;
	m_HC.RoadSeed(roadseed);
//	m_HC.RoadSeed(roadseed,"F:\\landcruiser\\TIFFIMAGE\\roads.bmp");
//	m_HC.RoadThin(roadseed,endpixel);
//	m_HC.RoadLink(roads,endpixel);
//	m_HC.RoadExpo(roadseed,endpixel,GetPathName());
	delete[]roadseed;
//	m_DIB.LookRegions(m_HC.tag,1);
}

void CImageDoc::OnPrepSr() 
{
	// TODO: Add your command handler code here

	BuildData(m_HC,GetPathName(),bWArray);

	int temp=32;
	m_HC.qttest(temp*2);
	int len=m_HC.GetWidth()*m_HC.GetHeight();
	int*tagArr=new int[len];
	m_HC.GetTag(tagArr);
	m_DIB.Origin();
	m_DIB.ShowReg(tagArr,temp);
	delete[] tagArr;
}


void CImageDoc::OnClassErode() 
{
	// TODO: Add your command handler code here
	BYTE *lp=new BYTE[30];
	memset(lp,0,sizeof(BYTE)*30);
	lp[6]=1;
	lp[7]=1;	
	lp[11]=1;
	lp[12]=1;
	lp[8]=1;
	lp[13]=1;
	lp[21]=1;
	lp[16]=1;
	lp[17]=1;
	lp[18]=1;
	lp[22]=1;
	lp[23]=1;
	BYTE thick;
	m_HC.BinMorph(lp,5,6,12,47,thick);
	thick=2*thick;
	delete []lp;
}
int CImageDoc::lookregion(int cx,int cy,int option)
{
	int mh=m_DIB.m_nHeight,mw=m_DIB.m_nWidth;
	if(cx<0||cy<0||cx>(mw-1)||cy>(mh-1))
	{
		AfxMessageBox("Click on the map!");
		return -1;
	}
	int len=m_HC.GetWidth()*m_HC.GetHeight();
	int*tagArr=new int[len];
	m_HC.GetTag(tagArr);
//	m_DIB.Origin();
	curRegion=tagArr[cy*mw+cx];
	if(curRegion<0)
	{
		delete[] tagArr;
		AfxMessageBox("tag array of region labels is modified or illegal");
		return -1;
	}
	switch (option)
	{
	case 0:
	default:
		{int area=m_HC.GetRegSize(curRegion);
		CRect rect=m_HC.GetBoundBox(curRegion);
		m_DIB.lookregion(curRegion,tagArr,rect,area);
		break;}
	case 1:
		m_HC.RegMoment(curRegion);
		break;
	}
	delete[] tagArr;

	return curRegion;
}

void CImageDoc::OnSegSave() 
{
	CSelPropDlg SDlg;
	SDlg.DoModal();
	SaveSeg2(m_HC,GetPathName(),SDlg.numb);
}

void CImageDoc::OnFileSaveAs() 
{
	// TODO: Add your command handler code here

	CFileDialog FileDlg( false, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
	FileDlg.m_ofn.nFilterIndex=1; 
	if( FileDlg.DoModal() == IDOK )
	{
		CString fileName =FileDlg.GetPathName();
		if(fileName=="")
		{
			AfxMessageBox("Input file name!");
			return;
		}
		int period = fileName.ReverseFind('.');
		CString real;
		if(period>0)		
		{
			real=fileName;		
		}
		else if(FileDlg.m_ofn.nFilterIndex==1 )			
		{
			real=fileName+".bmp";
		}
		else if(FileDlg.m_ofn.nFilterIndex==2 )			
		{
			real=fileName+".tif";
		}	
		else if(FileDlg.m_ofn.nFilterIndex==3 )			
		{
			real=fileName+".jpg";
		}
		else if(FileDlg.m_ofn.nFilterIndex==4 )			
		{
			real=fileName+".img";
		}
		else
			real=fileName+".tif";
		bool res=m_DIB.SaveToFile(GetPathName(),real);	
		if (res==false)
		remove((const char*)real);
		
	//	OnSaveDocument(real);	
	}
}

void CImageDoc::OnSegQuadTree() 
{
	// TODO: Add your command handler code here
		// TODO: Add your command handler code here
	StartTimer(); 
	SetEM();//add edge map
	BuildData(m_HC,GetPathName(),bWArray);

	m_HC.QTMerge();

	quantum=ElapsedTime();  
	m_HC.RegionLabel();
	eval=EvalQs(m_HC,GetPathName(),bWArray);

	int len=m_HC.GetWidth()*m_HC.GetHeight();
	int*tagArr=new int[len];
	m_HC.GetTag(tagArr);
	m_DIB.Origin();
	m_DIB.LookRegions(tagArr);
	SetModifiedFlag(FALSE);
	UpdateAllViews(NULL);
	delete[] tagArr;

}

void CImageDoc::OnPrepSobel() 
{
	// TODO: Add your command handler code here

	BeginWaitCursor(); 
	int L=m_DIB.GetLength();
	BYTE*EM=new BYTE[L];
	GetEM(GetPathName(),EM,edgeBand);
	int w=m_DIB.GetWidth(), h=m_DIB.GetHeight();
	EdgeMag(EM,w,h);
	m_DIB.CreateDIBFromBits(w,h,EM,8);
//	m_DIB.SaveEdge("edgemap.bmp",EM);
	delete[]EM;
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();
	pFrame->pImageView->Invalidate(FALSE);	
	// 恢复光标形状
	EndWaitCursor(); 
}

void CImageDoc::OnPrepCanny() 
{
	// TODO: Add your command handler code here
	CCannyPara edgeDlg;

	edgeDlg.m_low=40.f;
	edgeDlg.m_high=79.f;

	if(edgeDlg.DoModal()!=IDOK)
		return;
	BeginWaitCursor(); 
	int w=m_DIB.GetWidth(),h=m_DIB.GetHeight();
	CvSize size=cvSize(w,h);
	IplImage *img= cvCreateImage(size, 8, 1);
	IplImage *edgeMap= cvCreateImage(size, 8, 1);
	
	m_DIB.LoadDIBToIPL(img->imageData,8);
	int aperture_size=3;
	if(edgeDlg.m_low==-1&&edgeDlg.m_high==-1)
	{
		CvMat *dx = 0, *dy = 0; 
		double low_thresh, high_thresh;
		dx = cvCreateMat( size.height, size.width, CV_16SC1 );
		dy = cvCreateMat( size.height, size.width, CV_16SC1 );
		cvSobel( img, dx, 1, 0, aperture_size );
		cvSobel( img, dy, 0, 1, aperture_size );
		AdaptiveFindThreshold(dx, dy, low_thresh, high_thresh,0.7);
		cvCanny( img, edgeMap, low_thresh, high_thresh, aperture_size);
		cvReleaseMat( &dx );
		cvReleaseMat( &dy );
	}
	else
	cvCanny( img, edgeMap, edgeDlg.m_low, edgeDlg.m_high, aperture_size);

	m_DIB.CreateDIBFromIPL(w,h,edgeMap->imageData,8);
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();
	pFrame->pImageView->Invalidate(FALSE);	
	// 恢复光标形状
	EndWaitCursor(); 
}

void CImageDoc::OnPrepSusan() 
{
	// TODO: Add your command handler code here
	CSUSANDlg edgeDlg;
	edgeDlg.m_BT=20.f;

	if(edgeDlg.DoModal()!=IDOK)
		return;
	BeginWaitCursor(); 
	int L=m_DIB.GetLength();
	BYTE*EM=new BYTE[L];
	GetEM(GetPathName(),EM,edgeBand);
	int w=m_DIB.GetWidth(),h=m_DIB.GetHeight();
	if(edgeDlg.m_Radio==0)
	EdgeSusan(EM,w,h,edgeDlg.m_BT,false);
	else
	EdgeSusan(EM,w,h,edgeDlg.m_BT,true);	
	m_DIB.CreateDIBFromBits(w,h,EM,8);
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();
	pFrame->pImageView->Invalidate(FALSE);	
	// 恢复光标形状
	delete[]EM;

	EndWaitCursor(); 
}

void CImageDoc::OnPrepMorph() 
{
	// TODO: Add your command handler code here
	CMorphDlg md;
	md.m_EB=1;
	if(md.DoModal()!=IDOK)
		return;
	int L=m_DIB.GetLength();
	BYTE*EM=new BYTE[L];
	GetEM(GetPathName(),EM,md.m_EB);
	m_HC.SetWH(m_DIB.m_nWidth,m_DIB.m_nHeight);
	Morph(EM,m_DIB.GetWidth(),m_DIB.GetHeight(),md.iPos+1);
	m_DIB.SaveEdge("SKELETON.bmp",EM);
}


void CImageDoc::OnClassPreview() 
{
/*
	//for building change detection
//open shade mask
	CString fn1="E:\\landcruiser\\TIFFIMAGE\\1001SHADE.tif";

	IplImage*shadeImg=LoadGDALToIPL(fn1,1,8);
//load nochange mask
	fn1="E:\\landcruiser\\TIFFIMAGE\\nochange.tif";

	IplImage*changeImg=LoadGDALToIPL(fn1,1,8);
	if(m_HC.PrepPropMemo(10))
	{
		m_HC.RegionProps();
	ShadeRatio(m_HC,changeImg,false);
	ShadeRatio(m_HC,shadeImg,true);
	}*/
	// TODO: Add your command handler code here
	CSelPropDlg SDlg;
	CPreviewDlg IDlg;
	if(m_HC.PrepPropMemo(7))
	m_HC.RegionProps();

	m_HC.GetPreviewMask(IDlg.mask);
	SDlg.m_top=IDlg.mask.top;
	SDlg.m_bot=IDlg.mask.bottom;
	SDlg.m_left=IDlg.mask.left;
	SDlg.m_right=IDlg.mask.right;
	if( SDlg.DoModal() == IDOK )
	{
		if(SDlg.numb<0)
			SDlg.numb=0;
		m_HC.typeProp=SDlg.numb;
		IDlg.mask.top=SDlg.m_top;
		IDlg.mask.bottom=SDlg.m_bot;
		IDlg.mask.left=SDlg.m_left;
		IDlg.mask.right=SDlg.m_right;
	}
	IDlg.DoModal();
}

void CImageDoc::OnSegSavetour() 
{
	// TODO: Add your command handler code here
	CFileDialog FileDlg( false, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
	FileDlg.m_ofn.nFilterIndex=2; 
	if( FileDlg.DoModal() == IDOK )
	{
		CString PathName =FileDlg.GetPathName();
		if(PathName=="")
		{	
			PathName=GetPathName();
			int pos=PathName.ReverseFind('\\');
			PathName=PathName.Left(pos+1);
			PathName+="contour seg.tif";
		}
		if(PathName.ReverseFind('.')==-1)
		{
			PathName+=".tif";
		}
		SaveSeg(m_HC,PathName,GetPathName());
	}
}

void CImageDoc::OnSegExportshp() 
{
	// TODO: Add your command handler code here
	m_HC.Polygonize(GetPathName());	
}
//method=0,  manual thresholding
//method=1 kmeans clustering using mahal dist 
//method==2 kmeans using euclidean distance
//note tagx[exS[i].p]==i for each level

void CImageDoc::OnMultiFeatureDiff() 
{
	// TODO: Add your command handler code here
//when processing i channel images, if set isGray, make sure that the image is 32 float depth
	// TODO: Add your command handler code here
	CString dirName="F:\\landcruiser\\TIFFIMAGE\\testsite\\";
	CString fn1=dirName+"spotp87.tif";
	CString fn2=dirName+"spotp92.tif";
//open image at epochs 1 and 2 
	CFileDialog FileDlg( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
	FileDlg.m_ofn.lpstrInitialDir=dirName;
	FileDlg.m_ofn.nFilterIndex=2; 
	FileDlg.m_ofn.lpstrTitle="Select image at time 1 for change detection";
	CString pathName;
	if( FileDlg.DoModal() == IDOK)
	{	
		pathName =FileDlg.GetPathName();
		if(pathName=="")
		{
			pathName=fn1;
		}
		else fn1=pathName;
	}
	else
		return;
	FileDlg.m_ofn.lpstrTitle="Select image at time 2 for change detection";
	if( FileDlg.DoModal() == IDOK)
	{	
		pathName =FileDlg.GetPathName();
		if(pathName=="")
		{
			pathName=fn2;
		}
		else fn2=pathName;
	}
	else
		return;
	dirName=pathName.Left(pathName.ReverseFind('\\')+1);
	CMFeatDlg setDlg;
	setDlg.m_level=2;
	setDlg.m_maxDiff=40;
	setDlg.m_minDiff=20;
//	setDlg.GetDlgItem(IDC_STATIC_LEVELUSE)->ShowWindow(SW_HIDE);
//	setDlg.GetDlgItem(IDC_EDIT_LEVELUSE)->ShowWindow(SW_HIDE);
	setDlg.m_bandUse="";
	MultiDiff detector(dirName,ConnectedMerge);
	if( setDlg.DoModal() == IDOK)
	{
		detector.Init(setDlg.m_level,setDlg.m_minDiff,setDlg.m_maxDiff);
		GetBWArray(setDlg.m_bandUse,bWArray);
		GetLevelArray(setDlg.m_levelUse,levelUse);
	}
	else return;


	char bale[100]={0};
	char bulk[100]={0};
	BeginWaitCursor();
	//segment and fuse
	
	detector.SynMultiSeg(fn1,fn2,bWArray);

//compute region signitures and similarity 
	//COMPUTE features for dataset1

	sprintf(bale,"%sfeat1.txt",dirName);
	detector.EvalFeat(fn1,bale,bWArray);

	sprintf(bale,"%sfeat2.txt",dirName);
	detector.EvalFeat(fn2,bale,bWArray);


	int count=detector.GetComps();
	float *pts=new float[count];//store segments distance at two times
	detector.CalcDist(levelUse,pts);
	int*label=new int[count];
	int jack=0;
	if(jack!=0)
	{
		float stat[4];
		float*est[1]={stat};
		MyKmeans(pts,1,label,count,est,2);
	}
	else
	{
		//method manual
		CTHRESHDlg thug;
		float*vecs=pts;
		thug.CreateHist(vecs,count);
		thug.DoModal();//note the CDThresh is called when scrolling bars, so bug occurs 
		//if other operations initializing m_HC are performed, assert(ink==w*h) may fail!
		float thresh=thug.curMin;
		
		for(int i=0;i<count;++i)
			label[i]=thresh>vecs[i]?1:0;
	}

	detector.CreateChangeMask(label);
	resultDlg.ShowWindow(SW_SHOW);
	resultDlg.SetImage(detector.buf,detector.width,detector.height,8);
	detector.SaveChange(fn1,dirName+"changemap.tif");
	delete[]pts;
	delete[]label;
	EndWaitCursor();
}


void CImageDoc::OnPrepTextimg() 
{
	// TODO: Add your command handler code here

	CString fn1="\\band1.tif";


	CFileDialog FileDlg( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,"Text file (*.txt)|*.txt||");
	FileDlg.m_ofn.nFilterIndex=1; 
	FileDlg.m_ofn.lpstrTitle="Select text file for image";
	CString pathName;

	if( FileDlg.DoModal() == IDOK)
	{	
		pathName =FileDlg.GetPathName();
		if(pathName=="")
		{
			AfxMessageBox("No file selected!");
			return;
		}
		pathName.MakeUpper();	
	}
	else return;

	int pos1=pathName.ReverseFind('\\');
	fn1=pathName.Left(pos1)+fn1;
//	Text2Image((const char*)pathName,(const char*)fn1);
}



void CImageDoc::OnSegRandIndex() 
{
	// TODO: Add your command handler code here
	CFileDialog FileDlg( true, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
	FileDlg.m_ofn.nFilterIndex=2; 
	FileDlg.m_ofn.lpstrTitle="Select standard partition truth file";
	CString pathName;
	double ri,cr;
	if( FileDlg.DoModal() == IDOK)
	{	
		pathName =FileDlg.GetPathName();
		if(pathName=="")
		{
			AfxMessageBox("No file selected!");
			return;
		}
		pathName.MakeUpper();
		if(m_HC.GetSetSize()==0)
		{
			AfxMessageBox("No segmentation to compare!");
			return;
		}

		int* truthPart=new int[m_HC.GetHeight()*m_HC.GetWidth()];
		m_HC.GetTag(truthPart,pathName);
		m_HC.BenchMark(truthPart,ri,cr);
		CString messah;
		messah.Format("RandIndex and CR index:%.3f,%.3f.",ri,cr);
		AfxMessageBox(messah);
	}
}

void CImageDoc::OnPrepSatHue() 
{
	// TODO: Add your command handler code here
	int bandInd[3]={1,2,3};
	CBandSelDlg IDlg; 
	CString fn=GetPathName();
	int rasCount;
	ImageProps(rasCount,IDlg.m_DataType,fn);
	IDlg.m_Band=rasCount;
	IDlg.m_R=1;
	IDlg.m_G=2;
	IDlg.m_B=3;

	if(IDlg.DoModal()==IDOK) 
	{	
		bandInd[0]=IDlg.m_R;
		bandInd[1]=IDlg.m_G;
		bandInd[2]=IDlg.m_B;	
	}
	else return;

	for(int i=0;i<3;++i)
		if(bandInd[i]>rasCount)
			bandInd[i]=1;

	ColorTransform(bandInd,fn);

}


void CImageDoc::OnClassBuilding() 
{
	// TODO: Add your command handler code here
	CSetPropDlg build;
	HCParams opts;
	build.m_MaxLWR=opts.maxLWR;
	build.m_MinArea=opts.minArea;

	build.m_MinCmpct=opts.minCmpct;
	build.m_MaxArea=opts.maxArea;
	build.m_MinWid=opts.minWid;
	if(build.DoModal())
	{
		opts.maxLWR=build.m_MaxLWR;
		opts.minArea=build.m_MinArea;
		opts.minCmpct=build.m_MinCmpct;
		opts.maxArea=build.m_MaxArea;
		opts.minWid=build.m_MinWid;
	}

//	CString dirName="F:\\landcruiser\\TIFFIMAGE\\testsite\\";
//	CString fn1=dirName+"spotp87.tif";
//	CString fn2=dirName+"spotp92.tif";
//open image at epochs 1 and 2 
/*	CFileDialog FileDlg( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
	FileDlg.m_ofn.lpstrInitialDir=dirName;
	FileDlg.m_ofn.nFilterIndex=2; 
	FileDlg.m_ofn.lpstrTitle="Select An Image to Extract Buildings";
	CString pathName;
	if( FileDlg.DoModal() == IDOK)
	{	
		pathName =FileDlg.GetPathName();
		if(pathName=="")
		{
			pathName=fn1;
		}
		else fn1=pathName;
	}
	else
		return;
	FileDlg.m_ofn.lpstrTitle="Select image at time 2 for change detection";
	if( FileDlg.DoModal() == IDOK)
	{	
		pathName =FileDlg.GetPathName();
		if(pathName=="")
		{
			pathName=fn2;
		}
		else fn2=pathName;
	}
	else
		return;
	dirName=pathName.Left(pathName.ReverseFind('\\')+1);
	CString fmask=dirName+"Mask.tif";*/
	float curdiff=30.f;

//	GDALDataset* pDataset2=(GDALDataset *) GDALOpen(fn1,GA_ReadOnly);
//			if(!pDataset2)
//				AfxMessageBox("cannot open image for m_HC!");
	//	IplImage*mask=LoadGDALToIPL(fmask,1,8);
//	m_DIB.CreateDIBFromIPL(wid,heg,mask->imageData,8);
	//GDALClose((GDALDatasetH)pDataset1);
	CString fn1=GetPathName();
	BuildData(m_HC,fn1,bWArray);

	int wid=m_HC.GetWidth();
	int heg=m_HC.GetHeight();
	m_HC.InitiateRegionSet();
	m_HC.MRS(curdiff);
	m_HC.RegionLabel();	

	if(m_HC.PrepPropMemo(7))
	m_HC.RegionProps();
	IplImage*result=LoadGDALToIPL(fn1,3,8);
	BuildingCand(&m_HC,&opts,result);

	m_DIB.Clear();
	m_DIB.CreateDIBFromIPL(wid,heg,result->imageData,24);
//	m_DIB.SaveToFile("Buildingxx.bmp");
	cvReleaseImage(&result);

	m_HC.Clear();
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();
	pFrame->pImageView->Invalidate(FALSE);


}


void CImageDoc::ImgToRasterIp(RasterIpChannels** signal)
//将图像转换成为RasterIpChannels对象，用于meanshift算法；
{
	Octet**	datain = new Octet*[p_max];
	int	w, h;
	w = m_DIB.GetWidth();
	h = m_DIB.GetHeight();
	for ( INT i = 0; i < p_max; i++ ) 
	{
		datain[i] = new Octet[w * h];
	}
	BYTE*imageData=new BYTE[w*h*3];
	m_DIB.LoadDIBToBuf(imageData);
	for ( int y = 0; y<h; y++ ) 
	{
		LONG lstart = y*w;
		for ( int x = 0; x<w; x++ ) 
		{
			LONG lpos = (lstart + x);
			datain[0][lpos] = (Octet) imageData[lpos*3+2];
			datain[1][lpos] = (Octet) imageData[lpos*3+1];
			datain[2][lpos] = (Octet) imageData[lpos*3];
		}
	}
	
	XfRaster::Info	info;
	info.rows = h;
	info.columns = w;
	info.origin_x = 0;
	info.origin_y = 0;
	*signal = new RasterIpChannels( info, p_max, eDATA_OCTET,
				    datain, true );
}

void CImageDoc::OnPrepGaborTransform() 
{
	BeginWaitCursor();
	int imageWidth=m_DIB.GetWidth();
	int imageHeight=m_DIB.GetHeight();
	FLOAT* gchannel = new FLOAT[imageWidth*imageHeight];
	FLOAT* resultarr = new FLOAT[imageWidth*imageHeight];
	BYTE*imageData=new BYTE[imageWidth*imageHeight*3];
	m_DIB.LoadDIBToBuf(imageData);
	//得到亮度通道；
	for (INT y=0; y<imageHeight; y++)
	{
		for (INT x=0; x<imageWidth; x++)
		{
			INT pos = y*imageWidth + x;
			gchannel[pos] = (FLOAT) imageData[pos*3+1];
		}
	}

	CMyGabor myGabor;
	myGabor.GetImageGaborCoeff(gchannel, imageWidth, imageHeight
		, 4, 3, &resultarr);

	for (INT x=0; x<imageWidth; x++)
	{
		for (INT y=0; y<imageHeight; y++)
		{
			LONG pos = (y*imageWidth + x);
			imageData[pos*3] = (BYTE) (resultarr[pos]);
			imageData[pos*3+1] = (BYTE) (resultarr[pos]);
			imageData[pos*3+2] = (BYTE) (resultarr[pos]);
		}
	}

	delete [] resultarr;
	resultarr = NULL;
	delete [] gchannel;
	gchannel = NULL;	
	m_DIB.CreateDIBFromBits(imageWidth, imageHeight, imageData);
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();

	EndWaitCursor();
	delete[]imageData;

	pFrame->pImageView->Invalidate(FALSE);	

}
extern int option;//定义在segmentMS头文件中；
void CImageDoc::OnSegMeanShiftDzj() 
{
	// TODO: Add your command handler code here

	BeginWaitCursor();

	SegmenterMS::sRectangle rects[Max_rects];
	
	long	selects = 0L;
	selects |= Lightness; selects |= Ustar; selects |= Vstar; 
	int p_dim=3;
	
	assert( p_dim <= p_max );	// must be since all array dimensions are such
	Boolean       block = false;
	unsigned int	seed  = 29254088; //random # generator
	
	int		n_rect = 0;
	Boolean	done = false;
    //////////////////////////////////////////////////////////////////////////
    // w = undersegmentation inside a window"
    // u = undersegmentation"
    // o = oversegmentation"
    // q = quantization"
	//////////////////////////////////////////////////////////////////////////
	char received;
	received = 'u';
	while(1)
	{  
		if((received =='w') || (received=='W'))
		{
			option=2; break;
		}
		else if((received =='u') || (received=='U'))
		{
			option=2; done=true; break;
		}
		else if((received =='o') || (received=='O'))
		{
			option=1; done=true; break;
		}
		else if((received =='q') || (received=='Q'))
		{
			option=0; done=true; break;
		}
		else
		{ 
			;
		}
	}
	
	SegmenterMS	segmenter;
	//CString filename = "E:\\testmeanshift.jpg";
	RasterIpChannels*	signal = NULL;// segmenter.read_IMG_file( filename);
	ImgToRasterIp(&signal);
	
	segmenter.ms_segment( signal, rects, n_rect, selects, seed, block);

	RasterIpChannels*	sig_result = segmenter.result_ras_;
	//segmenter.write_IMG_file( "E:\\segedimg.jpg", sig_result );
	//在弹出窗口中显示轮廓；
	int width = sig_result->columns_;
	int height = sig_result->rows_;
 	BOOL* contour = segmenter.result_contour;
	BYTE* contourdata = new BYTE[m_DIB.GetWidth()*m_DIB.GetHeight()*3];
	for (int y=0; y<height; ++y)
	{
		int lstart = y * width;
		for (int x=0; x<width; ++x)
		{
			int orgpos = (lstart + x);
			int pos= orgpos*3;
			contourdata[pos] = ((BYTE)(1-contour[orgpos])) * 200;
			contourdata[pos+1] = contourdata[pos];
			contourdata[pos+2] = contourdata[pos];
		}
	}
	//debugging is needed for the following 2 lines to work out
	resultDlg.ShowWindow(SW_SHOW);
	resultDlg.SetImage(contourdata,width,height,24);
	delete [] contourdata; contourdata = NULL;

	//在VIEW中显示分割结果图像；
	Octet *temp0 = sig_result->chdata_[0];
	Octet *temp1 = sig_result->chdata_[1];
	Octet *temp2 = sig_result->chdata_[2]; 
 	BYTE*imageData=new BYTE[width*height*3];

	for (y=0; y<height; y++)
	{
		int lstart = y * width;
		for (int x=0; x<width; x++)
		{
			int orgpos = (lstart + x);
			int outpos = orgpos * 3;
			imageData[outpos] = temp2[orgpos];
			imageData[outpos+1] = temp1[orgpos];
			imageData[outpos+2] = temp0[orgpos];
		//	assert(temp2[orgpos]==temp1[orgpos]&&temp2[orgpos]==temp0[orgpos]);
		}
	}

	delete signal;
	delete sig_result;
	m_DIB.CreateDIBFromBits(width, height, imageData);
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();

	EndWaitCursor();
	
	delete[]imageData;
	pFrame->pImageView->Invalidate(FALSE);	

}

void CImageDoc::OnPrepEntropy() 
{
	BeginWaitCursor();

	int imageWidth=m_DIB.GetWidth();
	int imageHeight=m_DIB.GetHeight();
	BYTE*imageData=new BYTE[imageWidth*imageHeight*3];
	m_DIB.LoadDIBToBuf(imageData);

	CMyTexture      myTexture;
	//存放各点邻域信息；
	FLOAT* tempentropy = new FLOAT[imageWidth*imageHeight];

	for (INT y=0; y<imageHeight; y++)
	{
		for (INT x=0; x<imageWidth; x++)
		{
			INT temppos = y*imageWidth + x;
			BYTE* neiarr=NULL;
			INT tempnr = NEIRADIUS;
			GetNearPixelsGreenExt(x, y, imageData
				, imageWidth, imageHeight, tempnr
				, &neiarr);//计算邻域
			INT tempi = 2*tempnr + 1;
			tempentropy[temppos] = myTexture.CalcuEntropy(
				neiarr, tempi, tempi);
			
			delete [] neiarr; neiarr = NULL;
		}
	}

	//用临时数组替换原数组；
	for (y=0; y<imageHeight; y++)
	{
		for (INT x=0; x<imageWidth; x++)
		{
			LONG pos = (y*imageWidth + x);
			BYTE tempval = (BYTE)(tempentropy[pos]*30);
			imageData[pos*3] = tempval;
			imageData[pos*3+1] = tempval;
			imageData[pos*3+2] = tempval;
		}
	}

	delete [] tempentropy; tempentropy = NULL;

	m_DIB.CreateDIBFromBits(imageWidth, imageHeight, imageData);
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();

	EndWaitCursor();
	
	delete[]imageData;
	pFrame->pImageView->Invalidate(FALSE);
	
}

void CImageDoc::OnSegWatershedVincent() 
{
	BeginWaitCursor();

	int imageWidth=m_DIB.GetWidth();
	int imageHeight=m_DIB.GetHeight();
	BYTE*imageData=new BYTE[imageHeight*imageWidth*3];
	m_DIB.LoadDIBToBuf(imageData);
	MyColorSpace   myColorSpace;
	MyLUV* luvData= new MyLUV[imageWidth*imageHeight];
	myColorSpace.RgbtoLuvPcm(imageData, imageWidth, imageHeight, luvData);

	LONG imagelen = imageWidth*imageHeight;
	FLOAT* deltar = new FLOAT[imagelen];//梯度模数组；
	FLOAT* deltasita = new FLOAT[imagelen];//梯度角度数组；
	INT*   flag = new INT[imagelen];//各点标识数组；
	INT*  gradientfre = new INT[256];//图像中各点梯度值频率；
	INT*  gradientadd = new INT[257];//各梯度起终位置；
	memset( gradientfre, 0, 256*sizeof(INT));
	memset( gradientadd, 0, 257*sizeof(INT));

	//首先得到各点梯度；
    GetGradient(imageData, imageWidth, imageHeight
		, deltar, deltasita);

	LONG temptime1 = GetTickCount();//初始时刻；
	//以下统计各梯度频率；
	MyImageGraPt*  graposarr = new MyImageGraPt[imagelen];
	LONG xstart, imagepos, deltapos;
	xstart = imagepos = deltapos = 0;
	for (INT y=0; y<imageHeight; y++)
	{
		xstart = y*imageWidth;
		for (INT x=0; x<imageWidth; x++)
		{
			deltapos = xstart + x;
			if (deltar[deltapos]>255)
			{
				deltar[deltapos] = 255;
			}
			INT tempi = (INT)(deltar[deltapos]);
			gradientfre[tempi] ++;//灰度值频率；
		}
	}

	//统计各梯度的累加概率；
	INT added = 0;
	gradientadd[0] = 0;//第一个起始位置为0；
	for (INT ii=1; ii<256; ii++)
	{
		added += gradientfre[ii-1];
		gradientadd[ii] = added;
	}
	gradientadd[256] = imagelen;//最后位置；

	memset( gradientfre, 0, 256*sizeof(INT));//清零，下面用作某梯度内的指针；
	//自左上至右下sorting....
	for ( y=0; y<imageHeight; y++)
	{
		xstart = y*imageWidth;
		for (INT x=0; x<imageWidth; x++)
		{
			deltapos = xstart + x;
			INT tempi = (INT)(deltar[deltapos]);//当前点的梯度值，由于前面的步骤，最大只能为255；
			//根据梯度值决定在排序数组中的位置；
			INT tempos = gradientadd[tempi] + gradientfre[tempi];
			gradientfre[tempi] ++;//梯度内指针后移；
			graposarr[tempos].gradient = tempi;	//根据当前点的梯度将该点信息放后排序后数组中的合适位置中去；		
			graposarr[tempos].x = x;
			graposarr[tempos].y = y;
		}
	}

	LONG temptime2 = GetTickCount();//vincent泛洪前的时刻；
	INT rgnumber = 0;//分割后的区域数；
	FloodVincent(graposarr, imageWidth,imageHeight,gradientadd, 0, 255, flag, rgnumber);//Flooding；
	LONG temptime3 = GetTickCount();//vincent泛洪后的时刻；
	LONG kk0 = temptime2 - temptime1;//排序用时；
	LONG kk1 = temptime3 - temptime2;//flood用时;
	LONG allkk = temptime3 - temptime1;//总用时；
	allkk = temptime3 - temptime1;//总用时；

///*  //区域增长步骤
	//以下准备计算各个区域的LUV均值；
	MyRgnInfo*  rginfoarr = new MyRgnInfo[rgnumber+1];//分割后各个区的一些统计信息,第一个元素不用，图像中各点所属区域的信息存放在flag数组中；
	//清空该数组；
	for (INT i=0; i<=rgnumber; i++)
	{
		rginfoarr[i].isflag = FALSE;
		rginfoarr[i].ptcount = 0;
		rginfoarr[i].l = 0;
		rginfoarr[i].u = 0;
		rginfoarr[i].v = 0;
	}

	for (y=0; y<imageHeight; y++)
	{
		xstart = y*imageWidth;
		for (INT x=0; x<imageWidth; x++)
		{
			INT pos = xstart + x;
			INT rgid = flag[pos];//当前位置点所属区域在区统计信息数组中的位置；
			//以下将该点的信息加到其所属区信息中去；
			rginfoarr[rgid].ptcount ++;
			rginfoarr[rgid].l += luvData[pos].l;
			rginfoarr[rgid].u += luvData[pos].u;
			rginfoarr[rgid].v += luvData[pos].v;
		}
	}
	//求出各个区的LUV均值；
	for (i=0; i<=rgnumber; i++)
	{
		rginfoarr[i].l = (FLOAT) ( rginfoarr[i].l / rginfoarr[i].ptcount );
		rginfoarr[i].u = (FLOAT) ( rginfoarr[i].u / rginfoarr[i].ptcount );
		rginfoarr[i].v = (FLOAT) ( rginfoarr[i].v / rginfoarr[i].ptcount );
	}

	//根据各区LUV均值（rginfoarr）和各区之间邻接关系（用flag计算）进行区域融合
	INT* mergearr = new INT[rgnumber+1];
	memset( mergearr, -1, sizeof(INT)*(rgnumber+1) );
	INT mergednum = 0;
	LONG temptime4 = GetTickCount();
	MergeRgs(rginfoarr, rgnumber, flag, imageWidth, imageHeight, mergearr, mergednum);
	LONG temptime5 = GetTickCount();
	LONG kk2 = temptime5 - temptime4;//合并用时;
	//确定合并后各像素点所属区域；
	for (y=0; y<(imageHeight); y++)
	{
		xstart = y*imageWidth;
		for (INT x=0; x<(imageWidth); x++)
		{
			INT pos = xstart + x;
			INT rgid = flag[pos];//该点所属区域；
			flag[pos] = FindMergedRgn(rgid, mergearr);
		}
	}
	delete [] mergearr; mergearr = NULL;

	//用各区均值代替原像素点值；
	FLOAT* luvbuff = NULL;
	luvbuff = new FLOAT[imageWidth*imageHeight*3];

	for (y=1; y<(imageHeight-1); y++)
	{
		xstart = y*imageWidth;
		for (INT x=1; x<(imageWidth-1); x++)
		{
			INT pos = xstart + x;
			INT rgid = flag[pos];//该点所属区域；
			luvData[pos].l = rginfoarr[rgid].l;//luvData用于全局保存LUV值;
			luvbuff[pos*3] = rginfoarr[rgid].l;//luvbuff用于传递参数给LuvToRgb();
			luvData[pos].u = rginfoarr[rgid].u;
			luvbuff[pos*3+1] = rginfoarr[rgid].u;
			luvData[pos].v = rginfoarr[rgid].v;
			luvbuff[pos*3+2] = rginfoarr[rgid].v;
		}
	}

	myColorSpace.LuvToRgb(luvbuff, imageWidth
		, imageHeight, imageData);

	delete [] luvbuff; luvbuff = NULL;
	delete [] rginfoarr; rginfoarr = NULL;//大小等于区域总数
//*/

	
///*
	//在弹出窗口中显示轮廓；
	int width = imageWidth;
	int height = imageHeight;
	BYTE* contourdata = new BYTE[imageWidth*imageHeight*3];
	for ( y=1; y<height-1; y++)
	{
		int lstart = y * width;
		for (int x=1; x<width-1; x++)
		{
			int orgpos = (lstart + x);
			INT left = orgpos - 1;
			INT up = orgpos - imageWidth;
			INT right = orgpos +1;
			INT down = orgpos + imageWidth;
			INT pos =orgpos*3;
			if ( ( flag[right]!=flag[orgpos] )
				|| ( flag[down]!=flag[orgpos] ) )
			{
				contourdata[pos] = 200;
				contourdata[pos+1] = 200;
				contourdata[pos+2] = 200;
			}else
			{
				contourdata[pos] = 0;
				contourdata[pos+1] = 0;
				contourdata[pos+2] = 0;
			}
		}
	}
	//debugging need to make the 3 lines work out right
	resultDlg.ShowWindow(SW_SHOW);
	resultDlg.SetImage(contourdata, imageWidth, imageHeight,24);
	delete [] contourdata; contourdata = NULL;
//*/

	//以下根据标识数组查找边界点（不管四边点以减小复杂度）；
	for (y=1; y<(imageHeight-1); y++)
	{
		xstart = y*imageWidth;
		for (INT x=1; x<(imageWidth-1); x++)
		{
			INT pos = xstart + x;
			INT imagepos = pos * 3;	
			INT left = pos - 1;
			INT up = pos - imageWidth;
			INT right = pos +1;
			INT down = pos + imageWidth;
			if ( ( flag[right]!=flag[pos] )
				|| ( flag[down]!=flag[pos] ) )
			//if ( flag[pos]==0 )
			{
				imageData[imagepos] = 0;
				imageData[imagepos+1] = 0;
				imageData[imagepos+2] = 250;
			}

		}
	}

	delete [] gradientadd; gradientadd = NULL;//大小256
	delete [] gradientfre; gradientfre = NULL;//大小256
	delete [] deltasita; deltasita = NULL;//大小imagelen
	delete [] deltar;    deltar    = NULL;//大小imagelen
	delete [] graposarr; graposarr = NULL;//大小imagelen
	delete [] flag;      flag      = NULL;//大小imagelen


	m_DIB.CreateDIBFromBits(imageWidth, imageHeight, imageData,24);
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();

	EndWaitCursor();

	delete[]imageData;
	pFrame->pImageView->Invalidate(FALSE);
}

void CImageDoc::OnSegWatershedG() 
{
	BeginWaitCursor();
	int imageWidth=m_DIB.GetWidth();
	int imageHeight=m_DIB.GetHeight();
	BYTE*imageData=new BYTE[imageWidth*imageHeight*3];
	m_DIB.LoadDIBToBuf(imageData);
	MyMath myMath;
	LONG imagelen = imageWidth*imageHeight;
	FLOAT* deltar = new FLOAT[imagelen];//梯度模数组；
	FLOAT* deltasita = new FLOAT[imagelen];//梯度角度数组；
	INT*   flag = new INT[imagelen];//各点标识数组；

	//首先得到各点梯度；
    GetGradient(imageData, imageWidth, imageHeight
		, deltar, deltasita);

	//以下准备对各点梯度进行排序；
	//由于排序后各点位置信息将丢失，因此先将位置信息保存；
	MyImageGraPt*  graposarr = new MyImageGraPt[imagelen];
	LONG xstart, imagepos, deltapos;
	xstart = imagepos = deltapos = 0;
	for (INT y=0; y<imageHeight; y++)
	{
		xstart = y*imageWidth;
		for (INT x=0; x<imageWidth; x++)
		{
			deltapos = xstart + x;
/*
			if (deltar[deltapos]<=9)
			{
				deltar[deltapos] = 0;
			}
			if (deltar[deltapos]>=246)
			{
				deltar[deltapos] = 255;
			}
*/
			graposarr[deltapos].gradient = (INT)(deltar[deltapos]);
			graposarr[deltapos].x = x;
			graposarr[deltapos].y = y;
		}
	}

	//以下按梯度排序(这个排序操作最费时)；
	myMath.QickSort(graposarr, imagelen);

	//将标识数组清0；
	for (INT i=0; i<imagelen; i++)
	{
		flag[i] = -1;
	}

	//以下顺序处理排序后的梯度数组，对每一点查看其邻点(更低的)是否
	//已被标识，若是，则将赋旧标识，否则，分配一新的标识；
	//赋标识后，在标识数组中记录, 标识数组按行列顺序排列，
	//这样能够很快根据行列位置找到某一点的标识；
	INT rgncount = 0;
	for (i=0; i<imagelen; i++)
	{
		INT x = graposarr[i].x;
		INT y = graposarr[i].y;
		INT curpos = y*imageWidth + x;

		//找当前点的四邻域（也许可以改为八邻域），按左右上下次序，这一次序对最后结果应该无影响；
		INT left = curpos - 1;
		if ( (x>0) && (flag[left]>=0) )
		{
			flag[curpos] = flag[left];
			continue;
		}
		INT right = curpos + 1;
		if ( (x+1<imageWidth) && (flag[right]>=0) )
		{
			flag[curpos] = flag[right];
			continue;
		}
		INT up = curpos - imageWidth;
		if ( (y>0) && (flag[up]>=0) )
		{
			flag[curpos] = flag[up];
			continue;
		}
		INT down = curpos + imageWidth;
		if ( (y+1<imageHeight) && (flag[down]>=0) )
		{
			flag[curpos] = flag[down];
			continue;
		}
		flag[curpos] = i;//将标识号置为循环次数，这样标识号不会重复，当然，也导致标识号不连续，但这对后续操作并无影响；
		rgncount ++;
	}

	//以下根据标识数组查找边界点（不管四边点以减小复杂度）；
	for (y=1; y<(imageHeight-1); y++)
	{
		xstart = y*imageWidth;
		for (INT x=1; x<(imageWidth-1); x++)
		{
			INT pos = xstart + x;
			//找当前点的四邻域（也许可以改为八邻域），按左右上下次序，这一次序对最后结果应该无影响；
			INT left = pos - 1;
			INT up = pos - imageWidth;
			INT right = pos +1;
			INT down = pos + imageWidth;

			INT imagepos = pos * 3;			
/*
  //这样检测得到的边缘更加贴近真实边缘，也许和梯度计算有关
  //梯度计算时得到的值偏右下，边缘取左上抵消了这一偏移；
  ( flag[left]!=flag[pos] ) || ( flag[up]!=flag[pos] ) || 
*/
			if ( ( flag[right]!=flag[pos] )
				|| ( flag[down]!=flag[pos] ) )
			{
				imageData[imagepos] = 0;
				imageData[imagepos+1] = 0;
				imageData[imagepos+2] = 250;
			}
		}
	}

	delete [] deltasita; deltasita = NULL;
	delete [] deltar;    deltar    = NULL;
	delete [] graposarr; graposarr = NULL;
	delete [] flag;      flag      = NULL;


	m_DIB.CreateDIBFromBits(imageWidth, imageHeight, imageData);
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();

	EndWaitCursor();
	
	delete[]imageData;
	pFrame->pImageView->Invalidate(FALSE);
}

void CImageDoc::OnSegWatershedInver() 
{
	BeginWaitCursor();

	int imageWidth=m_DIB.GetWidth();
	int imageHeight=m_DIB.GetHeight();
	BYTE*imageData=new BYTE[imageWidth*imageHeight*3];
	m_DIB.LoadDIBToBuf(imageData);
	MyMath myMath;
	LONG imagelen = imageWidth*imageHeight;
	FLOAT* deltar = new FLOAT[imagelen];//梯度模数组；
	FLOAT* deltasita = new FLOAT[imagelen];//梯度角度数组；
	INT*   flag = new INT[imagelen];//各点标识数组；
	INT*  gradientfre = new INT[256];//图像中各点梯度值频率；
	INT*  gradientadd = new INT[257];//各梯度起终位置；
	memset( gradientfre, 0, 256*sizeof(INT));
	memset( gradientadd, 0, 257*sizeof(INT));

	//首先得到各点梯度；
    GetGradient(imageData, imageWidth, imageHeight
		, deltar, deltasita);

	LONG temptime1 = GetTickCount();
	//以下统计各梯度频率；
	MyImageGraPt*  graposarr = new MyImageGraPt[imagelen];
	LONG xstart, imagepos, deltapos;
	xstart = imagepos = deltapos = 0;

	for (INT y=0; y<imageHeight; y++)
	{
		xstart = y*imageWidth;
		for (INT x=0; x<imageWidth; x++)
		{
			deltapos = xstart + x;
			if (deltar[deltapos]>255)
			{
				deltar[deltapos] = 255;
			}
			INT tempi = (INT)(deltar[deltapos]);
			gradientfre[tempi] ++;//灰度值频率；
		}
	}

	//统计各梯度的累加概率；
	INT added = 0;
	gradientadd[0] = 0;//第一个起始位置为0；
	for (INT ii=1; ii<256; ii++)
	{
		added += gradientfre[ii-1];
		gradientadd[ii] = added;
	}
	gradientadd[256] = imagelen;//最后位置；

	memset( gradientfre, 0, 256*sizeof(INT));//清零，下面用作某梯度内的指针；
	//自左上至右下sorting....
	for ( y=0; y<imageHeight; y++)
	{
		xstart = y*imageWidth;
		for (INT x=0; x<imageWidth; x++)
		{
			deltapos = xstart + x;
			INT tempi = (INT)(deltar[deltapos]);//当前点的梯度值，由于前面的步骤，最大只能为255；
			//根据梯度值决定在排序数组中的位置；
			INT tempos = gradientadd[tempi] + gradientfre[tempi];
			gradientfre[tempi] ++;//梯度内指针后移；
			graposarr[tempos].gradient = tempi;	//根据当前点的梯度将该点信息放后排序后数组中的合适位置中去；		
			graposarr[tempos].x = x;
			graposarr[tempos].y = y;
		}
	}

	//将标识数组清0；
	for (INT i=0; i<imagelen; i++)
	{
		flag[i] = -1;
	}

	//以下顺序处理排序后的梯度数组，对每一点查看其邻点(更低的)是否
	//已被标识，若是，则将赋旧标识，否则，分配一新的标识；
	//赋标识后，在标识数组中记录, 标识数组按行列顺序排列，
	//这样能够很快根据行列位置找到某一点的标识；
	INT rgncount = 0;
	INT left, right, up, down; 
	INT minh=0, maxh=255;
	INT mergesize = (INT) (imagelen / 4);
	INT* merge1arr = new INT[mergesize];
	memset( merge1arr, -1, sizeof(INT)*mergesize );
	LONG temptime2 = GetTickCount();//错误flood之前时刻；
	for (INT h=minh; h<=maxh; h++)
	{
		INT stpos = gradientadd[h];
		INT edpos = gradientadd[h+1];
		INT levelmini = rgncount;//大于此编号者为本层新建minima;
		for (INT ini=stpos; ini<edpos; ini++)
		{
			INT x = graposarr[ini].x;
			INT y = graposarr[ini].y;
			INT curpos = y*imageWidth + x;
			myMath.GetNeiInt(x, y, curpos, imageWidth, imageHeight
				, left, right, up, down);
			
			//找当前点的四邻域（也许可以改为八邻域），按左右上下次序，这一次序对最后结果应该无影响；
			INT uppos = curpos - imageWidth;
			if ( (up>0) && (flag[uppos]>=0) )
			{
				flag[curpos] = flag[uppos];
			}
			INT leftpos = curpos - 1;
			if ( (left>0) && (flag[leftpos]>=0) )
			{
				if (flag[curpos]>=0)
				{
					if (flag[curpos]!=flag[leftpos])
					{
						//此前已赋不同的标号，则要么有错误的新区，要么该点为分水岭；
						if (flag[curpos]>levelmini || flag[leftpos]>levelmini)
						{
							//只要有一个大于levelmini,则有错误新区；
							if (flag[curpos]<=flag[leftpos])
							{
								merge1arr[flag[leftpos]] = flag[curpos];//记录合并；
								flag[leftpos] = flag[curpos];							
							}else
							{
								merge1arr[flag[curpos]] = flag[leftpos];
								flag[curpos] = flag[leftpos];
							}
						}//否则为分水岭，不作处理；
					}
				}else
				{
					//此前未赋标号，给它加上标号；
					flag[curpos] = flag[leftpos];
				}				
				//continue;
			}
			INT rightpos = curpos + 1;
			if ( (right>0) && (flag[rightpos]>=0) )
			{
				if (flag[curpos]>=0)
				{
					if (flag[curpos]!=flag[rightpos])
					{
						//此前已赋不同的标号，则要么有错误的新区，要么该点为分水岭；
						if (flag[curpos]>levelmini || flag[rightpos]>levelmini)
						{
							//只要有一个大于levelmini,则有错误新区；
							if (flag[curpos]<=flag[rightpos])
							{
								merge1arr[flag[rightpos]] = flag[curpos];//记录合并；
								flag[rightpos] = flag[curpos];							
							}else
							{
								merge1arr[flag[curpos]] = flag[rightpos];
								flag[curpos] = flag[rightpos];
							}
						}//否则为分水岭，不作处理；
					}
				}else
				{
					//此前未赋标号，给它加上标号；
					flag[curpos] = flag[rightpos];
				}				
				//continue;
			}
			INT downpos = curpos + imageWidth;
			if ( (down>0) && (flag[downpos]>=0) )
			{
				if (flag[curpos]>=0)
				{
					if (flag[curpos]!=flag[downpos])
					{
						//此前已赋不同的标号，则要么有错误的新区，要么该点为分水岭；
						if (flag[curpos]>levelmini || flag[downpos]>levelmini)
						{
							//只要有一个大于levelmini,则有错误新区；
							if (flag[curpos]<=flag[downpos])
							{
								merge1arr[flag[downpos]] = flag[curpos];//记录合并；
								flag[downpos] = flag[curpos];							
							}else
							{
								merge1arr[flag[curpos]] = flag[downpos];
								flag[curpos] = flag[downpos];
							}
						}//否则为分水岭，不作处理；
					}
				}else
				{
					//此前未赋标号，给它加上标号；
					flag[curpos] = flag[downpos];
				}				
				//continue;
			}
			if (flag[curpos]<0)
			{
				//仍然小于0，说明该点邻域均未标识，可能为新minima;
				rgncount ++;
    			flag[curpos] = rgncount;//将标识号置为循环次数，这样标识号不会重复，当然，也导致标识号不连续，但这对后续操作并无影响；
				merge1arr[rgncount] = 0;//合并数组中的值置为0,表示未被合并；
			}
		}
	}

	//重写各个区,步骤如下：
	//首先统计真正区数
	//另建一个数组，改写各个为0的真正区
	INT realcount = mergesize;//真正的区号，为了能够在程序中区分所以设成旧区不可能有的大值；
	for (i=0; i<mergesize; i++)
	{
		if (merge1arr[i]==0)
		{
			realcount ++;
			merge1arr[i] = realcount;
		}else if (merge1arr[i]>0)
		{
			merge1arr[i] = FindMergedRgnMaxbias(merge1arr[i], merge1arr, mergesize);
		}
	}
	realcount = realcount - mergesize;//真正的区域数；
	//得到各区真正所属minima;
	for (i=0; i<imagelen; i++)
	{
		if (merge1arr[flag[i]]>=0)
		{
			flag[i] = FindMergedRgnMaxbias(flag[i], merge1arr, mergesize) - mergesize;
		}
	}
	LONG temptime3 = GetTickCount();

	delete [] merge1arr; merge1arr = NULL;
	LONG kk0 = temptime2 - temptime1;//排序用时；
	LONG kk1 = temptime3 - temptime2;//flood用时;
	LONG allkk = temptime3 - temptime1;//总用时；
	allkk = temptime3 - temptime1;//总用时；

/*
	//////////////////////////////////////////////////////////////////////////
	//合并极小区
	//////////////////////////////////////////////////////////////////////////
	//以下准备计算各个区域的LUV均值；
	INT rgnumber = realcount;
	MyRgnInfo*  rginfoarr = new MyRgnInfo[rgnumber+1];//分割后各个区的一些统计信息,第一个元素不用，图像中各点所属区域的信息存放在flag数组中；
	//清空该数组；
	for (i=0; i<=rgnumber; i++)
	{
		rginfoarr[i].isflag = FALSE;
		rginfoarr[i].ptcount = 0;
		rginfoarr[i].l = 0;
		rginfoarr[i].u = 0;
		rginfoarr[i].v = 0;
	}

	for (y=0; y<imageHeight; y++)
	{
		xstart = y*imageWidth;
		for (INT x=0; x<imageWidth; x++)
		{
			INT pos = xstart + x;
			INT rgid = flag[pos];//当前位置点所属区域在区统计信息数组中的位置；
			//以下将该点的信息加到其所属区信息中去；
			rginfoarr[rgid].ptcount ++;
			rginfoarr[rgid].l += luvData[pos].l;
			rginfoarr[rgid].u += luvData[pos].u;
			rginfoarr[rgid].v += luvData[pos].v;
		}
	}
	//求出各个区的LUV均值；
	for (i=0; i<=rgnumber; i++)
	{
		rginfoarr[i].l = (FLOAT) ( rginfoarr[i].l / rginfoarr[i].ptcount );
		rginfoarr[i].u = (FLOAT) ( rginfoarr[i].u / rginfoarr[i].ptcount );
		rginfoarr[i].v = (FLOAT) ( rginfoarr[i].v / rginfoarr[i].ptcount );
	}

	//根据各区LUV均值（rginfoarr）和各区之间邻接关系（用flag计算）进行区域融合
	INT* mergearr = new INT[rgnumber+1];
	memset( mergearr, -1, sizeof(INT)*(rgnumber+1) );
	LONG temptime4 = GetTickCount();//错误flood之前时刻；
	INT mergednum = 0;
	MergeRgs(rginfoarr, rgnumber, flag, imageWidth, imageHeight, mergearr, mergednum);
	LONG temptime5 = GetTickCount();
	LONG kk2 = temptime5 - temptime4;//合并用时;

	//确定合并后各像素点所属区域；
	for (y=0; y<(imageHeight); y++)
	{
		xstart = y*imageWidth;
		for (INT x=0; x<(imageWidth); x++)
		{
			INT pos = xstart + x;
			INT rgid = flag[pos];//该点所属区域；
			flag[pos] = FindMergedRgn(rgid, mergearr);
		}
	}
	delete [] mergearr; mergearr = NULL;
*/


/*
	//用各区均值代替原像素点值；
	FLOAT* luvbuff = NULL;
	luvbuff = new FLOAT[imageWidth*imageHeight*3];

	for (y=1; y<(imageHeight-1); y++)
	{
		xstart = y*imageWidth;
		for (INT x=1; x<(imageWidth-1); x++)
		{
			INT pos = xstart + x;
			INT rgid = flag[pos];//该点所属区域；
			luvData[pos].l = rginfoarr[rgid].l;//luvData用于全局保存LUV值;
			luvbuff[pos*3] = rginfoarr[rgid].l;//luvbuff用于传递参数给LuvToRgb();
			luvData[pos].u = rginfoarr[rgid].u;
			luvbuff[pos*3+1] = rginfoarr[rgid].u;
			luvData[pos].v = rginfoarr[rgid].v;
			luvbuff[pos*3+2] = rginfoarr[rgid].v;
		}
	}

	myColorSpace.LuvToRgb(luvbuff, imageWidth
		, imageHeight, imageData);


	delete [] luvbuff; luvbuff = NULL;
	delete [] rginfoarr; rginfoarr = NULL;//大小等于区域总数
*/


	//以下根据标识数组查找边界点（不管四边点以减小复杂度）；
	for (y=1; y<(imageHeight-1); y++)
	{
		xstart = y*imageWidth;
		for (INT x=1; x<(imageWidth-1); x++)
		{
			INT pos = xstart + x;
			//找当前点的四邻域（也许可以改为八邻域），按左右上下次序，这一次序对最后结果应该无影响；
			INT left = pos - 1;
			INT up = pos - imageWidth;
			INT right = pos +1;
			INT down = pos + imageWidth;

			INT imagepos = pos * 3;			
/*
  //这样检测得到的边缘更加贴近真实边缘，也许和梯度计算有关
  //梯度计算时得到的值偏右下，边缘取左上抵消了这一偏移；
  ( flag[left]!=flag[pos] ) || ( flag[up]!=flag[pos] ) || 
*/
			if ( ( ( flag[down]!=flag[pos] )
				|| ( flag[pos]!=flag[right] ) ) )
			{
				imageData[imagepos] = 0;
				imageData[imagepos+1] = 0;
				imageData[imagepos+2] = 250;
			}
		}
	}

	delete [] deltasita; deltasita = NULL;
	delete [] deltar;    deltar    = NULL;
	delete [] graposarr; graposarr = NULL;
	delete [] flag;      flag      = NULL;


	m_DIB.CreateDIBFromBits(imageWidth, imageHeight, imageData);
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();

	EndWaitCursor();
	delete[]imageData;
	pFrame->pImageView->Invalidate(FALSE);
}

void CImageDoc::OnSegPyrMeanShift() 
{
	// TODO: Add your command handler code here
	int w=m_DIB.GetWidth(),h=m_DIB.GetHeight();
	CvSize size=cvSize(w,h);
	IplImage *src_img= cvCreateImage(size, 8, 3);
	IplImage *dst_img;
	
	m_DIB.LoadDIBToIPL(src_img->imageData,24);
	
	dst_img=cvCreateImage(cvGetSize(src_img),IPL_DEPTH_8U,3);
	if(w%4!=0||h%4!=0)
	{
		AfxMessageBox("Incompatible Width or Height!");
		return;
	}
	cvPyrMeanShiftFiltering(src_img,dst_img,20,40,2);
	//note errors occur if width or height cannot be divided by 2^max_level
	cvNamedWindow("MeanShift",1);
	cvShowImage("MeanShift",dst_img);
	cvWaitKey(0);
	cvReleaseImage(&src_img);
	cvReleaseImage(&dst_img);
	cvDestroyAllWindows();
	
}

void CImageDoc::OnClassMRFCD() 
{
//	AfxMessageBox("This command segments gray-scale difference image into 2 classes!");
	CMRFOptimDlg mdlg;
	mdlg.m_Beta=0.9;
	mdlg.m_C=0.98;
	mdlg.m_t=0.05;
	mdlg.m_ClsNum=2;
	mdlg.m_T0=4;
	mdlg.m_Alpha=0.1;
	if(mdlg.DoModal()!=IDOK)

		return;
	BeginWaitCursor();
	StartTimer(); 
	int w=m_DIB.GetWidth(),h=m_DIB.GetHeight();
	CvSize size=cvSize(w,h);
	IplImage *labelimg= cvCreateImage(size, 8, 1);
	IplImage *img= cvCreateImage(size, 8, 1);
	
	m_DIB.LoadDIBToIPL(img->imageData,8);
	char * out_win_name = "Label Image";
	
	
	int i,j,count;
	
	count=w*h;
	float *array=new float[w*h];
	for(j=0;j<h;++j)
		for(i=0;i<w;++i)
		{
			array[j*w+i]=((BYTE*)(img->imageData + img->widthStep*j))[i];
		}
		
	int *label=new int[count];
	float truck[4];
	float*est[1];
	est[0]=truck;

	MyKmeans(array,1, label,count,est);
	mrf1 mrf;
	mrf.SetBeta(mdlg.m_Beta);
	mrf.SetAlpha(mdlg.m_Alpha);
	mrf.SetT(mdlg.m_t);	
	mrf.SetT0(mdlg.m_T0);
	mrf.SetC(mdlg.m_C);
	mrf.in_image = img;
	mrf.out_image = labelimg;
	mrf.height = h;mrf.width =w;
	mrf.SetNoRegions(2);
	mrf.SetMeanAndVariance(0,est[0][0],est[0][1]);
	mrf.SetMeanAndVariance(1,est[0][2],est[0][3]);
	//	mrf.CalculateMeanAndVariance(0,mrf.width-40,4,22,22);
	//		mrf.CalculateMeanAndVariance(1,4,5,20,20);
	//		mrf.CalculateMeanAndVariance(2,84,88,22,22);
	//		mrf.CalculateMeanAndVariance(3,52,63,20,20);

	mrf.InitOutImage();
	if(mdlg.iPos==0)
	{
		mrf.Gibbs();
	}
	else if(mdlg.iPos==1)
	{
		mrf.Metropolis();
	}
	else if(mdlg.iPos==2)
	{
		mrf.Metropolis(true);
	}
	else
		mrf.ICM();
	
	mrf.CreateOutput();	
	quantum=ElapsedTime();  
//	m_DIB.CreateDIBFromBits(width, height, imageData);
	cvNamedWindow(out_win_name, CV_WINDOW_AUTOSIZE);
	cvShowImage(out_win_name, labelimg);
	cvWaitKey(0);
	cvReleaseImage(&labelimg);
	cvReleaseImage(&img);
	cvDestroyWindow(out_win_name);
	delete[]label;
	delete []array;
	EndWaitCursor(); 
	
}
//there is some problem with this function, but time is tight for me, so I reserve this problem later.
void CImageDoc::OnClassColorMRF() 
{
	// TODO: Add your command handler code here
	int i,count;
	CMRFOptimDlg mdlg;
	mdlg.m_Beta=2.5;
	mdlg.m_C=0.98;
	mdlg.m_t=0.05;
	mdlg.m_ClsNum=2;
	mdlg.m_T0=4;
	mdlg.m_Alpha=0.1;
	if(mdlg.DoModal()!=IDOK)
		return;
	BeginWaitCursor();
	 timer_valid = FALSE; // timer's value is invalid. Used by GetTimer()
	 timer.Reset();       // reset timer
	timer.Start();       // start timer
	int cls=mdlg.m_ClsNum;	
	int w=m_DIB.GetWidth(),h=m_DIB.GetHeight();
	count=w*h;
//use kmeans to classify color image using RGB bands
	int *label=new int[count];
	float*array=new float[count*3];	
	float*est[3];
		for (i=0;i<3;++i)
			est[i]=new float[cls*2];
	m_DIB.LoadDIBToBuf(array);	
	MyKmeans(array,3,label,count,est,cls);
	delete[]array;

	for (i=0;i<3;++i)
		delete[]	est[i];

	ColorMRF mrf;
	mrf.SetBeta(mdlg.m_Beta);
	mrf.SetAlpha(mdlg.m_Alpha);
	mrf.SetT(mdlg.m_t);	
	mrf.SetT0(mdlg.m_T0);
	mrf.SetC(mdlg.m_C);
	mrf.SetNoRegions(cls);

	mrf.SetImage(w,h);
	
	m_DIB.LoadDIBToBuf(mrf.in_data);
	mrf.SetLuv();
	//test line for prac1.bmp and prac1-4-colormrf.bmp
	//but still unable to pin down the bug
/*	double mcov[4][3][3]={
	{{100,0.00,0.00},{0.00,0.00,0.00},{0.00,0.00,0.00}},
	{{82.76,30.93,11.17},{13.26,2.57,0.80},{-4.55,-0.95,0.9}},
	{{39.33,0.92,-8.32},{7.78,0.37,1.11},{0.42,0.76,-0.08}},
	{{72.55,0.7,1.81},{4.23,0.43,0.81},{0.58,0.15,-0.32}}
	};
	for(i=0;i<cls;++i)
	{
		double*ptr[3]={mcov[i][0],mcov[i][1],mcov[i][2]};
		mrf.SetMeanVar(i, ptr);
	}
*/
	mrf.CalculateMeanAndCovariance(label);
	delete []label;
	
	if(mdlg.iPos==0)
	{
		mrf.Gibbs();
	}
	else if(mdlg.iPos==1)
	{
		mrf.Metropolis();
	}
	else if(mdlg.iPos==2)
	{
		mrf.Metropolis(true);
	}
	else
		mrf.ICM();
	    
  timer.Stop();       // stop timer
  timer_valid = TRUE; // timer's value is valid. Used by GetTimer()
  quantum=timer.GetElapsedTimeMs()/1000;
	m_DIB.CreateDIBFromBits(w,h,mrf.out_data);
	EndWaitCursor(); 
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();
	pFrame->pImageView->Invalidate(FALSE);	
}

void CImageDoc::OnClassKmeans() 
{
	// TODO: Add your command handler code here
	CString dirName="F:\\landcruiser\\TIFFIMAGE\\testsite\\";
	CString fn1=dirName+"spotp87.tif";
	CString fn2=dirName+"spotp92.tif";
//open image at epochs 1 and 2 
/*	CFileDialog FileDlg( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
	FileDlg.m_ofn.lpstrInitialDir=dirName;
	FileDlg.m_ofn.nFilterIndex=2; 
	FileDlg.m_ofn.lpstrTitle="Select image at time 1 for change detection";
	CString pathName;
	if( FileDlg.DoModal() == IDOK)
	{	
		pathName =FileDlg.GetPathName();
		if(pathName=="")
		{
			pathName=fn1;
		}
		else fn1=pathName;
	}
	else
		return;
*/
//	dirName=pathName.Left(pathName.ReverseFind('\\')+1);
	dirName=GetPathName();
	IplImage* t1=LoadGDALToIPL((const char*)dirName,1,8);
	//IplImage* t2=LoadGDALToIPL((const char*)fn2);

	int w=t1->width,h=t1->height;
	int i,j,count;
	count=w*h;

	int cls=2;	
	int *label=new int[count];
	
	float*est[1];
	float mustard[12];
	est[0]=mustard;
	float *array=new float[w*h];
	for(j=0;j<h;++j)
		for(i=0;i<w;++i)
		{
			array[j*w+i]=((BYTE*)(t1->imageData + t1->widthStep*j))[i];
		}
	MyKmeans(array,1,label,count,est,6);
	BYTE* outcome=new BYTE[w*h];
	BYTE*temp;
	for(j=0;j<h;++j)
		for(i=0;i<w;++i)
		{			
			temp=outcome+w*j+i;		
			*(temp)=est[0][label[j*w+i]*2];			
		}
	m_DIB.CreateDIBFromBits(w,h,outcome,8);
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();
	pFrame->pImageView->Invalidate(FALSE);	
	delete []outcome;
	delete []array;
	delete []label;

}

void CImageDoc::OnTextureRosinCD() 
{
	// TODO: Add your command handler code here
	CString dirName="F:\\landcruiser\\TIFFIMAGE\\testsite\\";
	CString fn1=dirName+"spotp87.tif";
	CString fn2=dirName+"spotp92.tif";
//open image at epochs 1 and 2 
	CFileDialog FileDlg( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
	FileDlg.m_ofn.lpstrInitialDir=dirName;
	FileDlg.m_ofn.nFilterIndex=2; 
	FileDlg.m_ofn.lpstrTitle="Select image at time 1 for change detection";
	CString pathName;
	if( FileDlg.DoModal() == IDOK)
	{	
		pathName =FileDlg.GetPathName();
		if(pathName=="")
		{
			pathName=fn1;
		}
		else fn1=pathName;
	}
	else
		return;
	FileDlg.m_ofn.lpstrTitle="Select image at time 2 for change detection";
	if( FileDlg.DoModal() == IDOK)
	{	
		pathName =FileDlg.GetPathName();
		if(pathName=="")
		{
			pathName=fn2;
		}
		else fn2=pathName;
	}
	else
		return;
	dirName=pathName.Left(pathName.ReverseFind('\\')+1);
	IplImage* t1=LoadGDALToIPL((const char*)fn1,3,8);
	IplImage* t2=LoadGDALToIPL((const char*)fn2,3,8);
	int w=t1->width,h=t1->height;
	IplImage* cd=cvCreateImage(cvSize(w,h),8,1);
	cvChangeDetection( t1,t2,cd);
	m_DIB.CreateDIBFromIPL(w,h,cd->imageData,8);
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();
	pFrame->pImageView->Invalidate(FALSE);	
	cvReleaseImage(&cd);

}
//note opencv cannnot load tif images

void CImageDoc::OnTextureCorrBinary() 
{
	// TODO: Add your command handler code here
	CString dirName="F:\\landcruiser\\TIFFIMAGE\\testsite\\";
	CString fn1=dirName+"spotp87.tif";
	CString fn2=dirName+"spotp92.tif";
//open image at epochs 1 and 2 
	CFileDialog FileDlg( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
	FileDlg.m_ofn.lpstrInitialDir=dirName;
	FileDlg.m_ofn.nFilterIndex=2; 
	FileDlg.m_ofn.lpstrTitle="Select image at time 1 for change detection";
	CString pathName;
	if( FileDlg.DoModal() == IDOK)
	{	
		pathName =FileDlg.GetPathName();
		if(pathName=="")
		{
			pathName=fn1;
		}
		else fn1=pathName;
	}
	else
		return;
	FileDlg.m_ofn.lpstrTitle="Select image at time 2 for change detection";
	if( FileDlg.DoModal() == IDOK)
	{	
		pathName =FileDlg.GetPathName();
		if(pathName=="")
		{
			pathName=fn2;
		}
		else fn2=pathName;
	}
	else
		return;
	dirName=pathName.Left(pathName.ReverseFind('\\')+1);
	IplImage*intens1=LoadGDALToIPL((const char*)fn1,1,32);
	IplImage*intens2=LoadGDALToIPL((const char*)fn2,1,32);
	
	int w=intens1->width,h=intens1->height;
	IplImage*corr=cvCreateImage(cvSize(w,h),IPL_DEPTH_32F,1);
	IplImage*binary=cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,1);
	cvWinCorr(intens1,intens2,9,corr);
	const char *out_win_name="Correlation CD";
	cvNamedWindow(out_win_name, CV_WINDOW_AUTOSIZE);


//255 denote change 0 for no change
	cvThreshold(corr,binary,0.6f,255,CV_THRESH_BINARY_INV);
	cvReleaseImage(&intens1);
	cvReleaseImage(&intens2);
	cvReleaseImage(&corr);
//use morphology operations to remove small objects and fill voids
	int n = 1;
    int an = n > 0 ? n : -n;
	IplConvKernel* element = 0;
	int element_shape = CV_SHAPE_RECT;

    element = cvCreateStructuringElementEx( an*2+1, an*2+1, an, an, element_shape, 0 );

	IplImage *dst=cvCloneImage(binary);
    for(int i=0;i<2;++i) 
    {
		//opening white area
		//closing black area
        cvErode(dst,dst,element,1);
        cvDilate(dst,dst,element,1);
		//opening black area with value 0  
        cvDilate(dst,dst,element,1);
        cvErode(dst,dst,element,1);
	
    }
	cvReleaseImage(&binary);
    cvReleaseStructuringElement(&element);
	//morphology done
	//mask image

	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();
	m_DIB.CreateDIBFromIPL(w,h,dst->imageData,8);
	pFrame->pImageView->Invalidate(FALSE);
	fn2=dirName+"Mask.tif";
	m_DIB.SaveToFile(GetPathName(),fn2);

	IplImage*mat1=LoadGDALToIPL((const char*)fn1,1,8);
	for(int j=0;j<h;++j)
	{
		for(int i=0;i<w;++i)
		{	
			if(((BYTE*)(dst->imageData + dst->widthStep*j))[i]==0)
			((BYTE*)(mat1->imageData + mat1->widthStep*j))[i]=0;
		}
	}
	cvReleaseImage(&dst);

	cvShowImage(out_win_name, mat1);
	cvWaitKey(0);
	
	cvDestroyWindow(out_win_name);
	cvReleaseImage(&mat1);

}
//gradient based change detection see "
//integrating intensity and texture difference for robust change detection"
//note opencv cannot deal with tiff
void CImageDoc::OnTextureGradCorr() 
{
	// TODO: Add your command handler code here
		// TODO: Add your command handler code here
	CString dirName="F:\\landcruiser\\TIFFIMAGE\\testsite\\";
	CString fn1=dirName+"spotp87.tif";
	CString fn2=dirName+"spotp92.tif";
//open image at epochs 1 and 2 
	CFileDialog FileDlg( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
	FileDlg.m_ofn.lpstrInitialDir=dirName;
	FileDlg.m_ofn.nFilterIndex=2; 
	FileDlg.m_ofn.lpstrTitle="Select image at time 1 for change detection";
	CString pathName;
	if( FileDlg.DoModal() == IDOK)
	{	
		pathName =FileDlg.GetPathName();
		if(pathName=="")
		{
			pathName=fn1;
		}
		else fn1=pathName;
	}
	else
		return;
	FileDlg.m_ofn.lpstrTitle="Select image at time 2 for change detection";
	if( FileDlg.DoModal() == IDOK)
	{	
		pathName =FileDlg.GetPathName();
		if(pathName=="")
		{
			pathName=fn2;
		}
		else fn2=pathName;
	}
	else
		return;
	dirName=pathName.Left(pathName.ReverseFind('\\')+1);
	IplImage* t1=LoadGDALToIPL((const char*)fn1,1,32);
	IplImage* t2=LoadGDALToIPL((const char*)fn2,1,32);

	int w=t1->width,h=t1->height;
	int i,j,l=5;float tw2=100;
	CvSize cvS=cvSize(w,h);

	IplImage* dfx1=cvCreateImage(cvS,IPL_DEPTH_32F, 1);
	IplImage* dfy1=cvCreateImage(cvS,IPL_DEPTH_32F, 1);
	cvSobel(t1, dfx1,1,0,3);
	cvSobel(t1, dfy1,0,1,3);

	IplImage* dfx2=cvCreateImage(cvS,IPL_DEPTH_32F, 1);
	IplImage* dfy2=cvCreateImage(cvS,IPL_DEPTH_32F, 1);

	cvSobel(t2, dfx2,1,0,3);
	cvSobel(t2, dfy2,0,1,3);
	cvReleaseImage(&t1);
	cvReleaseImage(&t2);
	IplImage* c12=cvCreateImage(cvS,IPL_DEPTH_32F, 1);
	cvZero(c12);
	cvMultiplyAcc(dfx1,dfx2,c12);
	cvMultiplyAcc(dfy1,dfy2,c12);
	P2PImgMultiply(dfx1,dfx1,dfx1);
	cvMultiplyAcc(dfy1,dfy1,dfx1);

	P2PImgMultiply(dfx2,dfx2,dfx2);
	cvMultiplyAcc(dfy2,dfy2,dfx2);
	cvReleaseImage(&dfy1);
	cvReleaseImage(&dfy2);
	IntegralImage(c12);
	IntegralImage(dfx1);
	IntegralImage(dfx2);
	IplImage* rtp=cvCreateImage(cvS,IPL_DEPTH_32F, 1);
	cvZero(rtp);
	int lf,rt,tp,bt,sz;
	int step=l/2, wS=dfx1->widthStep;
	float numer,denom;
	for(j=0;j<h;++j)
	{
		for(i=0;i<w;++i)
		{
			lf=i-step<0?0:(i-step);
			tp=j-step<0?0:(j-step);
			rt=i+step>=w?w-1:(i+step);
			bt=j+step>=h?h-1:(j+step);
			sz=(bt-tp+1)*(rt-lf+1);
			if(lf==0&&tp==0)
			{
				denom=((float*)(dfx1->imageData + wS*bt))[rt]+
				((float*)(dfx2->imageData + wS*bt))[rt];
				numer=	((float*)(c12->imageData + wS*bt))[rt];
					if(abs(denom)<1e-2)
				((float*)(rtp->imageData + wS*j))[i]=0;
				else
				((float*)(rtp->imageData + wS*j))[i]=1-numer*2/denom;
			}
			else if(tp==0&&lf>0)
			{
				denom=((float*)(dfx1->imageData + wS*bt))[rt]-
					((float*)(dfx1->imageData + wS*bt))[lf-1]+
					((float*)(dfx2->imageData + wS*bt))[rt]-
					((float*)(dfx2->imageData + wS*bt))[lf-1];
			
				numer=	((float*)(c12->imageData + wS*bt))[rt]-
				((float*)(c12->imageData + wS*bt))[lf-1];
					if(abs(denom)<1e-2)
				((float*)(rtp->imageData + wS*j))[i]=0;
				else
				((float*)(rtp->imageData + wS*j))[i]=1-numer*2/denom;
			}
			else if(lf==0&&tp>0)
			{
				denom=((float*)(dfx1->imageData + wS*bt))[rt]-
					((float*)(dfx1->imageData + wS*(tp-1)))[rt]+
					((float*)(dfx2->imageData + wS*bt))[rt]-
					((float*)(dfx2->imageData + wS*(tp-1)))[rt];
			
				numer=	((float*)(c12->imageData + wS*bt))[rt]-
				((float*)(c12->imageData + wS*(tp-1)))[rt];
					if(abs(denom)<1e-2)
				((float*)(rtp->imageData + wS*j))[i]=0;
				else
				((float*)(rtp->imageData + wS*j))[i]=1-numer*2/denom;
			
			}
			else
			{
					denom=((float*)(dfx1->imageData + wS*bt))[rt]+
					((float*)(dfx1->imageData + wS*(tp-1)))[lf-1]-
					((float*)(dfx1->imageData + wS*(tp-1)))[rt]-
					((float*)(dfx1->imageData + wS*bt))[lf-1]+
					((float*)(dfx2->imageData + wS*bt))[rt]+
					((float*)(dfx2->imageData + wS*(tp-1)))[lf-1]-
					((float*)(dfx2->imageData + wS*(tp-1)))[rt]-
					((float*)(dfx2->imageData + wS*bt))[lf-1];
			
				numer=((float*)(c12->imageData + wS*bt))[rt]+
					((float*)(c12->imageData + wS*(tp-1)))[lf-1]-
					((float*)(c12->imageData + wS*(tp-1)))[rt]-
					((float*)(c12->imageData + wS*bt))[lf-1];
					
					if(abs(denom)<1e-2)
				((float*)(rtp->imageData + wS*j))[i]=0;
				else
				((float*)(rtp->imageData + wS*j))[i]=1-numer*2/denom;
			}
		}
	}

//compute gip and store it in c12
	for(j=0;j<h;++j)
	{
		for(i=0;i<w;++i)
		{
			lf=i-step<0?0:(i-step);
			tp=j-step<0?0:(j-step);
			rt=i+step>=w?w-1:(i+step);
			bt=j+step>=h?h-1:(j+step);
			sz=(bt-tp+1)*(rt-lf+1);
			if(lf==0&&tp==0)
			{
				denom=((float*)(dfx1->imageData + wS*bt))[rt];
				numer=((float*)(dfx2->imageData + wS*bt))[rt];

				if(__max(numer,denom)==0)
					((float*)(c12->imageData + wS*j))[i]=0;
				else
				((float*)(c12->imageData + wS*j))[i]=sqrt(__max(numer,denom)/sz);
			}
			else if(tp==0&&lf>0)
			{
				denom=((float*)(dfx1->imageData + wS*bt))[rt]-
					((float*)(dfx1->imageData + wS*bt))[lf-1];
				numer=((float*)(dfx2->imageData + wS*bt))[rt]-
					((float*)(dfx2->imageData + wS*bt))[lf-1];	

				if(__max(numer,denom)==0)
					((float*)(c12->imageData + wS*j))[i]=0;
				else
				((float*)(c12->imageData + wS*j))[i]=sqrt(__max(numer,denom)/sz);
							
			}
			else if(lf==0&&tp>0)
			{
				denom=((float*)(dfx1->imageData + wS*bt))[rt]-
					((float*)(dfx1->imageData + wS*(tp-1)))[rt];
				numer=((float*)(dfx2->imageData + wS*bt))[rt]-
					((float*)(dfx2->imageData + wS*(tp-1)))[rt];

				if(__max(numer,denom)==0)
					((float*)(c12->imageData + wS*j))[i]=0;
				else
				((float*)(c12->imageData + wS*j))[i]=sqrt(__max(numer,denom)/sz);

			
			
			}
			else
			{
					denom=((float*)(dfx1->imageData + wS*bt))[rt]+
					((float*)(dfx1->imageData + wS*(tp-1)))[lf-1]-
					((float*)(dfx1->imageData + wS*(tp-1)))[rt]-
					((float*)(dfx1->imageData + wS*bt))[lf-1];
					numer=((float*)(dfx2->imageData + wS*bt))[rt]+
					((float*)(dfx2->imageData + wS*(tp-1)))[lf-1]-
					((float*)(dfx2->imageData + wS*(tp-1)))[rt]-
					((float*)(dfx2->imageData + wS*bt))[lf-1];
					if(__max(numer,denom)==0)
					((float*)(c12->imageData + wS*j))[i]=0;
				else
				((float*)(c12->imageData + wS*j))[i]=sqrt(__max(numer,denom)/sz);
			
			}
		}
	}
	for(j=0;j<h;++j)
	{
		for(i=0;i<w;++i)
		{
			((float*)(c12->imageData + wS*j))[i]/=(tw2);
		}
	}
	cvThreshold(c12,dfx1,1,0,CV_THRESH_TRUNC);
	P2PImgMultiply(dfx1,rtp,dfx1);

	IplImage*binary=cvCreateImage(cvS,IPL_DEPTH_8U,1);
	const char *out_win_name="Correlation CD";
	cvNamedWindow(out_win_name, CV_WINDOW_AUTOSIZE);
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();

		cvThreshold(dfx1,binary,0.4f,200,CV_THRESH_BINARY);//binary and dfx1 should be of same type, correction needed

		m_DIB.CreateDIBFromIPL(w,h,binary->imageData,8);
		pFrame->pImageView->Invalidate(FALSE);	
		cvShowImage(out_win_name, binary);
		cvWaitKey(0);
	
	cvDestroyWindow(out_win_name);

	cvReleaseImage(&c12);
	cvReleaseImage(&dfx1);
	cvReleaseImage(&dfx2);
	cvReleaseImage(&rtp);
	cvReleaseImage(&binary);

}

void CImageDoc::OnPrepOpening() 
{
	// TODO: Add your command handler code here
	int n = 2;
    int an = n > 0 ? n : -n;
	IplConvKernel* element = 0;
	int element_shape = CV_SHAPE_RECT;

    element = cvCreateStructuringElementEx( an*2+1, an*2+1, an, an, element_shape, 0 );
	int h=m_DIB.m_nHeight,w=m_DIB.m_nWidth;
	IplImage*src=cvCreateImage(cvSize(w,h),8,1);
	m_DIB.LoadDIBToIPL(src->imageData,8);
	IplImage *dst=cvCloneImage(src);
    if( n < 0 )//opening
    {
        cvErode(src,dst,element,1);
        cvDilate(dst,dst,element,1);
    }
    else//closing
    {
        cvDilate(src,dst,element,1);
        cvErode(dst,dst,element,1);
    }
    cvReleaseStructuringElement(&element);
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();
	m_DIB.CreateDIBFromIPL(w,h,dst->imageData,8);
	pFrame->pImageView->Invalidate(FALSE);
	cvReleaseImage(&src);
	cvReleaseImage(&dst);	
}

void CImageDoc::OnTextureHistoStatCD() 
{
	// TODO: Add your command handler code here
	CString dirName="F:\\landcruiser\\TIFFIMAGE\\testsite\\";
	CString fn1=dirName+"spotp87.tif";
	CString fn2=dirName+"spotp92.tif";
//open image at epochs 1 and 2 
	CFileDialog FileDlg( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
	FileDlg.m_ofn.lpstrInitialDir=dirName;
	FileDlg.m_ofn.nFilterIndex=2; 
	FileDlg.m_ofn.lpstrTitle="Select image at time 1 for change detection";
	CString pathName;
	if( FileDlg.DoModal() == IDOK)
	{	
		pathName =FileDlg.GetPathName();
		if(pathName=="")
		{
			pathName=fn1;
		}
		else fn1=pathName;
	}
	else
		return;
	FileDlg.m_ofn.lpstrTitle="Select image at time 2 for change detection";
	if( FileDlg.DoModal() == IDOK)
	{	
		pathName =FileDlg.GetPathName();
		if(pathName=="")
		{
			pathName=fn2;
		}
		else fn2=pathName;
	}
	else
		return;
	dirName=pathName.Left(pathName.ReverseFind('\\')+1);
	IplImage* t1=LoadGDALToIPL((const char*)fn1);
	IplImage* t2=LoadGDALToIPL((const char*)fn2);

	int w=t1->width,h=t1->height;
	CvSize cvS=cvSize(w,h);

	IplImage* corr=cvCreateImage(cvS,IPL_DEPTH_8U, 1);
	IplImage* binary=cvCreateImage(cvS,IPL_DEPTH_8U, 1);
	VRegSimi(t1,t2,16,corr);
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();

	cvThreshold(corr,binary,128,255,CV_THRESH_BINARY);
	m_DIB.CreateDIBFromIPL(w,h,binary->imageData,8);
	pFrame->pImageView->Invalidate(FALSE);	

	cvReleaseImage(&corr);
	cvReleaseImage(&binary);	
}

void CImageDoc::OnClassBuildingIsodata() 
{
	// TODO: Add your command handler code here
	CSetPropDlg build;
	HCParams opts;
	build.m_MaxLWR=opts.maxLWR;
	build.m_MinArea=opts.minArea;
	build.m_MinCmpct=opts.minCmpct;
	build.m_MaxArea=opts.maxArea;
	build.m_MinWid=opts.minWid;
	if(build.DoModal())
	{
		opts.maxLWR=build.m_MaxLWR;
		opts.minArea=build.m_MinArea;
		opts.minCmpct=build.m_MinCmpct;
		opts.maxArea=build.m_MaxArea;
		opts.minWid=build.m_MinWid;
	}
	CString fn1=GetPathName();

/*	CString dirName="F:\\landcruiser\\TIFFIMAGE\\testsite\\";
	CString fn1=dirName+"spotp87.tif";
	CString fn2=dirName+"spotp92.tif";
//open image at epochs 1 and 2 
	CFileDialog FileDlg( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
	FileDlg.m_ofn.lpstrInitialDir=dirName;
	FileDlg.m_ofn.nFilterIndex=2; 
	FileDlg.m_ofn.lpstrTitle="Select image for building extraction";
	CString pathName;
	if( FileDlg.DoModal() == IDOK)
	{	
		pathName =FileDlg.GetPathName();
		if(pathName=="")
		{
			pathName=fn1;
		}
		else fn1=pathName;
	}
	else
		return;
/*	FileDlg.m_ofn.lpstrTitle="Select image at time 2 for change detection";
	if( FileDlg.DoModal() == IDOK)
	{	
		pathName =FileDlg.GetPathName();
		if(pathName=="")
		{
			pathName=fn2;
		}
		else fn2=pathName;
	}
	else
		return;*/
//	dirName=pathName.Left(pathName.ReverseFind('\\')+1);
//	CString fmask=dirName+"Mask.tif";
/*	float curdiff=30.f;
	GDALDataset* pDataset1=(GDALDataset *) GDALOpen(fn1,GA_ReadOnly);
	if(!pDataset1)
		AfxMessageBox("cannot open image for m_HC!");
/*	GDALDataset* pDataset2=(GDALDataset *) GDALOpen(fn2,GA_ReadOnly);
			if(!pDataset2)
				AfxMessageBox("cannot open image for m_HC!");*/
	IplImage* t1=LoadGDALToIPL((const char*)fn1,1,8);
	//IplImage* t2=LoadGDALToIPL((const char*)fn2);
	
	int w=t1->width,h=t1->height;
		CvSize bound=cvSize(w,h);
	int i,j,count;
	count=w*h;
	enum Choice
	{
		IsoData=0,
		KMeans,
	};
	Choice method=KMeans;
	int *label=new int[count];
	if(method==IsoData)
	{
		for(j=0;j<h;++j)
			for(i=0;i<w;++i)
			{
				label[j*w+i]=((BYTE*)(t1->imageData + t1->widthStep*j))[i];
			}
	}
	else
	{
		// kmeans
		float*est[1];
		float mustard[10];
		est[0]=mustard;
		float *array=new float[w*h];
		for(j=0;j<h;++j)
			for(i=0;i<w;++i)
			{
				array[j*w+i]=((BYTE*)(t1->imageData + t1->widthStep*j))[i];
			}
			MyKmeans(array,1,label,count,est,5);
			delete[] array;
			//kmeans end
	}
	
	//morphology operations to fill holes and prune branches
	int n = 1;
	int an = n > 0 ? n : -n;
	IplConvKernel* element = 0;
	int element_shape = CV_SHAPE_RECT;
	
    element = cvCreateStructuringElementEx( an*2+1, an*2+1, an, an, element_shape, 0 );
	
	
	
	IplImage *dst=cvCreateImage(bound,IPL_DEPTH_8U,1);
	count=0;
	for(j=0; j<h; ++j)
	{	
		for(i=0; i<w; ++i) 
		{
			((BYTE*)(dst->imageData + dst->widthStep*j))[i]=(int)label[count];				
			++count;
		}
	}

    for(i=0;i<2;++i) 
    {
        cvDilate(dst,dst,element,1);
		
        cvErode(dst,dst,element,1);
		//opening black area with value 0  
		cvErode(dst,dst,element,1);
		
        cvDilate(dst,dst,element,1);
		
    }
	

	count=0;
	for(j=0; j<h; ++j)
	{
		for(i=0; i<w; ++i) 
		{
			label[count]=((BYTE*)(dst->imageData + dst->widthStep*j))[i];				
			++count;
		}
	}
	cvReleaseImage(&dst);
    cvReleaseStructuringElement(&element);
	
	m_HC.Clear();
	m_HC.SetWH(w,h);
	
	count=m_HC.MiniTag(label);
	//test 
	int mintemp=1e6,maxtemp=0;
	for(i=0;i<w*h;++i)
	{
		mintemp=__min(label[i],mintemp);
		maxtemp=__max(label[i],maxtemp);
	}
	assert(mintemp==0&&maxtemp==count-1);
	
	//	assert(count==comps);
	m_HC.DefReg(label,count);
	m_HC.SetTag(label);
	if(m_HC.PrepPropMemo(7))
	m_HC.RegionProps();

	//GDALClose((GDALDatasetH)pDataset1);
	
	IplImage*result=cvCreateImage(bound, IPL_DEPTH_8U,3);
	
	m_DIB.LoadDIBToIPL(result->imageData,24);
	BuildingCand(&m_HC,&opts,result);
	
	m_DIB.Clear();
	m_DIB.CreateDIBFromIPL(w,h,result->imageData,24);
	
	//	cvNamedWindow("Portal", 0);	
	//	cvShowImage("Portal", roof);
	//	cvSaveImage("building.bmp",portal);
	//	cvWaitKey(0);
	//	cvDestroyWindow("Portal");
	cvReleaseImage(&result);
	delete[]label;
	
	m_HC.Clear();
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();
	pFrame->pImageView->Invalidate(FALSE);	
}

void CImageDoc::OnClassMultiBuild() 
{
	// TODO: Add your command handler code here
	HCParams opts;
	float mindiff,maxdiff,curdiff;
	int k;
	CMFeatDlg setDlg;
	setDlg.m_level=2;
	setDlg.m_maxDiff=40;
	setDlg.m_minDiff=20;
//	setDlg.GetDlgItem(IDC_STATIC_LEVELUSE)->ShowWindow(SW_HIDE);
//	setDlg.GetDlgItem(IDC_EDIT_LEVELUSE)->ShowWindow(SW_HIDE);
	setDlg.m_bandUse="";
	if( setDlg.DoModal() == IDOK)
	{
		storey=setDlg.m_level;
		maxdiff=setDlg.m_maxDiff;
		mindiff=setDlg.m_minDiff;
	
		GetBWArray(setDlg.m_bandUse,bWArray);
		GetLevelArray(setDlg.m_levelUse,levelUse);
	}
	else return;
	if(maxdiff==mindiff) storey=1;

	CString fn1=GetPathName();

	IplImage*t1=LoadGDALToIPL(fn1,1,8);



	
	BuildData(m_HC,fn1,bWArray);
	int wid=m_HC.GetWidth(),heg=m_HC.GetHeight();
	m_HC.InitiateRegionSet();

	IplImage*result=LoadGDALToIPL(fn1,3,8);
//open shade mask
	fn1="E:\\landcruiser\\TIFFIMAGE\\1103SHADE.tif";

	IplImage*shadeImg=LoadGDALToIPL(fn1,1,8);
//load nochange mask
	fn1="E:\\landcruiser\\TIFFIMAGE\\Mask.tif";

	IplImage*changeImg=LoadGDALToIPL(fn1,1,8);

	for(k=0;k<storey;++k)
	{
		if(storey==1) curdiff=mindiff;
		else	curdiff=mindiff+(maxdiff-mindiff)/(storey-1)*k;
		m_HC.MRS(curdiff);
		m_HC.RegionLabel();
		if(m_HC.PrepPropMemo(10))
		{
			m_HC.RegionProps();
			ShadeRatio(m_HC,changeImg,false);
			ShadeRatio(m_HC,shadeImg,true);
		}
		
		BuildingCand2(m_HC,&opts,result);
	/*	int count;
		count=wid*heg;

		int *label=new int[count];
		float*est[1];
		float *mustard=new float[k*2];
		est[0]=mustard;
		float *array=new float[count];
		for(j=0;j<heg;++j){
			for(i=0;i<wid;++i)
			{
				array[j*wid+i]=((BYTE*)(t1->imageData + t1->widthStep*j))[i];
			}
		}
		MyKmeans(array,1,label,count,est,k);
		delete[] array;
		delete[]mustard;
		//kmeans end
		
		
		//morphology operations to fill holes and prune branches
		int n = 1;
		int an = n > 0 ? n : -n;
		IplConvKernel* element = 0;
		int element_shape = CV_SHAPE_RECT;
		
		element = cvCreateStructuringElementEx( an*2+1, an*2+1, an, an, element_shape, 0 );
		
		
		
		IplImage *dst=cvCreateImage(cvSize(wid,heg),IPL_DEPTH_8U,1);
		count=0;
		for(j=0; j<heg; ++j)
		{	
			for(i=0; i<wid; ++i) 
			{
				((BYTE*)(dst->imageData + dst->widthStep*j))[i]=(int)label[count];				
				++count;
			}
		}
		
		for(i=0;i<2;++i) 
		{
			cvDilate(dst,dst,element,1);
			
			cvErode(dst,dst,element,1);
			//opening black area with value 0  
			cvErode(dst,dst,element,1);
			
			cvDilate(dst,dst,element,1);
			
		}
		
		
		count=0;
		for(j=0; j<heg; ++j)
		{
			for(i=0; i<wid; ++i) 
			{
				label[count]=((BYTE*)(dst->imageData + dst->widthStep*j))[i];				
				++count;
			}
		}
		cvReleaseImage(&dst);
		cvReleaseStructuringElement(&element);
		
		m_HC.Clear();
		m_HC.SetWH(wid,heg);
		
		count=m_HC.MiniTag(label);
		//test 
		int mintemp=1e6,maxtemp=0;
		for(i=0;i<wid*heg;++i)
		{
			mintemp=__min(label[i],mintemp);
			maxtemp=__max(label[i],maxtemp);
		}
		assert(mintemp==0&&maxtemp==count-1);
		
		//	assert(count==comps);
		m_HC.DefReg(label,count);
		m_HC.SetTag(label);
		delete[]label;
		if(m_HC.PrepPropMemo(10))
		{
			m_HC.RegionProps();
			ShadeRatio(m_HC,changeImg,false);
			ShadeRatio(m_HC,shadeImg,true);
		}
		BuildingCand2(m_HC,&opts,result);*/
		
	}
	m_DIB.Clear();
	m_DIB.CreateDIBFromIPL(wid,heg,result->imageData,24);
	//	m_DIB.SaveToFile("Buildingxx.bmp");
	cvReleaseImage(&result);
	
	m_HC.Clear();
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->GetMainWnd();
	pFrame->pImageView->Invalidate(FALSE);

}

void CImageDoc::OnTextureGeometricCD() 
{
	// TODO: Add your command handler code here
	CString dirName="F:\\landcruiser\\TIFFIMAGE\\testsite\\";
	CString fn1=dirName+"spotp87.tif";
	CString fn2=dirName+"spotp92.tif";
//open image at epochs 1 and 2 
	CFileDialog FileDlg( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
	FileDlg.m_ofn.lpstrInitialDir=dirName;
	FileDlg.m_ofn.nFilterIndex=2; 
	FileDlg.m_ofn.lpstrTitle="Select image at time 1 for change detection";
	CString pathName;
	if( FileDlg.DoModal() == IDOK)
	{	
		pathName =FileDlg.GetPathName();
		if(pathName=="")
		{
			pathName=fn1;
		}
		else fn1=pathName;
	}
	else
		return;
	FileDlg.m_ofn.lpstrTitle="Select image at time 2 for change detection";
	if( FileDlg.DoModal() == IDOK)
	{	
		pathName =FileDlg.GetPathName();
		if(pathName=="")
		{
			pathName=fn2;
		}
		else fn2=pathName;
	}
	else
		return;
	dirName=pathName.Left(pathName.ReverseFind('\\')+1);
	CMFeatDlg setDlg;
	setDlg.m_level=2;
	setDlg.m_maxDiff=40;
	setDlg.m_minDiff=20;
//	setDlg.GetDlgItem(IDC_STATIC_LEVELUSE)->ShowWindow(SW_HIDE);
//	setDlg.GetDlgItem(IDC_EDIT_LEVELUSE)->ShowWindow(SW_HIDE);
	setDlg.m_bandUse="";

	if( setDlg.DoModal() == IDOK)
	{
//		detector.Init(setDlg.m_level,setDlg.m_minDiff,setDlg.m_maxDiff);
		GetBWArray(setDlg.m_bandUse,bWArray);
		GetLevelArray(setDlg.m_levelUse,levelUse);
	}
	else return;

	float curdiff=setDlg.m_minDiff;

	char bale[100]={0};
	char bulk[100]={0};
	BeginWaitCursor();
	//segment and fuse
	CHC m_HC;
	BuildData(m_HC,fn1,bWArray);//builddata can affect the values of bwarray
	
	m_HC.InitiateRegionSet();
	m_HC.MRS(curdiff);
	m_HC.RegionLabel();
	sprintf(bale,"%st1-rect-%.0f.txt",dirName,curdiff);
	sprintf(bulk,"%st1-tagmat-%.0f.txt",dirName,curdiff);
	m_HC.StoreSeg(bale,bulk);
	m_HC.Clear();
	BuildData(m_HC,fn2,bWArray);//builddata can affect the values of bwarray
	
	m_HC.InitiateRegionSet();
	
	m_HC.MRS(curdiff);
	m_HC.RegionLabel();
	sprintf(bale,"%st2-rect-%.0f.txt",dirName,curdiff);
	sprintf(bulk,"%st2-tagmat-%.0f.txt",dirName,curdiff);
	m_HC.StoreSeg(bale,bulk);
	m_HC.Clear();
	//	detector.CreateChangeMask(label);
	ChangeDetector alpha;
	sprintf(bale,"%st1-rect-%.0f.txt",dirName,curdiff);
	sprintf(bulk,"%st2-rect-%.0f.txt",dirName,curdiff);
	alpha.PrepRegList(bale,bulk);
	sprintf(bale,"%st1-tagmat-%.0f.txt",dirName,curdiff);
	sprintf(bulk,"%st2-tagmat-%.0f.txt",dirName,curdiff);
	alpha.PrepLabel(bale,bulk);
	alpha.InitPairs();
	alpha.MorphDiff(0.0,1.5);
	alpha.Detect(0.0,0.7);
	resultDlg.ShowWindow(SW_SHOW);
	resultDlg.SetImage(alpha.buf,alpha.width,alpha.height,8);
	alpha.SaveChange(fn1,dirName+"changemap.tif");

	EndWaitCursor();
	sprintf(bale,"%st1-rect-%.0f.txt",dirName,curdiff);
	sprintf(bulk,"%st2-rect-%.0f.txt",dirName,curdiff);
	remove(bale);
	remove(bulk);
		sprintf(bale,"%st1-tagmat-%.0f.txt",dirName,curdiff);
	sprintf(bulk,"%st2-tagmat-%.0f.txt",dirName,curdiff);
		remove(bale);
	remove(bulk);
}
