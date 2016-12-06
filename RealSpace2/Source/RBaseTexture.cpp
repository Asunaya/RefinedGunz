#include "stdafx.h"
#include "RealSpace2.h"
#include "RBaseTexture.h"
#include "MDebug.h"
#include "MZFileSystem.h"
#include "TextureLoader.h"

_USING_NAMESPACE_REALSPACE2

_NAMESPACE_REALSPACE2_BEGIN

#define _MANAGED
#define LOAD_FROM_DDS

static int g_nObjectTextureLevel;
static int g_nMapTextureLevel;
static int g_nTextureFormat; // 0 = 16 bit, 1 = 32bit

void SetObjectTextureLevel(int nLevel) {
	g_nObjectTextureLevel = nLevel == 3 ? 8 : nLevel; // HACK: Archetype's
}
void SetMapTextureLevel(int nLevel) {
	g_nMapTextureLevel = nLevel == 3 ? 8 : nLevel;
}
void SetTextureFormat(int nLevel) { g_nTextureFormat = nLevel; }
int GetObjectTextureLevel() { return g_nObjectTextureLevel; }
int GetMapTextureLevel() { return g_nMapTextureLevel; }
int GetTextureFormat() { return g_nTextureFormat; }

static int SubGetTexLevel(u32 tex_type)
{
	if (tex_type == RTextureType_Etc)
		return 0;
	else if (tex_type == RTextureType_Map)
		return GetMapTextureLevel();
	else if (tex_type == RTextureType_Object)
		return GetObjectTextureLevel();

	return 0;
}

RBaseTexture::~RBaseTexture() = default;

void RBaseTexture::Destroy()
{
}

void RBaseTexture::OnInvalidate()
{
	SAFE_RELEASE(m_pTex);
}

LPDIRECT3DTEXTURE9 RBaseTexture::GetTexture()
{
	m_dwLastUseTime = GetGlobalTimeMS();

	if (m_pTex) {
		return m_pTex.get();
	}
#ifndef _PUBLISH
	else {
		OnInvalidate();
		OnRestore();
	}
#endif

	return m_pTex.get();
}

//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define	LOAD_TEST
#endif

#ifdef LOAD_TEST
#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);
#else
#define __BP(i,n);
#define __EP(i);
#endif

///////////////////////////////////////////////////////////////////////////

BOOL IsCompressedTextureFormatOk(D3DFORMAT TextureFormat);

bool RBaseTexture::SubCreateTexture(char* TextureFileBuffer)
{
	const D3DPOOL pool =
#ifndef _MANAGED
		D3DPOOL_DEFAULT;
#else
		D3DPOOL_MANAGED;
#endif

#ifndef D3DX
	m_pTex = LoadTexture(TextureFileBuffer, m_nFileSize,
		1.0f / (1 << m_nTexLevel),
		m_Info, strrchr(m_szTextureName, '.'));

	if (!m_pTex)
	{
		MLog("RBaseTexture -- Failed to create texture %s\n",
			m_szTextureName);
		return false;
	}
	return true;
#else
	UINT Tex_w = D3DX_DEFAULT;
	UINT Tex_h = D3DX_DEFAULT;

	m_nTexLevel = SubGetTexLevel(m_nTexType);

	__BP(2011, "RBaseTexture::SubCreateTexture");

	if (FAILED(D3DXGetImageInfoFromFileInMemory(TextureFileBuffer, m_nFileSize, &m_Info)))
	{
		_RPT1(_CRT_WARN, "%s ---->> get image info failure \n", m_szTextureName);
		__EP(2011);
		return false;
	}

	if (m_nTexLevel) {
		Tex_w = m_Info.Width / (1 << m_nTexLevel);
		Tex_h = m_Info.Height / (1 << m_nTexLevel);
	}

	D3DFORMAT d3dformat = D3DFMT_UNKNOWN;

	if (g_nTextureFormat == 0) {
		switch (m_Info.Format) {
		case D3DFMT_A8R8G8B8:
			d3dformat = D3DFMT_A4R4G4B4;
			break;
		case D3DFMT_X8R8G8B8:
		case D3DFMT_R8G8B8:
			d3dformat = D3DFMT_R5G6B5;
			break;
		case D3DFMT_DXT1:
		case D3DFMT_DXT2:
		case D3DFMT_DXT3:
		case D3DFMT_DXT4:
		case D3DFMT_DXT5:
			d3dformat = m_Info.Format;
			break;
		default:
			_ASSERT(FALSE);
		}
	}
	else
		d3dformat = m_Info.Format;

	if (d3dformat == D3DFMT_DXT1 || d3dformat == D3DFMT_DXT2 || d3dformat == D3DFMT_DXT3 ||
		d3dformat == D3DFMT_DXT4 || d3dformat == D3DFMT_DXT5)
	{
		if (IsCompressedTextureFormatOk(m_Info.Format) == FALSE) {
			if (m_Info.Format == D3DFMT_DXT1) {
				d3dformat = D3DFMT_R5G6B5;
			}
			else
				d3dformat = D3DFMT_A4R4G4B4;
		}
	}

	if (FAILED(D3DXCreateTextureFromFileInMemoryEx(
		RGetDevice(), TextureFileBuffer, m_nFileSize, Tex_w, Tex_h,
		m_bUseMipmap ? D3DX_DEFAULT : 1, 0, d3dformat, pool,
		D3DX_FILTER_TRIANGLE | D3DX_FILTER_MIRROR,
		D3DX_FILTER_TRIANGLE | D3DX_FILTER_MIRROR,
		0, &m_Info, NULL, MakeWriteProxy(m_pTex))))
	{
		_RPT1(_CRT_WARN, "%s ---->> memory texture (re)create failure \n", m_szTextureName);
		__EP(2011);
		return false;
	}

	__EP(2011)

	return true;
#endif
}

bool RBaseTexture::OnRestore(bool bManaged)
{
	auto RBaseTextureOnRestore = MBeginProfile("RBaseTexture::OnRestore");

	assert(!m_pTex);

	MZFile mzf;

	m_bManaged = false;

	auto Open = MBeginProfile("RBaseTexture::OnRestore - MZFile::Open");

	MZFileSystem *pfs = m_bUseFileSystem ? g_pFileSystem : nullptr;

	// TODO: Fix this silly stuff. Add .dds to the filenames
	// in the xmls if there is actually one available
#ifdef LOAD_FROM_DDS
	char ddstexturefile[MAX_PATH];
	sprintf_safe(ddstexturefile, "%s.dds", m_szTextureName);
	if (!mzf.Open(ddstexturefile, pfs))
#endif
		if (!mzf.Open(m_szTextureName, pfs))
			return false;

	auto Read = MBeginProfile("RBaseTexture::OnRestore - MZFile::Read");
	m_nFileSize = mzf.GetLength();
	auto TextureFileBuffer = mzf.Release();
	if (!TextureFileBuffer.get())
	{
		MLog("RBaseTexture::OnRestore - MZFile::Release on %s failed\n", m_szTextureName);
		return false;
	}
	MEndProfile(Read);

	auto ret = SubCreateTexture(TextureFileBuffer.get());

	TextureFileBuffer.Destroy();

	MEndProfile(RBaseTextureOnRestore);

	return ret;
}

static RTextureManager* g_pTextureManager = NULL;

void RBaseTexture_Create()
{
	g_pTextureManager = new RTextureManager;
}

void RBaseTexture_Destroy()
{
	SAFE_DELETE(g_pTextureManager);
}

RTextureManager* RGetTextureManager() {
	return g_pTextureManager;
}

RTextureManager::~RTextureManager() {
	Destroy();
	g_pTextureManager = NULL;
}

void RTextureManager::Destroy() {
	while (size())
	{
		RBaseTexture *pTex = begin()->second;
		if (pTex->m_nRefCount > 0)
		{
			_RPT2(_CRT_WARN, " %s texture not destroyed. ( ref count = %d )\n",
				pTex->m_szTextureName, pTex->m_nRefCount);
		}
		delete pTex;
		erase(begin());
	}
}

void RTextureManager::OnInvalidate() {

#ifndef _MANAGED
	for (iterator i = begin(); i != end(); i++)
	{
		_RPT1(_CRT_WARN, "%s\n", i->second->m_szTextureName);
		i->second->OnInvalidate();
	}
#endif

};

int RTextureManager::UpdateTexture(u32 max_life_time)
{
	RBaseTexture* pTex = NULL;

	u32 this_time = GetGlobalTimeMS();
	int cnt = 0;

	for (iterator i = begin(); i != end(); i++) {
		pTex = i->second;
		if (pTex && pTex->m_bManaged && pTex->m_pTex) {
			if (pTex->m_dwLastUseTime + max_life_time < this_time) {
				pTex->OnInvalidate();
			}
			cnt++;
		}
	}

	return cnt;
}

void RTextureManager::OnChangeTextureLevel(u32 flag)
{
	RBaseTexture* pTex = NULL;

	for (iterator i = begin(); i != end(); i++) {
		pTex = i->second;
		if (pTex) {
			if (flag == RTextureType_All || (flag & pTex->m_nTexType)) {

				if (pTex->m_pTex) {
					pTex->OnInvalidate();
					pTex->OnRestore(pTex->m_bManaged);
				}
			}
		}
	}
}

int RTextureManager::PrintUsedTexture()
{
	RBaseTexture* pTex = NULL;

	int cnt = 0;
	int nUse = 0;

	for (iterator i = begin(); i != end(); i++) {

		pTex = i->second;
		nUse = 0;

		if (pTex) {

			if (pTex->m_pTex)
				nUse = 1;

			mlog("texture : %s Used %d RefCnt %d \n", pTex->m_szTextureName, nUse, pTex->m_nRefCount);
			cnt++;
		}
	}
	return cnt;
}

int RTextureManager::CalcUsedCount()
{
	int nCount = 0;

	RBaseTexture* pTex = NULL;
	for (iterator i = begin(); i != end(); i++) {
		pTex = i->second;
		if (pTex && pTex->m_pTex) {
			nCount++;
		}
	}
	return nCount;
}

static int Floorer2PowerSize(int v)
{
	if (v <= 2)			return 2;
	else if (v <= 4)	return 4;
	else if (v <= 8)	return 8;
	else if (v <= 16)	return 16;
	else if (v <= 32)	return 32;
	else if (v <= 64)	return 64;
	else if (v <= 128)	return 128;
	else if (v <= 256)	return 256;
	else if (v <= 512)	return 512;
	else if (v <= 1024)	return 1024;
	else if (v <= 2048)	return 2048;
	else if (v <= 4096)	return 4096;

	assert(FALSE);	// Too Big!

	return 2;
}

int RTextureManager::CalcUsedSize()
{
	int return_size = 0;

	RBaseTexture* pTex = NULL;

	int add_size = 0;

	for (iterator i = begin(); i != end(); i++) {
		pTex = i->second;
		if (pTex && pTex->m_pTex) {

			add_size = 0;

			add_size = Floorer2PowerSize(pTex->GetWidth()) / (1 << pTex->GetTexLevel()) *
				Floorer2PowerSize(pTex->GetHeight()) / (1 << pTex->GetTexLevel()) *
				(g_nTextureFormat == 0 ? 2 : 4);

			return_size += add_size;

		}
	}

	return return_size;
}

void RTextureManager::OnRestore() {
#ifndef _MANAGED
	for (iterator i = begin(); i != end(); i++)
	{
		i->second->OnRestore();
	}
#endif
};


RBaseTexture *RTextureManager::CreateBaseTexture(const char* filename, int tex_type, bool bUseMipmap,
	bool bUseFileSystem)
{
	return CreateBaseTextureSub(false, filename, tex_type, bUseMipmap, bUseFileSystem);
}

RBaseTexture *RTextureManager::CreateBaseTextureMg(const char* filename, int tex_type, bool bUseMipmap,
	bool bUseFileSystem)
{
	return CreateBaseTextureSub(true, filename, tex_type, bUseMipmap, bUseFileSystem);
}

RBaseTexture *RTextureManager::CreateBaseTextureSub(bool Managed, const char* filename,
	int tex_type, bool bUseMipmap, bool bUseFileSystem)
{
	if (filename == NULL || strlen(filename) == 0) return NULL;

	char texturefilename[256];

	strcpy_safe(texturefilename, filename);
	_strlwr_s(texturefilename);

	iterator i = find(texturefilename);

	if (i != end())
	{
		i->second->m_nRefCount++;
		return i->second;
	}

	RBaseTexture *pnew = new RBaseTexture;
	pnew->m_bUseFileSystem = bUseFileSystem;
	pnew->m_nRefCount = 1;
	pnew->m_bUseMipmap = bUseMipmap;
	pnew->m_nTexType = tex_type;
	pnew->m_nTexLevel = SubGetTexLevel(tex_type);

	strcpy_safe(pnew->m_szTextureName, texturefilename);

#ifndef _PUBLISH
	if (Managed == false) {
		if (!pnew->OnRestore(Managed)) {
			delete pnew;
			return NULL;
		}
	}
#else 
	if (!pnew->OnRestore(Mg))
	{
		delete pnew;
		return NULL;
	}
#endif

	insert(value_type(texturefilename, pnew));

	return pnew;
}

void RTextureManager::DestroyBaseTexture(RBaseTexture* pTex)
{
	DestroyBaseTexture(pTex->m_szTextureName);
}

void RTextureManager::DestroyBaseTexture(const char* szName)
{
	char texturefilename[256];

	strcpy_safe(texturefilename, szName);
	_strlwr_s(texturefilename);

	iterator i = find(texturefilename);

	if (i != end()) {
		RBaseTexture *pTTex = i->second;
		pTTex->m_nRefCount--;
		if (pTTex->m_nRefCount == 0)
		{
			delete pTTex;
			erase(i);
		}
	}
}

RBaseTexture* RCreateBaseTexture(const char* filename, DWORD tex_type, bool bUseMipmap, bool bUseFileSystem)
{
	return g_pTextureManager->CreateBaseTexture(filename, tex_type, bUseMipmap, bUseFileSystem);
}

RBaseTexture* RCreateBaseTextureMg(const char* filename, DWORD tex_type, bool bUseMipmap, bool bUseFileSystem)
{
	return g_pTextureManager->CreateBaseTextureMg(filename, tex_type, bUseMipmap, bUseFileSystem);
}

void RDestroyBaseTexture(RBaseTexture *pTex)
{
	if (g_pTextureManager == NULL || pTex == NULL) return;
	g_pTextureManager->DestroyBaseTexture(pTex);
}

void RBaseTexture_Invalidate()
{
	_RPT0(_CRT_WARN, "begin invalidate\n");
	g_pTextureManager->OnInvalidate();
	_RPT0(_CRT_WARN, "invalidate complete\n");
}

void RBaseTexture_Restore()
{
	_RPT0(_CRT_WARN, "begin restore\n");
	g_pTextureManager->OnRestore();
	_RPT0(_CRT_WARN, "restore complete\n");
}

void RChangeBaseTextureLevel(DWORD flag)
{
	_RPT0(_CRT_WARN, "begin change texture level\n");
	g_pTextureManager->OnChangeTextureLevel(flag);
	_RPT0(_CRT_WARN, "change texture level complete\n");
}

_NAMESPACE_REALSPACE2_END