// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__ECFC1FA5_43F4_4A1E_9F38_9A09BDF9A341__INCLUDED_)
#define AFX_MAINFRM_H__ECFC1FA5_43F4_4A1E_9F38_9A09BDF9A341__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "imageView.h"

#define PANE_LENGTH_UNIT		1
#define PANE_CURSOR_POSITION	2
#define PANE_IMAGE_SIZE			3
#define PANE_BIT_COUNT			4
#define PANE_REGION_ID			5
#define PANE_REGION_SIZE		6
#define PANE_RN_COUNT			7

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:	
	
	CImageView* pImageView;
	CStatusBar  m_wndStatusBar;
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members

	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__ECFC1FA5_43F4_4A1E_9F38_9A09BDF9A341__INCLUDED_)
