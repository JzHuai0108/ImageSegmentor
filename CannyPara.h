#if !defined(AFX_CANNYPARA_H__8B2DF16D_B940_4712_86A4_9D57C4A44628__INCLUDED_)
#define AFX_CANNYPARA_H__8B2DF16D_B940_4712_86A4_9D57C4A44628__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CannyPara.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCannyPara dialog

class CCannyPara : public CDialog
{
// Construction
public:
	CCannyPara(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCannyPara)
	enum { IDD = IDD_DIALOG_CANNY };
	float	m_high;
	float	m_low;
	float	m_sigma;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCannyPara)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCannyPara)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CANNYPARA_H__8B2DF16D_B940_4712_86A4_9D57C4A44628__INCLUDED_)
