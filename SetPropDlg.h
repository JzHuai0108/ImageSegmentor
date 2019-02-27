#if !defined(AFX_SETPROPDLG_H__69C242E9_ECB5_40C0_A7E9_7F14E86BFE3F__INCLUDED_)
#define AFX_SETPROPDLG_H__69C242E9_ECB5_40C0_A7E9_7F14E86BFE3F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetPropDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSetPropDlg dialog

class CSetPropDlg : public CDialog
{
// Construction
public:
	CSetPropDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetPropDlg)
	enum { IDD = IDD_DIALOG_SETTHRESH };
	float	m_MaxLWR;
	float	m_MinArea;
	float	m_MinCmpct;
	float	m_MinWid;
	float	m_MaxArea;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetPropDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetPropDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETPROPDLG_H__69C242E9_ECB5_40C0_A7E9_7F14E86BFE3F__INCLUDED_)
