#include "stdafx.h"
//#include "ZConfiguration.h"

#include <d3d9.h>

#include "RShaderMgr.h"
#include "RealSpace2.h"
#include "rmtrl.h"
#include "rmeshutil.h"
#include "MDebug.h"

RShaderMgr RShaderMgr::msInstance;
//bool RShaderMgr::shader_enabled = true;		// 지원 여부
bool RShaderMgr::mbUsingShader	= true;		// 사용 여부
RMtrl* RShaderMgr::mpMtrl = 0;
D3DLIGHT9 RShaderMgr::mLight[MAX_LIGHT];

RShaderMgr* RGetShaderMgr()
{
	return &RShaderMgr::msInstance;
}

RShaderMgr::RShaderMgr()
{
	if( mpMtrl == 0 )
	{
		mpMtrl = new RMtrl;
	}
	mbNeedUpdate[0] = false;
	mbNeedUpdate[1] = false;
	mbNeedUpdate[2] = false;

	mbLight[0]	= false;
	mbLight[1]	= false;
	mbUsingShader	= false;
}

RShaderMgr::~RShaderMgr()
{
	Release();
	SAFE_DELETE( mpMtrl );
}

bool RShaderMgr::Initialize()
{
	if( !RIsSupportVS() ) return false;
	
	if( m_ShaderVec.size() ==  0 )
	{
		HRESULT hr;
		LPDIRECT3DVERTEXDECLARATION9 _pVShaderDecl	= NULL;

		D3DVERTEXELEMENT9 decl[] = 
		{
			{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,		0 },
			{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT,	0 },
			{ 0, 20, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES,	0 },
			{ 0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0 },
			{ 0, 44, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,		0 },

			D3DDECL_END()
		};

/*
		D3DVERTEXELEMENT9 decl[] = 
		{
			{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,		0 },
			{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT,	0 },
			{ 0, 20, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES,	0 },
			{ 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0 },
			{ 0, 36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,		0 },

			D3DDECL_END()
		};
*/
		if(FAILED( hr=RGetDevice()->CreateVertexDeclaration(decl,&_pVShaderDecl) ) )
			return false;

		m_ShaderDeclVec.push_back(_pVShaderDecl);


//		if( !RShaderMgr::getShaderMgr()->addVertexShaderFromFile( "shader/skin.vsh" ))
		if( !RShaderMgr::getShaderMgr()->addVertexShaderFromResource( "shader/skin.vso" ))
		{
			_ASSERT(!"Shader Compile Error");			
			mlog("Shader Compile Error");
			mbUsingShader	= false;
			return false;
		}
	}
	
	mbUsingShader	= false;

	mlog("shader initialize successful : %d\n", m_ShaderVec.size() );
	return	true;
}

void RShaderMgr::Release()
{

	for( size_t i = 0 ; i < m_ShaderVec.size(); ++i )
	{
		SAFE_RELEASE(m_ShaderVec[i]);
	}

	for( i = 0 ; i < m_ShaderDeclVec.size(); ++i )
	{
		SAFE_RELEASE(m_ShaderDeclVec[i]);
	}

	m_ShaderVec.clear();
	m_ShaderDeclVec.clear();

//	mlog("shader destroy successful : %d\n", test );
}
bool RShaderMgr::addVertexShaderFromFile( char* fileName_ )
{
	HRESULT hr;
	LPD3DXBUFFER pCode;	

	LPDIRECT3DVERTEXSHADER9	_pVShader = NULL;

	if(FAILED( hr = D3DXAssembleShaderFromFile( fileName_,NULL,NULL,0,&pCode,NULL ) ) )
//	if(FAILED( hr = D3DXAssembleShaderFromFile( fileName_,NULL,NULL,D3DXSHADER_DEBUG,&pCode,NULL ) ) )
		return false;

	LPDIRECT3DDEVICE9 dev = RealSpace2::RGetDevice();

	hr = dev->CreateVertexShader((DWORD*)pCode->GetBufferPointer(),&_pVShader);

	SAFE_RELEASE(pCode);

	m_ShaderVec.push_back( _pVShader );

	if( m_ShaderVec.size() == 1 )
	{
		init();
	}

	return true;
}

bool RShaderMgr::addVertexShader( const char* shader_ )
{
/*
	DWORD handle;
	LPD3DXBUFFER Shader; 
	LPD3DXBUFFER Error;

	if( FAILED( D3DXAssembleShader( shader_, sizeof(shader_) -1, 0, NULL, &Shader, &Error )))
	{
		OutputDebugString((char*)Error->GetBufferPointer());
		return false;
	}

	HRESULT hr;
	hr = RealSpace2::RGetDevice()->CreateVertexShader( Decl_, (DWORD*)Shader->GetBufferPointer(), &handle, 0 );
	if(FAILED( hr ))
	{
		char szBuffer[100];
		OutputDebugString( "Vertex Shader Create Error\n" );
//		D3DXGetErrorString( hr, szBuffer, sizeof(szBuffer) );
		OutputDebugString( szBuffer );
		OutputDebugString( "\n" );
		return false;
	}

	Shader->Release();

	m_ShaderVec.push_back( handle );

	if( m_ShaderVec.size() == 1 )
	{
		init();
	}
*/	
	return true;
	
}

bool RShaderMgr::addVertexShaderFromResource( char* Resource_ )
{
	HRESULT hr;

//	DWORD handle;
	HANDLE hMap;
	HANDLE file = CreateFile( Resource_, GENERIC_READ,0,0,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,0);

	if(file != INVALID_HANDLE_VALUE) 
	{
		if(GetFileSize(file,0) > 0) 
			hMap = CreateFileMapping(file,0,PAGE_READONLY,0,0,0);
		else
		{
			CloseHandle(file);
			return false;	 	   
		}
	}	  
	else
		return false;

	DWORD* dwpVS = (DWORD *)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);

	LPDIRECT3DVERTEXSHADER9	_pVShader = NULL;

	LPDIRECT3DDEVICE9 dev = RealSpace2::RGetDevice();

	hr = dev->CreateVertexShader(dwpVS,&_pVShader);


	m_ShaderVec.push_back( _pVShader );

	if( m_ShaderVec.size() == 1 )
	{
		init();
	}

	UnmapViewOfFile(dwpVS);
	CloseHandle(hMap);
	CloseHandle(file);

	return true;
}

bool RShaderMgr::addPixelShader( char* fileName_ )
{
	return true;
}

LPDIRECT3DVERTEXSHADER9 RShaderMgr::getShader( int i_  )
{
	if( i_ >= (int)m_ShaderVec.size() )
	{
		mlog( " Shader Critical Error \n " );
	}
	return m_ShaderVec[i_];
}

LPDIRECT3DVERTEXDECLARATION9 RShaderMgr::getShaderDecl(int i)
{
	if( i >= (int)m_ShaderDeclVec.size() )
	{
		mlog( " ShaderDecl Critical Error \n " );
	}
	return m_ShaderDeclVec[i];
}

/*
DWORD RShaderMgr::getShader( char* shader_ , bool& stat )
{
	stdMapShaderHandle::iterator iter;
	iter = m_ShaderMap.find( string(shader_) );
	if( iter == m_ShaderMap.end() )
	{
		stat = false;
		return 0;
	}
	
	stat = true;

    return (*iter).second;
}
*/
void RShaderMgr::setMtrl( RMtrl* pmtrl_, float fVisAlpha_ )
{
	if(!mbUsingShader) return;

	if(!pmtrl_) return;

	mpMtrl->m_diffuse.r = 0.5f;
	mpMtrl->m_diffuse.g = 0.5f;
	mpMtrl->m_diffuse.b = 0.5f;
	mpMtrl->m_diffuse.a = fVisAlpha_;

	mpMtrl->m_ambient.r = 0.35f;
	mpMtrl->m_ambient.g = 0.35f;
	mpMtrl->m_ambient.b = 0.35f;
	mpMtrl->m_ambient.a = 1.0f;
	
	mpMtrl->m_specular.r = 1.0f;
	mpMtrl->m_specular.g = 1.0f;
	mpMtrl->m_specular.b = 1.0f;
	pmtrl_->m_specular.a	= 1.0f - fVisAlpha_;

	//TODO : 현재 쉐이더에서는 스펙큘러 라이팅에 대한 연산이 없다..ㅎㅎㅎ
	
	if(fVisAlpha_ != 1.f) 
	{
/*		mpMtrl->m_diffuse.a	= fVisAlpha_;
		mpMtrl->m_ambient.a	= fVisAlpha_;
		mpMtrl->m_specular.a = fVisAlpha_;//*/
	}

	mbNeedUpdate[2] = true;
}

void RShaderMgr::setMtrl( D3DXCOLOR& rColor_, float fVisAlpha_ )
{
	if(!mbUsingShader) return;

	mpMtrl->m_diffuse.r = rColor_.r;
	mpMtrl->m_diffuse.g = rColor_.g;
	mpMtrl->m_diffuse.b = rColor_.b;
	mpMtrl->m_diffuse.a = rColor_.a;

	mpMtrl->m_ambient.r = rColor_.r*0.2f;
	mpMtrl->m_ambient.g = rColor_.g*0.2f;
	mpMtrl->m_ambient.b = rColor_.b*0.2f;
	mpMtrl->m_ambient.a = 1.0f;

	mpMtrl->m_specular.r = 1.f;
	mpMtrl->m_specular.g = 1.f;
	mpMtrl->m_specular.b = 1.f;
	mpMtrl->m_specular.a = 1.f;;

	if(fVisAlpha_ != 1.f) 
	{
/*		mpMtrl->m_diffuse.a	= fVisAlpha_;
		mpMtrl->m_ambient.a	= fVisAlpha_;
		mpMtrl->m_specular.a = fVisAlpha_;
		//*/
	}

	mbNeedUpdate[2] = true;
}

void RShaderMgr::setLight( int iLignt_, D3DLIGHT9* pLight_ )
{
	if(!mbUsingShader) return;
	if(iLignt_>=MAX_LIGHT) return;

	mLight[iLignt_].Ambient = pLight_->Ambient;
	mLight[iLignt_].Diffuse = pLight_->Diffuse;
	mLight[iLignt_].Specular = pLight_->Specular;
	mLight[iLignt_].Range = pLight_->Range;
	mLight[iLignt_].Attenuation0 = pLight_->Attenuation0;
	mLight[iLignt_].Attenuation1 = pLight_->Attenuation1;
	mLight[iLignt_].Attenuation2 = pLight_->Attenuation2;
	mLight[iLignt_].Position = pLight_->Position;

	mbNeedUpdate[iLignt_] = true;
}

void RShaderMgr::setAmbient( DWORD value_  )
{
	if(!mbUsingShader) return;

	float global_ambient[4];
	global_ambient[3] = 0.f;
	global_ambient[0] = (( value_ & 0x00ff0000 ) >> 16 ) / (float)0xff ;
	global_ambient[1] = (( value_ & 0x0000ff00 ) >> 8 ) / (float)0xff ;
	global_ambient[2] = (( value_ & 0x000000ff ) ) / (float)0xff ;

	RGetDevice()->SetVertexShaderConstantF( GLOBAL_AMBIENT, (float*)&global_ambient, 1 );
}

void RShaderMgr::Update()
{
	LPDIRECT3DDEVICE9 dev = RealSpace2::RGetDevice();
	// update light

	D3DCOLORVALUE ambient;
	ambient.a = 0.f;
	ambient.b = 1.f;
	ambient.g = 1.f;
	ambient.r = 1.f;
	dev->SetVertexShaderConstantF( MATERIAL_SPECULAR, (float*)&ambient, 1);

	// Character Light 0
	//if( mbNeedUpdate[0] )
	{
		if( /*mbNeedUpdate[0] &&*/ mbLight[0] )
		{
			D3DXVECTOR3 attenuation = D3DXVECTOR3( mLight[0].Attenuation0, mLight[0].Attenuation1, mLight[0].Attenuation2 );
			dev->SetVertexShaderConstantF( LIGHT_ATTENUATION, (float*)&attenuation, 1);

			dev->SetVertexShaderConstantF( LIGHT0_POSITION, (float*)&mLight[0].Position, 1);
			dev->SetVertexShaderConstantF( LIGHT0_AMBIENT, (float*)&mLight[0].Ambient, 1);
			dev->SetVertexShaderConstantF( LIGHT0_DIFFUSE, (float*)&mLight[0].Diffuse, 1);
			dev->SetVertexShaderConstantF( LIGHT0_SPECULAR, (float*)&mLight[0].Specular, 1);
			dev->SetVertexShaderConstantF( LIGHT0_RANGE, (float*)&mLight[0].Range, 1);
			mbNeedUpdate[0] = false;
		}
		else
		{
			D3DXVECTOR4 temp = D3DXVECTOR4( 0,0,0,0 );
			dev->SetVertexShaderConstantF( LIGHT0_POSITION, (float*)&temp, 1 );
			dev->SetVertexShaderConstantF( LIGHT0_AMBIENT, (float*)&temp, 1 );
			dev->SetVertexShaderConstantF( LIGHT0_DIFFUSE, (float*)&temp, 1 );
			dev->SetVertexShaderConstantF( LIGHT0_SPECULAR, (float*)&temp, 1 );
			dev->SetVertexShaderConstantF( LIGHT0_RANGE, (float*)&temp, 1 );
			dev->SetVertexShaderConstantF( LIGHT_ATTENUATION, (float*)&temp, 1 );
		}

	}
	
	// Character Light 1
	//if( mbNeedUpdate[1] )
	{
		if( /*mbNeedUpdate[1]*/  mbLight[1] )
		{
			D3DXVECTOR3 attenuation = D3DXVECTOR3( mLight[1].Attenuation0, mLight[1].Attenuation1, mLight[1].Attenuation2 );
			dev->SetVertexShaderConstantF( LIGHT_ATTENUATION1, (float*)&attenuation, 1);

			dev->SetVertexShaderConstantF( LIGHT1_POSITION, (float*)&mLight[1].Position, 1);
			dev->SetVertexShaderConstantF( LIGHT1_AMBIENT, (float*)&mLight[1].Ambient, 1);
			dev->SetVertexShaderConstantF( LIGHT1_DIFFUSE, (float*)&mLight[1].Diffuse, 1);
			dev->SetVertexShaderConstantF( LIGHT1_SPECULAR, (float*)&mLight[1].Specular, 1);
			dev->SetVertexShaderConstantF( LIGHT1_RANGE, (float*)&mLight[1].Range, 1);
			mbNeedUpdate[1] = false;
		}
		else
		{
			D3DXVECTOR4 temp = D3DXVECTOR4( 0,0,0,0 );
			dev->SetVertexShaderConstantF( LIGHT1_POSITION, (float*)&temp, 1 );
			dev->SetVertexShaderConstantF( LIGHT1_AMBIENT, (float*)&temp, 1 );
			dev->SetVertexShaderConstantF( LIGHT1_DIFFUSE, (float*)&temp, 1 );
			dev->SetVertexShaderConstantF( LIGHT1_SPECULAR, (float*)&temp, 1 );
			dev->SetVertexShaderConstantF( LIGHT1_RANGE, (float*)&temp, 1 );
			dev->SetVertexShaderConstantF( LIGHT_ATTENUATION1, (float*)&temp, 1 );
		}
	}
	
	//	Material
	//D3DXCOLOR ambient = 0x00ffffff;
	//dev->SetVertexShaderConstantF( MATERIAL_AMBIENT, (float*)&ambient, 1 );
	//if( mbNeedUpdate[2] )
	{
		// update material
		dev->SetVertexShaderConstantF( MATERIAL_AMBIENT, (float*)&mpMtrl->m_ambient, 1 );
		dev->SetVertexShaderConstantF( MATERIAL_DIFFUSE, (float*)&mpMtrl->m_diffuse, 1 );
		dev->SetVertexShaderConstantF( MATERIAL_SPECULAR, (float*)&mpMtrl->m_specular, 1 );
		dev->SetVertexShaderConstantF( MATERIAL_POWER, (float*)&mpMtrl->m_power, 1 );
		mbNeedUpdate[2] = false;
	}
}

void RShaderMgr::init()
{
	LPDIRECT3DDEVICE9 dev = RealSpace2::RGetDevice();

	D3DXVECTOR4 constv = D3DXVECTOR4( 0, 0, 0, 0 );
	D3DXVECTOR4 constvatten = D3DXVECTOR4( 0.1, 0.1, 0.1, 0.1 );

	float fConst[] = {
		1.0f, -1.0f, 0.5f, 255.f
	};

	dev->SetVertexShaderConstantF( CONSTANTS, fConst, 1);

	//shader constant register initialize
	dev->SetVertexShaderConstantF( LIGHT_ATTENUATION, (float*)&constvatten, 1);
	dev->SetVertexShaderConstantF( LIGHT0_POSITION, (float*)&constv, 1);
	dev->SetVertexShaderConstantF( LIGHT0_AMBIENT, (float*)&constv, 1);
	dev->SetVertexShaderConstantF( LIGHT0_DIFFUSE, (float*)&constv, 1);
	dev->SetVertexShaderConstantF( LIGHT0_SPECULAR, (float*)&constv, 1);
	dev->SetVertexShaderConstantF( LIGHT0_RANGE, (float*)&constv, 1);

	dev->SetVertexShaderConstantF( LIGHT_ATTENUATION1, (float*)&constvatten, 1);
	dev->SetVertexShaderConstantF( LIGHT1_POSITION, (float*)&constv, 1);
	dev->SetVertexShaderConstantF( LIGHT1_AMBIENT, (float*)&constv, 1);
	dev->SetVertexShaderConstantF( LIGHT1_DIFFUSE, (float*)&constv, 1);
	dev->SetVertexShaderConstantF( LIGHT1_SPECULAR, (float*)&constv, 1);
	dev->SetVertexShaderConstantF( LIGHT1_RANGE, (float*)&constv, 1);

	// update material
	dev->SetVertexShaderConstantF( MATERIAL_AMBIENT, (float*)&constv, 1 );
	dev->SetVertexShaderConstantF( MATERIAL_DIFFUSE, (float*)&constv, 1 );
	//dev->SetVertexShaderConstantF( MATERIAL_SPECULAR, (float*)&constv, 1 );
	dev->SetVertexShaderConstantF( MATERIAL_POWER, (float*)&constv, 1 );
}

RShaderMgr* RShaderMgr::getShaderMgr()
{
	return &msInstance;
}

//////////////////////////////////////////////////////////////////////////
//	LightEnable ( on/off )
//////////////////////////////////////////////////////////////////////////
void RShaderMgr::LightEnable( int iLignt_, bool bEnable_ )
{
	if(iLignt_>=MAX_LIGHT) return;

	mbLight[iLignt_] = bEnable_;
}

void RShaderMgr::SetDisable()
{
	mbUsingShader = false;
	Release();
}

bool RShaderMgr::SetEnable()
{
	//if( !shader_enabled )
	//{
	//	return false;
	//}

	if( Initialize() )
	{
		mbUsingShader = true;
		return true;
	}
	else
	{
		mbUsingShader	= false;
		//shader_enabled	= false;
		return false;
	}
}