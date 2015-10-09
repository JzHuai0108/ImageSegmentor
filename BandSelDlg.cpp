// BandSelDlg.cpp : implementation file
//

#include "stdafx.h"
#include "image.h"
#include "BandSelDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBandSelDlg dialog


CBandSelDlg::CBandSelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBandSelDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBandSelDlg)
	m_B = 0;
	m_Band = 0;
	m_G = 0;
	m_R = 0;
	m_NDVI = 0;
	m_EB = 0;
	m_DataType = _T("");
	//}}AFX_DATA_INIT
}


void CBandSelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBandSelDlg)
	DDX_Text(pDX, IDC_EDIT_B, m_B);
	DDX_Text(pDX, IDC_EDIT_BAND, m_Band);
	DDX_Text(pDX, IDC_EDIT_G, m_G);
	DDX_Text(pDX, IDC_EDIT_R, m_R);
	DDX_Text(pDX, IDC_EDIT_NDVI, m_NDVI);
	DDX_Text(pDX, IDC_EDIT_EB, m_EB);
	DDX_Text(pDX, IDC_EDIT_TYPE, m_DataType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBandSelDlg, CDialog)
	//{{AFX_MSG_MAP(CBandSelDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBandSelDlg message handlers
