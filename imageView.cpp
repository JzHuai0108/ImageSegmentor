// imageView.cpp : implementation of the CImageView class
//

#include "stdafx.h"
#include "image.h"

#include "imageDoc.h"
#include "imageView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImageView

IMPLEMENT_DYNCREATE(CImageView, CScrollView)

BEGIN_MESSAGE_MAP(CImageView, CScrollView)
	//{{AFX_MSG_MAP(CImageView)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageView construction/destruction

CImageView::CImageView()
{
	// TODO: add construction code here
	m_nLengthUnit = MM_TEXT;
}

CImageView::~CImageView()
{
}

BOOL CImageView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CImageView drawing

void CImageView::OnDraw(CDC* pDC)
{
	CImageDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
	int w,h;
	// 获取位图的显示宽度
	w = int(pDoc->m_DIB.GetWidth()*pDoc->m_dRoom);
	// 获取位图的显示宽度
	h =int(pDoc->m_DIB.GetHeight()*pDoc->m_dRoom);
	// 显示位图
	pDoc->m_DIB.Stretch(pDC->m_hDC, 0, 0, w, h, 0, 0, 
		pDoc->m_DIB.GetWidth(), pDoc->m_DIB.GetHeight(), 
		DIB_RGB_COLORS, SRCCOPY);
}

void CImageView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	
	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx =1440; sizeTotal.cy =1500;
	SetScrollSizes(MM_TEXT, sizeTotal);
	CImageDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CString str,msg; 
	str.Format("%d,%d",pDoc->m_DIB.m_nWidth,pDoc->m_DIB.m_nHeight);
	msg.Format("%d位",pDoc->m_DIB.m_nBitCount);
	CMainFrame* pMainFrame= (CMainFrame*)( AfxGetApp()->m_pMainWnd);
	ASSERT_KINDOF(CMainFrame, pMainFrame);
    pMainFrame->m_wndStatusBar.SetPaneText(PANE_IMAGE_SIZE, str);
    pMainFrame->m_wndStatusBar.SetPaneText(PANE_BIT_COUNT, msg);
	CClientDC dc(this);
	m_nLengthUnit=dc.GetMapMode();
	SetStatusBarLengthUnit(m_nLengthUnit);
}

/////////////////////////////////////////////////////////////////////////////
// CImageView printing

BOOL CImageView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CImageView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CImageView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CImageView diagnostics

#ifdef _DEBUG
void CImageView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CImageView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CImageDoc* CImageView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CImageDoc)));
	return (CImageDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CImageView message handlers
void CImageView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CString str;
 	CClientDC dc(this);
 	OnPrepareDC(&dc);
 	dc.DPtoLP(&point);
	str.Format("X:%d,Y:%d",point.x,point.y);
	CMainFrame* pMainFrame= (CMainFrame*)( AfxGetApp()->m_pMainWnd);
	ASSERT_KINDOF(CMainFrame, pMainFrame);
    pMainFrame->m_wndStatusBar.SetPaneText(PANE_CURSOR_POSITION, str);
	CScrollView::OnMouseMove(nFlags, point);
}
void CImageView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	//SetCapture();viewfeatures==true
	CImageDoc*pDoc=GetDocument();
	CString rid,rs,rnc;
	if(pDoc->m_HC.tag)
	{
		CClientDC dc(this);
 		OnPrepareDC(&dc);
 		dc.DPtoLP(&point);
		int miss=pDoc->lookregion(point.x,point.y,1);
//		pDoc->curRegion=miss;
		rid.Format("ID:%d",miss);
	//	rs.Format("%d",pDoc->m_HC.S[miss].size);
	//	rnc.Format("%d",pDoc->m_HC.S[miss].perim);
	}
	CMainFrame* pMainFrame= (CMainFrame*)( AfxGetApp()->m_pMainWnd);
	ASSERT_KINDOF(CMainFrame, pMainFrame);
    pMainFrame->m_wndStatusBar.SetPaneText(PANE_REGION_ID, rid);
//	pMainFrame->m_wndStatusBar.SetPaneText(PANE_REGION_SIZE, rs);
//	pMainFrame->m_wndStatusBar.SetPaneText(PANE_RN_COUNT, rnc);
 	CScrollView::OnLButtonDown(nFlags, point);
	Invalidate();
}
void CImageView::SetStatusBarLengthUnit(int nUnit)
{
	CString str;
	switch (m_nLengthUnit)
	{
	case MM_HIENGLISH:
		str.LoadString(IDS_LENGTH_UNIT_INCH);
		break;
	case MM_HIMETRIC:
		str.LoadString(IDS_LENGTH_UNIT_MM);
		break;
	case MM_TEXT:
	default:
		str.LoadString(IDS_LENGTH_UNIT_PIXEL);
		break;
	}
	CMainFrame* pMainFrame= (CMainFrame*)( AfxGetApp()->m_pMainWnd);
	ASSERT_KINDOF(CMainFrame, pMainFrame);
    pMainFrame->m_wndStatusBar.SetPaneText(PANE_LENGTH_UNIT, str);
}