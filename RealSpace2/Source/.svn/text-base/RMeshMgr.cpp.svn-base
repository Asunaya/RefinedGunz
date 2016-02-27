#include "stdafx.h"
#include "MXml.h"

#include <string>

#include "RealSpace2.h"

#include "RMeshMgr.h"

#include "MZFileSystem.h"

#include "MDebug.h"

_USING_NAMESPACE_REALSPACE2

_NAMESPACE_REALSPACE2_BEGIN

RMeshMgr::RMeshMgr()
{
	m_id_last = 0;
	m_cur	  = 0;

	m_mtrl_auto_load = true;
	m_is_map_object = false;

	m_node_table.reserve(MAX_NODE_TABLE);//기본

	for(int i=0;i<MAX_NODE_TABLE;i++)
		m_node_table[i] = NULL;

}

RMeshMgr::~RMeshMgr()
{
	DelAll();
}

int RMeshMgr::Add(char* name,char* modelname,bool namesort)
{
//	mlog("RMeshMgr::Add begin name = %s  ",name);

	RMesh* node;
	node = new RMesh;

	if(node==NULL) {
		mlog("RMeshMgr::Add begin modelname = %s  ",modelname);
		return -1;
	}

	if(namesort)//이펙트의 내부노드를 이름순으로 정렬..
		node->m_bEffectSort = namesort;

	node->SetMtrlAutoLoad( m_mtrl_auto_load );
	node->SetMapObject(m_is_map_object);		//특별한 지정이 없다면 world model 들을 위해..

	if (!node->ReadElu(name)) {
		mlog("elu %s file loading failure !!!\n",name);
		return -1;
	}

	if(modelname) {
		node->SetName(modelname);
	}

//	mlog("CalcBox_b");

	node->CalcBox();

//	mlog("CalcBox_e");

//	m_node_table[m_id_last] = node;
	m_node_table.push_back(node);
//	node->m_u_id = u_id;
	node->m_id = m_id_last;

	if(m_id_last > MAX_NODE_TABLE)
		mlog("MeshNode 예약 사이즈를 늘리는것이 좋겠음...\n");

//	strcpy(node->m_name,name);

	m_list.push_back(node);
	m_id_last++;
//	mlog("RMeshMgr::Add end \n",name);
	return m_id_last-1;
}

//#define	LOAD_TEST

#ifdef	LOAD_TEST

#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);

#else

#define __BP(i,n) ;
#define __EP(i) ;

#endif

int	RMeshMgr::LoadXmlList(char* name,RFPROGRESSCALLBACK pfnProgressCallback, void *CallbackParam)
{
	__BP(2007,"RMeshMgr::LoadXmlList");

	MXmlDocument	XmlDoc;
	MXmlElement		PNode,Node,SNode;

	XmlDoc.Create();

//	if( !XmlDoc.LoadFromFile(name) ) 
//		return -1;

//	<-----------------

	char *buffer;
	MZFile mzf;

	if(g_pFileSystem) {
		if(!mzf.Open(name,g_pFileSystem)) {
			if(!mzf.Open(name))
				return -1;
		}
	} else {
		if(!mzf.Open(name))
			return -1;
	}

	buffer = new char[mzf.GetLength()+1];
	buffer[mzf.GetLength()] = 0;

	mzf.Read(buffer,mzf.GetLength());

	if(!XmlDoc.LoadFromMemory(buffer))
		return false;

	delete[] buffer;

	mzf.Close();

//	<------------------

	char Path[256];
	Path[0] = NULL;

	GetPath(name,Path);

	PNode = XmlDoc.GetDocumentElement();

	int nCnt = PNode.GetChildNodeCount();
	
	char NodeName[256];
	char IDName[256];
	char FileName[256];
	char WeaponMotionType[256];
//	char WeaponType[256];
	char LitModel[256];
	char NameSort[256];
	char AutoLoad[256];

	RMesh* pMesh = NULL;

	__BP(2008,"RMeshMgr::loop");

	for (int i=0; i<nCnt; i++) {
		
		// 콜백이 있으면 진행상황에 대한 정보를 알려준다
		if(pfnProgressCallback)
			pfnProgressCallback(CallbackParam,float(i)/float(nCnt));

		Node = PNode.GetChildNode(i);
		Node.GetTagName(NodeName);

		if (NodeName[0] == '#') continue;

		if (strcmp(NodeName, "AddXml")==0) {
			Node.GetAttribute(IDName, "name");
			Node.GetAttribute(FileName, "filename");
			Node.GetAttribute(AutoLoad, "autoload");

			bool bAutoLoad = true;

			if(AutoLoad[0])	{
				if(strncmp(AutoLoad,"false",5)==0)
					bAutoLoad = false;
			}

			if(AddXml(FileName,IDName,bAutoLoad)==-1) {
				mlog("%s not found\n",IDName);
				XmlDoc.Destroy();
				return -1;
			}
		}
		else if(strcmp(NodeName, "AddElu")==0) {

			Node.GetAttribute(IDName, "name");
			Node.GetAttribute(FileName, "filename");

			if(Add(FileName,IDName)==-1) {
				mlog("%s not found\n",IDName);
				XmlDoc.Destroy();
				return -1;
			}
		}
		else if(strcmp(NodeName, "AddWeaponElu")==0) {

			Node.GetAttribute(IDName, "name");
			Node.GetAttribute(WeaponMotionType, "weapon_motion_type");

			int id = AddXml(&Node,NULL,IDName,false);

			if(id==-1) {
				mlog("%s not found\n",IDName);
				XmlDoc.Destroy();
				return -1;
			}
			else {

				int motion_id = atoi(WeaponMotionType);

				pMesh = GetFast(id);

				if(pMesh) {
					pMesh->SetMeshWeaponMotionType((RWeaponMotionType)motion_id);
				}
			}
		}		

		else if( stricmp( NodeName, "MeshInfo" ) == 0 )
		{
			int nChild = Node.GetChildNodeCount();
			MXmlElement ChildNode;
			for( int iwi = 0; iwi < nChild; ++iwi )
			{
				ChildNode = Node.GetChildNode( iwi );
				ChildNode.GetTagName( NodeName );

				if( NodeName[0] == '#' ) continue;
				
				if( stricmp( NodeName, "AddWorldItemElu" ) == 0 )
				{
					ChildNode.GetAttribute( IDName, "name" );
					//ChildNode.GetAttribute( FileName, "filename" );
					//int id = Add( FileName, IDName );
					int id = AddXml(&ChildNode, "", IDName, false );
					if( id == -1 )
					{
						mlog("%s not found\n",IDName);
						XmlDoc.Destroy();
						return -1;
					}
					else
					{
						RMesh* pMesh = GetFast(id);
						pMesh->m_LitVertexModel = true;
					}
				}
			}
		}
		else if(strcmp(NodeName, "AddEffectElu")==0) {

			Node.GetAttribute(IDName, "name");
//			Node.GetAttribute(FileName, "filename");
			Node.GetAttribute(LitModel, "lit_model");
			Node.GetAttribute(NameSort, "name_sort");

			// alpha type 등을 지정할 수 있다..
			int name_sort = 0;
			int litmodel = 1;

			if(NameSort[0]) {
				name_sort = atoi(NameSort);
			}

			int id = AddXml(&Node,Path,IDName,name_sort ? true:false);

			if(id==-1) {
				mlog("%s not found\n",IDName);
				XmlDoc.Destroy();
				return -1;
			}
			else {

				if( LitModel[0] ) {
					litmodel = atoi(LitModel);
				}

				pMesh = GetFast(id);

				if(pMesh) {
					pMesh->m_LitVertexModel = litmodel? true : false;
				}
			}
		}		
	}

	__EP(2008);

	XmlDoc.Destroy();

	__EP(2007);

	return 1;
}

int RMeshMgr::AddXml(MXmlElement* pNode,char* Path,char* modelname,bool namesort)
{
	RMesh* node;
	node = new RMesh;

	// 이펙트의 내부노드를 이름 순으로 정렬..로딩전에 해야함...

	if(namesort)
		node->m_bEffectSort = namesort;

	if (!node->ReadXmlElement(pNode,Path)) {
		return -1;
	}

	if(modelname) {
		node->SetName(modelname);
	}

	node->CalcBox();

//	m_node_table[m_id_last] = node;
	m_node_table.push_back(node);

	node->m_id = m_id_last;

	if(m_id_last > MAX_NODE_TABLE)
		mlog("MeshNode 예약 사이즈를 늘리는것이 좋겠음...\n");

	m_list.push_back(node);
	m_id_last++;

	return m_id_last-1;
}

int RMeshMgr::AddXml(char* name,char* modelname,bool AutoLoad,bool namesort)
{
	RMesh* node;
	node = new RMesh;

	if(namesort)//이펙트의 내부노드를 이름순으로 정렬..로딩전에 해야함.
		node->m_bEffectSort = namesort;

	if(AutoLoad) {
		if (!node->ReadXml(name)) {
			mlog("xml %s file loading failure !!!\n",name);
			return -1;
		}
	}
	else {
		if(name)
			node->SetFileName(name);
	}

	if(modelname) {
		node->SetName(modelname);
	}

	node->CalcBox();

	m_node_table.push_back(node);

	node->m_id = m_id_last;

	if(m_id_last > MAX_NODE_TABLE)
		mlog("MeshNode 예약 사이즈를 늘리는것이 좋겠음...\n");

	m_list.push_back(node);
	m_id_last++;

	return m_id_last-1;
}

int	RMeshMgr::LoadList(char* fname)
{
	return 1;
}

int	RMeshMgr::SaveList(char* name)
{
	return 1;
}

///////////////////////////////////
// 삭제

void RMeshMgr::Del(RMesh* pMesh)
{
	if(m_list.empty()) return;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end();) {

		if((*node) == pMesh) {
			(*node)->ClearMtrl();					//material clear
			delete (*node);
			node = m_list.erase(node);
		}
		else ++node;
	}
}

void RMeshMgr::Del(int id)
{
	if(m_list.empty()) return;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end();) {

		if((*node)->m_id == id) {
			(*node)->ClearMtrl();					//material clear
			delete (*node);
			node = m_list.erase(node);
		}
		else ++node;
	}
}

///////////////////////////////////
// 모두제거

void RMeshMgr::DelAll()
{
	if(m_list.empty()) return;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end(); ) {

		(*node)->ClearMtrl();						//material clear
		delete (*node);
		(*node) = NULL;
		node = m_list.erase(node);// ++node
	}

	m_node_table.clear();//버퍼는 남아 있다..

	m_id_last = 0;
}

void RMeshMgr::ConnectMtrl()
{
	if(m_list.empty()) return;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end();  ++node) {
		(*node)->ConnectMtrl();
	}
}

//////////////////////////////
// all

void RMeshMgr::Render()
{
	if(m_list.empty()) return;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end();  ++node) {
		(*node)->Render(0,NULL);
	}
}

void RMeshMgr::ReloadAllAnimation()
{
	if(m_list.empty()) return;

	r_mesh_node node;

	RMesh* pMesh = NULL;
	int cnt = 0;

	for(node = m_list.begin(); node != m_list.end();  ++node) {
		pMesh = (*node);
		pMesh->ReloadAnimation();
		cnt++;
	}
}

void RMeshMgr::Render(int id)
{
	if(m_list.empty()) return;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end();) {

		if((*node)->m_id == id) {
			(*node)->Render(0,NULL);
			return;
		}
		else ++node;
	}
}

void RMeshMgr::RenderFast(int id,D3DXMATRIX* unit_mat)
{
	if(m_list.empty()) return;

	if(id == -1) return;
	_ASSERT(m_node_table[id]);
	m_node_table[id]->Render(unit_mat);
}

RMesh* RMeshMgr::GetFast(int id)
{
//	_ASSERT(m_node_table[id]);
//	if(id == -1) return NULL;
	if(id < 0)			return NULL;
	if(id > m_id_last)	return NULL;

	return m_node_table[id];
}

RMesh* RMeshMgr::Get(char* name)
{
	if(m_list.empty()) return NULL;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end(); ++node) {
		if( (*node)->CmpName(name) )
			return (*node);
	}
	return NULL;
}

// 우선은 xml 만 지원한다.

RMesh*	RMeshMgr::Load(char* name)	// 모델 set 을 메모리로 올린다.
{
	RMesh* pMesh = Get(name);

	if(pMesh) {

		if(pMesh->m_isMeshLoaded==false) {
		
			if (!pMesh->ReadXml( pMesh->GetFileName() )) {
				mlog("xml %s file loading failure !!!\n",name);
				return NULL;
			}

		}

		pMesh->m_bUnUsededCheck = true;
	}

	return pMesh;
}

void RMeshMgr::UnLoad(char* name)	// 모델 set 을 메모리에서 내린다.
{
	RMesh* pMesh = Get(name);

	if(pMesh) {

		string filename = pMesh->GetFileName();
		string modelname = pMesh->GetName();

		Del(pMesh);

		AddXml((char*)filename.c_str(),(char*)modelname.c_str(),false,false);
	}
}

void RMeshMgr::LoadAll()
{
	vector<string> t_vec;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end();  ++node) {
		t_vec.push_back( (*node)->GetName() );
	}

	int cnt = (int)t_vec.size();

	for(int i=0;i<cnt;i++) {
		Load( (char*)t_vec[i].c_str() );
	}

}

void RMeshMgr::UnLoadAll()
{
	vector<string> t_vec;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end();  ++node) {
		t_vec.push_back( (*node)->GetName() );
	}

	int cnt = (int)t_vec.size();

	for(int i=0;i<cnt;i++) {
		UnLoad( (char*)t_vec[i].c_str() );
	}

}

// 사용안하는걸로 모두 체크

void RMeshMgr::CheckUnUsed()
{
	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end();  ++node) {
		(*node)->m_bUnUsededCheck = false;
	}

}

void RMeshMgr::UnLoadChecked()
{
	vector<string> t_vec;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end();  ++node) {

		if((*node)->m_bUnUsededCheck == false )
			t_vec.push_back( (*node)->GetName() );
	}

	int cnt = (int)t_vec.size();

	for(int i=0;i<cnt;i++) {
		UnLoad( (char*)t_vec[i].c_str() );
	}
}


void RMeshMgr::GetPartsNode(RMeshPartsType parts,vector<RMeshNode*>& nodetable)
{
	r_mesh_node node;
	RMesh* pMesh = NULL;
	RMeshNode* pMeshNode = NULL;

	for(node = m_list.begin(); node != m_list.end(); ++node) {
		pMesh = (*node);
		pMesh->GetMeshData(parts,nodetable);
	}
}

RMeshNode* RMeshMgr::GetPartsNode(char* name)
{
	r_mesh_node node;
	RMesh* pMesh = NULL;
	RMeshNode* pMeshNode = NULL;

	for(node = m_list.begin(); node != m_list.end(); ++node) {
		pMesh = (*node);
		pMeshNode = pMesh->GetMeshData(name);
		if(pMeshNode)
			return pMeshNode;
	}
	return NULL;
}
/*
RMeshNode* RMeshMgr::GetParts(RMeshPartsType parts,char* name)
{
	r_mesh_node node;
	RMesh* pMesh = NULL;

	for(node = m_list.begin(); node != m_list.end(); ++node) {
	}
	return NULL;
}


void RMeshMgr::Frame()
{
	if(m_list.empty()) return;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end(); ++node) {
		(*node)->Frame();
	}
}

void RMeshMgr::Frame(int id)
{
	if(m_list.empty()) return;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end();) {

		if((*node)->m_id == id) {
			(*node)->Frame();
			return;
		}
		else ++node;
	}

}
*/
_NAMESPACE_REALSPACE2_END
