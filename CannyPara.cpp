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
