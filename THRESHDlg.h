#if !defined(AFX_THRESHDLG_H__D1B86E35_AB15_45D8_B232_7AAD3F5B12C6__INCLUDED_)
#define AFX_THRESHDLG_H__D1B86E35_AB15_45D8_B232_7AAD3F5B12C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// THRESHDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTHRESHDlg dialog
#define	GRAYLEVELS	256
#define	NUM_COORDINATEION	25
class CImageView;
class CTHRESHDlg : public CDialog
{
// Construction
public:
	int CreateHist(float*ptArray,int count);
	CTHRESHDlg(CWnd* pParent = NULL);   // standard constructor
	CRect mask;
	float*exterior;
	CImageView*pView;
	CSliderCtrl*p_Slider;
	CStatic*pCount,*pThresh,*pMax,*pMin;
	float m_RMax,m_RMin;//range max and min
	int m_nCur;//current position of slider range fropm 0 to 255
	float curMin;//current mininum for regions threshold
	BYTE	bGrayValue; //store current gray threshold
	int dataChannel[256];//bin the data from m_RMin to m_RMax
	int m_nMaxColor;//the max count of specific color
	struct	coordination
	{
		CPoint	point;
		BOOL	bMove;
	}rect_Coordination[25];//to build coordinate

	CRect	rectDraw;
	int		iArrowWidth, iArrowLen, iFlagLen, iTopReserve, iReserve;
	int		iLeft, iBottom, ivStep, ihStep, nMaxHeight, nMaxWidth;
	int		startleft, startbottom;
// Dialog Data
	//{{AFX_DATA(CTHRESHDlg)
	enum { IDD = IDD_DIALOG_CDTHRESH };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTHRESHDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTHRESHDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void DrawVertCoordination(CDC* pDC, float vertStep);
	void DrawHorizonVertCoordination(CDC* pDC);
	void DrawColorHistogram(CDC* pDC, int *colorArray);
	void DrawCurrentFavorPosition(CDC* pDC);
	void InitCoordinate();
	void DisplayRGBCounts();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_THRESHDLG_H__D1B86E35_AB15_45D8_B232_7AAD3F5B12C6__INCLUDED_)
