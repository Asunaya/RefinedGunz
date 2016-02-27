#if !defined(AFX_RMTRL_H__8A7FB4A5_9CA2_409D_99FE_AD7B159F607A__INCLUDED_)
#define AFX_RMTRL_H__8A7FB4A5_9CA2_409D_99FE_AD7B159F607A__INCLUDED_

//#if _MSC_VER > 1000
//#pragma once
//#endif // _MSC_VER > 1000

#include <list>
#include <vector>
#include <string>
#include <map>

#include <D3DX9.h>

#include "RBaseTexture.h"

#define SAFE_REL(p)      { if(p) { (p)->Release(); (p)=NULL; } }

// 임시 테스트용 //

/*
class RMtrlNode {
public:
	char				m_tex_name[MAX_NAME_LEN];
	LPDIRECT3DTEXTURE9	m_tex;

	D3DXCOLOR			m_ambient;
	D3DXCOLOR			m_diffuse;
	D3DXCOLOR			m_specular;

	float				m_power;

	RMtrlNode*			m_sub_mtrl;
	int					m_sub_mtrl_num;
};
*/

_USING_NAMESPACE_REALSPACE2

// xml 등으로 외부로 빼준다.. 
// 일괄적으로 상태 모두 정리한다.. 
// 디자이너가 자유롭게 사용가능하도록..

class RMtrl
{
public:
	RMtrl();
	~RMtrl();

	void CheckAniTexture();

	LPDIRECT3DTEXTURE9 GetTexture();

	void Restore(LPDIRECT3DDEVICE9 dev,char* path=NULL);

	void	SetTColor(DWORD color);
	DWORD	GetTColor();

public:

	RBaseTexture*  m_pTexture;

	// 확장..

	RBaseTexture*  m_pToonTexture;		// 디자이너가 지정.. 없다면 전역 Toon..

	D3DTEXTUREFILTERTYPE m_FilterType;		// 디자이너가 선택하도록 한다..
	D3DTEXTUREFILTERTYPE m_ToonFilterType;	// 디자이너가 선택하도록 한다..

	DWORD				m_AlphaRefValue;
	D3DTEXTUREOP		m_TextureBlendMode;
	D3DTEXTUREOP		m_ToonTextureBlendMode;

	D3DXCOLOR	m_ambient;
	D3DXCOLOR	m_diffuse;
	D3DXCOLOR	m_specular;

	float	m_power;

	char	m_mtrl_name[255];//지금은 tool 에서 구분하기 위해서 만들어서 사용..

	char	m_name[255];
	char	m_opa_name[255];

	char	m_name_ani_tex[255];
	char	m_name_ani_tex_ext[20];

	int		m_id;
	int		m_u_id;			//유저가 지정해준 ID
	int		m_mtrl_id;	  
	int		m_sub_mtrl_id;	//sub_mtrl ID

	int		m_sub_mtrl_num;

	bool	m_bDiffuseMap;
	bool	m_bTwoSided;
	bool	m_bAlphaMap;
	bool	m_bAlphaTestMap;
	bool	m_bAdditive;

	int		m_nAlphaTestValue;

	bool	m_bUse;

	// ani_tex

	bool	m_bAniTex;
	int 	m_nAniTexCnt;
	int 	m_nAniTexSpeed;//tick
	int 	m_nAniTexGap;
	DWORD	m_backup_time;

	bool	m_bObjectMtrl;// effect ,interface , object 를 구분해준다~

	DWORD	m_dwTFactorColor;

	RBaseTexture** m_pAniTexture;
};

using namespace std;


// 보통의 경우 재할당이 안일어나는 최대값을 지정..
#define MAX_MTRL_NODE 100

///////////////////////////////////////////////////

class RMtrlMgr :public list<RMtrl*>
{
public:
	RMtrlMgr();
	virtual ~RMtrlMgr();

	int		Add(char* name,int u_id = -1);
	int		Add(RMtrl* tex);

	void	Del(int id);

	int		LoadList(char* name);
	int		SaveList(char* name);

	void	DelAll();
	void	Restore(LPDIRECT3DDEVICE9 dev,char* path=NULL);

	void	CheckUsed(bool b);
	void	ClearUsedCheck();
	void	ClearUsedMtrl();

	void	SetObjectTexture(bool bObject) { m_bObjectMtrl = bObject; }

	RMtrl*	Get_s(int mtrl_id,int sub_id);

	LPDIRECT3DTEXTURE9 Get(int id);
	LPDIRECT3DTEXTURE9 Get(int id,int sub_id);
	LPDIRECT3DTEXTURE9 GetUser(int id);
	LPDIRECT3DTEXTURE9 Get(char* name);

	RMtrl*  GetMtrl(char* name);
	RMtrl*  GetToolMtrl(char* name);

	void	Del(RMtrl* tex);
	int		GetNum();

	vector<RMtrl*>	m_node_table;

	bool	m_bObjectMtrl;// effect ,interface , object 를 구분해준다~
	int		m_id_last;
};

#endif // !defined(AFX_RMTRL_H__8A7FB4A5_9CA2_409D_99FE_AD7B159F607A__INCLUDED_)
