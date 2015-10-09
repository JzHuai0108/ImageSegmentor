#if !defined(AFX_MRFOPTIMDLG_H__E4CD8704_A2C6_4443_93F4_FEBB01655F63__INCLUDED_)
#define AFX_MRFOPTIMDLG_H__E4CD8704_A2C6_4443_93F4_FEBB01655F63__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MRFOptimDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMRFOptimDlg dialog

class CMRFOptimDlg : public CDialog
{
// Construction
public:
	CMRFOptimDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMRFOptimDlg)
	enum { IDD = IDD_DIALOG_MRFOPTIM };
	double	m_Beta;
	double	m_C;
	int		m_ClsNum;
	double	m_t;
	double	m_T0;
	double	m_Alpha;
	//}}AFX_DATA
	int iPos;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMRFOptimDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMRFOptimDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelendokComboMRF();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MRFOPTIMDLG_H__E4CD8704_A2C6_4443_93F4_FEBB01655F63__INCLUDED_)
