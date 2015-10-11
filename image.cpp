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

// image.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "image.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "imageDoc.h"
#include "imageView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
char szFilter[] = "Bitmap (*.bmp)|*.bmp|Tagged image (*.tif)|*.tif|JPEG image (*.jpg)|*.jpg|Erdas image (*img)|*.img|All Files(*.*)|*.*||";
/////////////////////////////////////////////////////////////////////////////
// CImageApp
CImageApp* GetApp()
{
  return (CImageApp*) AfxGetApp();
}

CMainFrame* GetMainFrame()
{
  return (CMainFrame*) AfxGetMainWnd();
}

BEGIN_MESSAGE_MAP(CImageApp, CWinApp)
	//{{AFX_MSG_MAP(CImageApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageApp construction

CImageApp::CImageApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CImageApp object

CImageApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CImageApp initialization

BOOL CImageApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	//Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_IMAGETYPE,
		RUNTIME_CLASS(CImageDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CImageView));
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();
//	InitDoc();
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CImageApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CImageApp message handlers


void CImageApp::OnFileNew() 
{
	// TODO: Add your command handler code here
	/*	CString FileName ="C:\\Documents and Settings\\volcanoj\\×ÀÃæ\\cut_1.tif";
		FileName.MakeUpper();
		OpenDocumentFile(FileName);*/
}

void CImageApp::OnFileOpen() 
{
	// TODO: Add your command handler code here
	static int nIndex = 2;
 
	CFileDialog FileDlg( TRUE, NULL, NULL, OFN_HIDEREADONLY,szFilter);
	FileDlg.m_ofn.nFilterIndex = (DWORD) nIndex;

	if( FileDlg.DoModal() == IDOK)
	{
		CString PathName =FileDlg.GetPathName();
		PathName.MakeUpper();
		OpenDocumentFile(PathName);
		nIndex= (int) FileDlg.m_ofn.nFilterIndex;
	}
}


void CImageApp::InitDoc()
{
	CString PathName ="E:\\landcruiser\\TIFFIMAGE\\belle.jpg";
	PathName.MakeUpper();
	OpenDocumentFile(PathName);
}

void CAboutDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}
