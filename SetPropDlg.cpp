// SetPropDlg.cpp : implementation file
//

#include "stdafx.h"
#include "image.h"
#include "SetPropDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetPropDlg dialog


CSetPropDlg::CSetPropDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetPropDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetPropDlg)
	m_MaxLWR = 0.0f;
	m_MinArea = 0.0f;
	m_MinCmpct = 0.0f;
	m_MinWid = 0.0f;
	m_MaxArea = 0.0f;
	//}}AFX_DATA_INIT
}


void CSetPropDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetPropDlg)
	DDX_Text(pDX, IDC_EDIT_MAXLWR, m_MaxLWR);
	DDV_MinMaxFloat(pDX, m_MaxLWR, 0.f, 12.f);
	DDX_Text(pDX, IDC_EDIT_MINAREA, m_MinArea);
	DDV_MinMaxFloat(pDX, m_MinArea, 10.f, 1.e+009f);
	DDX_Text(pDX, IDC_EDIT_MINCMPCT, m_MinCmpct);
	DDV_MinMaxFloat(pDX, m_MinCmpct, 0.f, 0.9f);
	DDX_Text(pDX, IDC_EDIT_MINWID, m_MinWid);
	DDV_MinMaxFloat(pDX, m_MinWid, 1.f, 10000.f);
	DDX_Text(pDX, IDC_EDIT_MAXAREA, m_MaxArea);
	DDV_MinMaxFloat(pDX, m_MaxArea, 10.f, 1.e+009f);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetPropDlg, CDialog)
	//{{AFX_MSG_MAP(CSetPropDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetPropDlg message handlers
