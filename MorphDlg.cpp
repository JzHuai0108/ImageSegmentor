// MorphDlg.cpp : implementation file
//

#include "stdafx.h"
#include "image.h"
#include "MorphDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMorphDlg dialog


CMorphDlg::CMorphDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMorphDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMorphDlg)
	m_EB = 0;
	//}}AFX_DATA_INIT
}


void CMorphDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMorphDlg)
	DDX_Text(pDX, IDC_EDIT_EBAND, m_EB);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMorphDlg, CDialog)
	//{{AFX_MSG_MAP(CMorphDlg)
	ON_CBN_SELENDOK(IDC_COMBO_MORPH, OnSelendokComboMorph)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMorphDlg message handlers

BOOL CMorphDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_EB=0;
	iPos=0;
	// TODO: Add extra initialization here
	((CComboBox*)GetDlgItem(IDC_COMBO_MORPH))->SetCurSel(0);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMorphDlg::OnSelendokComboMorph() 
{
	// TODO: Add your control notification handler code here
	iPos=((CComboBox*)GetDlgItem(IDC_COMBO_MORPH))->GetCurSel();//当前选中的行。
}
