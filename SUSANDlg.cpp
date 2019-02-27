// SUSANDlg.cpp : implementation file
//

#include "stdafx.h"
#include "image.h"
#include "SUSANDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSUSANDlg dialog


CSUSANDlg::CSUSANDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSUSANDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSUSANDlg)
	m_BT = 0.0f;
	m_Radio = 0;
	//}}AFX_DATA_INIT
}


void CSUSANDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSUSANDlg)
	DDX_Text(pDX, IDC_EDIT_BT, m_BT);
	DDV_MinMaxFloat(pDX, m_BT, 10.f, 3000.f);
	DDX_Radio(pDX, IDC_RADIO_GAUSSIAN, m_Radio);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSUSANDlg, CDialog)
	//{{AFX_MSG_MAP(CSUSANDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSUSANDlg message handlers




