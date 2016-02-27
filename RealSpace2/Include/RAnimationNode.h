#ifndef _RAnimationNode_h
#define _RAnimationNode_h

#include "RAnimationDef.h"

_NAMESPACE_REALSPACE2_BEGIN

class RMesh;

class RAnimationNode : public RBaseObject
{
public:
	RAnimationNode();
	~RAnimationNode();

	float GetVisValue(int frame);

	D3DXQUATERNION	GetRotValue(int frame);
	D3DXVECTOR3		GetPosValue(int frame);
	rmatrix			GetTMValue(int frame);
	int 			GetVecValue(int frame,D3DXVECTOR3* pVecTable);

	void ConnectToNameID();//자기이름으로 연결

public:

	D3DXMATRIX		m_mat_base;
	int				m_node_id;
	RMesh*			m_pConnectMesh;

	int				m_pos_cnt;
	int				m_rot_cnt;
	int				m_mat_cnt;
	int				m_vis_cnt;

	RPosKey*		m_pos;
	RQuatKey*		m_quat;
	RTMKey*			m_mat;
	RVisKey*		m_vis;

	int				m_vertex_cnt;
	int				m_vertex_vcnt;
	DWORD*			m_vertex_frame;
	D3DXVECTOR3**	m_vertex;
};


_NAMESPACE_REALSPACE2_END

#endif//_RAnimationNode_h