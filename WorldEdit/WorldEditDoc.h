// WorldEditDoc.h : interface of the CWorldEditDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WORLDEDITDOC_H__3090997F_4BF6_4435_B38E_E33CBAED1A75__INCLUDED_)
#define AFX_WORLDEDITDOC_H__3090997F_4BF6_4435_B38E_E33CBAED1A75__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RNameSpace.h"
#include "RBspObject.h"

_USING_NAMESPACE_REALSPACE2

class CWorldEditDoc : public CDocument
{
protected: // create from serialization only
	CWorldEditDoc();
	DECLARE_DYNCREATE(CWorldEditDoc)

// Attributes
public:
	RBspObject *m_pBspObject;

	bool m_bLastPicked;
	RBSPPICKINFO m_LastPicked;

// Operations
public:
	bool ReOpen();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWorldEditDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWorldEditDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CWorldEditDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnCloseDocument();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WORLDEDITDOC_H__3090997F_4BF6_4435_B38E_E33CBAED1A75__INCLUDED_)
