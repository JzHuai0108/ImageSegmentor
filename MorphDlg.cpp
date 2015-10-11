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
