/**
* This file is part of ImageSegmentor.
*
* Copyright (C) 2012 Jianzhu Huai <huai dot 3 at osu dot edu> (The Ohio State University)
*
* ImageSegmentor is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ImageSegmentor is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ImageSegmentor. If not, see <http://www.gnu.org/licenses/>.
*/

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
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelPropDlg message handlers

BOOL CSelPropDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	numb=0;
	((CComboBox*)GetDlgItem(IDC_COMBO_SELPROP))->SetCurSel(0);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}





void CSelPropDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	numb= ((CComboBox*)GetDlgItem(IDC_COMBO_SELPROP))->GetCurSel(); 
	CString str;
	if(numb<1)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO_SELPROP))->GetWindowText(str);	
		numb=atoi((const char*)str);
		if(str=="Band Average")
			numb=-1;
	}
	else
	{
		numb+=10;
	}	
	// TODO: Add your message handler code here
	
}
