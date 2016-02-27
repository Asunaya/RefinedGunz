// RSObject.h  since 1998.3.20 by dubble

#ifndef __RSOBJECT_H
#define __RSOBJECT_H

#include "CMlist.h"
#include "RSAnimationInfo.h"
#include "RsTypes.h"
#include "rtexture.h"
#include "RSD3D.h"

class RealSpace;

struct RSMHEADER{
	int			RSMID;
	int			Build;
	FILETIME	MaterialTimeStamp;		// obsolete
};

class CFaces
{
public:
	int			iMaterial;
	int			nv,ni;
	VERTEX		*tnlvertices;
	WORD		*indicies,*indicies_original;
	LPDIRECT3DVERTEXBUFFER8	*pVertexBuffer;
	LPDIRECT3DINDEXBUFFER8	*pIndexBuffer;
	void Draw();

	CFaces();
	virtual	~CFaces();
};

enum RSALIGNSTYLE {
	RS_ALIGNSTYLE_NONE,
	RS_ALIGNSTYLE_POINT,
	RS_ALIGNSTYLE_LINE
};

#define ALIGN_POINT_STRING	"algn0"
#define ALIGN_LINE_STRING	"algn1"

class CMesh
{
public:
	char		*name;
	rvertex		*ver;
	CFaces		*faceshead;
	int			nV,nFaces;
	float		keyforsort;
	rvector		center;
	rmatrix		mat;
	rboundingbox m_bbox;
	RSALIGNSTYLE m_align;

	int GetVertexCount() { return nV; }
	rvertex *GetVertex(int i) { return &ver[i]; }
	rvector GetCenter() { return center; }

	CMesh();
	virtual ~CMesh();
};

class RSObject {
friend RealSpace;
public:
	RSObject();
	virtual ~RSObject();

	bool Load(FILE *file,RSMaterialManager *pMM);
	bool Load(const char *filename,RSMaterialManager *pMM);
	bool Load(const char *filename,const char *RMLName);
	bool Destroy();

	int GetRefferenceCount(){return RefferenceCount;}
	void IncreaseRefferenceCount(){RefferenceCount++;}
	void DecreaseRefferenceCount(){RefferenceCount--;}

	int GetShadowTextureHandle() { return m_hShadowTexture; }
	
	void ClearVertexBuffers();
	void GenerateVertexBuffers();

public:
	void CreateShadowTexture();

	RSMHEADER m_Header;
	char	name[200];
	int		nAnimSet;
	int		nMesh,nMaterial;
	int		RefferenceCount;

	rtexture	m_ShadowTexture;
	int		m_hShadowTexture;

	float	m_BoundingSphereRadius;
	float	m_BoundingSphereRadiusXY;	// 평평한 그림자 찍을때 쓴다
	rboundingbox m_bbox;
	CMesh		*meshes;
	RSMaterial	**materials;
//	RSAnimationInfo	*CurrentAnimation;
	CMLinkedList<RSAnimationInfo> AnimationList;

	RSMaterialManager *m_pMM;
	bool	b_needdelMM;
};

#endif
