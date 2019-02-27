#if !defined(AFX_MORPHDLG_H__E32809C2_0264_452F_8F7B_F2B090FFCA9D__INCLUDED_)
#define AFX_MORPHDLG_H__E32809C2_0264_452F_8F7B_F2B090FFCA9D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MorphDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMorphDlg dialog

class CMorphDlg : public CDialog
{
// Construction
public:
	CMorphDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMorphDlg)
	enum { IDD = IDD_DIALOG_MORPH };
	int		m_EB;
	int iPos;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMorphDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMorphDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelendokComboMorph();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MORPHDLG_H__E32809C2_0264_452F_8F7B_F2B090FFCA9D__INCLUDED_)
