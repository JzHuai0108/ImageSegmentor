// MFeatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "image.h"
#include "MFeatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFeatDlg dialog


CMFeatDlg::CMFeatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMFeatDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMFeatDlg)
	m_minDiff = 0.0f;
	m_maxDiff = 0.0f;
	m_level = 0;
	m_levelUse = _T("");
	m_bandUse = _T("");
	//}}AFX_DATA_INIT
}


void CMFeatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFeatDlg)
	DDX_Text(pDX, IDC_EDIT_MINDIFF, m_minDiff);
	DDV_MinMaxFloat(pDX, m_minDiff, 0.f, 1000.f);
	DDX_Text(pDX, IDC_EDIT_MAXDIFF, m_maxDiff);
	DDV_MinMaxFloat(pDX, m_maxDiff, 0.f, 1.e+020f);
	DDX_Text(pDX, IDC_EDIT_LEVEL, m_level);
	DDV_MinMaxInt(pDX, m_level, 0, 10);
	DDX_Text(pDX, IDC_EDIT_LEVELUSE, m_levelUse);
	DDX_Text(pDX, IDC_EDIT_BANDWT, m_bandUse);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMFeatDlg, CDialog)
	//{{AFX_MSG_MAP(CMFeatDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFeatDlg message handlers
