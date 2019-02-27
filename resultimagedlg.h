#if !defined(AFX_RESULTIMAGEDLG1_H__AFFD7B63_4216_4F44_B2BF_4482F17EA39B__INCLUDED_)
#define AFX_RESULTIMAGEDLG1_H__AFFD7B63_4216_4F44_B2BF_4482F17EA39B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResultImageDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CResultImageDlg dialog
#include "DIB.h"
class CResultImageDlg : public CDialog
{
// Construction
public:
	CResultImageDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CResultImageDlg)
	enum { IDD = IDD_DIALOG_RESULTIMAGE };
	CStatic	myResultImage;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResultImageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
public:
	void SetImage(BYTE* indata, INT width, INT height, int);//…Ë÷√ÕºœÒ£ª

protected:
	CDIB myImageObj; 
	BOOL isImageOk;

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CResultImageDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESULTIMAGEDLG1_H__AFFD7B63_4216_4F44_B2BF_4482F17EA39B__INCLUDED_)
