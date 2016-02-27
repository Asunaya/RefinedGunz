#ifndef _RMeshMgr_h
#define _RMeshMgr_h

//#if _MSC_VER > 1000
//#pragma once
//#endif // _MSC_VER > 1000

#include "RMesh.h"
#include <vector>

_NAMESPACE_REALSPACE2_BEGIN

typedef list<RMesh*>			r_mesh_list;
typedef r_mesh_list::iterator	r_mesh_node;

#define MAX_NODE_TABLE 1000

class RMeshMgr
{
public:

	RMeshMgr();
	~RMeshMgr();

	int		Add(char* name,char* modelname=NULL,bool namesort=false);
	int		AddXml(char* name,char* modelname=NULL,bool autoload=true,bool namesort=false);
	int     AddXml(MXmlElement* pNode,char* Path,char* modelname=NULL,bool namesort=false);

	int		LoadXmlList(char* name,RFPROGRESSCALLBACK pfnProgressCallback=NULL, void *CallbackParam=NULL);

	void	Del(int id);
	void	Del(RMesh* pMesh);

	int		LoadList(char* name);
	int		SaveList(char* name);

	void	DelAll();

	void	ConnectMtrl();
	void	Render();
	void	Render(int id);

	void	RenderFast(int id,D3DXMATRIX* unit_mat);

//	void	Frame();
//	void	Frame(int id);

	RMesh*	GetFast(int id);
	RMesh*	Get(char* name);

	RMesh*	Load(char* name);	// 모델 set 을 메모리로 올린다.
	void	UnLoad(char* name);	// 모델 set 을 메모리에서 내린다.

	void	LoadAll();
	void	UnLoadAll();		

	void	CheckUnUsed();
	void	UnLoadChecked();

	void ReloadAllAnimation();

	void GetPartsNode(RMeshPartsType parts,vector<RMeshNode*>& nodetable);
	RMeshNode* GetPartsNode(char* name);
//	RMeshNode* GetParts(RMeshPartsType parts,char* name);

	void SetMtrlAutoLoad(bool b) {
		m_mtrl_auto_load = b;
	}

	bool GetMtrlAutoLoad() {
		return m_mtrl_auto_load;
	}

	void SetMapObject(bool b) {
		m_is_map_object = b;
	}

	bool GetMapObject() {
		return m_is_map_object;
	}

	r_mesh_list m_list;
	int			m_id_last;

	bool		m_mtrl_auto_load;
	bool		m_is_map_object;

	vector<RMesh*> m_node_table;
	DWORD	m_cur;
};

_NAMESPACE_REALSPACE2_END

#endif//_RMeshMgr_h
