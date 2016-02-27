#ifndef _RVisualMeshMgr_h
#define _RVisualMeshMgr_h

//#if _MSC_VER > 1000
//#pragma once
//#endif // _MSC_VER > 1000

///////////////////////////////////////////////////////////

#include "RVisualMesh.h"

_NAMESPACE_REALSPACE2_BEGIN

typedef list<RVisualMesh*>		r_vmesh_list;
typedef r_vmesh_list::iterator	r_vmesh_node;

#define MAX_VMESH_TABLE 1000

class RVisualMeshMgr {
public:

	RVisualMeshMgr(); 
	~RVisualMeshMgr();

	int		Add(RMesh* pMesh);
	int		Add(RVisualMesh* pMesh);

	void	Del(int id);
	void	Del(RVisualMesh* pMesh);

	void	DelAll();

	void	Render();
	void	Render(int id);

	void	RenderFast(int id);

	void	Frame();
	void	Frame(int id);

	RVisualMesh* GetFast(int id);

	r_vmesh_list m_list;
	int			 m_id_last;

	vector<RVisualMesh*> m_node_table;
};

extern bool g_bBirdRenderTest;

_NAMESPACE_REALSPACE2_END

#endif//_RVisualMeshMgr_h