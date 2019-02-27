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
#include "MorphDlg.h"
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
	ON_COMMAND(ID_VIEW_REGIONS, OnViewRegions)
	ON_COMMAND(ID_SEG_QTHC, OnSegQthc)
	ON_COMMAND(ID_CLASS_REGPROP, OnClassRegprop)
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
	//}}AFX_MSG_MAP

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageDoc construction/destruction

CImageDoc::CImageDoc():m_dRoom(1),sortDM(true),quantum(0),m_pDataset(NULL),curRegion(-1)//export(true),
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
}

CImageDoc::~CImageDoc()
{
//	AfxMessageBox("IN imagedoc!");

	GDALClose((GDALDatasetH)m_pDataset);	
}

/////////////////////////////////////////////////////////////////////////////
// CImageDoc serialization

void CImageDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
	/*	CFile* cfptr=ar.GetFile();
		CString fileName=cfptr->GetFilePath();
		int period = fileName.ReverseFind('.');
		CString real;
		if(period>0)		
		{
			real=fileName.Right(3);
			real.MakeLower();
			if(real!="bmp")
				AfxMessageBox("Saving only supports bmp format!");	
		//	real=fileName.Left(period)+".bmp";
		}
		else			
		{
			AfxMessageBox("Please add .bmp to your save filename!");	
		//	real=fileName+".bmp";
		}
		m_DIB.SaveToFile(cfptr);*/
	
		
	}
	else
	{
		// TODO: add loading code here
		CFile* cfptr=ar.GetFile();
		CString FileName=cfptr->GetFilePath();
		BeginWaitCursor();
		BOOL res=false;
		m_pDataset = (GDALDataset *) GDALOpen(FileName,GA_ReadOnly);
		if (m_pDataset)	
		{
			int r=1,g=2,b=3,v=-1;
			CBandSelDlg IDlg; 
			IDlg.m_Band=m_pDataset->GetRasterCount();
			IDlg.m_DataType=m_DIB.GetDataType(m_pDataset);
			IDlg.m_R=1;
			IDlg.m_G=2;
			IDlg.m_B=3;
			IDlg.m_NDVI=4;
			IDlg.m_EB=4;
			if(IDlg.DoModal()==IDOK) 
			{	
				r=IDlg.m_R;
				g=IDlg.m_G;
				b=IDlg.m_B;
				v=IDlg.m_NDVI;
				edgeBand=IDlg.m_EB;
			}
			res=m_DIB.CreateDIB(r,g,b,m_pDataset);
		}
		EndWaitCursor();
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
	IDlg.m_Scale=m_HC.hdiff;
	if(IDlg.DoModal()==IDOK) 
	{
		sortDM=!IDlg.m_HM;
		m_HC.hdiff=IDlg.m_Scale;
		m_HC.K=IDlg.m_nKL;
		m_HC.minsize=IDlg.m_nMinsize;
		m_HC.wc=IDlg.m_SpeCoeff;
		m_HC.wp=IDlg.m_PacCoeff;
		//	m_HC.matrix3by3=IDlg.m_n3by3;
		m_HC.maxDelta=IDlg.m_nLamda;
		m_HC.loop=IDlg.m_nLoop;
//		if(IDlg.m_R2L==TRUE&&m_DIB.sampleperpixel==3)
//			m_HC.d_=1;
		m_HC.GetBWArray(IDlg.m_BWArray);
	}
}
void CImageDoc::OnViewOutcome() 
{
	// TODO: Add your command handler code here
	CResultDlg outDlg;
	outDlg.m_nComps=m_HC.comps;
	outDlg.m_nTime=quantum;
//	uint32 en=m_HC.countedge();
//	outDlg.m_RI=m_HC.RI;
	outDlg.m_nQS=m_HC.eval;
	outDlg.DoModal();	 
}


void CImageDoc::OnProcessGuass() 
{
	// TODO: Add your command handler code here
	StartTimer(); 
	int L=m_DIB.GetLength();
	m_HC.EM=new BYTE[L];
	GetEM(m_HC.EM,1);
	m_HC.SetWH(m_DIB.m_nWidth,m_DIB.m_nHeight);

	m_HC.GaussianFilter(0.8f);
	m_DIB.SaveEdge("GaussianSmooth.bmp",m_HC.EM);
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
	CRect   rect1(100,   0, 200, 300);
	CRect   rect2(  0, 100, 300, 200);
	CRect   rect3;
	
	rect2.UnionRect(&rect1, &rect2);
	
	CRect   rectResult(0, 0, 300, 300);
	ASSERT(rectResult == rect2);
	

	m_DIB.origin();
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
	m_HC.EvalQs(m_DIB.m_lpBits,m_DIB.m_nBitCount);
}



void CImageDoc::OnViewRegions() 
{
	// TODO: Add your command handler code here
	//m_HC.LookRegions();
	
	POSITION p=m_pDocTemplate->GetFirstDocPosition();
	CString cur="";//current active document title
	CImageDoc*pD=(CImageDoc*)m_pDocTemplate->GetNextDoc(p);
	CString a=pD->GetTitle();//the first document in the list
	cur=a;
	int i=0;
	while(a.Find("BENCH")==-1)
	{
		pD=(CImageDoc*)m_pDocTemplate->GetNextDoc(p);
		a=pD->GetTitle();//the second document in the list
		++i;
		if(a==cur)
			return;
	}
	if(a==GetTitle())
		return;
	int L=m_DIB.GetLength();
	int *cont=new int[L];
	m_HC.BenchMark(pD->GetTag(cont),pD->m_DIB.GetLength());
	delete []cont;
	//AfxMessageBox(a);
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
	m_HC.BuildData( m_pDataset);
	m_HC.InitiateRegionSet();
	if(sortDM)
	m_HC.MRS(0);
	else
	m_HC.HierClust();
	quantum=ElapsedTime();  	
	m_HC.RegionLabel();
	m_HC.EvalQs(m_DIB.m_lpBits,m_DIB.m_nBitCount);
	m_DIB.LookRegions(m_HC.tag);
	SetModifiedFlag(FALSE);
	UpdateAllViews(NULL);
}

void CImageDoc::OnSegQthc() 
{
	// TODO: Add your command handler code here
	StartTimer(); 
	SetEM();//add edge map
	m_HC.BuildData(m_pDataset);
	m_HC.QTMerge();
	if(sortDM)
	m_HC.MRS();
	else
	m_HC.HierClust();

	quantum=ElapsedTime();  
	m_HC.RegionLabel();
	m_HC.EvalQs(m_DIB.m_lpBits,m_DIB.m_nBitCount);
	m_DIB.LookRegions(m_HC.tag);
	SetModifiedFlag(FALSE);
	UpdateAllViews(NULL);
}
//input:allocated EM 
//output: EM initiated with data for edge detection
BYTE* CImageDoc::GetEM(BYTE *EM,int eB)
{
	int m_CurrentBand=1;
	if (eB<= 0 || eB>m_pDataset->GetRasterCount())
		eB = m_CurrentBand;
	int w=m_pDataset->GetRasterXSize(); //影响的高度，宽度
	int	h=m_pDataset->GetRasterYSize();

	BYTE* buf =	EM;
	GDALRasterBand  *m_pBand=NULL;
	m_pBand = m_pDataset->GetRasterBand(eB);
	if (m_pBand)
	{
		if (CE_None!=m_pBand->RasterIO( GF_Read, 0,0, w, h, buf, w,h, GDT_Byte, 0, 0 ))
		{
			AfxMessageBox("Error getting edge data!");
		}
	}
	return EM;
}
void CImageDoc::SetEM()
{
	if(m_HC.EM)
		return;
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
	m_HC.EM=pD->m_HC.EM;
	pD->m_HC.EM=NULL;
}
//input:tag must be allocated outside such as chc new int [L]
//output:tag initiated with values of benchmark image denoting classes
int*  CImageDoc::GetTag(int * tag)
{
	if(m_pDataset->GetRasterCount()!=1)
	{
		AfxMessageBox("Not benchmark or wrong function call!");
		return NULL;
	}
	int height=m_pDataset->GetRasterYSize();
	int width=m_pDataset->GetRasterXSize(); //影响的高度，宽度	
	int L=height*width;
	GDALRasterBand  *m_pBand=NULL;
	m_pBand = m_pDataset->GetRasterBand(1);

	GInt32* buf =tag;
	if (m_pBand)
	{
		if (CE_None!=m_pBand->RasterIO( GF_Read, 0,0, width, height, buf, width,height, GDT_Int32, 0, 0 ))
		{
			AfxMessageBox("error importing class tag for pixels!");
		}
	}
	return tag;
}

void CImageDoc::OnClassRegprop() 
{
	// TODO: Add your command handler code here
	int L=m_DIB.GetLength();
	m_HC.ndv=new float[L];
	GetNDVI(m_HC.ndv,4);
	m_HC.RegionProps();
	//m_HC.LenWidR();
}

void CImageDoc::OnSegGshc() 
{
	// TODO: Add your command handler code here
	StartTimer(); 
	SetEM();//add edge map
	m_HC.BuildData( m_pDataset);
	int cur=m_HC.SegGraph4();
	m_HC.InitiateRegions();
	if(sortDM)
		m_HC.MRS(cur);
	else
		m_HC.HierClust();
	quantum=ElapsedTime();  
	m_HC.RegionLabel();
	m_HC.EvalQs(m_DIB.m_lpBits,m_DIB.m_nBitCount);
	m_DIB.LookRegions(m_HC.tag);

	SetModifiedFlag(FALSE);
	UpdateAllViews(NULL);

}

void CImageDoc::OnSegGraph() 
{
	// TODO: Add your command handler code here
	StartTimer(); 
	SetEM();//add edge map
	m_HC.BuildData( m_pDataset);
	int cur=m_HC.SegGraph4();
	m_HC.InitiateRegions();
//	m_HC.HierClust();
//	m_HC.MRS(cur);
	quantum=ElapsedTime();  
	m_HC.RegionLabel();
	m_HC.EvalQs(m_DIB.m_lpBits,m_DIB.m_nBitCount);
	m_DIB.LookRegions(m_HC.tag);
	SetModifiedFlag(FALSE);
	UpdateAllViews(NULL);

}

float* CImageDoc::GetNDVI(float *ndv,int vB)
{
	if (vB<= 0 || vB>m_pDataset->GetRasterCount())
		return NULL;
	int sernum=0;
	int w=m_pDataset->GetRasterXSize(); //影像的高度，宽度
	int	h=m_pDataset->GetRasterYSize();
	GDALRasterBand  *m_pBand=NULL;
	m_pBand = m_pDataset->GetRasterBand(vB);
	float*buf=ndv;
	if (m_pBand)
	{
		if (CE_None!=m_pBand->RasterIO( GF_Read, 0,0, w, h, buf, w,h, GDT_Float32, 0, 0 ))
		{
			AfxMessageBox("Error getting vegetable index band!");
		}
	}	
	return ndv;
}

void CImageDoc::OnClassRoadclump() 
{
	// TODO: Add your command handler code here
	BYTE*roadseed=new BYTE[m_DIB.m_nHeight*m_DIB.m_nWidth];
	m_HC.SetWH(m_DIB.m_nWidth,m_DIB.m_nHeight);
	vector<int> endpixel;
	m_HC.RoadSeed(roadseed);
//	m_HC.RoadSeed(roadseed,"F:\\landcruiser\\TIFFIMAGE\\roads.bmp");
	m_HC.RoadThin(roadseed,endpixel);
//	m_HC.RoadLink(roads,endpixel);
	m_HC.RoadExpo(roadseed,endpixel,m_pDataset);
	delete[]roadseed;
//	m_DIB.LookRegions(m_HC.tag,1);
}

void CImageDoc::OnPrepSr() 
{
	// TODO: Add your command handler code here
	m_HC.BuildData(m_pDataset);
	int temp=32;
	m_HC.qttest(temp*2);
	m_DIB.ShowReg(m_HC.tag,temp);
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
	curRegion=m_HC.tag[cy*mw+cx];
	if(curRegion<0)
	{
		AfxMessageBox("tag array of region labels is modified or illegal");
		return -1;
	}
	switch (option)
	{
	case 0:
	default:
		{int area=m_HC.GetSize(curRegion);
		CRect rect=m_HC.GetBoundBox(curRegion);
		m_DIB.lookregion(curRegion,m_HC.tag,rect,area);
		break;}
	case 1:
		m_HC.RegMoment(curRegion);
		break;
	}
	return curRegion;
}

void CImageDoc::OnSegSave() 
{
	CSelPropDlg SDlg;

	if( SDlg.DoModal() == IDOK )
	{	
		m_HC.Type=SDlg.numb;
	}
	// TODO: Add your command handler code here
	m_HC.SaveSeg2(m_pDataset,GetPathName(),m_HC.Type);
}

void CImageDoc::OnFileSaveAs() 
{
	// TODO: Add your command handler code here

	CFileDialog FileDlg( false, "tif", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
	if( FileDlg.DoModal() == IDOK )
	{
		CString PathName =FileDlg.GetPathName();
		m_DIB.SaveToFile(m_pDataset,PathName);
	
	//	OnSaveDocument( PathName);
	
	}
}

void CImageDoc::OnSegQuadTree() 
{
	// TODO: Add your command handler code here
		// TODO: Add your command handler code here
	StartTimer(); 
	SetEM();//add edge map
	m_HC.BuildData(m_pDataset);
	m_HC.QTMerge();
//	m_HC.HierClust();
//	m_HC.MRS();
	quantum=ElapsedTime();  
	m_HC.RegionLabel();
	m_HC.EvalQs(m_DIB.m_lpBits,m_DIB.m_nBitCount);
	m_DIB.LookRegions(m_HC.tag);
	SetModifiedFlag(FALSE);
	UpdateAllViews(NULL);

}

void CImageDoc::OnPrepSobel() 
{
	// TODO: Add your command handler code here

	BeginWaitCursor(); 
	int L=m_DIB.GetLength();
	m_HC.EM=new BYTE[L];
	GetEM(m_HC.EM,edgeBand);

	m_HC.EdgeMag(m_DIB.GetWidth(),m_DIB.GetHeight());

	m_DIB.SaveEdge("edgemap.bmp",m_HC.EM);
	// 恢复光标形状
	EndWaitCursor(); 
}

void CImageDoc::OnPrepCanny() 
{
	// TODO: Add your command handler code here
	CCannyPara edgeDlg;
	edgeDlg.m_sigma=0.8f;
	edgeDlg.m_low=0.4f;
	edgeDlg.m_high=0.79f;

	if(edgeDlg.DoModal()!=IDOK)
		return;
	BeginWaitCursor(); 
	int L=m_DIB.GetLength();
	m_HC.EM=new BYTE[L];
	GetEM(m_HC.EM,edgeBand);
	
	m_HC.EdgePoints(m_DIB.GetWidth(),m_DIB.GetHeight(),edgeDlg.m_sigma,edgeDlg.m_low,edgeDlg.m_high);
	m_DIB.SaveEdge("edgemap.bmp",m_HC.EM);
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
	m_HC.EM=new BYTE[L];
	GetEM(m_HC.EM,edgeBand);
	if(edgeDlg.m_Radio==0)
	m_HC.EdgeSusan(m_DIB.GetWidth(),m_DIB.GetHeight(),edgeDlg.m_BT,false);
	else
	m_HC.EdgeSusan(m_DIB.GetWidth(),m_DIB.GetHeight(),edgeDlg.m_BT,true);	
	m_DIB.SaveEdge("edgemap.bmp",m_HC.EM);
	// 恢复光标形状
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
	m_HC.EM=new BYTE[L];
	GetEM(m_HC.EM,md.m_EB);
	m_HC.SetWH(m_DIB.m_nWidth,m_DIB.m_nHeight);
	m_HC.Morph(md.iPos+1);
	m_DIB.SaveEdge("SKELETON.bmp",m_HC.EM);
}


void CImageDoc::OnClassPreview() 
{
	// TODO: Add your command handler code here
	CSelPropDlg SDlg;
	CPreviewDlg IDlg;

	m_HC.GetPreviewMask(IDlg.mask);
	SDlg.m_top=IDlg.mask.top;
	SDlg.m_bot=IDlg.mask.bottom;
	SDlg.m_left=IDlg.mask.left;
	SDlg.m_right=IDlg.mask.right;
	if( SDlg.DoModal() == IDOK )
	{
		if(SDlg.numb<0)
			SDlg.numb=0;
		m_HC.Type=SDlg.numb;
		IDlg.mask.top=SDlg.m_top;
		IDlg.mask.bottom=SDlg.m_bot;
		IDlg.mask.left=SDlg.m_left;
		IDlg.mask.right=SDlg.m_right;
	}
	else
		return;
	
	if( IDlg.DoModal() == IDOK )
	{		
		return;
	}
}

void CImageDoc::OnSegSavetour() 
{
	// TODO: Add your command handler code here
	//m_DIB.SaveToFile("contour seg.bmp");
	m_HC.SaveSeg(m_pDataset,GetPathName());
}
