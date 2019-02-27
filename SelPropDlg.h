#if !defined(AFX_SELPROPDLG_H__58527578_3B4A_4C07_890D_B0E8901A6150__INCLUDED_)
#define AFX_SELPROPDLG_H__58527578_3B4A_4C07_890D_B0E8901A6150__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelPropDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelPropDlg dialog

class CSelPropDlg : public CDialog
{
// Construction
public:
	int numb;
	CSelPropDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSelPropDlg)
	enum { IDD = IDD_DIALOG_SELPROP };
	int		m_bot;
	int		m_left;
	int		m_right;
	int		m_top;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelPropDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelPropDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELPROPDLG_H__58527578_3B4A_4C07_890D_B0E8901A6150__INCLUDED_)
