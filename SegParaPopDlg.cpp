// SegParaPopDlg.cpp : implementation file
//

#include "stdafx.h"
#include "myimagedb.h"
#include "SegParaPopDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSegParaPopDlg dialog


CSegParaPopDlg::CSegParaPopDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSegParaPopDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSegParaPopDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSegParaPopDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSegParaPopDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSegParaPopDlg, CDialog)
	//{{AFX_MSG_MAP(CSegParaPopDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSegParaPopDlg message handlers
