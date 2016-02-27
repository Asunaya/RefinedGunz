// WorldEditDoc.cpp : implementation of the CWorldEditDoc class
//

#include "stdafx.h"
#include "WorldEdit.h"

#include "WorldEditDoc.h"

#include "RBspObject.h"
#include "FileInfo.h"
#include "RMaterialList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWorldEditDoc

IMPLEMENT_DYNCREATE(CWorldEditDoc, CDocument)

BEGIN_MESSAGE_MAP(CWorldEditDoc, CDocument)
	//{{AFX_MSG_MAP(CWorldEditDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorldEditDoc construction/destruction

CWorldEditDoc::CWorldEditDoc()
{
	// TODO: add one-time construction code here
	m_pBspObject=NULL;
	m_bLastPicked=false;
}

CWorldEditDoc::~CWorldEditDoc()
{
	SAFE_DELETE(m_pBspObject);
}

BOOL CWorldEditDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CWorldEditDoc serialization

void CWorldEditDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWorldEditDoc diagnostics

#ifdef _DEBUG
void CWorldEditDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CWorldEditDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWorldEditDoc commands

BOOL CWorldEditDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	/*
	char rmpath[256];
	ReplaceExtension(rmpath,lpszPathName,"rm2");

	m_pMaterialList=new RMaterialList;
	m_pMaterialList->Open(rmpath);

	// TODO: Add your specialized creation code here
	m_pBspObject=new RBspObject;
	m_pBspObject->Open(lpszPathName,m_pMaterialList);
	m_pBspObject->OpenLightmap();
	m_pBspObject->CreateVertexBuffer();
*/

	SAFE_DELETE(m_pBspObject);

	m_pBspObject=new RBspObject;
	if(!m_pBspObject->Open(lpszPathName,RBspObject::ROF_EDITOR))
	{
		SAFE_DELETE(m_pBspObject);
		AfxMessageBox("파일열기에 실패하였습니다.");
		return FALSE;
	}

	m_bLastPicked=false;
	return TRUE;
}

void CWorldEditDoc::OnCloseDocument()
{
	// TODO: Add your specialized code here and/or call the base class
	SAFE_DELETE(m_pBspObject);

	CDocument::OnCloseDocument();
}

bool CWorldEditDoc::ReOpen()
{
	m_bLastPicked=false;

	SAFE_DELETE(m_pBspObject);

	m_pBspObject=new RBspObject;
	return m_pBspObject->Open(GetPathName(),RBspObject::ROF_EDITOR);
}