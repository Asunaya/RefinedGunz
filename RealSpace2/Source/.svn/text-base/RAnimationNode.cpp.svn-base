#include "stdafx.h"
#include "RAnimationNode.h"

#include "RealSpace2.h"

_USING_NAMESPACE_REALSPACE2

_NAMESPACE_REALSPACE2_BEGIN

////////////////////////////////////////////////////////////////////

RAnimationNode::RAnimationNode() 
{
	m_node_id = -1;

	m_pos_cnt = 0;
	m_rot_cnt = 0;
	m_mat_cnt = 0;
	m_vis_cnt = 0;

	m_vertex_cnt = 0;
	m_vertex_vcnt = 0;

	m_pos = NULL;
	m_quat = NULL;
	m_mat = NULL;
	m_vis = NULL;

	m_vertex = NULL;
	m_vertex_frame = NULL;

	m_pConnectMesh = NULL;

	D3DXMatrixIdentity(&m_mat_base);
}

RAnimationNode::~RAnimationNode() 
{
	DEL2(m_pos);
	DEL2(m_quat);
	DEL2(m_mat);
	DEL2(m_vis);

	if(m_vertex_cnt) {
		for(int i=0;i<m_vertex_cnt;i++){
			DEL2(m_vertex[i]);
		}
		DEL2(m_vertex);
	}

	DEL2(m_vertex_frame);
}

void RAnimationNode::ConnectToNameID()
{
	int id = RGetMeshNodeStringTable()->Get( m_Name );

	if(id==-1) {//bip 가 아닌 일반 오브젝트들...

	}

	m_NameID = id;
}

// pos ,pos+1 을 사용하니까 범위에 주의...

float GetVisKey(RVisKey* pKey,int pos,int key_max,int frame)
{
	if(!pKey) return 1.f;

	if( pos < 0 || pos >= key_max ) // 범위를 넘어서면..
		return 1.0f;

	float d = 1.f;;

	int f2 = pKey[pos+1].frame;
	int f1 = pKey[pos].frame;

	int s = ( f2 - f1 );

	if(s != 0 )	d = (float)(frame - f1) /(float)s;

	d = pKey[pos].v + (pKey[pos+1].v - pKey[pos].v) * d;

	return d;
}

int GetVisKeyPos(RVisKey* pKey,int key_max,int frame)
{
	if(!pKey)		return 0;
	if(key_max==0)	return 0;

	for (int p=0;p<key_max;p++) {
		if ( pKey[p].frame > frame) {
			break;
		}
	}

	if(p) p--;

	return p;
}

float RAnimationNode::GetVisValue(int frame)
{
	if(m_vis_cnt == 0)	return 1.0f;
	if(m_vis==NULL) 	return 1.0f;

	int key_pos = GetVisKeyPos(m_vis,m_vis_cnt,frame);

	if( key_pos+2 > m_vis_cnt )
		return m_vis[m_vis_cnt-1].v;

	return GetVisKey(m_vis,key_pos,m_vis_cnt,frame);
}

D3DXQUATERNION RAnimationNode::GetRotValue(int frame)
{
	D3DXQUATERNION rq;

	if( m_rot_cnt==0 || m_quat==NULL ) {
		D3DXQuaternionRotationMatrix(&rq,&m_mat_base);
		return rq;
	}

	for (int p=0;p<m_rot_cnt;p++) {
		if ( m_quat[p].frame > frame) {
			break;
		}
	}

	if(p>=m_rot_cnt) {
		return m_quat[m_rot_cnt-1];
	}

	if(p)	p--;

	int s;
	float d = 1.f;

	s = (m_quat[p+1].frame - m_quat[p].frame );

	if (s != 0)	d = (float)(frame - m_quat[p].frame) / (float)s;

	D3DXQuaternionSlerp(&rq,&m_quat[p],&m_quat[p+1],d);

	return rq;
}

D3DXVECTOR3 RAnimationNode::GetPosValue(int frame)
{
	D3DXVECTOR3 v;

	if( m_pos_cnt == 0 || m_pos == NULL ) {
		return GetTransPos(m_mat_base);
	}

	for (int p=0;p<m_pos_cnt;p++)	{
		if ( m_pos[p].frame > frame ) {
			break;
		}
	}

	if(p >= m_pos_cnt) {
		return m_pos[m_pos_cnt-1];
	}

	if(p) p--;

	int s = (m_pos[p+1].frame - m_pos[p].frame );//tick

	float d = 1.f;

	if (s!=0)	d = (float)(frame%s)/(float)s;

	v = m_pos[p] - m_pos[p+1];

	if (v.x==0.f && v.y==0.f && v.z==0.f) d = 0;

	v = m_pos[p] - v * d;

	return v;
}

int	RAnimationNode::GetVecValue(int frame,D3DXVECTOR3* pVecTable)
{
	DWORD dwFrame = frame;

	for (int j=0;j<m_vertex_cnt;j++) {
		if ( m_vertex_frame[j] > dwFrame) 
			break;
	}

	if( j>= m_vertex_cnt) {

		int vcnt = m_vertex_vcnt;

		D3DXVECTOR3* v1 = m_vertex[vcnt-1];
		memcpy(pVecTable,v1,sizeof(D3DXVECTOR3)*vcnt);

		return vcnt;
	}

	if(j)	j--;

	int   s = m_vertex_frame[j+1] - m_vertex_frame[j];

	float d = 1;

	if (s != 0)	d = (frame - m_vertex_frame[j] )/(float)s;

	D3DXVECTOR3* v1 = m_vertex[j];
	D3DXVECTOR3* v2 = m_vertex[j+1];

	D3DXVECTOR3 v;

	int vcnt = m_vertex_vcnt;

	for(int k=0;k<vcnt;k++) {
		D3DXVec3Lerp(&v,&v1[k],&v2[k], d);
		pVecTable[k] = v;
	}

	return vcnt;
}

rmatrix RAnimationNode::GetTMValue(int frame)
{
	int j = 0;

	for (j=0;j<m_mat_cnt;j++) {
		if ( m_mat[j].frame > frame) break;
	}

	if(j >= m_mat_cnt) {
		return m_mat[m_mat_cnt-1];
	}

	if(j) j--;

	return m_mat[j];
/*
	// 행렬보간은 충분한 테스트후 풀기..

	D3DXMATRIX* matf;
	D3DXMATRIX* matb;

	D3DXMATRIX mat;
	D3DXVECTOR3 vec[3];
	D3DXVECTOR3 scale[3];
	D3DXQUATERNION quat[3];

	int s;
	float d = 1.f;

	s = (m_mat[j+1].frame - m_mat[j].frame );

	if (s != 0)	d = (float)(frame - m_mat[j].frame) / s;

	matf = (D3DXMATRIX*)&m_mat[j];
	matb = (D3DXMATRIX*)&m_mat[j+1];

	// rot

	D3DXQuaternionRotationMatrix(&quat[1],matf);
	D3DXQuaternionRotationMatrix(&quat[2],matb);

	D3DXQuaternionSlerp(&quat[0],&quat[1],&quat[2],d);

	D3DXMatrixRotationQuaternion(&mat,&quat[0]);

	// scale

	mat._11 = matf->_11+(matb->_11 - matf->_11) * d;
	mat._22 = matf->_22+(matb->_22 - matf->_22) * d;
	mat._33 = matf->_33+(matb->_33 - matf->_33) * d;

	// pos

	vec[1].x = matf->_41;
	vec[1].y = matf->_42;
	vec[1].z = matf->_43;

	vec[2].x = matb->_41;
	vec[2].y = matb->_42;
	vec[2].z = matb->_43;

	D3DXVec3Lerp(&vec[0],&vec[1],&vec[2],d);

	mat._41 = vec[0].x;
	mat._42 = vec[0].y;
	mat._43 = vec[0].z;

	return *matf;
*/
}

_NAMESPACE_REALSPACE2_END