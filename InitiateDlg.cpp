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

// InitiateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "image.h"
#include "InitiateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInitiateDlg dialog


CInitiateDlg::CInitiateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInitiateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInitiateDlg)
	m_nKL = 0;
	m_nMinsize = 0;
	m_nLamda = 0.0f;
	m_nLoop = 0;
	m_Scale = 0.0f;
	m_PacCoeff = 0.0f;
	m_SpeCoeff = 0.0f;
	m_HM =0;
	m_BWArray = _T("");
	//}}AFX_DATA_INIT
}


void CInitiateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInitiateDlg)
	DDX_Text(pDX, IDC_EDIT_K, m_nKL);
	DDX_Text(pDX, IDC_EDIT_MINISIZE, m_nMinsize);
	DDV_MinMaxUInt(pDX, m_nMinsize, 0, 1000);
	DDX_Text(pDX, IDC_EDIT_LAMDA, m_nLamda);
	DDV_MinMaxFloat(pDX, m_nLamda, 0.f, 1.e+008f);
	DDX_Text(pDX, IDC_EDIT_LOOP, m_nLoop);
	DDX_Text(pDX, IDC_EDIT_SCALE, m_Scale);
	DDX_Text(pDX, IDC_EDIT_PACWEI, m_PacCoeff);
	DDX_Text(pDX, IDC_EDIT_SPEWEI, m_SpeCoeff);
	DDX_Radio(pDX, IDC_RADIO_DM, m_HM);
	DDX_Text(pDX, IDC_EDIT_LWARRAY, m_BWArray);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInitiateDlg, CDialog)
	//{{AFX_MSG_MAP(CInitiateDlg)
	ON_CBN_SELENDOK(IDC_COMBO_METRIC, OnSelendokComboMetric)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInitiateDlg message handlers





BOOL CInitiateDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	iPos=0;
	// TODO: Add extra initialization here
	((CComboBox*)GetDlgItem(IDC_COMBO_METRIC))->SetCurSel(0);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInitiateDlg::OnSelendokComboMetric() 
{
	// TODO: Add your control notification handler code here
		iPos=((CComboBox*)GetDlgItem(IDC_COMBO_METRIC))->GetCurSel();//当前选中的行。
}
