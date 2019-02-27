// MyImageDB.h : main header file for the MYIMAGEDB application
//

#if !defined(AFX_MYIMAGEDB_H__A9BFE026_07AE_42A7_9973_E365D8556AE4__INCLUDED_)
#define AFX_MYIMAGEDB_H__A9BFE026_07AE_42A7_9973_E365D8556AE4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CMyImageDBApp:
// See MyImageDB.cpp for the implementation of this class
//

class CMyImageDBApp : public CWinApp
{
public:
	virtual BOOL OnIdle(LONG lCount);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CMyImageDBApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyImageDBApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CMyImageDBApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYIMAGEDB_H__A9BFE026_07AE_42A7_9973_E365D8556AE4__INCLUDED_)
