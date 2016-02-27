// RSBspObject.h: interface for the RSBspObject class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RSBSPOBJECT_H__CA689A63_BFE3_11D2_B3E9_00AA006E4A4E__INCLUDED_)
#define AFX_RSBSPOBJECT_H__CA689A63_BFE3_11D2_B3E9_00AA006E4A4E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include "rsutils.h"
#include "CMList.h"

class RSMaterialManager;

class RSBspNode
{
public:
	int		nFace;
	rface	*Face;
	RSBspNode *Positive,*Negative;

	rplane plane;
	rboundingbox bbTree;

	RSBspNode();
	virtual ~RSBspNode();

};

class RSBspObject
{ 
public:
	RSBspObject();
	virtual ~RSBspObject();

	bool Open(const char *,RSMaterialManager *);
	void Draw();

	BOOL CheckCollision(rvector *pos,rvector *npos,rvector *colpos,rvector *normal);
	void MoveVector(rvector *pos,rvector *npos);
	rface *GetCollisionFace(rvector *origin,rvector *direction);

private:
	int nVertices;
	rvertex *m_pVertices;
	RSBspNode *BspHead;
	rvector ViewPoint;

	void TraverseTreeAndRender(RSBspNode *bspNode);
	void DrawNodeFaces(RSBspNode *bspNode);

	RSBspNode *GetLeafNode(rvector *pos,RSBspNode *node);
	inline RSBspNode *GetLeafNode(rvector *pos) { return GetLeafNode(pos,BspHead); }
	rface *GetCollisionFaceR(rvector *origin,rplueckercoord *line,RSBspNode *node);

	int nMaterial,*pMaterialIndex;
// these are for ZClipped draw face
	void Check(rvertex *v1,rvertex *v2);
	void DrawFaceWithZClip(rface *face);

	rvertex added[6],*verlist[2][6],**v;
	rplane  *plane;
	int nv,nva;

// for loading
	bool ReadString(FILE *file,char *buffer,int nBufferSize);
	bool Open(RSBspNode *pNode,FILE *file,int *pTextureHandles);
};

#endif // !defined(AFX_RSBSPOBJECT_H__CA689A63_BFE3_11D2_B3E9_00AA006E4A4E__INCLUDED_)