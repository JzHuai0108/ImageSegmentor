// image.h : main header file for the IMAGE application
//

#if !defined(AFX_IMAGE_H__BE94E17D_1657_4966_A4DB_E07752BDFC83__INCLUDED_)
#define AFX_IMAGE_H__BE94E17D_1657_4966_A4DB_E07752BDFC83__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "MainFrm.h"		// main frame
/////////////////////////////////////////////////////////////////////////////
// CImageApp:
// See image.cpp for the implementation of this class
//

class CImageApp : public CWinApp
{
public:
	void InitDoc();
	CImageApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CImageApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();
	afx_msg void OnFileMultiFeat();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGE_H__BE94E17D_1657_4966_A4DB_E07752BDFC83__INCLUDED_)

CImageApp* GetApp();
CMainFrame* GetMainFrame();
