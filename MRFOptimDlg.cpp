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

// MRFOptimDlg.cpp : implementation file
//

#include "stdafx.h"
#include "image.h"
#include "MRFOptimDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMRFOptimDlg dialog


CMRFOptimDlg::CMRFOptimDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMRFOptimDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMRFOptimDlg)
	m_Beta = 0.0;
	m_C = 0.0;
	m_ClsNum = 0;
	m_t = 0.0;
	m_T0 = 0.0;
	m_Alpha = 0.0;
	//}}AFX_DATA_INIT
}


void CMRFOptimDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMRFOptimDlg)
	DDX_Text(pDX, IDC_EDIT_BETA, m_Beta);
	DDX_Text(pDX, IDC_EDIT_C, m_C);
	DDX_Text(pDX, IDC_EDIT_NC, m_ClsNum);
	DDX_Text(pDX, IDC_EDIT_T, m_t);
	DDX_Text(pDX, IDC_EDIT_T0, m_T0);
	DDX_Text(pDX, IDC_EDIT_ALPHA, m_Alpha);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMRFOptimDlg, CDialog)
	//{{AFX_MSG_MAP(CMRFOptimDlg)
	ON_CBN_SELENDOK(IDC_COMBO_MRF, OnSelendokComboMRF)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMRFOptimDlg message handlers

BOOL CMRFOptimDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	iPos=0;
	// TODO: Add extra initialization here
	((CComboBox*)GetDlgItem(IDC_COMBO_MRF))->SetCurSel(0);
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CMRFOptimDlg::OnSelendokComboMRF() 
{
	// TODO: Add your control notification handler code here
	iPos=((CComboBox*)GetDlgItem(IDC_COMBO_MRF))->GetCurSel();//当前选中的行。

}
