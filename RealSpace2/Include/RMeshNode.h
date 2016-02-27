#ifndef _RMeshNode_h
#define _RMeshNode_h

#include "RMeshNodeData.h"
#include "RShaderMgr.h"

_NAMESPACE_REALSPACE2_BEGIN


//#define USE_TOON_RENDER


///////////////////////////////////////////////////////////////
// 사용 구조체 정의

class CIndexBufferMake;

///////////////////////////////////////////////////////////////

enum CalcVertexBufferBboxMode
{
	CalcVertexBufferBboxMode_None = 0,

	CalcVertexBufferBboxMode_VertexAni,
	CalcVertexBufferBboxMode_Physique,
	CalcVertexBufferBboxMode_TM_MapObject,
	CalcVertexBufferBboxMode_TM_Object,

	CalcVertexBufferBboxMode_End,
};


class RBoneBaseMatrix
{
public:
	RBoneBaseMatrix() {
		D3DXMatrixIdentity(&mat);
		id = -1;
	}
	D3DXMATRIX	mat;
	int			id;
};

#define MAX_MATRIX		60
#define MAX_PRIMITIVE	10

class RMeshNodeInfo
{
public:
	RMeshNodeInfo();
	virtual ~ RMeshNodeInfo();

public:

	bool	m_isAddMeshNode;
	bool	m_isCollisionMesh;
	bool	m_isDummyMesh;
	bool	m_isWeaponMesh;
	bool	m_isDummy;
	bool	m_isAlphaMtrl;
	bool	m_isLastModel;

	bool	m_isClothMeshNode;
	bool	m_bClothMeshNodeSkip;
	float	m_vis_alpha;

	int						m_nAlign;

	CutParts				m_CutPartsType;			// 노드가 상체,하체에 속하는지 여부 타잎
	LookAtParts				m_LookAtParts;			// 프로그램으로 노드 관리 할 특수 노드 타잎
	WeaponDummyType			m_WeaponDummyType;		// 무기 모델에 붙는 더미들 타잎

	RMeshPartsType			m_PartsType;			// 모델 파츠 + 장비더미 타잎
	RMeshPartsPosInfoType	m_PartsPosInfoType;		// 바이패드 + 더미 위치용 타잎

	bool			m_bNpcWeaponMeshNode;			// NPC 의 손에 든 무기 모델인지 구분..
	float			m_AlphaSortValue;				// 캐릭터의 알파값 파츠의 경우 소팅값..

};

class RBatch
{
public:
	RBatch();
	virtual ~RBatch();

	bool CreateVertexBuffer(char* pVert,DWORD fvf,int vertexsize,int vert_num,DWORD flag);
	bool UpdateVertexBuffer(char* pVert);
	bool UpdateVertexBufferSoft(char* pVert);

	bool CreateIndexBuffer(int index,WORD* pIndex,int _size);

public:

	RIndexBuffer*		m_ib[MAX_PRIMITIVE];

	RVertexBuffer*		m_vb;
	RVertexBuffer*		m_vsb;

	bool				m_bIsMakeVertexBuffer;

};

class RMeshNode :public RMeshNodeData , public RMeshNodeMtrl, public RMeshNodeInfo , public RBatch
{
public:

	RMeshNode();
	virtual ~RMeshNode();

	///////////////////////////////////////////
	// animation

	void ConnectToNameID();// 자기 이름으로 연결
	bool ConnectMtrl();

	void UpdateNodeBuffer();
	void MakeNodeBuffer(DWORD flag);

	void MakeVertexBuffer(int index,bool lvert,char* pBuf,int _vsize,DWORD flag);
	bool MakeVSVertexBuffer();// vertex update 의 역할도 겸한다..	

	void RenderNodeVS(RMesh* pMesh,D3DXMATRIX* world_mat,ESHADER shader_ = SHADER_SKIN );

	void Render(D3DXMATRIX* pWorldMatrix=NULL);

	RBoneBaseMatrix* GetBaseMatrix(int pid);		// 각정점마다 index 지정하기 싫어서

	bool isSoftRender();

	void CalcVertexBuffer(D3DXMATRIX* world_mat,bool box=false);
	bool CalcPickVertexBuffer(D3DXMATRIX* world_mat,D3DXVECTOR3* pVec);
	int	 CalcVertexBuffer_VertexAni(int frame);
	void CalcVertexBuffer_Physique(D3DXMATRIX* world_mat,int frame);
	void CalcVertexBuffer_Tm(D3DXMATRIX* world_mat,int frame);
	void CalcVertexBuffer_Bbox(CalcVertexBufferBboxMode nBboxMode,rmatrix& mat);

	void CalcVertexNormal(D3DXMATRIX* world_mat);

	void CheckAlign(rmatrix* worldmat);
	void CheckAlignMapObject(rmatrix& hr_mat); // 맵오브젝트만..

	float GetNodeVisValue();
	int   GetNodeAniSetFrame();

	bool isAlphaMtrlNode();

	void ToonRenderSettingOnOld(RMtrl* pMtrl);// 임시,,
	void ToonRenderSettingOn(RMtrl* pMtrl);// 임시,,
	void ToonRenderSettingOff();

	void ToonRenderSilhouetteSettingOn();// 임시,,
	void ToonRenderSilhouetteSettingOff();

private:

	bool SetBVertData(RBlendVertex* pBVert,int i,int j ,int pv_index,int* DifferenceMap,int& matrixIndex);

public:

	///////////////////////////////////////////
	// reference pointer

	RMesh*			m_pParentMesh;	// 자신의 부모 모델 파일...
	RMesh*			m_pBaseMesh;	// 계층형 기본 모델 파일...
	RMeshNode*		m_pParent;
	RMeshNode*		m_Next;
	RMeshNode*		m_ChildRoot;

	float				m_fDist;

	RBoneBaseMatrix*	m_BoneBaseMatrix;
	int					m_nBoneBaseMatrixCnt;

	int m_MatrixMap[MAX_MATRIX];			// Matrix Register와 Bone의 index를 연결하기 위한 맵
	int m_MatrixCount;						// 쓰이는 Matrix의 갯수( 초기값 : -1 )

	D3DXMATRIX		m_ModelWorldMatrix;
	
	bool			m_bConnectPhysique;

#ifndef _BLEND_ANIMATION
	RAnimationNode*	m_pAnimationNode;
#else
	RAnimationNode* m_pAnimationNode[2];
#endif

};

_NAMESPACE_REALSPACE2_END

#endif//_RMeshNode_h
