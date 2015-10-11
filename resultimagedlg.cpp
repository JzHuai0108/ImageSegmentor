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

// ResultImageDlg1.cpp : implementation file
//

#include "stdafx.h"
#include "image.h"
#include "ResultImageDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResultImageDlg dialog


CResultImageDlg::CResultImageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CResultImageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CResultImageDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	isImageOk = FALSE;
}
void CResultImageDlg::SetImage(BYTE* indata, INT width, INT height, int nBits)
//ÉèÖÃÍ¼Ïñ£»
{

	myImageObj.CreateDIBFromBits(width, height, indata,nBits);
	CWnd* myMatrix = GetDlgItem(IDC_RESULTIMAGE);
	CRect matrixrect;
	myMatrix->GetClientRect(matrixrect);
	myMatrix->ClientToScreen(matrixrect);
	ScreenToClient(matrixrect);
	
	CClientDC dc(this);
	
	myImageObj.Stretch( dc.GetSafeHdc(), matrixrect.left
		, matrixrect.top, matrixrect.right-matrixrect.left
		, matrixrect.bottom-matrixrect.top,DIB_RGB_COLORS, SRCCOPY);
	isImageOk = TRUE;
}

void CResultImageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResultImageDlg)
	DDX_Control(pDX, IDC_RESULTIMAGE, myResultImage);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResultImageDlg, CDialog)
	//{{AFX_MSG_MAP(CResultImageDlg)
	ON_WM_SIZE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResultImageDlg message handlers

void CResultImageDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
}

void CResultImageDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	if (isImageOk)
	{
		CWnd* myMatrix = GetDlgItem(IDC_RESULTIMAGE);
		CRect matrixrect;
		myMatrix->GetClientRect(matrixrect);
		myMatrix->ClientToScreen(matrixrect);
		ScreenToClient(matrixrect);
		
		CClientDC dc(this);
		
		myImageObj.Stretch( dc.GetSafeHdc(), matrixrect.left
			, matrixrect.top, matrixrect.right-matrixrect.left
			, matrixrect.bottom-matrixrect.top, DIB_RGB_COLORS, SRCCOPY);	
	}	
	// Do not call CDialog::OnPaint() for painting messages
	// Do not call CDialog::OnPaint() for painting messages
}
