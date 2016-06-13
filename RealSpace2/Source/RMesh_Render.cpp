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
#include "vector"

#include "ROcclusionList.h"

using namespace std;

#ifndef _PUBLISH

#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);

#else

#define __BP(i,n) ;
#define __EP(i) ;

#endif

bool g_bVertex_Soft = false;

/////////////////////////////////////////////////////////////////////////////

_NAMESPACE_REALSPACE2_BEGIN

int g_poly_render_cnt;


#define RENDER_NODE_MAX 1000

int			g_render_cnt=0;
RRenderNode	g_render_node[ RENDER_NODE_MAX ];

RRenderNodeList	g_RenderNodeList[eRRenderNode_End];
RRenderNodeList	g_RenderLNodeList[eRRenderNode_End];

bool g_rmesh_render_start_begin = false;

int	 g_vert_index_pos = 0;
int	 g_lvert_index_pos = 0;

////////////////////////////////////////////////////////////

int __cdecl _SortAlpha( const VOID* arg1, const VOID* arg2 )
{
	RMeshNode* p1 = *(RMeshNode**)arg1;
	RMeshNode* p2 = *(RMeshNode**)arg2;

	if (p1->m_AlphaSortValue < p2->m_AlphaSortValue)
		return +1;

	return -1;
}

int __cdecl _SortLastName( const VOID* arg1, const VOID* arg2 )
{
	RMeshNode* p1 = *(RMeshNode**)arg1;
	RMeshNode* p2 = *(RMeshNode**)arg2;

	if (p1->m_Name < p2->m_Name)
		return +1;

	return -1;
}

void RMesh::Render(D3DXMATRIX* world_mat,bool NoPartsChange)
{
	RenderSub(world_mat,NoPartsChange,false);
}

RMeshNode* RMesh::FindNode(RMeshPartsPosInfoType type)
{
	RMeshNode* pNode = NULL;

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	while (it_obj !=  m_list.end()) {

		pNode = (*it_obj);

		if(pNode->m_PartsPosInfoType==type)
			return pNode;

		it_obj++;
	}

	return NULL;
}

bool RMesh::CalcParts(RMeshNode* pPartsMeshNode,RMeshNode* pMeshNode,bool NoPartsChange)
{
	return false;
}

#define _SORT_ALPHA_NODE_TABLE	100
#define _SORT_LAST_NODE_TABLE	100

void RMesh::RenderSub(D3DXMATRIX* world_mat,bool NoPartsChange,bool bRenderBuffer)
{
	__BP(198, "RMesh::RenderSub");

	LPDIRECT3DDEVICE9 dev = RGetDevice();

	D3DXMATRIX	buffer,Inv;

	m_vBBMax = D3DXVECTOR3(-9999.f,-9999.f,-9999.f);
	m_vBBMin = D3DXVECTOR3( 9999.f, 9999.f, 9999.f);

 	m_vBBMaxNodeMatrix = m_vBBMax;
	m_vBBMinNodeMatrix = m_vBBMin;

	DWORD _color = 0;

	__BP(202, "RMesh::RenderSub::RenderFrame");

	RenderFrame();	// 에니 처리

	__EP(202);

	__BP(199, "RMesh::RenderSub::State b");

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	RMeshNode* pMeshNode = NULL;
	RMeshNode* pPartsMeshNode = NULL;

	// init render state

	// 모델단위? light setting 이전에 렌더링한것과 visualmesh 가 다르다면 라이트 셋팅을 다시해준다..

	if(m_pVisualMesh && !m_LitVertexModel) { 
		m_pVisualMesh->UpdateLight();
	}

	if(m_LitVertexModel)
		dev->SetRenderState( D3DRS_LIGHTING, FALSE );
	else 
		dev->SetRenderState( D3DRS_LIGHTING, TRUE );

	dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	dev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	dev->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );

	__EP(199);

	static RMeshNode* pAlphaNode[_SORT_ALPHA_NODE_TABLE];	//한 오브젝트에 알파 노드가 100개넘으면 무시..]
	static RMeshNode* pLastNode[_SORT_LAST_NODE_TABLE];		//한 오브젝트에 Last 노드가 100개넘으면 무시..]

	int nAlphaNodeCnt = 0;
	int nLastNodeCnt = 0;

	__BP(500, "RMesh::RenderSub::Loop");

	if(m_pVisualMesh && m_pVisualMesh->m_bIsCharacter)
		m_pVisualMesh->ClearPartInfo();

	while (it_obj !=  m_list.end()) {

		pMeshNode = (*it_obj);

		pMeshNode->CheckAlign(world_mat);

		CalcNodeMatrixBBox( pMeshNode );

		pPartsMeshNode = pMeshNode;

		if( GetToolMesh() && m_pToolSelectNode ) {

			if(pMeshNode==m_pToolSelectNode)// 툴이고 선택된 노드라면 박스그려주기...
				draw_box(&pMeshNode->m_mat_result,pMeshNode->m_max,pMeshNode->m_min,0xffff0000);
		}

		if(m_pVisualMesh) {	// visual mesh 가 있는경우

			if(NoPartsChange)	pPartsMeshNode = NULL;
			else				pPartsMeshNode = m_pVisualMesh->GetParts(pMeshNode->m_PartsType); // 장비된 모델 포인터 얻어내기..

			m_pVisualMesh->UpdateWeaponDummyMatrix(pMeshNode);

			if( pMeshNode->m_isWeaponMesh ) { // 캐릭터에 달린 무기더미들 , 칼더미,총더미등..

				m_pVisualMesh->UpdateWeaponPartInfo(pMeshNode);

				it_obj++;
				continue;
			} 
			else {								// 장비품들 과 뼈대노드..

				if( pPartsMeshNode==NULL ) {
					pPartsMeshNode = pMeshNode;
				} 
				else if( pPartsMeshNode->m_PartsType != pMeshNode->m_PartsType)	// 같은 장착위치가 아니면
				{
					pPartsMeshNode = pMeshNode;
				}
			}
		}

		pPartsMeshNode->m_pBaseMesh = this;// base model 이 이 파일임을 등록,,,

		if(pMeshNode->m_isAddMeshNode) //pPartsMeshNode 조사하면 안됨.. pMeshNode 가 더미임...
		{
			if(pMeshNode->m_pParent) {
				pPartsMeshNode->m_mat_result = pMeshNode->m_mat_add * pMeshNode->m_pParent->m_mat_result;
			}
		}

		if(pPartsMeshNode->m_WeaponDummyType != weapon_dummy_etc) {//무기에 붙은 더미면~
			it_obj++;
			continue;
		}

		if(pPartsMeshNode->m_isDummyMesh)	{ //Bip,Bone,Dummy Skip

			if( m_pVisualMesh ) {
				if( m_pVisualMesh->m_pBipMatrix ) {
					if(pMeshNode->m_PartsPosInfoType!=eq_parts_pos_info_etc) { 
						m_pVisualMesh->m_pBipMatrix[pMeshNode->m_PartsPosInfoType] = pMeshNode->m_mat_result;
					}
				}
			}
			it_obj++;
			continue;
		}		

		pPartsMeshNode->GetNodeVisValue(); // 이번프레임의 vis_alpha 값을 얻어낸다..내부에서 m_vis 값을 갱신,,,

		// Cloth MeshNode Skip

		if( pPartsMeshNode->m_PartsType == eq_parts_chest && !NoPartsChange && !m_is_map_object) //low poly model  이 아니여야함..
		{
			if( m_pVisualMesh && m_pVisualMesh->isChestClothMesh() && pPartsMeshNode->m_isClothMeshNode ) {

				__BP(405, "RMesh::Draw::Cloth::update&render");

				m_pVisualMesh->UpdateCloth();
				m_pVisualMesh->RenderCloth();
				
				__EP(405);

				++it_obj;
				continue;
			}
		}

//		if(pPartsMeshNode->CheckName("hair"))
//			pPartsMeshNode->m_isLastModel = true;

		if( pPartsMeshNode->m_isLastModel ) { // 자신의 모델단위에서 나중에 그려져야 하는것들..

			pLastNode[nLastNodeCnt] = pPartsMeshNode;
			nLastNodeCnt++;

			if(nLastNodeCnt > _SORT_LAST_NODE_TABLE) {
				nLastNodeCnt = _SORT_LAST_NODE_TABLE;
				mlog("%s 오브젝트의 Last 노드가 100개가 넘는다..\n",GetFileName());
				nLastNodeCnt--;
			}

			it_obj++;
			continue;
		}

//		pPartsMeshNode->m_isAlphaMtrl = false;

		if( pPartsMeshNode->isAlphaMtrlNode() ) {//알파 오브젝트는 뒤에 그린다..

			if( RRenderNodeMgr::m_bRenderBuffer ) {

//				RenderNodeMgr_Add( pMeshNode->m_ModelWorldMatrix,m,pMeshNode,0);

				//	보관되어야 할값들은?
				//	visual mesh.. 자신의 고유한값들..
				//	m_pParentMesh->SetVisualMesh( pVMesh );
				//	m_pParentMesh->RenderNode( pMNode ); //frame 값등은 보관되는가?
				//	visualmesh 는 자신의 노드 매트릭스를 매 프레임 갱신 보관...
				//	카메라에서 충분히 먼 캐릭터라면... 1/2 1/3 1/4 빈도로 프레임 갱신... 
				//	이전 버퍼를 그냥 사용하기 가능..그려 주기만 한다..
				//	mat_result 보관되어 있어야 함..
				//	light 설정값 보관해야함.. ( visual mesh 가 보관..
				//	align , part_node 에 대한 문제.. vis_alpha 값...
				
				//	calcVertexBuffer 에서... pMesh->Scale 참조함... ----- frame 값을 보관하고있어야 함..
				//	vertex_ani + soft_vs 는 버텍스 버퍼가 변경됨...
				//	baseMesh 의 상하체 프레임 값을 사용함.... 프레임 다시 설정해야함...상하체 에니메이션 자체가 바뀌어 있을수도 있음... 

				//  visualmesh->render 에서 set 하는것들 다시해야 의미가 있음,,,
				//  
			}
			else {
			
				pAlphaNode[nAlphaNodeCnt] = pPartsMeshNode;
				nAlphaNodeCnt++;

				if(nAlphaNodeCnt > _SORT_ALPHA_NODE_TABLE-1) {
					nAlphaNodeCnt = _SORT_ALPHA_NODE_TABLE-1;
					mlog("%s 오브젝트의 알파 노드가 100개가 넘는다..\n",GetFileName());
					nAlphaNodeCnt--;
				}

			}

			it_obj++;
			continue;

		}

		RenderNode(pPartsMeshNode,world_mat);

		it_obj++;
	}

	RMeshNode* pATMNode = NULL;
	RMeshNode* pTNodeHead = NULL;	
	RMeshNode* pTLastModel = NULL;

//	qsort( pAlphaNode, nAlphaNodeCnt, 4 , _SortAlpha );

	///////////////////////////////////////////////////////////////////

	for(int n=0;n<nAlphaNodeCnt;n++) {

		pATMNode = pAlphaNode[n];

		if( pATMNode == NULL ) continue;

		if(pATMNode->m_PartsType == eq_parts_head ) {
			pTNodeHead = pATMNode;
			continue;
		}

		RenderNode(pATMNode,world_mat);
	}

	if(pTNodeHead) {
		RenderNode(pTNodeHead,world_mat);
	}

	///////////////////////////////////////////////////////////////////

	qsort( pLastNode, nLastNodeCnt, 4 , _SortLastName );

	for(int n=0;n<nLastNodeCnt;n++) {

		pATMNode = pLastNode[n];

		if( pATMNode == NULL ) continue;

		RenderNode(pATMNode,world_mat);
	}


	///////////////////////////////////////////////////////////////////


	static D3DXMATRIX _init_mat = GetIdentityMatrix();

	dev->SetTransform( D3DTS_WORLD, &_init_mat );

	__EP(500);

	__EP(198);
}

// billboard(aline) type 이라면 마지막 회전 매트릭스 수정 점 가공..
 
void OutPutMatrixLog(RMesh* pMesh,RMeshNode* pNode,char* pos,char* name,rmatrix& m)
{
	return;

	if(!pMesh||!pNode) 
		return;

	RAnimation* pAniSet = pMesh->GetNodeAniSet(pNode);

	if(pAniSet==NULL) 
		return;

	if( pos[0] && name[0] ) {

		mlog("%s %s %f,%f,%f,%f %f,%f,%f,%f %f,%f,%f,%f %f,%f,%f,%f \n", pos , name ,
			m._11,m._12,m._13,m._14,
			m._21,m._22,m._23,m._24,
			m._31,m._32,m._33,m._34,
			m._41,m._42,m._43,m._44	);

	} else {

		mlog("%f,%f,%f,%f %f,%f,%f,%f %f,%f,%f,%f %f,%f,%f,%f \n", 
			m._11,m._12,m._13,m._14,
			m._21,m._22,m._23,m._24,
			m._31,m._32,m._33,m._34,
			m._41,m._42,m._43,m._44	);
	}
}

////////////////////////////////////////////////////////////////////////////////////


class RIVec
{
public:

	RIVec() {
		m_size = 0;
	}

	void Add(int v) {

		m_Value[m_size] = v;
		m_size++;

		if(m_size>2)
			m_size=2;
	}

	int m_size;
	int m_Value[3];
};

class CIndexBufferMake
{
public:
	CIndexBufferMake() {

		m_pFaceIndex = NULL;
		m_tAddPos = 0;
	}

	~CIndexBufferMake() {
		Clear();
	}

	void Create(int face_num,int point_num) {//면인덱스만큼메모리할당..
		
		if(face_num) {

			m_pFaceIndex = new WORD[face_num*3];
			memset(m_pFaceIndex,0,sizeof(WORD)*face_num*3);
		}

		m_tAddPos = point_num;
		
	}

	void Clear() {

		if(m_pFaceIndex)
			delete[] m_pFaceIndex;
	}

	void MakeLVertex(RMeshNode* pMNode,RFaceInfo* pFace,int i,DWORD color)
	{
		int in = 0;

		RFaceNormalInfo* pFNL = &pMNode->m_face_normal_list[i];

		for(int k=0;k<3;k++) {

			int in = GetLIndex( pFace->m_point_index[k] , &pFace->m_point_tex[k] );

//			SetLVertex(((RLVertex*)g_pVert)+in,g_point_list[pFace->m_point_index[k]], color, pFace->m_point_tex[ k ]);

			m_pFaceIndex[i*3+k] = in;
//			mlog("face (%d,%d) = %d \n",i,k,in);
		}
	}

	// n번쨰 면에 대해서
	// 중복되지 않게 3개의 점을 만들어야 한다..
	void MakeVertex(RMeshNode* pMNode,RFaceInfo* pFace,int i)
	{
		int in = 0;

		RFaceNormalInfo* pFNL = &pMNode->m_face_normal_list[i];

		for(int k=0;k<3;k++) {

			int in = GetIndex( pFace->m_point_index[k] , &pFace->m_point_tex[k] );

//			SetVertex(g_vert+in,g_point_list[pFace->m_point_index[k]], color, pFace->m_point_tex[ k ]);

//			if( !g_bVertexNormalOnOff ) // tool 용
//				SetVertex( ((RVertex*)g_pVert) +in  ,g_point_list[pFace->m_point_index[k]],pFNL->m_normal,pFace->m_point_tex[k]);
//			else 
//				SetVertex( ((RVertex*)g_pVert) +in  ,g_point_list[pFace->m_point_index[k]],pFNL->m_pointnormal[k],pFace->m_point_tex[k]);
			
			m_pFaceIndex[i*3+k] = in;
//			mlog("face (%d,%d) = %d \n",i,k,in);
		}
	}

	int GetLIndex(int in,rvector* puv) // 실재점이 만들어질 인덱스 넘기기..
	{
		static float _uv[4];

		int _size = m_pPointTable[in].m_size;
//		int index;

		if(_size) { // 이미 등록된 점이라면
/*
			RLVertex* pVert=(RLVertex*)g_pVert;

			for(int i=0;i<_size;i++) {
				index = m_pPointTable[in].m_Value[i];
				GetLUV(_uv,&pVert[index],puv);
				if(isEqualUV(_uv)) {//uv가 같으면 같은점을 사용..
					return index;
				}
			}

			m_pPointTable[in].Add( m_tAddPos );//마지막점을 사용
			m_tAddPos++;
*/
			return m_tAddPos-1;//마지막점의 인덱스..
		}

		m_pPointTable[in].Add( in ); // 새로 등록 해야하는 경우 그냥 사용..

		return in;
	}

	int GetIndex(int in,rvector* puv) // 실재점이 만들어질 인덱스 넘기기..
	{
		static float _uv[4];

		int _size = m_pPointTable[in].m_size;
//		int index;

		if(_size) { // 이미 등록된 점이라면
/*
			RVertex* pVert = (RVertex*)g_pVert;

			for(int i=0;i<_size;i++) {
				index = m_pPointTable[in].m_Value[i];
				GetUV(_uv,&pVert[index],puv);
				if(isEqualUV(_uv)) {//uv가 같으면 같은점을 사용..
					return index;
				}
			}

			m_pPointTable[in].Add( m_tAddPos );//마지막점을 사용
			m_tAddPos++;
*/
			return m_tAddPos-1;//마지막점의 인덱스..
		}

		m_pPointTable[in].Add( in ); // 새로 등록 해야하는 경우 그냥 사용..

		return in;
	}

	void GetLUV(float* uv,RLVertex* pVert,rvector* v) {
		uv[0] = pVert->tu;
		uv[1] = pVert->tv;
		uv[2] = v->x;
		uv[3] = v->y;
	}


	void GetUV(float* uv,RVertex* pVert,rvector* v) {
		uv[0] = pVert->tu;
		uv[1] = pVert->tv;
		uv[2] = v->x;
		uv[3] = v->y;
	}

	bool isEqualUV(float u1,float v1,float u2,float v2) {

		if(u1 == u2 )
			if(v1 == v2 )
				return true;
//		if(u1 - u2 < 0.00001f)
//			if(v1 - v2 < 0.00001f)
//				return true;
		return false;
	}

	bool isEqualUV(float* uv) {
		return isEqualUV(uv[0],uv[1],uv[2],uv[3]);
	}

	RIVec	m_pPointTable[2000];
//	vector<int>	m_pPointTable[2000];//우선 2000개의 점까지만
	WORD*	m_pFaceIndex;
	
	int		m_tAddPos;

};


void BBoxSubCalc(D3DXVECTOR3* max,D3DXVECTOR3* min);

bool RMesh::CheckOcclusion(RMeshNode *pMeshNode)
{
	if( m_is_map_object && !mbSkyBox)
	{
		if(m_pVisualMesh)
		{
			if(m_pVisualMesh->m_pTOCCL)
			{
				rboundingbox bb;

				rmatrix _mm;

				static rmatrix _mrot = RGetRotY(180) * RGetRotX(90);
	
				_mm = pMeshNode->m_mat_base * _mrot;

				bb.vmax = pMeshNode->m_max * _mm;
				bb.vmin = pMeshNode->m_min * _mm;
				
				BBoxSubCalc(&bb.vmax,&bb.vmin);

				if(m_pVisualMesh->m_bCheckViewFrustum) {//tool 이라면 false
					if(isInViewFrustumWithZ( &bb, RGetViewFrustum()) == false) {
						return false;
					}
				}

				return m_pVisualMesh->m_pTOCCL->IsVisible(bb);
			}
		}
	}
	return true;
}

void RMesh::RenderNode(RMeshNode *pMeshNode,D3DXMATRIX* world_mat)
{
	if(pMeshNode->m_face_num == 0) return;

	if(m_pVisualMesh)
		if(m_pVisualMesh->m_bRenderMatrix)//매트릭스 갱신만 원하는 경우라면....
			return;

	if(CheckOcclusion(pMeshNode)==false) // 모델이 아닌 노드 단위 컬링..
		return;

	__BP(501,"RMesh::RenderNode");

	SetMtrlUvAni_ON();

	bool bDrawCharPhysique = false;

	if( RIsSupportVS() && RShaderMgr::mbUsingShader )
	{
		if((m_pAniSet[0]) && (RAniType_Bone == m_pAniSet[0]->GetAnimationType()) 
			&& mHardwareAccellated && pMeshNode->m_MatrixCount > 0 )
		{
			bDrawCharPhysique = true;
		}
	}

	if(bDrawCharPhysique) {

		pMeshNode->RenderNodeVS(this,world_mat);

	}
//	else if( RRenderNodeMgr::m_bRenderBuffer ) { // 모아 그리기..설정이 되어있고..알파라면?

//		pMeshNode->CalcVertexBuffer(world_mat);
//		RenderNodeMgr_Add( pMeshNode->m_ModelWorldMatrix,pMeshNode,0);
		
//	} 
	else { // 즉시 그려야 한다면 - 스크린이펙트등 - 뷰나 다른것과 작용하는 것...
		
//		RGetDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

		pMeshNode->CalcVertexBuffer(world_mat);
		pMeshNode->Render();
	}

	SetMtrlUvAni_OFF();

	__EP(501);

}

////////////////////////////////////////////////////////////////////

// find face

bool find_intersects_triangle_sub(const rvector& orig, const rvector& dir,
	const rvector& v0, const rvector& v1, const rvector& v2, float* t, float* u, float* v )
{
	rvector edge1 = v1 - v0;
	rvector edge2 = v2 - v0;

	rvector pvec;
	D3DXVec3Cross( &pvec, &dir, &edge2 );

	FLOAT det = D3DXVec3Dot( &edge1, &pvec );
	if( det < 0.0001f )
		return false;

	rvector tvec = orig - v0;

	*u = D3DXVec3Dot( &tvec, &pvec );

	if( *u < 0.0f || *u > det )
		return false;

	rvector qvec;
	D3DXVec3Cross( &qvec, &tvec, &edge1 );

	*v = D3DXVec3Dot( &dir, &qvec );

	if( *v < 0.0f || *u + *v > det )
		return false;

	*t = D3DXVec3Dot( &edge2, &qvec );

	FLOAT fInvDet = 1.0f / det;

	*t *= fInvDet;
	*u *= fInvDet;
	*v *= fInvDet;

	return true;
}

inline bool find_intersects_triangle_sub(const rvector* vec, const rvector* vPoint, float* t, float* u, float* v ) {
	return find_intersects_triangle_sub(vec[0],vec[1], vPoint[0],vPoint[1], vPoint[2],t,u,v );
}

bool RMesh::CalcIntersectsTriangle(const rvector* vInVec, RPickInfo* pInfo, D3DXMATRIX* world_mat,bool fastmode)
{
	//////////////////////////////////////////////////

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	D3DXVECTOR3 _v;

	float best_t = 9999.f;

	RMeshNode* pFindMeshNode = NULL;
	RMeshNode* pPartsMeshNode = NULL;

	rvector		vFindVec[3];
	
	bool  bFind = false;

	rmatrix result_mat;

	while (it_obj !=  m_list.end())	{

		RMeshNode*	pMeshNode = (*it_obj);

		pPartsMeshNode = UpdateNodeAniMatrix(pMeshNode);

		// 충돌용 노드만 사용
		if( m_PickingType == pick_collision_mesh ) {
			if( !pPartsMeshNode->m_isCollisionMesh ) {
				it_obj++;
				continue;
			}
		}
		else if( m_PickingType == pick_real_mesh ) {
			if( pPartsMeshNode->m_isDummyMesh ) {//Bip,Bone,Dummy Skip
				it_obj++;
				continue;
			}
		}

		if(pPartsMeshNode->m_face_num==0) {
			it_obj++;
			continue;
		}

		static D3DXVECTOR3 pVecPick[10000];// 작동하면 다시 처리..

//		pPartsMeshNode->CalcVertexBuffer(world_mat,true);//world mat 곱한 vertex

		pPartsMeshNode->CalcPickVertexBuffer(world_mat,pVecPick);

		float t,u,v;
		rvector vec[3];//우선 지저 분해 보이니까 카피

		for (int i = 0; i < pPartsMeshNode->m_face_num  ; i ++) {

			vec[0] = pVecPick[ pPartsMeshNode->m_face_list[i].m_point_index[0] ];
			vec[1] = pVecPick[ pPartsMeshNode->m_face_list[i].m_point_index[1] ];
			vec[2] = pVecPick[ pPartsMeshNode->m_face_list[i].m_point_index[2] ];
			
			// pos , dir , vec
			if( find_intersects_triangle_sub( vInVec , vec, &t, &u, &v ) ) {

				if(t < best_t) {
					best_t = t;
					pFindMeshNode = pPartsMeshNode;
					memcpy(vFindVec,vec,sizeof(rvector)*3);
				}

				if(fastmode) {	// 가장 먼저찾은것
					if(pInfo) {
						pInfo->vOut	= vec[0] + u * ( vec[1] - vec[0] ) + v * ( vec[2] - vec[0] );
						pInfo->t = best_t;
					}
					return true;
				}

				bFind = true;
			}
		}
		it_obj++;
	}

	if( bFind ) {

		if(pInfo) {

			D3DXPLANE pl;
			D3DXPlaneFromPoints(&pl,&vFindVec[0],&vFindVec[1],&vFindVec[2]);

			rvector p,at;

			at  = vInVec[0];
			at += vInVec[1] * 10000.f;//대충 충분히 먼 거리..

			D3DXPlaneIntersectLine(&p,&pl,&vInVec[0],&at);

			pInfo->vOut	 = p;
			pInfo->t	 = best_t;
			pInfo->parts = pFindMeshNode->m_PartsType;
		}
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

static RRenderNodeMgr g_render_node_mgr;

// 자기 자료로 그리기..
void RRenderNode::Render()
{
	if(m_pNode && m_pNode->m_pParentMesh) {
		m_pNode->Render(&m_matWorld);
	}
}

void RRenderNodeMgr::Clear() 
{
	for(int i=0;i<eRRenderNode_End;i++) {
		m_RenderNodeList[i].Clear();
	}

	m_nTotalCount = 0;
}

int RRenderNodeMgr::Add(rmatrix& m,int mode,RMeshNode* pMNode,int nMtrl)
{
	bool lit = false;

	if(pMNode==NULL)
		return m_nTotalCount;

	RRenderNode* pNode = new RRenderNode;

	pNode->Set(mode,m,pMNode,nMtrl,0,0,1.f);

	m_RenderNodeList[mode].push_back( pNode );

#ifdef _DEBUG
	int _size = m_RenderNodeList[mode].size();
	m_RenderNodeList[mode].m_data[_size-1] = pNode;
#endif

	m_nTotalCount++;

	return m_nTotalCount;
}

void RRenderNodeMgr::Render()
{
	// sort

	// normal	- name
	// diffuse	- name
	// alpha	- name
	// add		- dist

	// light on
	
	LPDIRECT3DDEVICE9 dev = RGetDevice();

	for(int i=0;i<eRRenderNode_End;i++) {
		m_RenderNodeList[i].Render();
	}

	// light off

	Clear();
}

void RenderNodeMgr_Add(rmatrix& m,int mode,RMeshNode* pMNode,int nMtrl)
{
	if(RRenderNodeMgr::m_bRenderBuffer==false) 
		return;

	g_render_node_mgr.Add(m,mode,pMNode,nMtrl);
}

void RenderNodeMgr_Render()
{
	if(RRenderNodeMgr::m_bRenderBuffer==false) 
		return;

	g_render_node_mgr.Render();
}

// temp --------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////
// 모아찍기용..테스트 코드

void RMeshRenderS(bool lit,int Rmode,rmatrix m,RMeshNode* pMNode,RMtrl* pMtrl,int begin,int size ,float vis_alpha);
bool RMeshRenderSBegin();
bool RMeshRenderSEnd();

RRenderNode* GetNewRenderNode() {
	if(g_render_cnt > RENDER_NODE_MAX-1) {
//		mlog("g_render_buffer 를 늘려라\n");
		return NULL;
	}
	g_render_node[g_render_cnt].Clear();
	g_render_cnt++;
	return &g_render_node[g_render_cnt-1];
}

void ClearRenderNodeBuffer() {
	g_render_cnt = 0;
}

void RMeshRenderS(bool lit,int Rmode,rmatrix m,RMeshNode* pMNode,RMtrl* pMtrl,int begin,int size ,float vis_alpha)
{
	RRenderNode* pRNode = GetNewRenderNode();

	if(pRNode==NULL) return;

	pRNode->Set(Rmode,m,pMNode,pMtrl,begin,size,vis_alpha);

	if(lit) {
		g_RenderLNodeList[Rmode].push_back(pRNode);
	}
	else {
		g_RenderNodeList[Rmode].push_back(pRNode);
	}
}

bool RMeshRenderSBegin()
{
	if(g_rmesh_render_start_begin) {
		mlog("RMeshRenderSEnd() 을 먼저 한 후 사용\n");
		return false;
	}

	g_rmesh_render_start_begin = true;

	ClearRenderNodeBuffer();

	g_vert_index_pos = 0;
	g_lvert_index_pos = 0;

	return true;
}

//void SetMtrl(RMtrl* pMtrl,float vis_alpha);

bool RMeshRenderSEnd()
{
/*
	if(!g_rmesh_render_start_begin) {
		mlog("RMeshRenderSBegin() 을 먼저 한 후 사용\n");
		return false;
	}

	g_rmesh_render_start_begin = false;

	// buffer render..

	RRenderNodeList::iterator node;
	RRenderNode* pRNode = NULL;
	RMtrl* pBackupMtrl = NULL;

	// sort
//	for(int i=0;i<eRRenderNode_End;i++) {
//		g_RenderNodeList[i].sort();
//		g_RenderLNodeList[i].sort();
//	}

	LPDIRECT3DDEVICE9 dev = RGetDevice();

//	dev->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
	dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
//	dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	dev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	RSetWBuffer(true);
	dev->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );

	for(int i=0;i<eRRenderNode_End;i++) {

		dev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		dev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		dev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		dev->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

		if(i == eRRenderNode_Normal) {
			dev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE);
			dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
			dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
			dev->SetRenderState( D3DRS_ZWRITEENABLE, TRUE);

		}
		else if(i == eRRenderNode_Alpha) {

			dev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
			dev->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
			dev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
			dev->SetRenderState( D3DRS_ZWRITEENABLE, TRUE);

			dev->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
			dev->SetRenderState( D3DRS_ALPHAREF, 0x04 );
			dev->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

			dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );//vis ani ignore
			dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );//light 
		}
		else if(i == eRRenderNode_Add) {

			dev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
			dev->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
			dev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE);
			dev->SetRenderState( D3DRS_ZWRITEENABLE, FALSE);

			dev->SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
			dev->SetRenderState( D3DRS_ALPHAREF,         0x04 );
			dev->SetRenderState( D3DRS_ALPHAFUNC,  D3DCMP_GREATEREQUAL );

			dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );//light ignore
		//	dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
			dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );

		}
		else if(i == eRRenderNode_Diffuse) {

//			SetMtrl(&pSMtrl->m_diffuse,GetMeshNodeVis(pMeshNode));

//			dev->SetTexture( 0, NULL);

//			if(GetMeshNodeVis(pMeshNode) != 1.f) {
//				dev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
//				dev->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
//				dev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
//			}
//			else {
//				dev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
//			}

//			dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
//			dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );

//			dev->SetFVF( RVertexType );
//			dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, t_face_cnt , (LPVOID) g_vert, sizeof(RVertex));

//			g_poly_render_cnt += t_face_cnt;//<-----

		//	SetMtrl(pSMtrl,GetMeshNodeVis(pMeshNode));

//			dev->SetRenderState( D3DRS_ALPHABLENDENABLE,   FALSE );
			dev->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
			dev->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
			dev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
			dev->SetRenderState( D3DRS_ZWRITEENABLE, TRUE);

			dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_BLENDTEXTUREALPHA );
//			dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
			dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );//vis ignore
		}

		dev->SetRenderState( D3DRS_LIGHTING, TRUE );
		dev->SetFVF( RVertexType );

		for(node = g_RenderNodeList[i].begin(); node != g_RenderNodeList[i].end(); ) {
			pRNode = (*node);

			if(pBackupMtrl != pRNode->m_pMtrl) {//최대한 mtrl 이 같도록 정렬한다..
				pBackupMtrl = pRNode->m_pMtrl;

//				if(pBackupMtrl->m_bTwoSided) {
//					dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
//				}

				SetMtrl(pBackupMtrl,pRNode->m_vis_alpha);

				if(g_bTextureRenderOnOff && pBackupMtrl) // debug func
					dev->SetTexture( 0, pBackupMtrl->GetTexture());
				else
					dev->SetTexture( 0, NULL);
			}
			dev->SetTransform(D3DTS_WORLD, &pRNode->m_matWorld);
//			dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, pRNode->m_size, (LPVOID) (g_vert_s + pRNode->m_begin) , sizeof(RVertex));
			dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, pRNode->m_size, (LPVOID) &g_vert_s[pRNode->m_begin] , sizeof(RVertex));

			g_poly_render_cnt += pRNode->m_size;
			++node;
		}

		dev->SetRenderState( D3DRS_LIGHTING, FALSE );
		dev->SetFVF( RLVertexType );

		for(node = g_RenderLNodeList[i].begin(); node != g_RenderLNodeList[i].end(); ) {
			pRNode = (*node);

			if(pBackupMtrl != pRNode->m_pMtrl) {
				pBackupMtrl = pRNode->m_pMtrl;

//				if(pBackupMtrl->m_bTwoSided) {
//					dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
//				}

//				SetMtrl(pBackupMtrl,);

				if(g_bTextureRenderOnOff && pBackupMtrl) // debug func
					dev->SetTexture( 0, pBackupMtrl->GetTexture());
				else
					dev->SetTexture( 0, NULL);
			}

			dev->SetTransform(D3DTS_WORLD, &pRNode->m_matWorld);
			dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, pRNode->m_size , (LPVOID) &g_lvert_s[pRNode->m_begin] , sizeof(RLVertex));
			g_poly_render_cnt += pRNode->m_size;
			++node;
		}
	}

	for(int i=0;i<eRRenderNode_End;i++) {
		g_RenderNodeList[i].clear();
		g_RenderLNodeList[i].clear();
	}
*/
	return true;
}

_NAMESPACE_REALSPACE2_END