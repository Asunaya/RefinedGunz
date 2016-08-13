#ifndef __RBSPOBJECT_H
#define __RBSPOBJECT_H

#include <stdio.h>
#include <list>

#include "RTypes.h"
//#include "RPath.h"
#include "RLightList.h"
#include "RMeshMgr.h"
#include "RAnimationMgr.h"
#include "RMaterialList.h"
#include "ROcclusionList.h"
#include "RDummyList.h"
#include "RSolidBsp.h"
#include "RNavigationMesh.h"

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
	int nCall,nPolygon;
	int nMapObjectFrustumCulled;
	int nMapObjectOcclusionCulled;
	RSolidBspNode		*pLastColNode;
};

struct BSPVERTEX {

	float x, y, z;		// world position
//	float nx,ny,nz;		// normal				// 지금은 의미없다
    float tu1, tv1;		// texture coordinates
	float tu2, tv2;

	rvector *Coord() { return (rvector*)&x; }
//	rvector *Normal() { return (rvector*)&nx; }
}; 

//#define BSP_FVF	(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2)
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

class RMapObjectList : public list<ROBJECTINFO*> {
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

	int				nVertice;		// 버텍스 수
	BSPVERTEX		*pVertices;		// 버텍스
	int				nIndicesOffset;	// index가 시작하는곳의 옵셋
	int				nTriangleCount;	// 삼각형 갯수
	rplane			*pPlanes;		// 평면의 방정식(삼각형개수만큼)
	rvector			*pUAxis;		// uv 계산에 필요한 기준축
	rvector			*pVAxis;		// uv 계산에 필요한 기준축
};

class RSBspNode
{
public:
	int				nPolygon;
//	int				nPosition;			// vertex buffer 내의 위치.
	RPOLYGONINFO	*pInfo;
	RDrawInfo		*pDrawInfo;		// material별 해당 폴리곤을 그리기위한 정보

	int				nFrameCount;		// 마지막 렌더링된 프레임..

//	bool			bVisibletest;		// pvs 테스트용 . 임시.
//	bool			bSolid;

	RSBspNode		*m_pPositive,*m_pNegative;

	rplane plane;
	rboundingbox	bbTree;

	RSBspNode();
	virtual ~RSBspNode();

	RSBspNode *GetLeafNode(rvector &pos);
	void DrawWireFrame(int nFace,DWORD color);
	void DrawBoundingBox(DWORD color);
};

// 자잘한 lightmap 을 큰 텍스쳐에 한번에 올리는걸 도와준다.

typedef list<POINT> RFREEBLOCKLIST;
struct RLIGHTMAPTEXTURE {
	int nSize;
	DWORD *data;
	bool bLoaded;
	POINT position;
	int	nLightmapIndex;
};

struct RBSPMATERIAL : public RMATERIAL {
	RBSPMATERIAL() {	texture=NULL; }
	RBSPMATERIAL(RMATERIAL *mat)
	{
		Ambient=mat->Ambient;
		Diffuse=mat->Diffuse;
		DiffuseMap=mat->DiffuseMap;
		dwFlags=mat->dwFlags;
		Name=mat->Name;
		Power=mat->Power;
		Specular=mat->Specular;
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

	void SetSize(int nSize) { m_nSize=nSize; }
	void SetData(DWORD *pData) { Destroy(); m_pData=pData; }

	bool Add(DWORD *data,int nSize,POINT *retpoint);
	// 2^nLevel 크기의 사용되지않은 RECT를 빼내준다..
	bool GetFreeRect(int nLevel,POINT *pt);

	void Save(const char *filename);

	// 실행과는 관계가 없고. 단순히 참고하기 위한 데이터임
	// 남은 양 0~1 을 계산해서 m_fUnused로 넣는다
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
	FogInfo(){ bFogEnable = false; }
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

// 에디터에서 Generate Lightmap에 쓰일 Progress bar 나타낼때 쓰는 콜백펑션 타입. 취소되었으면 리턴 = false
typedef bool (*RGENERATELIGHTMAPCALLBACK)(float fProgress);



class RBspObject
{ 
public:
	enum ROpenFlag{
		ROF_RUNTIME,
		ROF_EDITOR
	} m_OpenMode;

	RBspObject();
	~RBspObject();
	RBspObject(const RBspObject&) = delete;
	RBspObject(RBspObject&&) = default;

	void ClearLightmaps();

	// open 을 수행하면 기본 확장자로 다음의 Open...펑션들을 순서대로 부른다.
	bool Open(const char *, ROpenFlag nOpenFlag = ROF_RUNTIME, RFPROGRESSCALLBACK pfnProgressCallback = nullptr, void *CallbackParam = nullptr, bool PhysOnly = false);

	bool OpenDescription(const char *);				// 디스크립션 팔일		.xml 파일을 연다.
	bool OpenRs(const char *);						// 실제 월드 정보파일	.rs 파일을 연다. 
	bool OpenBsp(const char *);						// bsp 정보 파일		.bsp 파일을 연다. 
	bool OpenLightmap();							// 라이트맵 정보파일	.lm  파일을 연다.
//	bool OpenPathNode(const char *);				// 경로 정보파일		.pat 파일을 연다.
	bool OpenCol(const char *);						// collision 정보 파일	.col 파일을 연다. 
	bool OpenNav(const char *);						// 네비게이션맵 정보파일 .nav 파일을 연다.

	void OptimizeBoundingBox();	// 게임의 런타임에서는 실제바운딩박스로 타이트하게 잡아준다.
	
	bool IsVisible(rboundingbox &bb);		// occlusion 에 의해 가려져있으면 false 를 리턴.

	bool Draw();
	void DrawObjects();


	bool DrawLight(RSBspNode *pNode,int nMaterial);
	void DrawLight(D3DLIGHT9 *pLight);			// 광원 처리를.. 멀티 패스로 덧그린다.

	bool GenerateLightmap(const char *filename, int nMaxLightmapSize, int nMinLightmapSize, int nSuperSample, float fToler, RGENERATELIGHTMAPCALLBACK pProgressFn = nullptr);
	bool GeneratePathData(const char *filename, float fAngle, float fToler);
	void GeneratePathNodeTable();

	void SetWireframeMode(bool bWireframe) { m_bWireframe=bWireframe;}
	bool GetWireframeMode() { return m_bWireframe; }
	void SetShowLightmapMode(bool bShowLightmap) { m_bShowLightmap=bShowLightmap; }
	bool GetShowLightmapMode() { return m_bShowLightmap; }

	// 월드 좌표 pos 에서 dir 방향의 반직선에 대해 pick !
	bool Pick(const rvector &pos, const rvector &dir,RBSPPICKINFO *pOut,DWORD dwPassFlag=RM_FLAG_ADDITIVE | RM_FLAG_USEOPACITY | RM_FLAG_HIDE);
	bool PickTo(const rvector &pos, const rvector &to,RBSPPICKINFO *pOut,DWORD dwPassFlag=RM_FLAG_ADDITIVE | RM_FLAG_USEOPACITY | RM_FLAG_HIDE);
	bool PickOcTree(const rvector &pos, const rvector &dir,RBSPPICKINFO *pOut,DWORD dwPassFlag=RM_FLAG_ADDITIVE | RM_FLAG_USEOPACITY | RM_FLAG_HIDE);

	// pathnode 는 봉인..
	/*
	// 화면 x,y 좌표에 있는 pathnode 를 리턴한다..
	bool PickPathNode(int x,int y,RPathNode **pOut,rvector *ColPos);
	// 월드 좌표 from 에서 to 로 가는 반직선에 대해 pick pathnode !
	bool PickPathNode(rvector &from,rvector &to,RPathNode **pOut,rvector *ColPos);

	RPathList *GetPathList() { return &m_PathNodes; }
	RPathNode *GetPathNode(RSBspNode *pNode,int nIndex);
*/

	// 해당 위치의 lightmap 을 얻어낸다.
	DWORD GetLightmap(rvector &Pos,RSBspNode *pNode,int nIndex);

	// 해당 폴리곤의 Material을 얻어낸다.
	RBSPMATERIAL *GetMaterial(RSBspNode *pNode,int nIndex)		{ return GetMaterial(pNode->pInfo[nIndex].nMaterial); }

	// material 을 얻어낸다.
	int	GetMaterialCount()	{ return m_nMaterial; }
	RBSPMATERIAL *GetMaterial(int nIndex);

	RMapObjectList	*GetMapObjectList() { return &m_ObjectList; }
	RDummyList		*GetDummyList()		{ return &m_DummyList; }
	RBaseTexture *GetBaseTexture(int n) { if(n>=0 && n<m_nMaterial) return m_pMaterials[n].texture; return NULL; }

	RLightList *GetMapLightList() { return &m_StaticMapLightList; }
	RLightList *GetObjectLightList() { return &m_StaticObjectLightList; }
	RLightList *GetSunLightList() { return &m_StaticSunLigthtList; }

	RSBspNode *GetOcRootNode() { return m_pOcRoot; }
	RSBspNode *GetRootNode() { return m_pBspRoot; }

	rvector GetDimension();

	int	GetVertexCount()		{ return m_nVertices; }
	int	GetPolygonCount()		{ return m_nPolygon; }
	int GetNodeCount()			{ return m_nNodeCount; }
	int	GetBspPolygonCount()	{ return m_nBspPolygon; }
	int GetBspNodeCount()		{ return m_nBspNodeCount; }
	int GetConvexPolygonCount() { return m_nConvexPolygon; }
	int GetLightmapCount()		{ return m_nLightmap; }
	float GetUnusedLightmapSize(int index) { return m_LightmapList[index]->m_fUnused; }

	// origin 에서 targetpos 로 이동하는데 미끄러짐을 감안해서 targetpos 를 조절해서 리턴해준다.
	bool CheckWall(rvector &origin,rvector &targetpos,float fRadius,float fHeight=0.f,RCOLLISIONMETHOD method=RCW_CYLINDER,int nDepth=0,rplane *pimpactplane=NULL);
//	bool CheckWall2(RImpactPlanes &impactPlanes,rvector &origin,rvector &targetpos,float fRadius,float fHeight=0.f,RCOLLISIONMETHOD method=RCW_CYLINDER);

	// center를 밑면의 중심 pole을 꼭지점, radius를 밑면의 반지름으로 하는 콘에 들어오는 가장 가까운 점을 리턴. (카메라에 사용)
//	bool CheckWall_Corn(rvector *pOut,rvector &center,rvector &pole,float fRadius);

	// solid 영역 안에 있는지 ?
	bool CheckSolid(rvector &pos,float fRadius,float fHeight=0.f,RCOLLISIONMETHOD method=RCW_CYLINDER);
	
	// 위치에서 바닥에 닿는 점을 구한다.
	rvector GetFloor(rvector &origin,float fRadius,float fHeight,rplane *pimpactplane=NULL);
	
	void OnInvalidate();
	void OnRestore();

	// 맵에 등록된 오브젝트 이외의 오브젝트 처리 ( 캐릭터 선택화면등 )
	void SetObjectLight(rvector pos);
	void SetCharactorLight(rvector pos);

	bool GetShadowPosition( rvector& pos_, rvector& dir_, rvector* outNormal_, rvector* outPos_ );

	RMeshMgr*	GetMeshManager() {
		return &m_MeshList;
	}

	void test_MakePortals();

	// debug 를 위해 그려볼만한것들.
	void DrawPathNode();
	void DrawBoundingBox();
	void DrawOcclusions();
	void DrawNormal(int nIndex,float fSize=1.f);	// index : convex polygon index

	void DrawCollision_Polygon();
	void DrawCollision_Solid();

	void DrawSolid();					// 모든 solid 노드들을 그린다
	void DrawSolidNode();				// 마지막 충돌한 노드를 그린다
	void DrawColNodePolygon(rvector &pos);

	void DrawNavi_Polygon();
	void DrawNavi_Links();

	RSolidBspNode *GetColRoot() { return m_pColRoot; }

	void LightMapOnOff(bool b);
	static void SetDrawLightMap(bool b);

	FogInfo GetFogInfo(){ return m_FogInfo;}
	list<AmbSndInfo*> GetAmbSndList() { return m_AmbSndInfoList;	}

	void GetNormal(int nConvexPolygon,rvector &position,rvector *normal);

	static bool CreateShadeMap(const char *szShadeMap);
	static void DestroyShadeMap();

	RDEBUGINFO *GetDebugInfo() { return &m_DebugInfo; }
	RNavigationMesh* GetNavigationMesh() { return &m_NavigationMesh; }

	void SetMapObjectOcclusion(bool b) { m_bNotOcclusion = b;}
private:

	bool PhysOnly;

	string m_filename,m_descfilename;
	
	// 에디터및 디버그를 위한 모드 
	bool m_bWireframe;
	bool m_bShowLightmap;

// 직접호출될일은 없다..
	bool DrawTNT(RSBspNode *bspNode,int nMaterial);	// no hardware T&L
	bool Draw(RSBspNode *bspNode,int nMaterial);

	void SetDiffuseMap(int nMaterial);

	bool Pick(RSBspNode *pNode, const rvector &v0, const rvector &v1);
//	bool PickCol(RSolidBspNode *pNode,rvector v0,rvector v1);
//	bool PickPathNode(RSBspNode *pNode);
	bool PickShadow(rvector &pos,rvector &to,RBSPPICKINFO *pOut);
	bool PickShadow(RSBspNode *pNode,rvector &v0,rvector &v1);

	void ChooseNodes(RSBspNode *bspNode);
	int ChooseNodes(RSBspNode *bspNode,rvector &center,float fRadius);			// 리턴값은 선택된 노드의 개수
	void TraverseTreeAndRender(RSBspNode *bspNode);
	void DrawNodeFaces(RSBspNode *bspNode);

	inline RSBspNode *GetLeafNode(rvector &pos) { return m_pBspRoot->GetLeafNode(pos); }

	void GetFloor(rvector *ret,RSBspNode *pNode,rvector &origin,const rvector &diff);

// for loading
	bool ReadString(MZFile *pfile,char *buffer,int nBufferSize);
	bool Open_Nodes(RSBspNode *pNode,MZFile *pfile);
	bool Open_ColNodes(RSolidBspNode *pNode,MZFile *pfile);
	bool Open_MaterialList(MXmlElement *pElement);
	bool Open_LightList(MXmlElement *pElement);
	bool Open_ObjectList(MXmlElement *pElement);
	bool Open_DummyList(MXmlElement *pElement);
//	bool Open_LightList(MZFile *pfile);
	bool Open_ConvexPolygons(MZFile *pfile);
	bool Open_OcclusionList(MXmlElement *pElement);
	bool Make_LenzFalreList();
	bool Set_Fog(MXmlElement *pElement);
	bool Set_AmbSound(MXmlElement *pElement);

	void CreatePolygonTable();
	void CreatePolygonTable(RSBspNode *pNode);
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

// 실제 트리
	BSPVERTEX *m_pBspVertices,*m_pOcVertices;
	WORD	*m_pBspIndices,*m_pOcIndices;
	RSBspNode *m_pBspRoot,*m_pOcRoot;
	RPOLYGONINFO *m_pBspInfo,*m_pOcInfo;
	int m_nPolygon,m_nNodeCount,m_nVertices,m_nIndices;
	int m_nBspPolygon,m_nBspNodeCount,m_nBspVertices,m_nBspIndices;
	LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;
	LPDIRECT3DINDEXBUFFER9 m_pIndexBuffer;
	
// Materials & texture map
	int m_nMaterial;
	RBSPMATERIAL *m_pMaterials;

	rplane m_localViewFrustum[6];

// occlusions
	ROcclusionList m_OcclusionList;

	/*
	int m_nOcclusion;
	ROcclusion	*m_pOcclusion;
	*/


// 실제로 텍스쳐 메모리에 올라가는 라이트맵
	int							m_nLightmap;
	LPDIRECT3DTEXTURE9			*m_ppLightmapTextures;
	vector<RBspLightmapManager*> m_LightmapList;

// lightmap 텍스처로 생성시 필요한 것..
	void CalcLightmapUV(RSBspNode *pNode,int *pLightmapInfo,vector<RLIGHTMAPTEXTURE*> *pLightmaps);

// interpolated normal
	void GetNormal(RCONVEXPOLYGONINFO *poly,rvector &position,rvector *normal,int au,int av);
// interpolated uv
	void GetUV(rvector &Pos,RSBspNode *pNode,int nIndex,float *uv);

// 원본 폴리곤 보관
	int					m_nConvexPolygon,m_nConvexVertices;
	rvector				*m_pConvexVertices;
	rvector				*m_pConvexNormals;
	RCONVEXPOLYGONINFO	*m_pConvexPolygons;

// 광원정보들..
	rvector		m_AmbientLight;
	RLightList	m_StaticMapLightList;
	RLightList	m_StaticObjectLightList;
	RLightList	m_StaticSunLigthtList;

// pathfinding 에 필요한 자료
//	RPathList m_PathNodes;

// 맵에 기본적으로 있는 object 들..
	RMeshMgr			m_MeshList;
	RAnimationMgr		m_AniList;
	RMapObjectList		m_ObjectList;
	bool				m_bNotOcclusion;

// 충돌체크용 맵
	RSolidBspNode		*m_pColRoot;
	rvector				*m_pColVertices;

// 네비게이션용 맵
	RNavigationMesh		m_NavigationMesh;


// 더미 리스트
	RDummyList	m_DummyList;

// Fog정보
	FogInfo m_FogInfo;

// 사운드 정보
	list<AmbSndInfo*>	m_AmbSndInfoList;

// 디버그 정보
	RDEBUGINFO			m_DebugInfo;
};

extern int g_nPoly, g_nCall;

_NAMESPACE_REALSPACE2_END


#endif