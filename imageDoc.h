// imageDoc.h : interface of the CImageDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGEDOC_H__0E897C5C_5061_4C9B_BE99_D7A20747B092__INCLUDED_)
#define AFX_IMAGEDOC_H__0E897C5C_5061_4C9B_BE99_D7A20747B092__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "DIB.h"
#include "HC.h"


class CImageDoc : public CDocument
{
protected: // create from serialization only
	CImageDoc();
	DECLARE_DYNCREATE(CImageDoc)

// Attributes
public:
	int lookregion(int cx,int cy,int=0);
	float* GetNDVI(float*ndv=NULL,int vB=4);
	void SetEM();
	int* GetTag(int * tag);
	BYTE* GetEM(BYTE*EM,int eB);


	int edgeBand;//the band for edge poing extraction or edge strength computation
	double m_dRoom;
//	CPoint current;
	bool viewRegion;
	bool sortDM;
//	bool export;
	int curRegion;
//	CStroke* NewStroke();
//	CPen m_penCur;
//	uint32 m_nPenWidth;
//	CTypedPtrList<CObList,CStroke*> m_strokeList;
	double quantum;
	GDALDataset* m_pDataset;//当前影像
	//long m_bandNum;//波段的数目
	CDIB m_DIB;
	CHC m_HC;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageDoc)
	public:
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

	virtual ~CImageDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CImageDoc)
	afx_msg void OnProcessHistoequal();
	afx_msg void OnSegInitiate();
	afx_msg void OnViewOutcome();
	afx_msg void OnProcessGuass();
	afx_msg void OnViewZoomin();
	afx_msg void OnViewOrigin();
	afx_msg void OnViewZoomout();
	afx_msg void OnEvalQs();
	afx_msg void OnViewRegions();
	afx_msg void OnSegQthc();
	afx_msg void OnClassRegprop();
	afx_msg void OnSegGshc();
	afx_msg void OnSegGraph();
	afx_msg void OnClassRoadclump();
	afx_msg void OnPrepSr();
	afx_msg void OnClassErode();
	afx_msg void OnSegSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnSegQuadTree();
	afx_msg void OnPrepSobel();
	afx_msg void OnPrepCanny();
	afx_msg void OnPrepSusan();
	afx_msg void OnPrepMorph();
	afx_msg void OnSegRegMeg();
	afx_msg void OnClassPreview();
	afx_msg void OnSegSavetour();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGEDOC_H__0E897C5C_5061_4C9B_BE99_D7A20747B092__INCLUDED_)
