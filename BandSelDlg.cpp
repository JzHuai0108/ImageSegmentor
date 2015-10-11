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
