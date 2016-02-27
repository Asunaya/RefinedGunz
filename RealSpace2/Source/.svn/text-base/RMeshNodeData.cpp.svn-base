#include "stdafx.h"

#include "RMeshNodeData.h"
#include "RealSpace2.h"

_USING_NAMESPACE_REALSPACE2

_NAMESPACE_REALSPACE2_BEGIN

//////////////////////////////////////////////////////////////////////////////////
// RMeshNodeData Class

RMeshNodeData::RMeshNodeData() 
{
	m_id = -1;
	m_u_id = -1;
	m_mtrl_id = -1;

	m_spine_local_pos.x = 0.f;
	m_spine_local_pos.y = 0.f;
	m_spine_local_pos.z = 0.f;

	m_ap_scale = rvector(1.f,1.f,1.f);

	m_axis_scale = rvector(0.f,0.f,0.f);
	m_axis_scale_angle = 0.f;

	m_axis_rot = rvector(0.f,0.f,0.f);
	m_axis_rot_angle = 0.f;

	D3DXMatrixIdentity(&m_mat_base);
	D3DXMatrixIdentity(&m_mat_parent_inv);
	D3DXMatrixIdentity(&m_mat_local);
	D3DXMatrixIdentity(&m_mat_inv);
	D3DXMatrixIdentity(&m_mat_etc);
	D3DXMatrixIdentity(&m_mat_flip);
	D3DXMatrixIdentity(&m_mat_scale);
	D3DXMatrixIdentity(&m_mat_result);

	D3DXMatrixIdentity(&m_mat_add);

	D3DXMatrixIdentity(&m_mat_ref);
	D3DXMatrixIdentity(&m_mat_ref_inv);

	m_point_list		= NULL;
	m_face_normal_list	= NULL;
//	m_point_normal_list = NULL;
	m_point_color_list	= NULL;
	m_face_list			= NULL;
//	m_face_normal		= NULL;
	m_physique			= NULL;

	m_point_num			= 0;
	m_face_num			= 0;
	m_physique_num		= 0;
	m_point_normal_num	= 0;
	m_point_color_num	= 0;

	m_min = rvector( 999.f, 999.f, 999.f);
	m_max = rvector(-999.f,-999.f,-999.f);

}

RMeshNodeData::~RMeshNodeData() 
{
	if(m_point_num) {
		DEL2(m_point_list);
//		DEL2(m_point_normal_list);
	}

	if( m_point_color_num ) {
		DEL2(m_point_color_list);
	}

	if(m_face_num) {
		DEL2(m_face_list);
		DEL2(m_face_normal_list);
	}

	if(m_physique_num){
		DEL2(m_physique);
	}
}

void RMeshNodeData::BBoxClear()
{
	m_min = rvector( 999.f, 999.f, 999.f);
	m_max = rvector(-999.f,-999.f,-999.f);
}

void RMeshNodeData::SubCalc(D3DXVECTOR3* v)
{
	if(v==NULL) return;

	if (v->x < m_min.x) m_min.x = v->x;
	if (v->y < m_min.y) m_min.y = v->y;
	if (v->z < m_min.z) m_min.z = v->z;

	if (v->x > m_max.x) m_max.x = v->x;
	if (v->y > m_max.y) m_max.y = v->y;
	if (v->z > m_max.z) m_max.z = v->z; 
}

void RMeshNodeData::CalcLocalBBox()
{
	BBoxClear();

	for(int i=0;i<m_point_num;i++)
	{
		SubCalc(&m_point_list[i]);
	}
}

////////////////////////////////////////////////////////////////////////
// mtrl

RMeshNodeMtrl::RMeshNodeMtrl()
{
//	m_dwTFactorColor = D3DCOLOR_COLORVALUE(1.0f,0.0f,0.0f,1.0f);
	m_dwTFactorColor = D3DCOLOR_COLORVALUE(0.0f,1.0f,0.0f,0.0f);	//	완전 녹색..색이 없다는걸 의미
//	m_dwTFactorColor = 0xff808080;
	m_pMtrlTable = NULL;
	m_nMtrlCnt = 0;
}

RMeshNodeMtrl::~RMeshNodeMtrl()
{
	if(m_pMtrlTable)
		DEL2(m_pMtrlTable);
}

void RMeshNodeMtrl::SetTColor(DWORD color)
{
	m_dwTFactorColor = color;
}

DWORD RMeshNodeMtrl::GetTColor()
{
	return m_dwTFactorColor;
}

int RMeshNodeMtrl::GetMtrlCount()
{
	return m_nMtrlCnt;
}

RMtrl* RMeshNodeMtrl::GetMtrl(int i)
{
	if(i<m_nMtrlCnt) {
		if(m_pMtrlTable) {
			return m_pMtrlTable[i];
		}
	}

	return NULL;
}
/*
void RMeshNodeMtrl::SetLMtrl(DWORD& color,float vis_alpha) 
{
	color = D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,vis_alpha);
}

void RMeshNodeMtrl::SetLMtrl(DWORD& color,D3DXCOLOR& mc,float vis_alpha) 
{
	color = D3DCOLOR_COLORVALUE( mc.r, mc.g, mc.b,vis_alpha);
}
*/
// 아직 안쓴다.

void RMeshNodeMtrl::SetMtrlDiffuse(RMtrl* pMtrl,float vis_alpha)
{
	if(!pMtrl) return;

	D3DMATERIAL9 mtrl;

	ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );

	D3DXCOLOR* c = &pMtrl->m_diffuse;

	mtrl.Diffuse.r = c->r*0.5f;
	mtrl.Diffuse.g = c->g*0.5f;
	mtrl.Diffuse.b = c->b*0.5f;
	mtrl.Diffuse.a = 1.0f;

	mtrl.Ambient.r = c->r*0.1f;
	mtrl.Ambient.g = c->g*0.1f;
	mtrl.Ambient.b = c->b*0.1f;
	mtrl.Ambient.a = 1.0f;

	mtrl.Specular.r = 0.5f;
	mtrl.Specular.g = 0.5f;
	mtrl.Specular.b = 0.5f;
	mtrl.Specular.a = 1.f;

	mtrl.Power = pMtrl->m_power;

	if( pMtrl->m_power ) {
		RGetDevice()->SetRenderState( D3DRS_SPECULARENABLE, TRUE );
	}
	else {
		RGetDevice()->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
	}

	if(vis_alpha != 1.f) {
		mtrl.Diffuse.a	= vis_alpha;
		mtrl.Ambient.a	= vis_alpha;
		mtrl.Specular.a = vis_alpha;
	}

	RGetDevice()->SetMaterial( &mtrl );
	if(RGetShaderMgr()->mbUsingShader )
	{
		RGetShaderMgr()->setMtrl( pMtrl, vis_alpha );
	}
}

void RMeshNodeMtrl::SetMtrl(RMtrl* pMtrl,float vis_alpha,bool bNpc,D3DCOLORVALUE color)
{
	if(!pMtrl) return;

	D3DMATERIAL9 mtrl;

	ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );

	if(bNpc) {

		mtrl.Diffuse.r = color.r;
		mtrl.Diffuse.g = color.g;
		mtrl.Diffuse.b = color.b;
		mtrl.Diffuse.a = 1.0f;

		mtrl.Ambient.r = color.r/2.f;
		mtrl.Ambient.g = color.g/2.f;
		mtrl.Ambient.b = color.b/2.f;
		mtrl.Ambient.a = 1.0f;
	}
	else 
	{
		mtrl.Diffuse.r = color.r;
		mtrl.Diffuse.g = color.g;
		mtrl.Diffuse.b = color.b;
		mtrl.Diffuse.a = 1.0f;

		mtrl.Ambient.r = color.r/2.f;
		mtrl.Ambient.g = color.g/2.f;
		mtrl.Ambient.b = color.b/2.f;
		mtrl.Ambient.a = 1.0f;

/*
		mtrl.Diffuse.r = 0.5f;
		mtrl.Diffuse.g = 0.5f;
		mtrl.Diffuse.b = 0.5f;
		mtrl.Diffuse.a = 1.0f;
	
		mtrl.Ambient.r = 0.35f;
		mtrl.Ambient.g = 0.35f;
		mtrl.Ambient.b = 0.35f;
		mtrl.Ambient.a = 1.0f;
*/
	}

	mtrl.Specular.r = 0.5f;
	mtrl.Specular.g = 0.5f;
	mtrl.Specular.b = 0.5f;
	mtrl.Specular.a = 1.f;

	mtrl.Power = pMtrl->m_power;

	if( pMtrl->m_power ) {
		RGetDevice()->SetRenderState( D3DRS_SPECULARENABLE, TRUE );
	}
	else {
		RGetDevice()->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
	}

	if(vis_alpha != 1.f) {
		mtrl.Diffuse.a	= vis_alpha;
		mtrl.Ambient.a	= vis_alpha;
		mtrl.Specular.a = vis_alpha;
	}

	RGetDevice()->SetMaterial( &mtrl );
	if(RGetShaderMgr()->mbUsingShader )
	{
		RGetShaderMgr()->setMtrl( pMtrl, vis_alpha );
	}
}

void RMeshNodeMtrl::SetMtrl(D3DXCOLOR* c,float vis_alpha) 
{
	if(!c) return;

	D3DMATERIAL9 mtrl;

	ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );

	mtrl.Diffuse.r = c->r;
	mtrl.Diffuse.g = c->g;
	mtrl.Diffuse.b = c->b;
	mtrl.Diffuse.a = c->a;

	mtrl.Ambient.r = c->r*0.5f;
	mtrl.Ambient.g = c->g*0.5f;
	mtrl.Ambient.b = c->b*0.5f;
	mtrl.Ambient.a = 1.0f;

	mtrl.Specular.r = 1.0f;
	mtrl.Specular.g = 1.0f;
	mtrl.Specular.b = 1.0f;
	mtrl.Specular.a = 1.0f;

	if(vis_alpha != 1.f) {
		mtrl.Diffuse.a	= vis_alpha;
		mtrl.Ambient.a	= vis_alpha;
		mtrl.Specular.a = vis_alpha;
	}

	RGetDevice()->SetMaterial( &mtrl );
	if(RGetShaderMgr()->mbUsingShader )
	{
		RGetShaderMgr()->setMtrl( *c, vis_alpha );
	}
}



_NAMESPACE_REALSPACE2_END