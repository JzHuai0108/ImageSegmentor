// PreviewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "image.h"
#include "PreviewDlg.h"
#include "ImageDoc.h"
#include "ImageView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPreviewDlg dialog


CPreviewDlg::CPreviewDlg(CWnd* pParent /*=NULL*/)
: CDialog(CPreviewDlg::IDD, pParent)
{
	m_nMaxColor=0;
	bGrayValue = 128;
	m_RMax=255;
	m_RMin=0;
	m_nCur=128;
	for (int i=0; i<GRAYLEVELS; i++)
	{	
		dataChannel[i] = 0;
	}


	InitCoordinate();
	//{{AFX_DATA_INIT(CPreviewDlg)
	//}}AFX_DATA_INIT
}


void CPreviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPreviewDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPreviewDlg, CDialog)
	//{{AFX_MSG_MAP(CPreviewDlg)
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPreviewDlg message handlers

BOOL CPreviewDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	pCount=(CStatic*)GetDlgItem(IDC_STATIC_COUNT); 
	pThresh=(CStatic*)GetDlgItem(IDC_STATIC_THRESH); 
	pMax=(CStatic*)GetDlgItem(IDC_STATIC_MAX); 
	pMin=(CStatic*)GetDlgItem(IDC_STATIC_MIN); 
	p_Slider=(CSliderCtrl*)GetDlgItem(IDC_SLIDER1);
	pCount->SetWindowText("Count");
	pThresh->SetWindowText("Threshold");
	pMax->SetWindowText("255");
	pMin->SetWindowText("0");

	// TODO: Add extra initialization here

	p_Slider->SetRange(0, 255, TRUE );
	p_Slider->SetTic( 50 );
	p_Slider->SetTicFreq( 8 );//origin 1,1,1,now 8,4,4
	p_Slider->SetLineSize( 1 );//for increment by arrow move 
	p_Slider->SetPageSize( 4 );//for increment by dragging by mouse or by pageup ordown
	p_Slider->SetPos( m_nCur );
	pView = (CImageView*)GetMainFrame()->GetActiveFrame()->GetActiveView();
	((CImageDoc*)(pView->GetDocument()))->m_HC.GetHistogramData(dataChannel,m_RMax,m_RMin);

	curMin=m_RMin;
	CString msg;
	msg.Format("%.4f",m_RMin);
	pMin->SetWindowText(msg);
	msg.Format("%.4f",m_RMax);
	pMax->SetWindowText(msg);
	msg.Format("%.4f",curMin);
	pThresh->SetWindowText(msg);
	pPic=(CStatic*)GetDlgItem(IDC_STATIC_PICBOX); 

	pPic->ModifyStyle(0,SS_BITMAP);
	if(mask.IsRectEmpty())
	{
		AfxMessageBox("The picture box for preview dialog is not initiated correctly! ");
	}


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPreviewDlg::InitCoordinate()
{
	iArrowWidth = 2;
	iArrowLen = 15;
	iFlagLen = 10;
	iTopReserve = 10;
	iReserve = 10;

	iLeft = 42;
	iBottom = 20 ;
	rectDraw = CRect( 0, 0, 400, 300);
	nMaxHeight = rectDraw.bottom - rectDraw.top - iBottom - iReserve - iArrowLen - iTopReserve;
	nMaxWidth  = rectDraw.right - rectDraw.left - iLeft   - iReserve - iArrowLen - iTopReserve;
	ihStep = nMaxWidth / 4;
	ivStep = nMaxHeight / 4;

	//第一条垂直线标记
	rect_Coordination[0].bMove = TRUE;
	rect_Coordination[0].point = CPoint(iLeft, rectDraw.bottom - iBottom - ivStep );
	rect_Coordination[1].bMove = FALSE;
	rect_Coordination[1].point = CPoint(iLeft + iFlagLen, rectDraw.bottom - iBottom - ivStep );
	//第二条垂直线标记
	rect_Coordination[2].bMove = TRUE;
	rect_Coordination[2].point = CPoint(iLeft, rectDraw.bottom - iBottom - 2*ivStep );
	rect_Coordination[3].bMove = FALSE;
	rect_Coordination[3].point = CPoint(iLeft + iFlagLen, rectDraw.bottom - iBottom - 2*ivStep );
	//第三条垂直线标记
	rect_Coordination[4].bMove = TRUE;
	rect_Coordination[4].point = CPoint(iLeft, rectDraw.bottom - iBottom - 3*ivStep );
	rect_Coordination[5].bMove = FALSE;
	rect_Coordination[5].point = CPoint(iLeft + iFlagLen, rectDraw.bottom - iBottom - 3*ivStep );
	//第四条垂直线标记
	rect_Coordination[6].bMove = TRUE;
	rect_Coordination[6].point = CPoint(iLeft, rectDraw.bottom - iBottom - 4*ivStep );
	rect_Coordination[7].bMove = FALSE;
	rect_Coordination[7].point = CPoint(iLeft + iFlagLen, rectDraw.bottom - iBottom - 4*ivStep );

	//第一条水平线标记
	rect_Coordination[8].bMove = TRUE;
	rect_Coordination[8].point = CPoint(iLeft + ihStep, rectDraw.bottom - iBottom );
	rect_Coordination[9].bMove = FALSE;
	rect_Coordination[9].point = CPoint(iLeft + ihStep, rectDraw.bottom - iBottom - iFlagLen);
	//第二条水平线标记
	rect_Coordination[10].bMove = TRUE;
	rect_Coordination[10].point = CPoint(iLeft + 2*ihStep, rectDraw.bottom - iBottom );
	rect_Coordination[11].bMove = FALSE;
	rect_Coordination[11].point = CPoint(iLeft + 2*ihStep, rectDraw.bottom - iBottom - iFlagLen);
	//第三条水平线标记
	rect_Coordination[12].bMove = TRUE;
	rect_Coordination[12].point = CPoint(iLeft + 3*ihStep, rectDraw.bottom - iBottom );
	rect_Coordination[13].bMove = FALSE;
	rect_Coordination[13].point = CPoint(iLeft + 3*ihStep, rectDraw.bottom - iBottom - iFlagLen);
	//第四条水平线标记
	rect_Coordination[14].bMove = TRUE;
	rect_Coordination[14].point = CPoint(iLeft + 4*ihStep, rectDraw.bottom - iBottom );
	rect_Coordination[15].bMove = FALSE;
	rect_Coordination[15].point = CPoint(iLeft + 4*ihStep, rectDraw.bottom - iBottom - iFlagLen);


	rect_Coordination[16].bMove = TRUE;
	rect_Coordination[16].point = CPoint(iLeft, iReserve );
	rect_Coordination[17].bMove = FALSE;
	rect_Coordination[17].point = CPoint(iLeft, rectDraw.bottom - iBottom);	//画垂直线
	rect_Coordination[18].bMove = FALSE;
	rect_Coordination[18].point = CPoint(rectDraw.right - iReserve, rectDraw.bottom - iBottom);	//画水平线
	//画垂直线箭头
	rect_Coordination[19].bMove = TRUE;
	rect_Coordination[19].point = CPoint(iLeft - iArrowWidth, iArrowLen + iReserve );
	rect_Coordination[20].bMove = FALSE;
	rect_Coordination[20].point = CPoint(iLeft, iReserve );
	rect_Coordination[21].bMove = FALSE;
	rect_Coordination[21].point = CPoint(iLeft + iArrowWidth, iArrowLen + iReserve );

	//画水平线箭头
	rect_Coordination[22].bMove = TRUE;
	rect_Coordination[22].point = CPoint(rectDraw.right - iReserve - iArrowLen, rectDraw.bottom - iBottom - iArrowWidth );
	rect_Coordination[23].bMove = FALSE;
	rect_Coordination[23].point = CPoint(rectDraw.right - iReserve, rectDraw.bottom - iBottom );
	rect_Coordination[24].bMove = FALSE;
	rect_Coordination[24].point = CPoint(rectDraw.right - iReserve - iArrowLen, rectDraw.bottom - iBottom + iArrowWidth );

	startleft = 2;
	startbottom = rect_Coordination[17].point.y + 2;

}

void CPreviewDlg::DrawVertCoordination(CDC* pDC, float vertStep)
{
	pDC->SelectStockObject( BLACK_PEN );
	pDC->SetBkColor( RGB(255,255,255) );
	float ivalue = 0;	CString str;
	ivalue += vertStep;		str.Format("%.4f", ivalue);
	pDC->TextOut( startleft, rect_Coordination[0].point.y, str);
	ivalue += vertStep;		str.Format("%.4f", ivalue);
	pDC->TextOut( startleft, rect_Coordination[2].point.y, str);
	ivalue += vertStep;		str.Format("%.4f", ivalue);
	pDC->TextOut( startleft, rect_Coordination[4].point.y, str);
	ivalue += vertStep;		str.Format("%.4f", ivalue);
	pDC->TextOut( startleft, rect_Coordination[6].point.y, str);
}

void CPreviewDlg::DrawHorizonVertCoordination(CDC* pDC)
{
	pDC->SelectStockObject( BLACK_PEN );
	pDC->SetBkColor( RGB(255,255,255) );
	float ivalue = m_RMin;	CString str;
	float horistep=(m_RMax-m_RMin)/4;
	str.Format("%.4f", ivalue);
	pDC->TextOut( rect_Coordination[17].point.x, startbottom, str);
	ivalue += horistep;		str.Format("%.4f", ivalue);
	pDC->TextOut( rect_Coordination[8].point.x, startbottom, str);
	ivalue += horistep;		str.Format("%.4f", ivalue);
	pDC->TextOut( rect_Coordination[10].point.x, startbottom, str);
	ivalue += horistep;		str.Format("%.4f", ivalue);
	pDC->TextOut( rect_Coordination[12].point.x, startbottom, str);
	ivalue += horistep;		str.Format("%.4f", ivalue);
	pDC->TextOut( rect_Coordination[14].point.x, startbottom, str);
}

void CPreviewDlg::DrawColorHistogram(CDC* pDC, int *colorArray)
{
	int y;
	y = rectDraw.bottom - iBottom;
	if (m_nMaxColor)
		y -= colorArray[0]*nMaxHeight/m_nMaxColor;
//		y -= RedChannel[0]*nMaxHeight/m_nMaxColor;
	pDC->MoveTo( iLeft , y);
	for ( int i = 1; i<256; i++)
	{
		y = rectDraw.bottom - iBottom;
		if (m_nMaxColor)
			y -= colorArray[i]*nMaxHeight/m_nMaxColor;
		pDC->LineTo( iLeft + (int)(i*nMaxWidth/256), y);
	}
}

void CPreviewDlg::DrawCurrentFavorPosition(CDC* pDC)
{
	pDC->SelectStockObject( 7 );
	pDC->MoveTo( iLeft + (int)(m_nCur*nMaxWidth/256), iTopReserve + iReserve );
	pDC->LineTo( iLeft + (int)(m_nCur*nMaxWidth/256), rectDraw.bottom - iBottom );
}

void CPreviewDlg::DisplayRGBCounts()
{
	CString msg;
	msg.Format("Number:%d", dataChannel[m_nCur] );	
	pCount->SetWindowText(msg);
}

void CPreviewDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

//	Init();
	int	i;
	
	for ( i=0; i<GRAYLEVELS; i++ )
	{
		m_nMaxColor = max(m_nMaxColor, dataChannel[i]);	
	}	
	int nMaxHeight = rectDraw.Height() - 40;
//	CPaintDC dc(this);
//	CClientDC dc(this);

	dc.SelectStockObject( WHITE_BRUSH );
	dc.Rectangle( rectDraw );
	//draw the chart axis
	CPen BlackPen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen* pOldPen = dc.SelectObject(&BlackPen);

	for( i = 0; i < NUM_COORDINATEION; i++ )
		if( rect_Coordination[i].bMove )
			dc.MoveTo( rect_Coordination[i].point );
		else
			dc.LineTo( rect_Coordination[i].point );

	DrawHorizonVertCoordination( &dc );
	DrawVertCoordination(  &dc , m_nMaxColor / 4.0f );

	CPen GreenPen(PS_SOLID, 1, RGB(0, 255, 0));
			pOldPen = dc.SelectObject(&GreenPen);
			DrawColorHistogram(  &dc , dataChannel );
			dc.SelectObject(pOldPen);	
	
	DrawCurrentFavorPosition(  &dc );
	DisplayRGBCounts();

	CDialog::OnPaint();
	// Do not call CDialog::OnPaint() for painting messages
}
void CPreviewDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	if( bGrayValue == p_Slider->GetPos ())
		return;
	curMin= m_RMin+m_nCur*(m_RMax-m_RMin)/256;
	CString msg;
	msg.Format("%.4f", curMin);
	pThresh->SetWindowText(msg);
	m_nCur = p_Slider->GetPos ();
	bGrayValue=m_nCur;
	InvalidateRect( rectDraw );

	CvSize sz = cvSize(mask.Width(),mask.Height());
	IplImage* img = cvCreateImage( sz, 8,3 ); 
	RegionThresh(&(((CImageDoc*)(pView->GetDocument()))->m_HC),curMin,mask,img);

	CRect rect;
	pPic->GetClientRect(&rect);
	CDC *pDC=pPic->GetDC();
	HDC hDC=pDC->GetSafeHdc();
	
	CvvImage cvvImg;
	cvvImg.CopyOf(img);
	cvvImg.DrawToHDC(hDC,&rect);
	
    cvReleaseImage( &img);
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


