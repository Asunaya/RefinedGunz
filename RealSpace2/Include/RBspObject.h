#pragma once

#include <cstdio>
#include <list>

#include "RTypes.h"
#include "RLightList.h"
#include "RMeshMgr.h"
#include "RAnimationMgr.h"
#include "RMaterialList.h"
#include "ROcclusionList.h"
#include "RDummyList.h"
#include "RSolidBsp.h"
#include "RNavigationMesh.h"
#include "MUtil.h"

class MZFile;
class MZFileSystem;
class MXmlElement;

_NAMESPACE_REALSPACE2_BEGIN

struct RMATERIAL;
class RMaterialList;
class RDummyList;
class RBaseTexture;
class RSBspNode;


struct RDEBUGINFO {
	int nCall, nPolygon;
	int nMapObjectFrustumCulled;
	int nMapObjectOcclusionCulled;
	RSolidBspNode		*pLastColNode;
};

struct BSPVERTEX {

	float x, y, z;		// world position
	float tu1, tv1;		// texture coordinates
	float tu2, tv2;

	rvector *Coord() { return (rvector*)&x; }
};

#define BSP_FVF	(D3DFVF_XYZ | D3DFVF_TEX2)

#define LIGHT_BSP_FVF	(D3DFVF_XYZ | D3DFVF_TEX2 | D3DFVF_DIFFUSE)

struct RPOLYGONINFO {
	rplane	plane;
	int		nMaterial;
	int		nConvexPolygon;
	int		nLightmapTexture;
	int		nPolygonID;
	DWORD	dwFlags;

	BSPVERTEX *pVertices;
	int		nVertices;
	int		nIndicesPos;
};

struct RCONVEXPOLYGONINFO {
	rplane	plane;
	rvector *pVertices;
	rvector *pNormals;
	int	nVertices;
	int	nMaterial;
	float fArea;
	DWORD	dwFlags;
};

struct ROBJECTINFO {
	string		name;
	int			nMeshID;
	RVisualMesh *pVisualMesh;
	RLIGHT		*pLight;
	float		fDist;
};

struct RBSPPICKINFO {
	RSBspNode* pNode;
	int nIndex;
	rvector PickPos;
	RPOLYGONINFO	*pInfo;
};

class RMapObjectList : public std::list<ROBJECTINFO*> {
public:
	virtual ~RMapObjectList();

	iterator Delete(iterator i);
};

class RDrawInfo {
public:
	RDrawInfo() {
		nVertice = 0;
		pVertices = NULL;
		nIndicesOffset = 0;
		nTriangleCount = 0;
		pPlanes = NULL;
		pUAxis = NULL;
		pVAxis = NULL;

	}

	~RDrawInfo() {
		SAFE_DELETE(pVertices);
		SAFE_DELETE(pPlanes);
		SAFE_DELETE(pUAxis);
		SAFE_DELETE(pVAxis);
	}

	int				nVertice;
	BSPVERTEX		*pVertices;
	int				nIndicesOffset;
	int				nTriangleCount;
	rplane			*pPlanes;
	rvector			*pUAxis;
	rvector			*pVAxis;
};

class RSBspNode
{
public:
	int				nPolygon;
	RPOLYGONINFO	*pInfo;
	RDrawInfo		*pDrawInfo;

	int				nFrameCount;

	RSBspNode		*m_pPositive, *m_pNegative;

	rplane plane;
	rboundingbox	bbTree;

	RSBspNode();
	virtual ~RSBspNode();

	RSBspNode *GetLeafNode(rvector &pos);
	void DrawWireFrame(int nFace, DWORD color);
	void DrawBoundingBox(DWORD color);
};

typedef std::list<POINT> RFREEBLOCKLIST;
struct RLIGHTMAPTEXTURE {
	int nSize;
	DWORD *data;
	bool bLoaded;
	POINT position;
	int	nLightmapIndex;
};

struct RBSPMATERIAL : public RMATERIAL {
	RBSPMATERIAL() { texture = NULL; }
	RBSPMATERIAL(RMATERIAL *mat)
	{
		Ambient = mat->Ambient;
		Diffuse = mat->Diffuse;
		DiffuseMap = mat->DiffuseMap;
		dwFlags = mat->dwFlags;
		Name = mat->Name;
		Power = mat->Power;
		Specular = mat->Specular;
	};
	RBaseTexture *texture;
};

class RBspLightmapManager {

public:
	RBspLightmapManager();
	virtual ~RBspLightmapManager();

	void Destroy();

	int GetSize() { return m_nSize; }
	DWORD *GetData() { return m_pData; }

	void SetSize(int nSize) { m_nSize = nSize; }
	void SetData(DWORD *pData) { Destroy(); m_pData = pData; }

	bool Add(DWORD *data, int nSize, POINT *retpoint);
	bool GetFreeRect(int nLevel, POINT *pt);

	void Save(const char *filename);

	float CalcUnused();
	float m_fUnused;

protected:
	RFREEBLOCKLIST *m_pFreeList;
	DWORD *m_pData;
	int m_nSize;
};

struct FogInfo
{
	bool bFogEnable;
	DWORD dwFogColor;
	float fNear;
	float fFar;
	FogInfo() { bFogEnable = false; }
};

struct AmbSndInfo
{
	int itype;
	char szSoundName[64];
	rvector min;
	rvector center;
	rvector max;
	float radius;
};

#define AS_AABB		0x01
#define AS_SPHERE	0x02
#define AS_2D		0x10
#define AS_3D		0x20

typedef bool(*RGENERATELIGHTMAPCALLBACK)(float fProgress);



class RBspObject
{
public:
	enum ROpenFlag {
		ROF_RUNTIME,
		ROF_EDITOR
	} m_OpenMode;

	RBspObject();
	~RBspObject();
	RBspObject(const RBspObject&) = delete;
	RBspObject(RBspObject&&) = default;

	void ClearLightmaps();

	bool Open(const char *, ROpenFlag nOpenFlag = ROF_RUNTIME, RFPROGRESSCALLBACK pfnProgressCallback = nullptr,
		void *CallbackParam = nullptr, bool PhysOnly = false);

	bool OpenDescription(const char *);
	bool OpenRs(const char *);
	bool OpenBsp(const char *);
	bool OpenLightmap();
	bool OpenCol(const char *);
	bool OpenNav(const char *);

	void OptimizeBoundingBox();

	bool IsVisible(rboundingbox &bb) const;

	bool Draw();
	void DrawObjects();


	bool DrawLight(RSBspNode *pNode, int nMaterial);
	void DrawLight(D3DLIGHT9 *pLight);

	bool GenerateLightmap(const char *filename, int nMaxLightmapSize, int nMinLightmapSize, int nSuperSample,
		float fToler, RGENERATELIGHTMAPCALLBACK pProgressFn = nullptr);

	void SetWireframeMode(bool bWireframe) { m_bWireframe = bWireframe; }
	bool GetWireframeMode() { return m_bWireframe; }
	void SetShowLightmapMode(bool bShowLightmap) { m_bShowLightmap = bShowLightmap; }
	bool GetShowLightmapMode() { return m_bShowLightmap; }

	bool Pick(const rvector &pos, const rvector &dir, RBSPPICKINFO *pOut, DWORD dwPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_USEOPACITY | RM_FLAG_HIDE);
	bool PickTo(const rvector &pos, const rvector &to, RBSPPICKINFO *pOut, DWORD dwPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_USEOPACITY | RM_FLAG_HIDE);
	bool PickOcTree(const rvector &pos, const rvector &dir, RBSPPICKINFO *pOut, DWORD dwPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_USEOPACITY | RM_FLAG_HIDE);

	DWORD GetLightmap(rvector &Pos, RSBspNode *pNode, int nIndex);

	RBSPMATERIAL *GetMaterial(RSBspNode *pNode, int nIndex) { return GetMaterial(pNode->pInfo[nIndex].nMaterial); }

	int	GetMaterialCount() const { return m_nMaterial; }
	RBSPMATERIAL *GetMaterial(int nIndex);

	RMapObjectList	*GetMapObjectList() { return &m_ObjectList; }
	RDummyList		*GetDummyList() { return &m_DummyList; }
	RBaseTexture* GetBaseTexture(int n) {
		if (n >= 0 && n < m_nMaterial)
			return m_pMaterials[n].texture;
		return nullptr;
	}

	RLightList *GetMapLightList() { return &m_StaticMapLightList; }
	RLightList *GetObjectLightList() { return &m_StaticObjectLightList; }
	RLightList *GetSunLightList() { return &m_StaticSunLigthtList; }

	auto* GetOcRootNode() { return m_pOcRoot; }
	auto* GetRootNode() { return m_pBspRoot; }

	rvector GetDimension() const;

	int	GetVertexCount() { return OcVertices.size(); }
	int	GetPolygonCount() { return m_nPolygon; }
	int GetNodeCount() { return m_nNodeCount; }
	int	GetBspPolygonCount() { return m_nBspPolygon; }
	int GetBspNodeCount() { return m_nBspNodeCount; }
	int GetConvexPolygonCount() { return m_nConvexPolygon; }
	int GetLightmapCount() { return m_nLightmap; }

	bool CheckWall(rvector &origin, rvector &targetpos, float fRadius, float fHeight = 0.f,
		RCOLLISIONMETHOD method = RCW_CYLINDER, int nDepth = 0, rplane *pimpactplane = NULL);

	bool CheckSolid(rvector &pos, float fRadius, float fHeight = 0.f, RCOLLISIONMETHOD method = RCW_CYLINDER);

	rvector GetFloor(rvector &origin, float fRadius, float fHeight, rplane *pimpactplane = NULL);

	void OnInvalidate();
	void OnRestore();

	void SetObjectLight(rvector pos);
	void SetCharactorLight(rvector pos);

	bool GetShadowPosition(rvector& pos_, rvector& dir_, rvector* outNormal_, rvector* outPos_);

	auto* GetMeshManager() { return &m_MeshList; }

	void test_MakePortals();

	void DrawBoundingBox();
	void DrawOcclusions();
	void DrawNormal(int nIndex, float fSize = 1.f);

	void DrawCollision_Polygon();
	void DrawCollision_Solid();

	void DrawSolid();
	void DrawSolidNode();
	void DrawColNodePolygon(rvector &pos);

	void DrawNavi_Polygon();
	void DrawNavi_Links();

	RSolidBspNode *GetColRoot() { return m_pColRoot; }

	void LightMapOnOff(bool b);
	static void SetDrawLightMap(bool b);

	FogInfo GetFogInfo() { return m_FogInfo; }
	std::list<AmbSndInfo*> GetAmbSndList() { return m_AmbSndInfoList; }

	void GetNormal(int nConvexPolygon, rvector &position, rvector *normal);

	static bool CreateShadeMap(const char *szShadeMap);
	static void DestroyShadeMap();

	RDEBUGINFO *GetDebugInfo() { return &m_DebugInfo; }
	RNavigationMesh* GetNavigationMesh() { return &m_NavigationMesh; }

	void SetMapObjectOcclusion(bool b) { m_bNotOcclusion = b; }

private:
	bool PhysOnly;

	std::string m_filename, m_descfilename;

	bool m_bWireframe;
	bool m_bShowLightmap;

	bool DrawNoTNL(RSBspNode *bspNode, int nMaterial);
	bool Draw(RSBspNode *bspNode, int nMaterial);

	void SetDiffuseMap(int nMaterial);

	bool Pick(RSBspNode *pNode, const rvector &v0, const rvector &v1, struct PickInfo&);
	bool PickShadow(rvector &pos, rvector &to, RBSPPICKINFO *pOut);
	bool PickShadow(RSBspNode *pNode, rvector &v0, rvector &v1, struct PickInfo&);

	void ChooseNodes(RSBspNode *bspNode);
	int ChooseNodes(RSBspNode *bspNode, rvector &center, float fRadius);

	auto* GetLeafNode(rvector &pos) { return m_pBspRoot->GetLeafNode(pos); }

	bool ReadString(MZFile *pfile, char *buffer, int nBufferSize);
	bool Open_Nodes(RSBspNode *pNode, MZFile *pfile, BSPVERTEX** Vertices);
	bool Open_ColNodes(RSolidBspNode *pNode, MZFile *pfile);
	bool Open_MaterialList(MXmlElement *pElement);
	bool Open_LightList(MXmlElement *pElement);
	bool Open_ObjectList(MXmlElement *pElement);
	bool Open_DummyList(MXmlElement *pElement);
	bool Open_ConvexPolygons(MZFile *pfile);
	bool Open_OcclusionList(MXmlElement *pElement);
	bool Make_LenzFalreList();
	bool Set_Fog(MXmlElement *pElement);
	bool Set_AmbSound(MXmlElement *pElement);

	void CreatePolygonTable();
	void CreatePolygonTable(RSBspNode *pNode, u16** Indices);
	void Sort_Nodes(RSBspNode *pNode);

	bool CreateVertexBuffer();
	bool UpdateVertexBuffer();

	bool CreateIndexBuffer();
	bool UpdateIndexBuffer();

	bool CreateDynamicLightVertexBuffer();
	void InvalidateDynamicLightVertexBuffer();
	bool FlushLightVB();
	bool LockLightVB();
	LPDIRECT3DVERTEXBUFFER9 m_pDynLightVertexBuffer;

	static RBaseTexture *m_pShadeMap;

	std::vector<BSPVERTEX>	BspVertices;
	std::vector<BSPVERTEX>	OcVertices;
	std::vector<u16>		OcIndices;
	RSBspNode *m_pBspRoot, *m_pOcRoot;
	RPOLYGONINFO *m_pBspInfo, *m_pOcInfo;
	int m_nPolygon, m_nNodeCount;
	int m_nBspPolygon, m_nBspNodeCount, m_nBspVertices, m_nBspIndices;
	LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;
	LPDIRECT3DINDEXBUFFER9 m_pIndexBuffer;

	int m_nMaterial;
	RBSPMATERIAL *m_pMaterials;

	rplane m_localViewFrustum[6];

	ROcclusionList m_OcclusionList;

	int m_nLightmap;
	std::vector<D3DPtr<IDirect3DTexture9>> LightmapTextures;

	void CalcLightmapUV(RSBspNode *pNode, int *pLightmapInfo,
		std::vector<RLIGHTMAPTEXTURE*>& SourceLightmaps, std::vector<RBspLightmapManager>& LightmapList);

	// interpolated normal
	void GetNormal(RCONVEXPOLYGONINFO *poly, rvector &position, rvector *normal, int au, int av);

	int					m_nConvexPolygon, m_nConvexVertices;
	rvector				*m_pConvexVertices;
	rvector				*m_pConvexNormals;
	RCONVEXPOLYGONINFO	*m_pConvexPolygons;

	rvector		m_AmbientLight;
	RLightList	m_StaticMapLightList;
	RLightList	m_StaticObjectLightList;
	RLightList	m_StaticSunLigthtList;

	RMeshMgr			m_MeshList;
	RAnimationMgr		m_AniList;
	RMapObjectList		m_ObjectList;
	bool				m_bNotOcclusion;

	RSolidBspNode		*m_pColRoot;
	rvector				*m_pColVertices;

	RNavigationMesh		m_NavigationMesh;

	RDummyList	m_DummyList;

	FogInfo m_FogInfo;

	std::list<AmbSndInfo*>	m_AmbSndInfoList;

	RDEBUGINFO			m_DebugInfo;
};

extern int g_nPoly, g_nCall;

_NAMESPACE_REALSPACE2_END