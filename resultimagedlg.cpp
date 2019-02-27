// ResultImageDlg1.cpp : implementation file
//

#include "stdafx.h"
#include "image.h"
#include "ResultImageDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResultImageDlg dialog


CResultImageDlg::CResultImageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CResultImageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CResultImageDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	isImageOk = FALSE;
}
void CResultImageDlg::SetImage(BYTE* indata, INT width, INT height, int nBits)
//ÉèÖÃÍ¼Ïñ£»
{

	myImageObj.CreateDIBFromBits(width, height, indata,nBits);
	CWnd* myMatrix = GetDlgItem(IDC_RESULTIMAGE);
	CRect matrixrect;
	myMatrix->GetClientRect(matrixrect);
	myMatrix->ClientToScreen(matrixrect);
	ScreenToClient(matrixrect);
	
	CClientDC dc(this);
	
	myImageObj.Stretch( dc.GetSafeHdc(), matrixrect.left
		, matrixrect.top, matrixrect.right-matrixrect.left
		, matrixrect.bottom-matrixrect.top,DIB_RGB_COLORS, SRCCOPY);
	isImageOk = TRUE;
}

void CResultImageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResultImageDlg)
	DDX_Control(pDX, IDC_RESULTIMAGE, myResultImage);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResultImageDlg, CDialog)
	//{{AFX_MSG_MAP(CResultImageDlg)
	ON_WM_SIZE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResultImageDlg message handlers

void CResultImageDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
}

void CResultImageDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	if (isImageOk)
	{
		CWnd* myMatrix = GetDlgItem(IDC_RESULTIMAGE);
		CRect matrixrect;
		myMatrix->GetClientRect(matrixrect);
		myMatrix->ClientToScreen(matrixrect);
		ScreenToClient(matrixrect);
		
		CClientDC dc(this);
		
		myImageObj.Stretch( dc.GetSafeHdc(), matrixrect.left
			, matrixrect.top, matrixrect.right-matrixrect.left
			, matrixrect.bottom-matrixrect.top, DIB_RGB_COLORS, SRCCOPY);	
	}	
	// Do not call CDialog::OnPaint() for painting messages
	// Do not call CDialog::OnPaint() for painting messages
}
