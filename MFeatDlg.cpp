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

// MFeatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "image.h"
#include "MFeatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFeatDlg dialog


CMFeatDlg::CMFeatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMFeatDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMFeatDlg)
	m_minDiff = 0.0f;
	m_maxDiff = 0.0f;
	m_level = 0;
	m_levelUse = _T("");
	m_bandUse = _T("");
	//}}AFX_DATA_INIT
}


void CMFeatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFeatDlg)
	DDX_Text(pDX, IDC_EDIT_MINDIFF, m_minDiff);
	DDV_MinMaxFloat(pDX, m_minDiff, 0.f, 1000.f);
	DDX_Text(pDX, IDC_EDIT_MAXDIFF, m_maxDiff);
	DDV_MinMaxFloat(pDX, m_maxDiff, 0.f, 1.e+020f);
	DDX_Text(pDX, IDC_EDIT_LEVEL, m_level);
	DDV_MinMaxInt(pDX, m_level, 0, 10);
	DDX_Text(pDX, IDC_EDIT_LEVELUSE, m_levelUse);
	DDX_Text(pDX, IDC_EDIT_BANDWT, m_bandUse);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMFeatDlg, CDialog)
	//{{AFX_MSG_MAP(CMFeatDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFeatDlg message handlers
