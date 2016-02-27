#ifndef _RMeshNodeData_h
#define _RMeshNodeData_h

#include "RMtrl.h"
#include "RMeshUtil.h"
#include "RMeshNodeStringTable.h"

_NAMESPACE_REALSPACE2_BEGIN

class RMesh;
class RAnimationNode;
class RAnimation;

// 순수 모델 데이터 정보

class RMeshNodeData :public RBaseObject
{
public:
	RMeshNodeData();
	virtual ~RMeshNodeData();

	// bbox

	void SubCalc(D3DXVECTOR3* v);
	void BBoxClear();
	void CalcLocalBBox();

public:

	// id

	int				m_id;
	int				m_u_id;
	int				m_mtrl_id;

	// name

	char			m_Parent[MAX_NAME_LEN];
	int				m_nParentNodeID;

	// bbox

	D3DXVECTOR3		m_max;
	D3DXVECTOR3		m_min;

	// matrix

	D3DXMATRIX		m_mat_base;
	D3DXMATRIX		m_mat_parent_inv;
	D3DXMATRIX		m_mat_local;
	D3DXMATRIX		m_mat_inv;
	D3DXMATRIX		m_mat_etc;
	D3DXMATRIX		m_mat_flip;
	D3DXMATRIX		m_mat_scale;
	D3DXMATRIX		m_mat_result;

	D3DXMATRIX		m_mat_add;			// 추가 장작된 모델..

	D3DXMATRIX		m_mat_ref;
	D3DXMATRIX		m_mat_ref_inv;

	int				m_point_num;
	int				m_face_num;
	int				m_physique_num;
	int				m_point_normal_num;
	int				m_point_color_num;

	D3DXVECTOR3*	m_point_list;
	D3DXVECTOR3*	m_point_color_list;

	RFaceInfo*		m_face_list;
	RPhysiqueInfo*	m_physique;

	RFaceNormalInfo* m_face_normal_list;

	// temp

	D3DXVECTOR3		m_spine_local_pos;
	D3DXVECTOR3		m_ap_scale;

	D3DXVECTOR3		m_axis_scale;
	float			m_axis_scale_angle;

	D3DXVECTOR3		m_axis_rot;
	float			m_axis_rot_angle;

};

//////////////////////////////////////////////////////////////////
// Node Mtrl 

class RMeshNodeMtrl
{
public:
	RMeshNodeMtrl();
	virtual ~ RMeshNodeMtrl();

	void	SetTColor(DWORD color);
	DWORD	GetTColor();

	int		GetMtrlCount();
	RMtrl*	GetMtrl(int i=0);// MtrlTable

//	void	SetLMtrl(DWORD& color,float vis_alpha);
//	void	SetLMtrl(DWORD& color,D3DXCOLOR& mc,float vis_alpha);

	void	SetMtrlDiffuse(RMtrl* pMtrl,float vis_alpha);
	void	SetMtrl(RMtrl* pMtrl,float vis_alpha,bool bNpc,D3DCOLORVALUE color );
	void	SetMtrl(D3DXCOLOR* c,float vis_alpha);

public:

	DWORD			m_dwTFactorColor;

	int				m_nMtrlCnt;
	RMtrl**			m_pMtrlTable;
};


_NAMESPACE_REALSPACE2_END

#endif//_RMeshNodeData_h