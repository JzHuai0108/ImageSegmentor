#if !defined(AFX_SUSANDLG_H__6FA15C16_0061_4F0B_A8CA_7EA582183CA3__INCLUDED_)
#define AFX_SUSANDLG_H__6FA15C16_0061_4F0B_A8CA_7EA582183CA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SUSANDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSUSANDlg dialog

class CSUSANDlg : public CDialog
{
// Construction
public:
	CSUSANDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSUSANDlg)
	enum { IDD = IDD_DIALOG_SUSAN };
	float	m_BT;
	int		m_Radio;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSUSANDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSUSANDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SUSANDLG_H__6FA15C16_0061_4F0B_A8CA_7EA582183CA3__INCLUDED_)
