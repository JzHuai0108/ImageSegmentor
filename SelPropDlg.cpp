// SelPropDlg.cpp : implementation file
//

#include "stdafx.h"
#include "image.h"
#include "SelPropDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelPropDlg dialog


CSelPropDlg::CSelPropDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSelPropDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelPropDlg)
	m_bot = 0;
	m_left = 0;
	m_right = 0;
	m_top = 0;
	//}}AFX_DATA_INIT
}


void CSelPropDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelPropDlg)
	DDX_Text(pDX, IDC_EDIT_BOT, m_bot);
	DDX_Text(pDX, IDC_EDIT_LEFT, m_left);
	DDX_Text(pDX, IDC_EDIT_RIGHT, m_right);
	DDX_Text(pDX, IDC_EDIT_TOP, m_top);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelPropDlg, CDialog)
	//{{AFX_MSG_MAP(CSelPropDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelPropDlg message handlers

BOOL CSelPropDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	pSP=(CComboBox*)GetDlgItem(IDC_COMBO_SELPROP);	
	pSP->SetCurSel(0);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelPropDlg::OnOK() 
{
	// TODO: Add extra validation here
	numb= pSP->GetCurSel(); 
	CString str;
	if(numb<1)
	{
		pSP->GetWindowText(str);	
		numb=atoi((const char*)str);
		if(str=="Band Average")
			numb=-1;
	}
	else
	{
		numb+=10;
	}	
	CDialog::OnOK();
}
