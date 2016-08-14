#if !defined(AFX_RMESH_H__6FD23F3A_D138_4F55_B03F_A629D35788CB__INCLUDED_)
#define AFX_RMESH_H__6FD23F3A_D138_4F55_B03F_A629D35788CB__INCLUDED_

#include <vector>
#include <unordered_map>

#include "RMeshNode.h"
#include "RAnimationMgr.h"
#include "mempool.h"

class MXmlElement;

_NAMESPACE_REALSPACE2_BEGIN

/////////////////////////////////////////////////////////////////////
// 각각의 캐릭터가 가지는 장비 메시 파일과 와 에니메이션 파일,
// 머터리얼 파일등을 따로 관리한다. 통합관리 안함. 
// ( 오브젝트 단위로 올리고 내리고 )

#pragma warning(disable : 4996)

typedef RHashList<RMeshNode*>			RMeshNodeHashList;
typedef RHashList<RMeshNode*>::iterator	RMeshNodeHashList_Iter;

/*
class AniTreeNodeMgr {
public:
	AniTreeNodeMgr() {
		m_AniTreeRoot = NULL;
	}
	~AniTreeNodeMgr() {
	}

	void AddNode(RMeshNode* pnode,RMeshNode* node) {

		if(pnode==NULL){
			if(m_AniTreeRoot==NULL) {
				m_AniTreeRoot = node;
				node->m_Next = NULL;
			}
			else {
				node->m_Next = m_AniTreeRoot;
				m_AniTreeRoot = node;
			}
		}
		else {
			if(pnode->m_ChildRoot==NULL) {
				pnode->m_ChildRoot = node;
				node->m_Next = NULL;
			} else {
				node->m_Next = pnode->m_ChildRoot;
				pnode->m_ChildRoot = node;
			}
		}
	}

	RMeshNode* m_AniTreeRoot;
};
*/

//////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////

class RMeshMgr;
class RVisualMesh;
class ROcclusionList;
class RPickInfo;

extern int g_poly_render_cnt;

////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////

enum {

	eRRenderNode_Alpha = 0,
	eRRenderNode_Add,

	eRRenderNode_End,
};

//	visual mesh

class RRenderNode //: public CMemPoolSm<RRenderNode>
{
public:
	RRenderNode() {
		Clear();
	}

	RRenderNode(int Rmode,rmatrix& m,RMeshNode* pMNode,int nMtrl,int begin,int size,float vis_alpha) {
		Clear();
		Set(Rmode,m,pMNode,nMtrl,begin,size,vis_alpha);
	}

	// 임시 예전것때문..
	void Set(int Rmode,rmatrix& m,RMeshNode* pMNode,RMtrl* pMtrl,int begin,int size,float vis_alpha) {
		m_RenderMode = Rmode;
		m_matWorld = m;
		m_pNode = pMNode;
		m_pMtrl = pMtrl;
		m_begin = begin;
		m_size	= size;
		m_vis_alpha = vis_alpha;
	}

	void Set(int Rmode,rmatrix& m,RMeshNode* pMNode,int nMtrl,int begin,int size,float vis_alpha) {
		m_RenderMode = Rmode;
		m_matWorld = m;
		m_pNode = pMNode;
		m_nMtrl = nMtrl;
		m_begin = begin;
		m_size	= size;
		m_vis_alpha = vis_alpha;
	}

	void Clear() {

		D3DXMatrixIdentity(&m_matWorld);
		m_pNode = NULL;
		m_nMtrl = 0;
		m_pMtrl = NULL;
		m_RenderMode = eRRenderNode_Alpha;

		m_begin = 0;
		m_size	= 0;

		m_vis_alpha = 1.0f;
	}

	void Render();

public:

	float		m_vis_alpha;
	int			m_RenderMode;
	rmatrix		m_matWorld;
	RMeshNode*	m_pNode;

	int			m_nMtrl;
	RMtrl*		m_pMtrl;

	int			m_begin;
	int			m_size;
};

class RRenderNodeList : public list<RRenderNode*>
{
public:
	RRenderNodeList() {

	}

	void Render() {

		if(empty())
			return;

		list<RRenderNode*>::iterator node = begin();

		RRenderNode* pNode;

		while( node != end() ) {

			pNode = (*node);

			if(pNode)
				(pNode)->Render();

			node++;
		}
	}

	void Clear() {

		if(empty()) return;

		list<RRenderNode*>::iterator node = begin();

		RRenderNode* pNode;

		for(node = begin(); node != end(); ) {
			pNode = (*node);
			delete pNode;
			pNode = NULL;
			node = erase(node);// ++node
		}

		clear();
	}

#ifdef _DEBUG
	RRenderNode* m_data[1024*1000];//debug
#endif

};

// 작동하면 메모리풀과 자체 list 를 사용한다..

#define VERTEX_NODE_MAX_CNT		1000
#define LVERTEX_NODE_MAX_CNT	1000

#define DEF_RRenderNode_ADD_SIZE 10

class RRenderNodeMgr
{
public:
	RRenderNodeMgr() {
		m_nTotalCount = 0;
		Clear();
	}

	virtual ~RRenderNodeMgr() {
	}

	int Add(rmatrix& m,int mode,RMeshNode* pMNode,int nMtrl);

	void Clear();

	void Render();

	int m_nTotalCount;

public:

	static bool m_bRenderBuffer;

public:

	RRenderNodeList	m_RenderNodeList[eRRenderNode_End];
};

// effect , weapon , character , mapobject
// soft 라도 각자 자신의 버퍼를 가지고 있어야 한다.

inline bool render_alpha_sort(RMeshNode* _a,RMeshNode* _b) 
{
	if( _a->m_fDist < _b->m_fDist )
		return true;
	return false;
}

class RMeshRenderMgr
{
	RMeshRenderMgr() {

	}

public:


	~RMeshRenderMgr() {

	}

	enum RMeshRenderMgrType {

		RRMT_Normal = 0,
		RRMT_DIffuse,
		RRMT_Alpha,
		RRMT_Add,
		RRMT_End
	};

	void Add(int RenderType,RMeshNode* pNode) {

	}

	void Sort(rvector vCameraPos) {
//		for(int i=0;i<RRMT_End;i++) {
//			m_node_list[i].sort(func);
//		}
//		알파만 소트..
//		카메라와의 거리계산하고..
		m_node_list[RRMT_Alpha].sort(render_alpha_sort);
	}

	// alpha 거리 sort 포기?

	void Render() {
		// normal,diffuse
		// alpha
		// add 
/*
		for(int i=0;i<RRMT_End;i++) {
			m_node_list[i]
			
		}
*/
	}

	void Clear() {
		for(int i=0;i<RRMT_End;i++) {
//			m_node_list[i].clear(func);
		}
	}

	RMeshNodeHashList m_node_list[RRMT_End];
};

class RParticleLinkInfo
{
public:
	RParticleLinkInfo() {

	}

	void Set(char* _name,char* _dummy_name) {
		name = _name;
		dummy_name = _dummy_name;
	}

public:
	string name;
	string dummy_name;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class RMesh {
public:

	RMesh();
	~RMesh();
	RMesh(const RMesh& rhs);

	void Init();
	void Destroy();


	bool ReadXml(char* fname);
	bool SaveXml(char* fname);

	bool ReadXmlElement(MXmlElement* pNode,char* Path);

	bool ReadElu(const char* fname);
	bool SaveElu(char* fname);

	void RenderFrame();

	void Render(D3DXMATRIX* world_mat,bool NoPartsChange=false);

//private:

	void RenderFrameSingleParts();
	void RenderFrameMultiParts();

	void CalcLookAtParts(D3DXMATRIX& pAniMat,RMeshNode* pMeshNode,RVisualMesh* pVisualMesh);
	void CalcLookAtParts2(D3DXMATRIX& pAniMat,RMeshNode* pMeshNode,RVisualMesh* pVisualMesh);

	void RenderSub(D3DXMATRIX* world_mat,bool NoPartsChange,bool RenderBuffer);
	bool CalcParts(RMeshNode* pTMeshNode,RMeshNode* pMeshNode,bool NoPartsChange);

	void RenderNode(RMeshNode *pMeshNode,D3DXMATRIX* world_mat);

private:

	bool SetAnimation1Parts(RAnimation* pAniSet);
	bool SetAnimation2Parts(RAnimation* pAniSet,RAnimation* pAniSetUpper);

public:

	RAnimation* GetNodeAniSet(RMeshNode* pNode);

//	static void RenderSBegin();
//	static void RenderSEnd();

	void SetLitVertexModel(bool v);

	// RenderState

	void SetShaderDiffuseMap(RMtrl* pMtrl,DWORD color);
	void SetShaderAlphaMap();
	void SetShaderAdditiveMap();
	void SetShaderNormalMap();
	void SetShaderAlphaTestMap(int value,float fVis);

	void SetShaderDiffuseMap_OFF();
	void SetShaderAlphaMap_OFF();
	void SetShaderAdditiveMap_OFF();
	void SetShaderNormalMap_OFF();
	void SetShaderAlphaTestMap_OFF();

	void SetMtrlUvAni_ON();
	void SetMtrlUvAni_OFF();

	void SetCharacterMtrl_ON(RMtrl* pMtrl,RMeshNode* pMNode,float vis_alpha,DWORD color);
	void SetCharacterMtrl_OFF(RMtrl* pMtrl,float vis_alpha);
	int  GetCharacterMtrlMode(RMtrl* pMtrl,float vis_alpha);

	void ReloadAnimation();	//다시 로딩 하는것이 아니고..로딩 안된것을 읽는다..

	void SkyBoxMtrlOn();
	void SkyBoxMtrlOff();

public:

	float GetMeshVis();
	void  SetMeshVis(float vis);

	float GetMeshNodeVis(RMeshNode* pNode);

	void SetVisualMesh(RVisualMesh* vm);

	RVisualMesh* GetVisualMesh();

	RMeshNode* FindNode(RMeshPartsPosInfoType type);

	void CheckNodeAlphaMtrl();
	void ClearVoidMtrl();

	bool ConnectPhysiqueParent(RMeshNode* pNode);
	bool CalcLocalMatrix(RMeshNode* pNode);

	void DelMeshList();

	int _FindMeshId(int e_name);
	int _FindMeshId(const char* name);
	int	 FindMeshId(RMeshNode* pNode);

	int  FindMeshParentId(RMeshNode* pMeshNode);
	int  FindMeshId(RAnimationNode* pANode);
	int  FindMeshIdSub(RAnimationNode* pANode);

	void RenderBox(D3DXMATRIX* world_mat=NULL);
	void CalcBox(D3DXMATRIX* world_mat=NULL);
	void CalcBoxFast(D3DXMATRIX* world_mat=NULL);
	void CalcBoxNode(D3DXMATRIX* world_mat=NULL);

	void  SetFrame(int nFrame,int nFrame2 = -1);

	bool AddNode(char* name,char* pname,rmatrix& base_mat);
	bool DelNode(RMeshNode* data); // 제거시 자신을 부모로 가지는 오브젝트에 대해서 고려~

	bool ConnectAnimation(RAnimation* pAniSet);
	bool ConnectMtrl();

	void CheckNameToType(RMeshNode* pNode);
	void ConnectMatrix();

	void MakeAllNodeVertexBuffer();

	bool CheckOcclusion( RMeshNode *pMeshNode );

	void CalcNodeMatrixBBox(RMeshNode* pNode);
	void CalcBBox(D3DXVECTOR3* v);
	void SubCalcBBox(D3DXVECTOR3* max,D3DXVECTOR3* min,D3DXVECTOR3* v);

//private:

	void _RGetRotAniMat(RMeshNode* pMeshNode,int frame,D3DXMATRIX& t_ani_mat);
	void _RGetPosAniMat(RMeshNode* pMeshNode,int frame,D3DXMATRIX& t_ani_mat);
	void _RGetAniMat(RMeshNode* pMeshNode,int frame,D3DXMATRIX& t_ani_mat);

public:

	bool SetAnimation(RAnimation* pAniSet,RAnimation* pAniSetUpper=NULL);	//외부의 에니메이션과 연결시
	bool SetAnimation(char* ani_name,char* ani_name_lower = NULL);			//내부의 에니메이션과 연결시
	void ClearAnimation();

	void SetMtrlAutoLoad(bool b);
	bool GetMtrlAutoLoad();

	void SetMapObject(bool b);
	bool GetMapObject();

	char* GetFileName();
	void  SetFileName(const char* name);
	bool  CmpFileName(char* name);

	char* GetName();
	void  SetName(char* name);
	bool  CmpName(char* name);

	void GetMeshData(RMeshPartsType type,vector<RMeshNode*>& nodetable);
	void GetPartsNode(RMeshPartsType type,vector<RMeshNode*>& nodetable);

	RMeshNode* GetMeshData(RMeshPartsType type);
	RMeshNode* GetMeshData(const char* name);
	RMeshNode* GetPartsNode(const char* name);

	// 항상쓰는것도아니니까 나중에속도가 떨어진다면 사용
//	RMeshNode* GetPartsNode(RMeshPartsType parts,char* name);

	// RVisualMesh class 로 옮기기 ( m_world_mat 내장한... )

	bool Pick(int x,int y,RPickInfo* pInfo,rmatrix* world_mat=NULL);
	bool Pick(const rvector* vInVec,RPickInfo* pInfo,rmatrix* world_mat=NULL);
	bool Pick(const rvector& pos, const rvector& dir,RPickInfo* pInfo,rmatrix* world_mat=NULL);

	void ClearMtrl();
	void SetBaseMtrlMesh(RMesh* pMesh);
	void SetScale(rvector& v);
	void ClearScale();

	void SetPickingType(RPickType type);

	RPickType GetPickingType();

	// 모델이 무기모델인경우 의미를 갖는다...
	void SetMeshWeaponMotionType(RWeaponMotionType t);
	RWeaponMotionType GetMeshWeaponMotionType();

	// 맵 오브젝트의 기본 위치를 얻기 위해 사용
	// 계층을 가지는 에니메이션이 사용되는 경우는 bbox 를 구해서 중심 위치를 사용해야함..
	rvector GetOrgPosition();

	void SetPhysiqueMeshMesh(bool b);
	bool GetPhysiqueMesh();

	bool isVertexAnimation(RMeshNode* pNode);

	void SetSpRenderMode(int mode);

private:

	bool CalcIntersectsTriangle(const rvector* vInVec,RPickInfo* pInfo, D3DXMATRIX* world_mat=NULL,bool fastmode=false);

	void GetNodeAniMatrix(RMeshNode* pMeshNode,D3DXMATRIX& m);

	RMeshNode* UpdateNodeAniMatrix(RMeshNode* pNode);

public:

	RParticleLinkInfo	m_ParticleLinkInfo;

	float	m_fVis;

	bool	m_isPhysiqueMesh;
	bool	m_bUnUsededCheck;

	string	m_FileName;
	string	m_ModelName;

	int		m_id;

	RMeshNodeHashList	m_list;

//	RMeshNode*			m_data[MAX_MESH_NODE_TABLE];
	vector<RMeshNode*>	m_data;

	RWeaponMotionType	m_MeshWeaponMotionType;

	RPickType		m_PickingType;

	int				m_data_num;

	int				m_max_frame[2];
	int				m_frame[2];

	D3DXVECTOR3		m_vBBMax;
	D3DXVECTOR3		m_vBBMin;

	D3DXVECTOR3		m_vBBMaxNodeMatrix;
	D3DXVECTOR3		m_vBBMinNodeMatrix;

	bool			m_is_use_ani_set;	//temp

	rvector			m_vAddBipCenter;

	bool			m_isScale;
	rvector			m_vScale;

	RVisualMesh*	m_pVisualMesh;
	RAnimation*		m_pAniSet[2];

	/////////////////////////////////////
	// 자신의 파츠관리 남과 공유 안함..

	RMeshMgr*		m_parts_mgr;

	/////////////////////////////////////
	// 기본 mesh 에 가지고 사용. 
	// 전역관리 해도 관계 없음

	RAnimationMgr	m_ani_mgr;

	bool			m_isNPCMesh;
	bool			m_isCharacterMesh;
	bool			m_LitVertexModel;
	bool			m_mtrl_auto_load;
	bool			m_is_map_object;
	bool			m_bEffectSort;


	RMesh*			m_base_mtrl_mesh;

	////////////////////////////////////
	// mtrl list - tex list 
	
	RMtrlMgr		m_mtrl_list_ex;

	bool			mbSkyBox;

	static int	m_nRMeshCnt;

	static bool mHardwareAccellated;
	static unsigned int mNumMatrixConstant;

	bool m_isMultiAniSet;// 에니메이션을 타잎별로 어러벌 가지고 있는 모델

	int	 m_nSpRenderMode;

	bool m_isMeshLoaded;

	////////////////////////////////////
	// tool

	void		SetToolSelectNode(RMeshNode* pMNode) {	m_pToolSelectNode = pMNode;	}
	RMeshNode*	GetToolSelectNode()					 { return m_pToolSelectNode;  }
	void		SetToolSelectNodeName(char* name)	 { SetToolSelectNode(GetMeshData(name)); }

private:

	RMeshNode*	m_pToolSelectNode;

public:

public:

	// util

	static bool  m_bTextureRenderOnOff;
	static bool  m_bVertexNormalOnOff;
	static bool  m_bToolMesh;
	static bool  m_bSilhouette;
	static float m_fSilhouetteLength;


	static int  m_parts_mesh_loading_skip;

	static void SetPartsMeshLoadingSkip(int nSkip) { m_parts_mesh_loading_skip = nSkip;	}

	static void SetToolMesh(bool b) { m_bToolMesh = b; }
	static bool GetToolMesh() { return m_bToolMesh; }

	static void SetTextureRenderOnOff(bool b) { m_bTextureRenderOnOff = b; }
	static bool GetTextureRenderOnOff() { return m_bTextureRenderOnOff; }

	static void SetVertexNormalOnOff(bool b) { m_bVertexNormalOnOff = b; }
	static bool GetVertexNormalOnOff() { return m_bVertexNormalOnOff; }

};

class RPickInfo {
public:
	RPickInfo() {

		vOut.x = 0.f;
		vOut.y = 0.f;
		vOut.z = 0.f;

		t = 0;

		parts=eq_parts_etc;
	}

	~RPickInfo() {
		
	}

public:
	rvector		vOut;
	float		t;
	RMeshPartsType parts;
};

////////////////////////////////////////////////////////////
// help func

void ConvertMat(rmatrix& mat1,rmatrix& mat2);

bool RMeshRenderSBegin();
bool RMeshRenderSEnd();

void SetMtrl(RMtrl* pMtrl,float vis_alpha);

void RenderNodeMgr_Add(rmatrix& m,RMeshNode* pMNode,int nMtrl);
void RenderNodeMgr_Render();

_NAMESPACE_REALSPACE2_END


#endif // !defined(AFX_RMESH_H__6FD23F3A_D138_4F55_B03F_A629D35788CB__INCLUDED_)
