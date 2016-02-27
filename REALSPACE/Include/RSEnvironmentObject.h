// RSEnvironmentObject.h: interface for the RSEnvironmentObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RSENVIRONMENTOBJECT_H__4CBDD761_F977_11D2_B4A7_00AA006E4A4E__INCLUDED_)
#define AFX_RSENVIRONMENTOBJECT_H__4CBDD761_F977_11D2_B4A7_00AA006E4A4E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RSBase3D.h"
#include "RSD3D.h"

class CMesh;
class RSObject;
class RSVisualObject;
class RSEnvironmentObject : public RSBase3D  
{
public:
	RSEnvironmentObject();
	virtual ~RSEnvironmentObject();

	bool Create(RSObject *pObject,int hTexture,bool bAlpha=false);
	void Move(float fTime);
	void Draw();

	void SetDrawLevel(int lv) {m_nDrawLevel = lv;}
	int	 GetDrawLevel() {return m_nDrawLevel; }

private:

	rmatrix44 m_Transform;

	// 스카이박스.
	RSVisualObject *m_pVO;

	// 구름을 위한것.
	bool m_bAlpha;
	int nv,ni;
	int	m_TextureHandle;
	LPDIRECT3DVERTEXBUFFER8	*pVertexBuffer;
	LPDIRECT3DINDEXBUFFER8	*pIndexBuffer;
	float *uvs;
	float m_fou,m_fov;
	int m_nDrawLevel;
};

#endif // !defined(AFX_RSENVIRONMENTOBJECT_H__4CBDD761_F977_11D2_B4A7_00AA006E4A4E__INCLUDED_)
