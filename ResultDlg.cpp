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

// ResultDlg.cpp : implementation file
//

#include "stdafx.h"
#include "image.h"
#include "ResultDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResultDlg dialog


CResultDlg::CResultDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CResultDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CResultDlg)
	m_nComps = 0;
	m_nTime = 0.0;
	m_nQS = 0.0f;
	//}}AFX_DATA_INIT
}


void CResultDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResultDlg)
	DDX_Text(pDX, IDC_EDIT_COMPS, m_nComps);
	DDX_Text(pDX, IDC_EDIT_TIME, m_nTime);
	DDX_Text(pDX, IDC_EDIT_QS, m_nQS);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResultDlg, CDialog)
	//{{AFX_MSG_MAP(CResultDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResultDlg message handlers
