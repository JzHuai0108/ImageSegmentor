#if !defined(AFX_INITIATEDLG_H__9DF4F924_8F48_4694_8622_C841926E12D5__INCLUDED_)
#define AFX_INITIATEDLG_H__9DF4F924_8F48_4694_8622_C841926E12D5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InitiateDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInitiateDlg dialog
class CInitiateDlg : public CDialog
{
// Construction
public:
	CInitiateDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CInitiateDlg)
	enum { IDD = IDD_DIALOG_INITIATE };
	float	m_nKL;
	UINT	m_nMinsize;
	float	m_nLamda;
	int		m_nLoop;
	float	m_Scale;
	float	m_PacCoeff;
	float	m_SpeCoeff;
	int		m_HM;
	CString	m_BWArray;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInitiateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInitiateDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INITIATEDLG_H__9DF4F924_8F48_4694_8622_C841926E12D5__INCLUDED_)
