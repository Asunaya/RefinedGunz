#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <tchar.h>

#include "MXml.h"

#include "RealSpace2.h"
#include "RMesh.h"
#include "RMeshMgr.h"

#include "MDebug.h"

#include "RAnimationMgr.h"
#include "RVisualmeshMgr.h"

#include "MZFileSystem.h"
#include "fileinfo.h"

#include "RShaderMgr.h"

#ifndef _PUBLISH

#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);

#else

#define __BP(i,n) ;
#define __EP(i) ;

#endif

_USING_NAMESPACE_REALSPACE2

_NAMESPACE_REALSPACE2_BEGIN


bool			RMesh::mHardwareAccellated = false;
unsigned int	RMesh::mNumMatrixConstant = 0;

bool			RMesh::m_bTextureRenderOnOff = true;
bool			RMesh::m_bVertexNormalOnOff = true;
bool			RMesh::m_bToolMesh	= false;
bool			RMesh::m_bSilhouette = false;
float			RMesh::m_fSilhouetteLength = 300.f;
int				RMesh::m_parts_mesh_loading_skip = 0;

bool			RRenderNodeMgr::m_bRenderBuffer = false;


//////////////////////////////////////////////////////////////////////////////////
// RMesh Class


RMesh::RMesh() 
{
	Init();
}

RMesh::~RMesh() 
{
	Destroy();
}

// Do this sometime!! -__-
#define COPY_MEMBER(member) member = rhs.member
RMesh::RMesh(const RMesh& rhs)
{
	COPY_MEMBER(m_ParticleLinkInfo);
	COPY_MEMBER(m_fVis);
	COPY_MEMBER(m_isPhysiqueMesh);
	COPY_MEMBER(m_bUnUsededCheck);
	COPY_MEMBER(m_FileName);
	COPY_MEMBER(m_ModelName);
	COPY_MEMBER(m_id);
	COPY_MEMBER(m_list);
	COPY_MEMBER(m_data);
	COPY_MEMBER(m_MeshWeaponMotionType);
	COPY_MEMBER(m_PickingType);
	COPY_MEMBER(m_data_num);
	for (int i = 0; i < 2; i++)
		COPY_MEMBER(m_max_frame[i]);
	for (int i = 0; i < 2; i++)
		COPY_MEMBER(m_frame[i]);
	COPY_MEMBER(m_vBBMax);
	COPY_MEMBER(m_vBBMin);
	COPY_MEMBER(m_vBBMaxNodeMatrix);
	COPY_MEMBER(m_vBBMinNodeMatrix);
	COPY_MEMBER(m_is_use_ani_set);
	COPY_MEMBER(m_vAddBipCenter);
}
#undef COPY_MEMBER

void RMesh::Init()
{
	m_id			= -1;

	m_data_num		= 0;

	m_max_frame[0]	= 0;
	m_max_frame[1]	= 0;

	m_frame[0]		= 0;
	m_frame[1]		= 0;

	m_pVisualMesh	= NULL;

	m_is_use_ani_set = false;
	m_mtrl_auto_load = true;
	m_is_map_object = false;

	m_bUnUsededCheck = false;

	m_pAniSet[0] = NULL;
	m_pAniSet[1] = NULL;

	m_parts_mgr = NULL;

	m_base_mtrl_mesh = NULL;

	// 보통의 경우 재할당이 안일어나는 최대값을 지정..

	m_data.reserve(MAX_MESH_NODE_TABLE);//기본

	m_isScale = false;
	m_vScale = rvector(1.f,1.f,1.f);

//	m_PickingType = pick_collision_mesh;
	m_PickingType = pick_real_mesh;
	
	m_MeshWeaponMotionType	= eq_weapon_etc;//모델이 무기모델인경우 의미를 갖는다

	m_LitVertexModel	= false;
	m_bEffectSort		= false;
	m_isPhysiqueMesh	= false;

	m_fVis = 1.0f;

	mbSkyBox = false;

	m_vBBMax = D3DXVECTOR3(-9999.f,-9999.f,-9999.f);
	m_vBBMin = D3DXVECTOR3( 9999.f, 9999.f, 9999.f);

	m_vBBMaxNodeMatrix = m_vBBMax;
	m_vBBMinNodeMatrix = m_vBBMin;

	m_vAddBipCenter = rvector(0,0,0);

	m_isMultiAniSet = false;
	m_isCharacterMesh = false;
	m_isNPCMesh = false;

	m_nSpRenderMode = 0;

	m_isMeshLoaded = false;

	m_pToolSelectNode = NULL;

}

void RMesh::Destroy()
{
	DelMeshList();

	if(m_parts_mgr) {
		delete m_parts_mgr;
		m_parts_mgr = NULL;
	}

	m_isMeshLoaded = false;
}

void RMesh::ReloadAnimation() 
{
	m_ani_mgr.ReloadAll();
}

float RMesh::GetMeshVis() 
{ 
	return m_fVis;	
}

void  RMesh::SetMeshVis(float vis) 
{ 
	m_fVis = vis; 
}

float RMesh::GetMeshNodeVis(RMeshNode* pNode) 
{
	if(pNode==NULL) return 1.f;

	return max(min(pNode->m_vis_alpha,m_fVis),0.f);
}

void RMesh::SetVisualMesh(RVisualMesh* vm) 
{ 
	m_pVisualMesh = vm; 
}

RVisualMesh* RMesh::GetVisualMesh() 
{ 
	return m_pVisualMesh; 
}

void RMesh::SetMtrlAutoLoad(bool b) 
{ 
	m_mtrl_auto_load = b;
}

bool RMesh::GetMtrlAutoLoad() 
{ 
	return m_mtrl_auto_load; 
}

void RMesh::SetMapObject(bool b) 
{ 
	m_is_map_object = b; 
}

bool RMesh::GetMapObject() 
{ 
	return m_is_map_object; 
}


char* RMesh::GetFileName()
{
	return (char*)m_FileName.c_str();
}

void RMesh::SetFileName(const char* name)
{
	if(!name[0]) return;

	m_FileName = name;
}

void RMesh::SetBaseMtrlMesh(RMesh* pMesh) 
{
	m_base_mtrl_mesh = pMesh; 
}

void RMesh::SetScale(rvector& v) 
{
	m_vScale = v;
	m_isScale = true;
}

void RMesh::ClearScale() 
{
	m_vScale = rvector(1.f,1.f,1.f);
	m_isScale = false;
}

void RMesh::SetPickingType(RPickType type) 
{
	m_PickingType = type;
}

RPickType RMesh::GetPickingType() 
{
	return m_PickingType;
}

void RMesh::SetMeshWeaponMotionType(RWeaponMotionType t) 
{
	m_MeshWeaponMotionType = t;
}

RWeaponMotionType RMesh::GetMeshWeaponMotionType() 
{
	return m_MeshWeaponMotionType;
}

void RMesh::SetPhysiqueMeshMesh(bool b) 
{
	m_isPhysiqueMesh = b;
}

bool RMesh::GetPhysiqueMesh() 
{
	return m_isPhysiqueMesh;
}

bool RMesh::isVertexAnimation(RMeshNode* pNode) 
{
	RAnimation* pAniSet = GetNodeAniSet(pNode);

	if(pAniSet) 
		if( pAniSet->GetAnimationType() == RAniType_Vertex ) 
			return true;

	return false;
}

void RMesh::SetSpRenderMode(int mode) 
{
	m_nSpRenderMode = mode;
}

bool RMesh::CmpFileName(char* name)
{
	if(!name[0]) return false;
	
//	if(strcmp(m_name,name)==0) 
//		return true;
	if(m_FileName == name )
		return true;
	return false;
}

char* RMesh::GetName()
{
	return (char*)m_ModelName.c_str();
}

void RMesh::SetName(char* name)
{
	if(!name[0]) return;

	m_ModelName = name;
}

bool RMesh::CmpName(char* name)
{
	if(!name[0]) return false;

//	if(strcmp(m_modelname,name)==0)
//		return true;
	if(m_ModelName == name )
		return true;

	return false;
}


///////////////////////////////////////////////////////////////////////

void RMesh::GetMeshData(RMeshPartsType type,vector<RMeshNode*>& nodetable)
{
	RMeshNode*	pMesh = NULL;

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	while (it_obj !=  m_list.end()) {
		pMesh = (*it_obj);
		if(pMesh->m_PartsType==type) {
			nodetable.push_back(pMesh);
		}
		it_obj++;
	}
}

RMeshNode* RMesh::GetMeshData(RMeshPartsType type)
{
	RMeshNode*	pMesh = NULL;

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	while (it_obj !=  m_list.end()) {
		pMesh = (*it_obj);
		if(pMesh->m_PartsType==type)
			return pMesh;
		it_obj++;
	}
	return NULL;
}

RMeshNode* RMesh::GetMeshData(const char* name)
{
	RMeshNode*	pMesh = NULL;

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	while (it_obj !=  m_list.end()) {
		pMesh = (*it_obj);
		if(strcmp(pMesh->GetName(),name)==0)
			return pMesh;
		it_obj++;
	}
	return NULL;
}

RMeshNode* RMesh::GetPartsNode(const char* name)
{
	if(!m_parts_mgr)
		return NULL;

	return m_parts_mgr->GetPartsNode(name);
}

void RMesh::GetPartsNode(RMeshPartsType type,vector<RMeshNode*>& nodetable)
{
	if(!m_parts_mgr)
		return;

	m_parts_mgr->GetPartsNode(type,nodetable);
}

/////////////////////////////////////////////////////////////////////////////

void RMesh::DelMeshList()
{
	if (m_list.empty())
		return;

	RMeshNodeHashList_Iter node =  m_list.begin();

	while (node !=  m_list.end()) {
		delete (*node);
		node =  m_list.Erase(node);
	}

	m_data_num = 0;
}

int RMesh::FindMeshId(RAnimationNode* pANode)
{
	if( pANode == NULL )
		return -1;

	int ret_id = -1;

	bool bReConnect = false;

	if(pANode->m_pConnectMesh != this)//연결된 모델이 틀리다면
		bReConnect = true;
	else if( pANode->m_node_id == -1 )//모델은 있지만 초기값이면?
		bReConnect = true;

	if( bReConnect ) {

		ret_id = FindMeshIdSub( pANode );

		pANode->m_node_id = ret_id;
		pANode->m_pConnectMesh = this;

		return ret_id;
	}

	return pANode->m_node_id;
}

int RMesh::FindMeshParentId(RMeshNode* pMeshNode)
{
	if(pMeshNode==NULL)
		return -1;

	int ret_id = -1;

	if( pMeshNode->m_nParentNodeID == -1 ) {
		ret_id = _FindMeshId(pMeshNode->m_Parent);
		pMeshNode->m_nParentNodeID = ret_id;
		return ret_id;
	}
		
	return pMeshNode->m_nParentNodeID;
}

int RMesh::_FindMeshId(int e_name)
{
	if (m_list.empty())
		return -1;

	__BP(307, "RMesh::FindMeshId");

	RMeshNode* pNode = m_list.Find(e_name);
	if (pNode != NULL)
	{
		__EP(307);
		return pNode->m_id;
	}

	__EP(307);

	return -1;
}

int RMesh::_FindMeshId(char* name)
{
	if (m_list.empty())
		return -1;

	__BP(306, "RMesh::FindMeshId");

	RMeshNode* pNode = m_list.Find(name);
	if (pNode != NULL)
	{
		__EP(306);
		return pNode->m_id;
	}

	__EP(306);

	return -1;
}

int RMesh::FindMeshId(RMeshNode* pNode)
{
	if(!pNode) return -1;

	if(pNode->m_NameID != -1)
		return _FindMeshId(pNode->m_NameID);

	return _FindMeshId(pNode->GetName());
}

int RMesh::FindMeshIdSub(RAnimationNode* pANode)
{
	if(!pANode) return -1;

	if(pANode->m_NameID != -1)
		return _FindMeshId(pANode->m_NameID);

	return _FindMeshId(pANode->GetName());
}

void __SetPosMat(D3DXMATRIX* m1,D3DXMATRIX* m2) {

	m1->_41 = m2->_41;
	m1->_42 = m2->_42;
	m1->_43 = m2->_43;
}

void __SetRotMat(D3DXMATRIX* m1,D3DXMATRIX* m2) {

	D3DXMATRIX m;

	m = *m1;
	*m1 = *m2;

	m1->_41 = m._41;
	m1->_42 = m._42;
	m1->_43 = m._43;
}

bool RMesh::ConnectMtrl()
{
	RMeshNode*	pMeshNode = NULL;

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	while (it_obj !=  m_list.end()) {
		pMeshNode = (*it_obj);

		if(pMeshNode) {
			if(pMeshNode->m_face_num)
				pMeshNode->ConnectMtrl();
		}

		it_obj++;
	}

	return NULL;
}

// 이미 id 등록된 pAniSet으로..

bool RMesh::ConnectAnimation(RAnimation* pAniSet)
{
	if(!pAniSet) 
		return false;

	RAnimationNode* pANode = NULL;

	int pid = -1;

	int node_cnt = pAniSet->GetAniNodeCount();

	for(int i=0;i< node_cnt;i++) {

		pANode = pAniSet->GetAniNode(i);

//		pid = FindMeshId(pANode->m_Name);
//		if(pid != -1) {
//			pANode->m_node_id = pid;
//		}

		pid = FindMeshId(pANode);
	}

	pAniSet->m_isConnected = true;

	return true;
}

// 모델단위 id 연결설정...
// 

bool RMesh::SetAnimation1Parts(RAnimation* pAniSet) {

	__BP(305, "RMesh::SetAnimation1Parts");

	bool bNodeHoldFrame = false;
	bool bNodeUpdate = false;

	if( m_pAniSet[1] )  // 이전에는 상위가 있었고 이번에 null 이라면 갱신필요..
		bNodeUpdate = true;
	
	if(m_pAniSet[0] == pAniSet) {	// pAniSet 은 null 체크하고 같지않으니까 m_pAniSet 은 null 이 아님..
		if( pAniSet->m_isConnected ){
			if(m_pAniSet[0]->CheckName( pAniSet->GetName() ) ) {
				if(!bNodeUpdate) {
					return true;
				}
				else {
					bNodeHoldFrame = true;	// 같은에니메이션 상태를 유지하면서 상반신만 다시 노드연결..
				}
			}
		}
	}

	// 아직연결안된거라면...

	if(!pAniSet->m_isConnected) { 
		ConnectAnimation(pAniSet);
	}


	m_pAniSet[0] = pAniSet;
	m_pAniSet[1] = NULL;

	RAnimationNode* pANode = NULL;

	int pid = -1;

	m_is_use_ani_set = true;

	RMeshNode* pMeshNode = NULL;

	// 다르다고 판단된다면 시작..

	if(!bNodeHoldFrame)//상태는 같고 하반신 노드연결만 원할경우..
		m_frame[0] = 0;

	if(pAniSet->GetAniNodeCount() != m_data_num) {
//		mlog("RMesh::SetAnimation()  node key 갯수가 맞지 않음\n");
	}

	int i=0;

	for(i=0;i<m_data_num;i++) {
		m_data[i]->m_pAnimationNode = NULL;
	}

	int node_cnt = pAniSet->GetAniNodeCount();

	for(i=0;i<node_cnt;i++) {

		pANode = pAniSet->GetAniNode(i);

//		pid = FindMeshId(pANode->m_Name);
		pid = FindMeshId(pANode);

		if(pid != -1) {

			RMeshNode* pM = m_data[pid];

			if(pM) {

				pM->m_pAnimationNode = pANode;
				memcpy(&pM->m_mat_base,&pANode->m_mat_base ,sizeof(D3DXMATRIX));
				memcpy(&pM->m_mat_local,&pM->m_mat_base,sizeof(D3DXMATRIX));
				RMatInv(pM->m_mat_inv,pM->m_mat_local);
			}
		}
	}
/*
	// 제거 대상..
	if( m_pVisualMesh && m_pVisualMesh->m_pAniNodeTable )
	{
		RAnimationNode* pAniNode = NULL;
		RMeshNode* pMeshNode = NULL;

		for(i=0;i<m_data_num;i++)
		{
			pANode = m_pVisualMesh->m_pAniNodeTable[i];
			pMeshNode = m_data[i];

			if( pANode && pMeshNode ) {

				pMeshNode->m_pAnimationNode = pANode;
				memcpy(&pMeshNode->m_mat_base,&pANode->m_mat_base ,sizeof(D3DXMATRIX));
				memcpy(&pMeshNode->m_mat_local,&pMeshNode->m_mat_base,sizeof(D3DXMATRIX));
				RMatInv(pMeshNode->m_mat_inv,pMeshNode->m_mat_local);
			}
		}

		m_max_frame[0] = pAniSet->m_max_frame;

	}
*/
	if(pAniSet)
		m_max_frame[0] = pAniSet->GetMaxFrame();

	__EP(305);

	return true;
}
// 바뀐것만 갱신해야함...수정하기...
bool RMesh::SetAnimation2Parts(RAnimation* pAniSet,RAnimation* pAniSetUpper) {

	__BP(304, "RMesh::SetAnimation2Parts");

	int i=0;

	bool bC1 = true;
	bool bC2 = true;

	if(m_pAniSet[0] == pAniSet) {	
		if( pAniSet->m_isConnected ){
			if( m_pAniSet[0]->CheckName( pAniSet->GetName() ) ) {
				bC1 = false;
			}
		}
	}

	if(m_pAniSet[1] == pAniSetUpper) {	
		if( pAniSetUpper->m_isConnected ){
			if(m_pAniSet[1]->CheckName( pAniSetUpper->GetName() ) ) {
				bC2 = false;
			}
		}
	}

	if( !bC1 && !bC2 )
	{
		__EP(304);
		return true;
	}

	RAnimationNode* pANode = NULL;

	int pid = -1;

	m_is_use_ani_set = true;

	RMeshNode* pMeshNode = NULL;

//	-> 더미들때문에 당연히 틀림
//	if(pAniSet->m_ani_node_cnt != m_data_num) {
//		mlog("RMesh::SetAnimation()  node key 갯수가 맞지 않음\n");
//	}

	for(i=0;i<m_data_num;i++) {
		m_data[i]->m_pAnimationNode = NULL;
	}

	if( bC1 ) {

		// 아직연결안된거라면...
		if(!pAniSet->m_isConnected) {
			ConnectAnimation(pAniSet);
		}

		m_pAniSet[0] = pAniSet;
		m_frame[0] = 0;

		m_max_frame[0] = pAniSet->GetMaxFrame();
	}

	if( bC2 ) {
	
		if(!pAniSetUpper->m_isConnected) {
			ConnectAnimation(pAniSetUpper);
		}

		m_pAniSet[1] = pAniSetUpper;
		m_frame[1] = 0;

		m_max_frame[1] = pAniSetUpper->GetMaxFrame();
	}

	// 서로 에니 노드 갯수를 보장받을수없으니 2번 돌림...

	// 하반신 등록..
	int node_cnt = pAniSet->GetAniNodeCount();

	for(i=0;i<node_cnt;i++) {

		pANode = pAniSet->GetAniNode(i);

		if(pANode){
		
//			pid = FindMeshId(pANode->m_Name);
			pid = FindMeshId(pANode);

			if(pid != -1) {

				RMeshNode* pM = m_data[pid];

				if(pM) {
					if(pM->m_CutPartsType == cut_parts_lower_body) {
						pM->m_pAnimationNode = pANode;
						memcpy(&pM->m_mat_base,&pANode->m_mat_base ,sizeof(D3DXMATRIX));
						memcpy(&pM->m_mat_local,&pM->m_mat_base,sizeof(D3DXMATRIX));
						RMatInv(pM->m_mat_inv,pM->m_mat_local);
					}
				}
			}
		}
	}

	// 상반신 등록..
	node_cnt = pAniSetUpper->GetAniNodeCount();

	for(i=0;i<node_cnt;i++) {

		pANode = pAniSetUpper->GetAniNode(i);

		if(pANode) {

//			pid = FindMeshId(pANode->m_Name);
			pid = FindMeshId(pANode);

			if(pid != -1) {

				RMeshNode* pM = m_data[pid];

				if(pM) {
					if( pM->m_CutPartsType == cut_parts_upper_body ){
						pM->m_pAnimationNode = pANode;
						memcpy(&pM->m_mat_base,&pANode->m_mat_base ,sizeof(D3DXMATRIX));
						memcpy(&pM->m_mat_local,&pM->m_mat_base,sizeof(D3DXMATRIX));
						RMatInv(pM->m_mat_inv,pM->m_mat_local);
						//spine 의 경우 더미를 대신할 자신의 원본로컬을 갖는다.
						if(pM->m_LookAtParts == lookat_parts_spine1) {
							rmatrix m,inv;
							RAnimationNode* pANodePa = pAniSetUpper->GetNode(pM->m_Parent);
							RMatInv(inv,pANodePa->m_mat_base);

							m = pANode->m_mat_base * inv;

							pM->m_spine_local_pos.x = m._41;
							pM->m_spine_local_pos.y = m._42;
							pM->m_spine_local_pos.z = m._43;
						}
					}
				}
			}
		}
	}

	RAnimationNode* pANode1 = pAniSet->GetNode("Bip01");
	RAnimationNode* pANode2 = pAniSetUpper->GetNode("Bip01");

	m_vAddBipCenter.x = 0.f;
	m_vAddBipCenter.y = 0.f;
	m_vAddBipCenter.z = 0.f;

	if( pANode1 && pANode2 ) {
		m_vAddBipCenter.x = pANode2->m_mat_base._41 - pANode1->m_mat_base._41;
		m_vAddBipCenter.y = pANode2->m_mat_base._42 - pANode1->m_mat_base._42;
		m_vAddBipCenter.z = pANode2->m_mat_base._43 - pANode1->m_mat_base._43;
	}

	__EP(304);

	return true;
}

// 좀더 최적화 필요........

bool RMesh::SetAnimation(RAnimation* pAniSet,RAnimation* pAniSetUpper)
{
	if(!pAniSet) return false;

	if(pAniSetUpper)
		return SetAnimation2Parts(pAniSet,pAniSetUpper);

	return SetAnimation1Parts(pAniSet);
}

bool RMesh::SetAnimation(char* name,char* ani_name_upper) {

	if(m_ani_mgr.m_list.empty())
		return false;

	RAnimation* pAniSet = NULL;
	RAnimation* pAniSetUpper = NULL;

	pAniSet = m_ani_mgr.GetAnimation(name,-1);

	if(ani_name_upper) {
		pAniSetUpper = m_ani_mgr.GetAnimation( ani_name_upper,-1 );
	}

	return SetAnimation(pAniSet,pAniSetUpper);
}

void RMesh::ClearAnimation()
{
	m_is_use_ani_set = false;
	m_pAniSet[0] = NULL;
	m_pAniSet[1] = NULL;
}

// 구조가 바뀌면서 보장 받을 수 없다..

bool RMesh::Pick(int mx,int my,RPickInfo* pInfo,rmatrix* world_mat)
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();

	int sw = RGetScreenWidth();
	int sh = RGetScreenHeight();

	rvector pos,dir;

	rmatrix matProj = RProjection;

	rvector v;

	v.x =  ( ( ( 2.0f * mx ) / sw ) - 1 ) / matProj._11;
	v.y = -( ( ( 2.0f * my ) / sh ) - 1 ) / matProj._22;
	v.z =  1.0f;

	rmatrix m,matView = RView;

	D3DXMatrixInverse( &m, NULL, &matView );

	dir.x  = v.x*m._11 + v.y*m._21 + v.z*m._31;
	dir.y  = v.x*m._12 + v.y*m._22 + v.z*m._32;
	dir.z  = v.x*m._13 + v.y*m._23 + v.z*m._33;

	pos.x = m._41;
	pos.y = m._42;
	pos.z = m._43;

	D3DXVec3Normalize(&dir,&dir);

	rvector vInVec[2]; 

	vInVec[0] = pos; 
	vInVec[1] = dir;

	return CalcIntersectsTriangle(vInVec, pInfo, world_mat );

}

bool RMesh::Pick(rvector& pos,rvector& dir,RPickInfo* pInfo,rmatrix* world_mat)
{
	rvector v[2]; v[0] = pos; v[1] = dir;
	return CalcIntersectsTriangle(v, pInfo, world_mat);
}

bool RMesh::Pick(rvector* vInVec,RPickInfo* pInfo,rmatrix* world_mat)
{
	return Pick(vInVec[0],vInVec[1], pInfo, world_mat);
}

void RMesh::ClearMtrl() {
	m_mtrl_list_ex.DelAll();
}

// 속도를 생각한 대략의 BBOX

void RMesh::CalcBoxNode(D3DXMATRIX* world_mat)
{
	m_vBBMax = D3DXVECTOR3(-9999.f,-9999.f,-9999.f);
	m_vBBMin = D3DXVECTOR3( 9999.f, 9999.f, 9999.f);

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	RMeshNode* pTMeshNode = NULL;

	while (it_obj !=  m_list.end()) {

		RMeshNode*	pMeshNode = (*it_obj);

		CalcNodeMatrixBBox(pMeshNode);//bbox

		pTMeshNode = UpdateNodeAniMatrix(pMeshNode);

		it_obj++;
	}

}

void RMesh::CalcBoxFast(D3DXMATRIX* world_mat)
{

}

void RMesh::CalcBox(D3DXMATRIX* world_mat)
{
	if( m_list.empty())	return;

	RMeshNode* pTMeshNode  = NULL;

	m_vBBMax = D3DXVECTOR3(-9999.f,-9999.f,-9999.f);
	m_vBBMin = D3DXVECTOR3( 9999.f, 9999.f, 9999.f);

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	while (it_obj !=  m_list.end()) {

		RMeshNode*	pMeshNode = (*it_obj);

		CalcNodeMatrixBBox(pMeshNode);//bbox

		pTMeshNode = UpdateNodeAniMatrix(pMeshNode);

		if(pTMeshNode->m_isDummyMesh) {//Bip,Bone,Dummy Skip
			it_obj++;
			continue;
		}

		if(pTMeshNode->m_face_num != 0) {// 대충 점 * resultmat
			if(world_mat)
				pTMeshNode->CalcVertexBuffer(world_mat,true);
		}

		it_obj++;
	}
}

void RMesh::CalcNodeMatrixBBox(RMeshNode* pNode)
{
	SubCalcBBox(
		&m_vBBMaxNodeMatrix,
		&m_vBBMinNodeMatrix,
		&rvector( pNode->m_mat_result._41, pNode->m_mat_result._42, pNode->m_mat_result._43));
}

void RMesh::CalcBBox(D3DXVECTOR3* v) 
{
	SubCalcBBox( &m_vBBMax, &m_vBBMin, v);
}

void RMesh::SubCalcBBox(D3DXVECTOR3* max,D3DXVECTOR3* min,D3DXVECTOR3* v)
{
	if ( (max==NULL) || (min==NULL) || (v==NULL) ) return;

	min->x = min(min->x, v->x);
	min->y = min(min->y, v->y);
	min->z = min(min->z, v->z);

	max->x = max(max->x, v->x);
	max->y = max(max->y, v->y);
	max->z = max(max->z, v->z);
}

void RMesh::RenderBox(D3DXMATRIX* world_mat)
{
//	if(world_mat)
//		RGetDevice()->SetTransform( D3DTS_WORLD, world_mat );

	draw_box(world_mat,m_vBBMax,m_vBBMin,0xffffffff);
}

rvector RMesh::GetOrgPosition()
{
	rvector v = rvector(0,0,0);

	RMeshNode* pMNode = m_data[0];

	if( pMNode ) {
		v.x = pMNode->m_mat_base._41;
		v.y = pMNode->m_mat_base._42;
		v.z = pMNode->m_mat_base._43;
	}
	
	return v;
}

_NAMESPACE_REALSPACE2_END
