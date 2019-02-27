// ResultDlg.cpp : implementation file
//

#include "stdafx.h"
#include "image.h"
#include "ResultDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResultDlg dialog


CResultDlg::CResultDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CResultDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CResultDlg)
	m_nComps = 0;
	m_nTime = 0.0;
	m_nQS = 0.0f;
	//}}AFX_DATA_INIT
}


void CResultDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResultDlg)
	DDX_Text(pDX, IDC_EDIT_COMPS, m_nComps);
	DDV_MinMaxInt(pDX, m_nComps, 1, 2147483647);
	DDX_Text(pDX, IDC_EDIT_TIME, m_nTime);
	DDV_MinMaxDouble(pDX, m_nTime, 0., 1000.);
	DDX_Text(pDX, IDC_EDIT_QS, m_nQS);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResultDlg, CDialog)
	//{{AFX_MSG_MAP(CResultDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResultDlg message handlers
