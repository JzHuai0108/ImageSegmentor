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

#if !defined(AFX_BANDSELDLG_H__9B66612E_854A_4CE7_9228_27FE7CF1A220__INCLUDED_)
#define AFX_BANDSELDLG_H__9B66612E_854A_4CE7_9228_27FE7CF1A220__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BandSelDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBandSelDlg dialog

class CBandSelDlg : public CDialog
{
// Construction
public:
	CBandSelDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBandSelDlg)
	enum { IDD = IDD_DIALOG_BANDSEL };
	int		m_B;
	int		m_Band;
	int		m_G;
	int		m_R;
	int		m_NDVI;
	int		m_EB;
	CString	m_DataType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBandSelDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBandSelDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BANDSELDLG_H__9B66612E_854A_4CE7_9228_27FE7CF1A220__INCLUDED_)
