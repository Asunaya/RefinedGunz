#include "stdafx.h"
#include "RMtrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdio.h>

#include "RMtrl.h"
#include "assert.h"
#include <tchar.h>

#include "RBaseTexture.h"

#include "MDebug.h"

_USING_NAMESPACE_REALSPACE2

RMtrl::RMtrl()	
{
	m_name[0]	= 0;
	m_opa_name[0] = 0;

	m_id		= -1;
	m_u_id		= -1;
	m_mtrl_id	= -1;	//
	m_sub_mtrl_id = -1;	//

	m_bDiffuseMap	= false;
	m_bAlphaMap		= false;
	m_bTwoSided		= false;
	m_bAdditive		= false;
	m_bAlphaTestMap	= false;

	m_nAlphaTestValue = -1;

	m_ambient  = 0xffffffff;
	m_diffuse  = 0xffffffff;
	m_specular = 0xffffffff;

	m_power = 1.0f;

	m_bUse = true;
	m_pTexture = NULL;
	m_pAniTexture = NULL;

	m_pToonTexture		= NULL;
	m_FilterType		= D3DTEXF_LINEAR;
	m_ToonFilterType	= D3DTEXF_POINT;
	m_AlphaRefValue		= 0x05;
	m_TextureBlendMode		= D3DTOP_BLENDTEXTUREALPHA;
//	m_ToonTextureBlendMode	= D3DTOP_ADDSIGNED;
	m_ToonTextureBlendMode	= D3DTOP_MODULATE2X;

	m_bAniTex = false;
	m_nAniTexCnt = 0;
	m_nAniTexSpeed = 0;
	m_nAniTexGap = 0;
	m_backup_time = 0;

	m_name_ani_tex[0] = 0;
	m_name_ani_tex_ext[0] = 0;
	m_bObjectMtrl = false;

	m_dwTFactorColor = D3DCOLOR_COLORVALUE(0.0f,1.0f,0.0f,0.0f);	//	알파도 없는 완전 녹색..색이 없다는걸 의미
}

RMtrl::~RMtrl()
{
	if( m_bAniTex ) {
		if(m_pAniTexture) {
			for(int i=0;i < m_nAniTexCnt;i++) {
				RDestroyBaseTexture(m_pAniTexture[i]);
			}
			delete [] m_pAniTexture;
		}
	}
	else {
		RDestroyBaseTexture(m_pTexture);
		m_pTexture = NULL;
	}
}

LPDIRECT3DTEXTURE9 RMtrl::GetTexture() {

	if(m_bAniTex) {

		DWORD this_time = timeGetTime();

		DWORD gap = (this_time - m_backup_time);

		if(gap > (DWORD)m_nAniTexSpeed) {
			gap %= m_nAniTexSpeed;
			m_backup_time = this_time;
		}

		int pos = gap / m_nAniTexGap;

		if((pos < 0) || (pos > m_nAniTexCnt-1))
			pos = 0;

		if(m_pAniTexture[pos]) {

			return 	m_pAniTexture[pos]->GetTexture();
		}

		return NULL;
	}
	else {
		if(!m_pTexture) return NULL; 
		return m_pTexture->GetTexture(); 
	}
}

void RMtrl::SetTColor(DWORD color)
{
	m_dwTFactorColor = color;
}

DWORD RMtrl::GetTColor()
{
	return m_dwTFactorColor;
}

void RMtrl::CheckAniTexture()
{
	if(m_name[0]) {

		char drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];
		_splitpath(m_name,drive,dir,fname,ext);

		if( strnicmp( fname,"txa",3)==0) {

			static char texname[256];

			int imax = 0;
			int ispeed = 0;

			// ex> txa_10_100_test01.tga
			// test02.tga

			sscanf( fname,"txa %d %d %s",&imax,&ispeed,texname );

			m_pAniTexture = new RBaseTexture*[imax];

			for(int i=0;i<imax;i++) { 
				m_pAniTexture[i] = NULL; 
			}

			int n = (int) strlen(texname);
			
			if(dir[0]) {
				strcpy(m_name_ani_tex,dir);
				strncat(m_name_ani_tex,texname,n-2);
				int pos = strlen(dir) + n - 2;
				m_name_ani_tex[ pos ] = NULL;
			}
			else {
				strncpy(m_name_ani_tex,texname,n-2);//숫자제거
				m_name_ani_tex[n-2] = NULL;
			}

			strcpy(m_name_ani_tex_ext,ext);// ext

			m_nAniTexSpeed = ispeed;
			m_nAniTexCnt =  imax;
			m_nAniTexGap =  ispeed / imax;
			
			m_bAniTex = true;
		}
	}
}

void RMtrl::Restore(LPDIRECT3DDEVICE9 dev,char* path)
{
	if(m_name[0] == 0) return;

	static char name[256];
	static char name2[256];

	int level = RTextureType_Etc;

	if(m_bObjectMtrl) {
		level = RTextureType_Object;
	}

	// filename 에 path 가 있다면..맵 path 무시 - 맵 오브젝트 texture 같이 쓰기 위해..

	bool map_path = false;

	char drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];
	_splitpath(m_name,drive,dir,fname,ext);

	if( dir[0] )
		map_path = true;

	if( !map_path &&  path && path[0] ) {

		if(m_bAniTex) {

			// 1번..
			strcpy(name,path);
			strcat(name,m_name);

			m_pAniTexture[0] = RCreateBaseTextureMg(name,level);

			if(m_pAniTexture[0]==NULL) {
				m_pAniTexture[0] = RCreateBaseTextureMg(m_name,level);
			}

			// 2번째부터..

			strcpy(name2,path);
			strcat(name2,m_name_ani_tex);//보관..

			for(int i=1;i<m_nAniTexCnt;i++) {
				sprintf(name,"%s%02d%s",name2,i,m_name_ani_tex_ext);
				m_pAniTexture[i] = RCreateBaseTextureMg(name,level);

				if(m_pAniTexture[i]==NULL) {// 경로에 없을 경우 현 폴더에서 찾아보기
					sprintf(name,"%s%2d.%s",m_name_ani_tex,i,m_name_ani_tex_ext);
					m_pAniTexture[i] = RCreateBaseTextureMg(name,level);
					if(m_pAniTexture[i]==NULL)
						int a= 0;
				}
			}
		}
		else {

			strcpy(name,path);
			strcat(name,m_name);

			m_pTexture = NULL;
			m_pTexture = RCreateBaseTextureMg(name,level);

			if(!m_pTexture)		// 경로에 없을 경우 현 폴더에서 찾아보기
				m_pTexture = RCreateBaseTextureMg(m_name,level);
		}
	} 
	else {

		if(m_bAniTex) {

			m_pAniTexture[0] = RCreateBaseTextureMg(m_name,level);

			for(int i=1;i<m_nAniTexCnt;i++) {
				sprintf(name,"%s%02d%s",m_name_ani_tex,i,m_name_ani_tex_ext);
				m_pAniTexture[i] = RCreateBaseTextureMg(name,level);
			}

		}
		else {

			m_pTexture=RCreateBaseTextureMg(m_name,level);
		}
	}
}

RMtrlMgr::RMtrlMgr()
{
	m_id_last = 0;

	m_node_table.reserve(MAX_MTRL_NODE);//기본

	for(int i=0;i<MAX_MTRL_NODE;i++)
		m_node_table[i] = NULL;

	m_bObjectMtrl = false;
}

RMtrlMgr::~RMtrlMgr()
{
	DelAll();
}

int RMtrlMgr::Add(char* name,int u_id)
{
	RMtrl* node;
	node = new RMtrl;

	node->m_id = m_id_last;
	node->m_u_id = u_id;
	node->m_bObjectMtrl = m_bObjectMtrl;

	strcpy(node->m_name,name);

	// tool 구분용..

	sprintf(node->m_mtrl_name,"%s%d",name,m_id_last);

	m_node_table.push_back(node);
//	m_data[m_id_last] = node;

	push_back(node);
	m_id_last++;
	return m_id_last-1;
}

int RMtrlMgr::Add(RMtrl* tex)
{
	tex->m_id = m_id_last;
	tex->m_bObjectMtrl = m_bObjectMtrl;

	sprintf(tex->m_mtrl_name,"%s%d",tex->m_name,tex->m_id);

	m_node_table.push_back(tex);
//	m_node_table[m_id_last] = tex;

	push_back(tex);
	m_id_last++;
	return m_id_last-1;
}

int	RMtrlMgr::LoadList(char* fname)
{

/*
	FILE *fp;

	fp = fopen(fname, "rt");

	if ( fp == NULL )	
	{ 
		mlog("texture list 파일이 없음");
		return 0;
	}

	char t_line[256];
	char name[256];

	int u_id;

	while ( !feof(fp) )
	{
		fgets(t_line, 256, fp);
		sscanf(t_line, "%d %s", &u_id,name);

		Add(name,u_id);
	}

	fclose(fp);
*/

	return 1;
}

int	RMtrlMgr::SaveList(char* name)
{

	return 1;
}

void RMtrlMgr::Del(RMtrl* tex)
{
	iterator node;

	for(node = begin(); node != end();) {
		if((*node) == tex) {
			RDestroyBaseTexture((*node)->m_pTexture);
			(*node)->m_pTexture=NULL;
			delete (*node);
			node = erase(node);
		}
		else ++node;
	}
}

void RMtrlMgr::Del(int id)
{
	iterator node;

	for(node = begin(); node != end();) {
		if((*node)->m_id == id) {
			RDestroyBaseTexture((*node)->m_pTexture);
			(*node)->m_pTexture=NULL;
			delete (*node);
			node = erase(node);
		}
		else ++node;
	}
}

void RMtrlMgr::DelAll()
{
	if(size()==0) return;
	
	iterator node;
	RMtrl *pMtrl;

	for(node = begin(); node != end(); ) {
		/*
	    if(!m_bAssigned)
			SAFE_REL( (*node)->m_tex);
		*/
		pMtrl = *node;
		delete pMtrl;
		pMtrl = NULL;

		node = erase(node);// ++node
	}

	m_node_table.clear();//버퍼는 남아 있다..

	m_id_last = 0;
}

/////////////////////////////////////

void RMtrlMgr::Restore(LPDIRECT3DDEVICE9 dev,char* path)
{
	if(size()==0) return;

	iterator node;

	RMtrl* mtrl;

	for(node = begin(); node != end(); ++node) {

		mtrl = (*node);

		if(!mtrl) continue;

		mtrl->Restore(dev,path);
	}
}

void RMtrlMgr::ClearUsedCheck()
{
	iterator node;

	for(node = begin(); node != end(); ++node) {
		(*node)->m_bUse = false;
	}
}

void RMtrlMgr::ClearUsedMtrl()
{
	if(size()==0) return;

	iterator node;
	RMtrl* pMtrl;

	for(node = begin(); node != end(); ) {

		pMtrl = (*node);

		if(pMtrl->m_bUse==false) {
			delete pMtrl;
			pMtrl = NULL;
			node = erase(node);// ++node
		} else {
			++node;
		}
	}

	int _size = size();

}

#include "MProfiler.h"

RMtrl*	RMtrlMgr::Get_s(int mtrl_id,int sub_id)
{
//	_BP("RMtrlMgr::Get_s");

	if(size()==0)
		return NULL;

	iterator node;

	RMtrl* pMtrl = NULL;

	//////////////////////////////////////////////

/*
	static int _cnt = 0;
	static int _size = 0;
	static char _buffer[256];

	_cnt++;
	_size = size();

	sprintf(_buffer,"%d : RMtrlMgr::Get_s : %d\n",_cnt,_size);

	OutputDebugString(_buffer);

*/
	///////////////////////////////////////////////


	for(node = begin(); node != end(); ++node) {

		pMtrl = (*node);

		if(pMtrl) {
			if(pMtrl->m_mtrl_id == mtrl_id) {
				if(pMtrl->m_sub_mtrl_id == sub_id) {
//					_EP("RMtrlMgr::Get_s");
					return pMtrl;
				}
			}
		}
	}

//	_EP("RMtrlMgr::Get_s");

	return NULL;
}

LPDIRECT3DTEXTURE9 RMtrlMgr::Get(int id)
{
	iterator node;

	for(node = begin(); node != end(); ++node) {
		if((*node)->m_id == id) {
			return (*node)->GetTexture();
		}
	}
	return NULL;
}

LPDIRECT3DTEXTURE9 RMtrlMgr::Get(int id,int sub_id)
{
	iterator node;

	for(node = begin(); node != end(); ++node) {
		if((*node)->m_mtrl_id == id && (*node)->m_sub_mtrl_id == sub_id ) {
			return (*node)->GetTexture();
		}
	}
	return NULL;
}


LPDIRECT3DTEXTURE9 RMtrlMgr::GetUser(int u_id)
{
	iterator node;

	for(node = begin(); node != end(); ++node) {
		if((*node)->m_u_id == u_id) {
			return (*node)->GetTexture();
		}
	}
	return NULL;
}


LPDIRECT3DTEXTURE9 RMtrlMgr::Get(char* name)
{
	iterator node;

	for(node = begin(); node != end(); ++node) {
		if(!strcmp((*node)->m_name,name)) {
			return (*node)->GetTexture();
		}
	}
	return NULL;
}

RMtrl* RMtrlMgr::GetMtrl(char* name)
{
	iterator node;

	for(node = begin(); node != end(); ++node) {
		if(!strcmp((*node)->m_name,name)) {
			return (*node);
		}
	}
	return NULL;
}

RMtrl* RMtrlMgr::GetToolMtrl(char* name)
{
	iterator node;

	for(node = begin(); node != end(); ++node) {
		if(!strcmp((*node)->m_mtrl_name,name)) {
			return (*node);
		}
	}
	return NULL;
}

int	RMtrlMgr::GetNum()
{
	return size();
}
