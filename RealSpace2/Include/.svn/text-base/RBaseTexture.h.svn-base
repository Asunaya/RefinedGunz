#ifndef _RBASETEXTURE_H
#define _RBASETEXTURE_H

#include <D3DX9.h>

#include <string>
#include <map>
#include <algorithm>

#include "RNameSpace.h"

class MZFileSystem;

using namespace std;

_NAMESPACE_REALSPACE2_BEGIN

/////////////////////////////////////////////////////////
// texture loading base class
// bmp : jpg : tga : dds 지원
// 파일에서의 로딩과 메모리에서의 로딩지원
// 최소기능지원 필요한것은 확장해서 사용~
class RBaseTexture  
{
public:
	RBaseTexture();
	virtual ~RBaseTexture();

	void Destroy();

/*
	bool Create(const char* filename,bool bUseMipmap=false);
	bool Create(void* data,int data_size,bool bUseMipmap=false);
	*/

	void OnInvalidate();
	bool OnRestore(bool bManaged=false);//관리 되기를 원하면 true 로~

	int GetWidth()			{return m_Info.Width; }
	int GetHeight()			{return m_Info.Height;}
	int GetDepth()			{return m_Info.Depth; }
	int GetMipLevels()		{return m_Info.MipLevels;}

	int GetTexLevel()			{ return m_nTexLevel; }
	void SetTexLevel(int level) { m_nTexLevel = level;}

	int GetTexType()			{ return m_nTexType; }
	void SetTexType(int type)	{ m_nTexType = type; }

	D3DFORMAT GetFormat()	{return m_Info.Format;}

	LPDIRECT3DTEXTURE9	GetTexture();

private:

	bool SubCreateTexture();

public:
	bool	m_bManaged;
	DWORD	m_dwLastUseTime;
	char*	m_pTextureFileBuffer;
	int		m_nFileSize;
	char	m_szTextureName[256];
	int		m_nRefCount;
	bool	m_bUseMipmap;
	bool	m_bUseFileSystem;

	int		m_nTexLevel;
	DWORD	m_nTexType;

	D3DXIMAGE_INFO m_Info;
	LPDIRECT3DTEXTURE9 m_pTex;
};


// RTextureManager 

class RTextureManager : public map<string,RBaseTexture*> 
{
public:
	virtual ~RTextureManager();

	void Destroy();

	RBaseTexture *CreateBaseTextureSub(bool mg,const char* filename,int texlevel,bool bUseMipmap=false,bool bUseFileSystem=true);
	RBaseTexture *CreateBaseTexture(const char* filename,int texlevel,bool bUseMipmap=false,bool bUseFileSystem=true);
	RBaseTexture *CreateBaseTextureMg(const char* filename,int texlevel,bool bUseMipmap=false,bool bUseFileSystem=true);

	void DestroyBaseTexture(RBaseTexture*);
	void DestroyBaseTexture(char* szName);

	void OnInvalidate();
	void OnRestore();
	void OnChangeTextureLevel(DWORD flag);

	int UpdateTexture(DWORD max_life_time=5000);
	int CalcUsedSize();
	int CalcAllUsedSize();
	int PrintUsedTexture();
	int CalcUsedCount();
};

void RBaseTexture_Create();
void RBaseTexture_Destory();

void RBaseTexture_Invalidate();
void RBaseTexture_Restore();

RTextureManager* RGetTextureManager();

// 옵션에 따른 텍스쳐 디테일을 사용하려면~

#define RTextureType_Etc		0
#define RTextureType_Map		1<<1
#define RTextureType_Object		1<<2
#define RTextureType_All		1<<3

void SetObjectTextureLevel(int nLevel);
void SetMapTextureLevel(int nLevel);
void SetTextureFormat(int nLevel);	// 0 = 16 bit , 1 = 32bit

int GetObjectTextureLevel();
int GetMapTextureLevel();
int GetTextureFormat();


///////////////////// 다음의 펑션으로 텍스쳐를 만들고 지워주세요

RBaseTexture*	RCreateBaseTexture(const char* filename,DWORD nTexType = RTextureType_Etc,bool bUseMipmap=false,bool bUseFileSystem=true);
RBaseTexture*	RCreateBaseTextureMg(const char* filename,DWORD nTexType = RTextureType_Etc,bool bUseMipmap=false,bool bUseFileSystem=true);

void			RDestroyBaseTexture(RBaseTexture*);

void			RChangeBaseTextureLevel(DWORD flag);

_NAMESPACE_REALSPACE2_END

#endif//_RBASETEXTURE_H
