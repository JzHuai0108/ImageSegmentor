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

#if !defined(AFX_PREVIEWDLG_H__7B1EB4F9_5A8B_4D7E_AC6C_0F938AE7C657__INCLUDED_)
#define AFX_PREVIEWDLG_H__7B1EB4F9_5A8B_4D7E_AC6C_0F938AE7C657__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PreviewDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPreviewDlg dialog
#define	GRAYLEVELS	256
#define	NUM_COORDINATEION	25
class CImageView;
class CPreviewDlg : public CDialog
{
// Construction
public:
	CPreviewDlg(CWnd* pParent = NULL);   // standard constructor


public:

	CRect mask;
	
	CImageView*pView;
	CSliderCtrl*p_Slider;
	CStatic*pCount,*pThresh,*pMax,*pMin,*pPic;
	float m_RMax,m_RMin;
	int m_nCur;//current position of slider range fropm 0 to 255
	float curMin;//current mininum for regions threshold
	BYTE	bGrayValue; //store current gray threshold
	int dataChannel[256];//bin the data from m_RMin to m_RMax
	int m_nMaxColor;//the max count of specific color
	struct	coordination
	{
		CPoint	point;
		BOOL	bMove;
	}rect_Coordination[25];//to build coordinate

	CRect	rectDraw;
	int		iArrowWidth, iArrowLen, iFlagLen, iTopReserve, iReserve;
	int		iLeft, iBottom, ivStep, ihStep, nMaxHeight, nMaxWidth;
	int		startleft, startbottom;

//	int		iFavorPos;

// Dialog Data
	//{{AFX_DATA(CPreviewDlg)
	enum { IDD = IDD_DIALOG_PREVIEW };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPreviewDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPreviewDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void DrawVertCoordination(CDC* pDC, float vertStep);
	void DrawHorizonVertCoordination(CDC* pDC);
	void DrawColorHistogram(CDC* pDC, int *colorArray);
	void DrawCurrentFavorPosition(CDC* pDC);
	void InitCoordinate();
	void DisplayRGBCounts();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREVIEWDLG_H__7B1EB4F9_5A8B_4D7E_AC6C_0F938AE7C657__INCLUDED_)
