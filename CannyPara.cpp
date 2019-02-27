// CannyPara.cpp : implementation file
//

#include "stdafx.h"
#include "image.h"
#include "CannyPara.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCannyPara dialog


CCannyPara::CCannyPara(CWnd* pParent /*=NULL*/)
	: CDialog(CCannyPara::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCannyPara)
	m_high = 0.0f;
	m_low = 0.0f;
	//}}AFX_DATA_INIT
}


void CCannyPara::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCannyPara)
	DDX_Text(pDX, IDC_EDIT_HIGH, m_high);
	DDX_Text(pDX, IDC_EDIT_LOW, m_low);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCannyPara, CDialog)
	//{{AFX_MSG_MAP(CCannyPara)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCannyPara message handlers
