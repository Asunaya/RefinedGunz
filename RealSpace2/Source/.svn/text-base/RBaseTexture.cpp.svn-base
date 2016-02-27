#include "stdafx.h"
#include "RealSpace2.h"
#include "RBaseTexture.h"
#include "MDebug.h"
#include "MZFileSystem.h"

using namespace std;
_USING_NAMESPACE_REALSPACE2

_NAMESPACE_REALSPACE2_BEGIN

///////////////////////////////////////////////////////////////////////

static int g_nObjectTextureLevel = 0;
static int g_nMapTextureLevel = 0;
static int g_nTextureFormat = 0;	// 0 = 16 bit , 1 = 32bit

void SetObjectTextureLevel(int nLevel)
{
	g_nObjectTextureLevel = nLevel;
}

void SetMapTextureLevel(int nLevel)
{
	g_nMapTextureLevel = nLevel;
}

void SetTextureFormat(int nLevel)	// 0 = 16 bit , 1 = 32bit
{
	g_nTextureFormat = nLevel;
}

int GetObjectTextureLevel() 
{
	return g_nObjectTextureLevel;
}

int GetMapTextureLevel() 
{
	return g_nMapTextureLevel;
}

int GetTextureFormat()
{
	return g_nTextureFormat;
}

int SubGetTexLevel(DWORD tex_type)
{
	int hr = 0;

	if(tex_type == RTextureType_Etc) {

		hr = 0;

	} else if(tex_type == RTextureType_Map ) {

		hr = GetMapTextureLevel();

	} else if(tex_type == RTextureType_Object ) {

		hr = GetObjectTextureLevel();
	}

	return hr;
}

///////////////////////////////////////////////////////////////////////

#define _MANAGED


RBaseTexture::RBaseTexture()
{
	m_nRefCount=0;
	memset(&m_Info,0,sizeof(D3DXIMAGE_INFO));
	m_pTex = NULL;
	m_szTextureName[0]=0;
	m_nFileSize = 0;
	m_pTextureFileBuffer = NULL;
	m_bManaged = false;
	m_nTexLevel = 0;
	m_nTexType = RTextureType_Etc;
	m_bUseFileSystem = true;
}

RBaseTexture::~RBaseTexture()
{
	Destroy();
}

void RBaseTexture::Destroy()
{
	OnInvalidate();
	SAFE_DELETE_ARRAY(m_pTextureFileBuffer);
}

void RBaseTexture::OnInvalidate()
{
	SAFE_RELEASE(m_pTex);
}

#define LOAD_FROM_DDS


LPDIRECT3DTEXTURE9	RBaseTexture::GetTexture()
{
	m_dwLastUseTime = timeGetTime();//마지막으로 사용된 시간

	if(m_pTex) {
		return m_pTex;
	}

#ifndef _PUBLISH

	else {//없다면..
	
		OnInvalidate();
		OnRestore();
	}

#endif

/*
	if( m_bManaged && m_pTextureFileBuffer ) {

		if(SubCreateTexture()==false) {

			delete [] m_pTextureFileBuffer;
			m_pTextureFileBuffer = NULL;

			return NULL;
		}
		else {
			delete [] m_pTextureFileBuffer;
			m_pTextureFileBuffer = NULL;
		}
	}
*/
	return m_pTex;
}

//////////////////////////////////////////////////////////////////////////

//#define	LOAD_TEST

#ifdef	LOAD_TEST

#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);

#else

#define __BP(i,n) ;
#define __EP(i) ;

#endif

///////////////////////////////////////////////////////////////////////////

BOOL IsCompressedTextureFormatOk( D3DFORMAT TextureFormat );

bool RBaseTexture::SubCreateTexture()
{
	const D3DPOOL pool = 
#ifndef _MANAGED
		D3DPOOL_DEFAULT;
#else
		D3DPOOL_MANAGED;
#endif

	UINT Tex_w = D3DX_DEFAULT;
	UINT Tex_h = D3DX_DEFAULT;

	m_nTexLevel = SubGetTexLevel( m_nTexType );

	__BP(2011,"RBaseTexture::SubCreateTexture");

	if( FAILED( D3DXGetImageInfoFromFileInMemory( m_pTextureFileBuffer,m_nFileSize,&m_Info) ) ) 
	{
		_RPT1(_CRT_WARN,"%s ---->> get image info failure \n",m_szTextureName);
		__EP(2011);
		return false;
	}

	if( m_nTexLevel ) { // 옵션정도에 따라~  

//		기본사이즈 : 0
//		1/2 사이즈 : 1
//		1/4 사이즈 : 2
//		1/8 사이즈 : 4 <- HardwareTNL을 지원하지 않는 그래픽 카드일 경우

		/*
		if( FAILED( D3DXGetImageInfoFromFileInMemory( m_pTextureFileBuffer,m_nFileSize,&m_Info) ) ) {
			_RPT1(_CRT_WARN,"%s ---->> memory texture (re)create failure \n",m_szTextureName);

			__EP(2011);

			return false;
		}
		else {
			Tex_w = m_Info.Width  / (m_nTexLevel*2);
			Tex_h = m_Info.Height / (m_nTexLevel*2);
		}
		*/

		Tex_w = m_Info.Width  / (1 << m_nTexLevel);
		Tex_h = m_Info.Height / (1 << m_nTexLevel);
	}

	D3DFORMAT d3dformat = D3DFMT_UNKNOWN;

	if(g_nTextureFormat==0) { // 16비트이면 포맷을 적절히 변환해준다
		switch(m_Info.Format) {
			case D3DFMT_A8R8G8B8 :
				d3dformat =	 D3DFMT_A4R4G4B4;
				break;			
			case D3DFMT_X8R8G8B8 :
			case D3DFMT_R8G8B8 :
				d3dformat =	 D3DFMT_R5G6B5;
				break;
			case D3DFMT_DXT1 :
			case D3DFMT_DXT2 :
			case D3DFMT_DXT3 :
			case D3DFMT_DXT4 :
			case D3DFMT_DXT5 :
				d3dformat =	 m_Info.Format;
				break;
			default :
				_ASSERT(FALSE);
		}
	}else
		d3dformat = m_Info.Format;

	// 압축텍스쳐인경우 지원하지 않으면
	
	if( d3dformat==D3DFMT_DXT1 || d3dformat==D3DFMT_DXT2 || d3dformat==D3DFMT_DXT3 ||
		d3dformat==D3DFMT_DXT4 || d3dformat==D3DFMT_DXT5) 
	{
		if(IsCompressedTextureFormatOk(m_Info.Format)==FALSE) {
			// 너무도 저사양이니까 무조건 16bit		
			if( m_Info.Format == D3DFMT_DXT1 ) {
				d3dformat = D3DFMT_R5G6B5;
			}
			else //alpha
				d3dformat = D3DFMT_A4R4G4B4;
		}
	}

	if( FAILED(D3DXCreateTextureFromFileInMemoryEx(
		RGetDevice(), m_pTextureFileBuffer ,m_nFileSize, Tex_w, Tex_h,
		m_bUseMipmap ? D3DX_DEFAULT : 1, 0, d3dformat,pool,
		D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
		D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
		0, &m_Info, NULL, &m_pTex ))) {
			_RPT1(_CRT_WARN,"%s ---->> memory texture (re)create failure \n",m_szTextureName);

			__EP(2011);

			return false;
		}

	__EP(2011);

	return true;
}

bool RBaseTexture::OnRestore(bool bManaged)
{
	__BP(2010,"RBaseTexture::OnRestore");

	_ASSERT(!m_pTex);

	MZFile mzf;

	m_bManaged = bManaged;
	m_bManaged = false;//우선 예전 방식으로 돌아가자

	if(m_pTextureFileBuffer == NULL) {

	__BP(2012,"RBaseTexture::mzf.Open");

	MZFileSystem *pfs = m_bUseFileSystem ? g_pFileSystem : NULL;

#ifdef LOAD_FROM_DDS
	char ddstexturefile[MAX_PATH];
	sprintf(ddstexturefile,"%s.dds",m_szTextureName);
	if(!mzf.Open(ddstexturefile,pfs))
#endif
	{
		if(!mzf.Open(m_szTextureName,pfs)) {
			if(!mzf.Open(m_szTextureName)) {
				__EP(2012);
				__EP(2010);
				return NULL;
			}
		}
	}

	__BP(2013,"RBaseTexture::mzf.Open : new ");
	m_nFileSize = mzf.GetLength();
	m_pTextureFileBuffer = new char[mzf.GetLength()];
	__EP(2013);

	__BP(2014,"RBaseTexture::mzf.Read ");

	if(!mzf.Read( m_pTextureFileBuffer ,mzf.GetLength() ))
		return false;

	__EP(2014);

	}

	__EP(2012);

//	if(m_bManaged==false) {

		if(SubCreateTexture()==false) {

			mzf.Close();

			if(m_pTextureFileBuffer) {
				delete [] m_pTextureFileBuffer;
				m_pTextureFileBuffer = NULL;
			}
			__EP(2010);

			return false;
		}
		// 나중 m_bManaged 를 위해가지고 있어야하는가?
//	}

	if(m_pTextureFileBuffer) {
		delete [] m_pTextureFileBuffer;
		m_pTextureFileBuffer = NULL;
	}

	mzf.Close();

	__EP(2010);

	return true;
}

// RTextureManager   나중에 커지면 파일을 분리한다

RTextureManager *g_pTextureManager=NULL;

void RBaseTexture_Create()
{
	g_pTextureManager=new RTextureManager;
}

void RBaseTexture_Destory()
{
	SAFE_DELETE(g_pTextureManager);
}

RTextureManager* RGetTextureManager() {
	return g_pTextureManager;
}

RTextureManager::~RTextureManager() { 
	Destroy();
	g_pTextureManager=NULL;
}

void RTextureManager::Destroy() {
	while(size())
	{
		RBaseTexture *pTex=begin()->second;
		if(pTex->m_nRefCount>0)
		{
			_RPT2(_CRT_WARN," %s texture not destroyed. ( ref count = %d )\n",pTex->m_szTextureName,pTex->m_nRefCount);
		}
		delete pTex;
		erase(begin());
	}
}

void RTextureManager::OnInvalidate() {

#ifndef _MANAGED
	for(iterator i=begin();i!=end();i++)
	{
		_RPT1(_CRT_WARN,"%s\n",i->second->m_szTextureName);
		i->second->OnInvalidate();
	}
#endif

};

int RTextureManager::UpdateTexture(DWORD max_life_time)
{
	RBaseTexture* pTex = NULL;

	DWORD this_time = timeGetTime();
	int cnt = 0;
/*
	int cur_size = CalcUsedSize();//파일 사이즈 단위
	int vid_size = RGetVidioMemory();

	// 자기 그래픽 카드의 최대량과 비교해서 넘는다면~
	if(cur_size / (float)vid_size < 0.7f)
		return 0;
*/
	// 리스트 하나 만들어서 시간순서대로 얻어내고... 
	// 가장오래된것부터 하나하나 지워가면서 일정사이즈 유지하기..

	for(iterator i=begin();i!=end();i++) {
		pTex = i->second;
		if( pTex && pTex->m_bManaged && pTex->m_pTex ) {
			// 오래된 것 순으로 필요한 만큼만 해제 넣기로 개선하기..
			if( pTex->m_dwLastUseTime + max_life_time < this_time ) {
				pTex->OnInvalidate();
			}
			cnt++;
		}
	}

	return cnt;//사용중인 texture
}

void RTextureManager::OnChangeTextureLevel(DWORD flag)
{
	RBaseTexture* pTex = NULL;

	for(iterator i=begin();i!=end();i++) {
		pTex = i->second;
		if( pTex ) {
			if( flag==RTextureType_All || (flag & pTex->m_nTexType) ) {

				if(pTex->m_pTex) {// 사용중인 텍스쳐만..
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

	for(iterator i=begin();i!=end();i++) {

		pTex = i->second;
		nUse = 0;

		if( pTex ) {

			if(pTex->m_pTex)
				nUse = 1;
			
			mlog("texture : %s Used %d RefCnt %d \n",pTex->m_szTextureName,nUse,pTex->m_nRefCount);
			cnt++;
		}
	}
	return cnt;
}

int RTextureManager::CalcUsedCount()
{
	int nCount=0;

	RBaseTexture* pTex = NULL;
	for(iterator i=begin();i!=end();i++) {
		pTex = i->second;
		if( pTex && pTex->m_pTex ) {
			nCount++;
		}
	}
	return nCount;
}

static int Floorer2PowerSize(int v)
{
	if(v<=2)			return 2;
	else if(v<=4)		return 4;
	else if(v<=8)		return 8;
	else if(v<=16)		return 16;
	else if(v<=32)		return 32;
	else if(v<=64)		return 64;
	else if(v<=128)		return 128;
	else if(v<=256)		return 256;
	else if(v<=512)		return 512;
	else if(v<=1024)	return 1024;

	_ASSERT(FALSE);	// Too Big!

	return 2;
}

int RTextureManager::CalcUsedSize()
{
	int return_size = 0;

	RBaseTexture* pTex = NULL;

	int add_size = 0;

	for(iterator i=begin();i!=end();i++) {
		pTex = i->second;
		if( pTex && pTex->m_pTex ) {

			add_size = 0;

			// 메모리 차지하는 양으로 다시 추정
			
			add_size = Floorer2PowerSize(pTex->GetWidth()) / ( 1 << pTex->GetTexLevel() ) *
						Floorer2PowerSize(pTex->GetHeight()) / ( 1 << pTex->GetTexLevel() ) * 
						(g_nTextureFormat==0 ? 2 : 4) ;

			/*
			//대충추정치

			if(pTex->m_nFileSize) {

				 if(pTex->m_nTexLevel==0) add_size = pTex->m_nFileSize;
			else if(pTex->m_nTexLevel==1) add_size = pTex->m_nFileSize/4;
			else if(pTex->m_nTexLevel==2) add_size = pTex->m_nFileSize/16;
			else if(pTex->m_nTexLevel==3) add_size = pTex->m_nFileSize/64;

			}
			*/

			return_size += add_size;

		}
	}

	return return_size;
}

void RTextureManager::OnRestore() {
#ifndef _MANAGED
	for(iterator i=begin();i!=end();i++)
	{
		i->second->OnRestore();
	}
#endif
};


RBaseTexture *RTextureManager::CreateBaseTexture(const char* filename,int tex_type,bool bUseMipmap,bool bUseFileSystem)
{
	return CreateBaseTextureSub(false,filename, tex_type ,bUseMipmap, bUseFileSystem);
}

RBaseTexture *RTextureManager::CreateBaseTextureMg(const char* filename,int tex_type,bool bUseMipmap,bool bUseFileSystem)
{
	return CreateBaseTextureSub(true,filename, tex_type ,bUseMipmap, bUseFileSystem);
}

RBaseTexture *RTextureManager::CreateBaseTextureSub(bool Mg,const char* filename,int tex_type,bool bUseMipmap,bool bUseFileSystem)
{
	if(filename==NULL || strlen(filename)==0) return NULL;

	char texturefilename[256];

	strcpy(texturefilename,filename);
	strlwr(texturefilename);

	iterator i=find(string(texturefilename));

	if(i!=end())
	{
		i->second->m_nRefCount++;
		return i->second;
	}

	RBaseTexture *pnew=new RBaseTexture;
	pnew->m_bUseFileSystem = bUseFileSystem;
	pnew->m_nRefCount=1;
	pnew->m_bUseMipmap	= bUseMipmap;
	pnew->m_nTexType	= tex_type;
	pnew->m_nTexLevel	= SubGetTexLevel(tex_type);

	strcpy(pnew->m_szTextureName,texturefilename);

#ifndef _PUBLISH

	if(Mg==false) {// .elu 계열...
		if(!pnew->OnRestore(Mg)) {
			delete pnew;
			return NULL;
		}
	}
#else 

	if(!pnew->OnRestore(Mg))
	{
		delete pnew;
		return NULL;
	}

#endif

	insert(value_type(texturefilename,pnew));

	/*
	char ddstexturefile[MAX_PATH];
	sprintf(ddstexturefile,"%s.dds",texturefilename);
	D3DXSaveTextureToFile(ddstexturefile,D3DXIFF_DDS,pnew->GetTexture(),NULL);
	*/

	return pnew;
}

void RTextureManager::DestroyBaseTexture(RBaseTexture* pTex)
{
	DestroyBaseTexture(pTex->m_szTextureName);
}

void RTextureManager::DestroyBaseTexture(char* szName)
{
	char texturefilename[256];

	strcpy(texturefilename,szName);
	strlwr(texturefilename);

	iterator i=find(string(texturefilename));

	if(i!=end()) {
		RBaseTexture *pTTex=i->second;
		pTTex->m_nRefCount--;
		if(pTTex->m_nRefCount==0)
		{
			delete pTTex;
			erase(i);
		}
	}
}

RBaseTexture* RCreateBaseTexture(const char* filename,DWORD tex_type,bool bUseMipmap,bool bUseFileSystem)
{
	return g_pTextureManager->CreateBaseTexture(filename,tex_type,bUseMipmap,bUseFileSystem);
}

RBaseTexture* RCreateBaseTextureMg(const char* filename,DWORD tex_type,bool bUseMipmap,bool bUseFileSystem)
{
	return g_pTextureManager->CreateBaseTextureMg(filename,tex_type,bUseMipmap,bUseFileSystem);
}

void RDestroyBaseTexture(RBaseTexture *pTex)
{
	if(g_pTextureManager==NULL || pTex==NULL) return;
	g_pTextureManager->DestroyBaseTexture(pTex);
}


// framework 에서 불리는 펑션
void RBaseTexture_Invalidate()
{
	_RPT0(_CRT_WARN,"begin invalidate\n");
	g_pTextureManager->OnInvalidate();
	_RPT0(_CRT_WARN,"invalidate complete\n");
}

void RBaseTexture_Restore()
{
	_RPT0(_CRT_WARN,"begin restore\n");
	g_pTextureManager->OnRestore();
	_RPT0(_CRT_WARN,"restore complete\n");
}

void RChangeBaseTextureLevel(DWORD flag)
{
	_RPT0(_CRT_WARN,"begin change texture level\n");
	g_pTextureManager->OnChangeTextureLevel(flag);
	_RPT0(_CRT_WARN,"change texture level complete\n");
}

_NAMESPACE_REALSPACE2_END
