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

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "image.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_DROPFILES()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
		// length unit
	ID_SEPARATOR,
		// cursor position
	ID_SEPARATOR,
		// image size
	ID_SEPARATOR,
		// bit count
	ID_SEPARATOR,
	//region label
	ID_SEPARATOR, 
	//region size
	ID_SEPARATOR,
	//region neighbor size
	ID_SEPARATOR,	
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}
void CALLBACK EXPORT TimerProc(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime)
{
	AfxGetApp()->OnIdle(0);
}
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
		// Enable drag/drop open
	DragAcceptFiles(TRUE);
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
	// add panes
    m_wndStatusBar.SetPaneInfo( PANE_LENGTH_UNIT, ID_SEPARATOR, SBPS_NORMAL, 25);
    m_wndStatusBar.SetPaneInfo( PANE_CURSOR_POSITION, ID_SEPARATOR, SBPS_NORMAL, 72);
    m_wndStatusBar.SetPaneInfo( PANE_IMAGE_SIZE, ID_SEPARATOR, SBPS_NORMAL, 72);
    m_wndStatusBar.SetPaneInfo( PANE_BIT_COUNT, ID_SEPARATOR, SBPS_NORMAL, 28);
	m_wndStatusBar.SetPaneInfo( PANE_REGION_ID, ID_SEPARATOR, SBPS_NORMAL, 60);
    m_wndStatusBar.SetPaneInfo( PANE_REGION_SIZE, ID_SEPARATOR, SBPS_NORMAL, 30);
    m_wndStatusBar.SetPaneInfo( PANE_RN_COUNT, ID_SEPARATOR, SBPS_NORMAL, 15);
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG









void CMainFrame::OnDropFiles(HDROP hDropInfo) 
{
	// TODO: Add your message handler code here and/or call default
	const int fc=DragQueryFile(hDropInfo,(UINT)-1,NULL,0);
	ASSERT(fc>=1);
	TCHAR fn[MAX_PATH]={0};
	for(int i=0; i<fc;++i)
	{
		DragQueryFile(hDropInfo,i,fn,MAX_PATH);
		AfxGetApp()->OpenDocumentFile(fn);
	}

	CMDIFrameWnd::OnDropFiles(hDropInfo);
}


