
//////////////////////////////////////////////////////////////////////////
//	Includes
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "MXml.h"
#include "MDebug.h"
#include "RealSpace2.h"
#include "RMeshUtil.h"

#include "RBspObject.h"
#include "RLenzFlare.h"

//////////////////////////////////////////////////////////////////////////
//	Define
//////////////////////////////////////////////////////////////////////////
#define	NUM_ELEMENT	10
#define MAX_FLARE_ELEMENT_WIDTH		1000
#define MAX_FLARE_ELEMENT_HEIGHT	1000
#define MAX_ALPHA	0.5

_USING_NAMESPACE_REALSPACE2

_NAMESPACE_REALSPACE2_BEGIN

//////////////////////////////////////////////////////////////////////////
//	Global
//////////////////////////////////////////////////////////////////////////
//LPDIRECT3DVERTEXBUFFER9 RLenzFlare::msVB = 0;
//LPDIRECT3DTEXTURE9		RLenzFlare::msTextures[MAX_NUMBER_TEXTURE];
RBaseTexture*			RLenzFlare::msTextures[MAX_NUMBER_TEXTURE];
sFlareElement			RLenzFlare::msElements[MAX_NUMBER_ELEMENT];
bool					RLenzFlare::mbIsReady = FALSE;
//int					RLenzFlare::msRef = 0;
RLenzFlare				RLenzFlare::msInstance;

//struct  ScrVertex
//{
//	D3DXVECTOR4 p;
//	DWORD color;
//};
//
//#define ScrVertexType (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)
//
//static const ScrVertex[4] = 
//{
//	{ D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF },
//	{ D3DXVECTOR4( 0.0f, 1.0f, 0.0f, 0.0f), 0xFFFFFFFF },
//	{ D3DXVECTOR4( 1.0f, 1.0f, 0.0f, 0.0f), 0xFFFFFFFF },
//	{ D3DXVECTOR4( 1.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF },
//};

//////////////////////////////////////////////////////////////////////////
//	Functions
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//	Render
//	1. 광원과 중심과의 거리가 가까울수록 플레어의 크기가 커지고 알파값이 
//		커진다
//	2. 광원과 중심과의 거리를 구하고 그것을 엘레멘트의 수로 나누어 
//		엘레멘트간의 간격을 구하고 엘레멘트들을 그려준다.
//////////////////////////////////////////////////////////////////////////
bool RLenzFlare::Render( rvector& light_pos_, rvector& centre_, RBspObject* pbsp_  ) 
{
	rvector rDir	= light_pos_ - centre_;
	rvector cDir	= RCameraDirection;
	if( D3DXVec3Dot( &rDir, &cDir )  < 0 )
	{
		return false;
	}

	// 화면에서의 광원과 중심 위치
	// TODO : 만약 중심이 화면의 중앙이라면 연산을 쉽게 할수 있다...
	rmatrix view, proj;
	RGetDevice()->GetTransform( D3DTS_VIEW, &view );
	RGetDevice()->GetTransform( D3DTS_PROJECTION, &proj );
	
	rvector pos = ( light_pos_ * view * proj ) ;	// -1 ~ 1 사이 아니면 광원이 보이지 않는다
	pos.z = 0.0f;									// 화면에서 광원이 사라졌을 경우 렌즈 플레어가 보이지 않는다.

	if((pos.x < -1 || pos.x > 1) || (pos.y < -1 || pos.y > 1))
	{
        //return false;
	}

	float alpha = max(min(((1 - pos.x)*(1 + pos.x)+(1-pos.y)*(1+pos.y)-1.0f)*0.5f, 0.6f ), 0.0f);

	RBSPPICKINFO info;
	rvector	dir;
	float distance;
	dir = light_pos_ - RCameraPosition;
	distance	= D3DXVec3LengthSq( &dir );
	D3DXVec3Normalize( &dir, &dir );
	if( pbsp_->Pick( RCameraPosition, dir, &info,  RM_FLAG_ADDITIVE) )
	{
		if( distance > MagnitudeSq(RCameraPosition - info.PickPos) )
		{
			return false;
		}
	}
	
	
    rvector centre = ( centre_ * view * proj );
	centre.z = 0.0f;

	//for test
	//카메라 위치로 했을 경우 아주 불안함..캐릭터의 머리 부분 정도가 되야 할것 같음
	{
		centre.x = 0;
		centre.y = 0;
	}
	//end for test

	float dist = Magnitude(pos - centre);
	float scale_factor = 1/dist;

	pos.x = (pos.x + 1) * 0.5 * RGetScreenWidth();			// 실제 화면에서의 광원 위치
	pos.y = (-pos.y + 1) * 0.5 * RGetScreenHeight();
	centre.x = (centre.x + 1) * 0.5 * RGetScreenWidth();	// 실제 화면에서의 중심 위치
	centre.y = (-centre.y + 1) * 0.5 * RGetScreenHeight();
	
	rvector temp = pos - centre;

	float dx = centre.x + (centre.x - pos.x);
	float dy = centre.y + (centre.y - pos.y);

	float xInc = (dx - pos.x) / miNumFlareElement;
	float yInc = (dy - pos.y) / miNumFlareElement;

	int index;
	for( int i = 0 ; i < miNumFlareElement; ++i )
	{
		index = miElementOrder[i];

		float width = msElements[index].width * scale_factor;
		if( width > MAX_FLARE_ELEMENT_WIDTH )
		{
			width = MAX_FLARE_ELEMENT_WIDTH;
		}
		float height = msElements[index].height * scale_factor;
		if( height > MAX_FLARE_ELEMENT_HEIGHT )
		{
			height = MAX_FLARE_ELEMENT_HEIGHT;
		}

		float px = pos.x + (xInc * i) - width* 0.5;
		float py = pos.y + (yInc * i) - height * 0.5;

		float alpha = scale_factor * 0.2;

		if( alpha > MAX_ALPHA )
		{
			alpha = MAX_ALPHA;
		}

		if( !draw( px, py, width, height, alpha, msElements[index].color, msElements[index].iTextureIndex ) )
		{
			mlog( "Fail to Draw %dth Flare Element!\n",i );
			return false;
		}
	}

	//화면 뿌옇게 만들기
	draw( 0, 0, RGetScreenWidth(), RGetScreenHeight(), alpha, 0xFFFFFFFF, -1 );

    return true;
}

//////////////////////////////////////////////////////////////////////////
//	Render
//////////////////////////////////////////////////////////////////////////
bool	RLenzFlare::Render( rvector& centre_, RBspObject* pbsp_  )
{
	for( int i = 0 ; i < miNumLight; ++i )
	{
		Render( mLightList[i], centre_, pbsp_ ) ;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// SetLight
//////////////////////////////////////////////////////////////////////////
bool RLenzFlare::SetLight( rvector& pos_ )
{
	if( miNumLight >= MAX_LENZFLARE_NUMBER )
	{
		mlog("Can't Set Light...: Light Buffer for LenzFlare is Full already..\n" );
        return false;
	}
	mLightList[miNumLight++] = pos_;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//	Initialize
//////////////////////////////////////////////////////////////////////////
void RLenzFlare::Initialize()
{
	_ASSERT( miElementOrder==NULL );		// Initialize 을 두번 부른경우
	SAFE_DELETE_ARRAY( miElementOrder );

	miElementOrder = new int[NUM_ELEMENT];
	miNumFlareElement = NUM_ELEMENT;
	for( int i = 0 ; i < miNumFlareElement; ++i )
	{
		miElementOrder[i] = i;
	}
}
//////////////////////////////////////////////////////////////////////////mbIsReady
//	Draw
//////////////////////////////////////////////////////////////////////////
bool RLenzFlare::draw( float x_, float y_,  float width_, float height_,  
					  float alpha_,  DWORD color_, int textureIndex_ )
{
	//if( msVB == NULL )
	//{
	//	mlog("vertex buffer for lenz flare is not ready\n");
	//	return false;
	//}

	RTLVertex vertices[4];

	if( color_ > 0x00ffffff )
	{
		DWORD alpha_value = color_ & 0xff000000 ;
		color_ = color_ - alpha_value;
	}

	// vertex setup
	vertices[0].p.x = x_; 
	vertices[0].p.y = y_;
	vertices[0].p.z = 0.f;
	vertices[0].p.w = 1.0f;
	vertices[0].color	=	D3DXCOLOR(0,0,0,alpha_);
	//vertices[0].color	+=	color_;
	vertices[0].color += 0x00FFFFFF;
	vertices[0].tu	=	0.f;	
	vertices[0].tv	=	0.f;

	vertices[1].p.x = x_ + width_; 
	vertices[1].p.y = y_;
	vertices[1].p.z = 0.f;
	vertices[1].p.w = 1.0f;
	vertices[1].color	=	D3DXCOLOR(0,0,0,alpha_);
	//vertices[1].color	+=	color_;
	vertices[1].color += 0x00FFFFFF;
	vertices[1].tu	=	1.f;	
	vertices[1].tv	=	0.f;

	vertices[2].p.x = x_ + width_; 
	vertices[2].p.y = y_ + height_;
	vertices[2].p.z = 0.f;
	vertices[2].p.w = 1.0f;
	vertices[2].color	=	D3DXCOLOR(0,0,0,alpha_);
	//vertices[2].color	+=	color_;
	vertices[2].color += 0x00FFFFFF;
	vertices[2].tu	=	1.f;	
	vertices[2].tv	=	1.f;

	vertices[3].p.x = x_; 
	vertices[3].p.y = y_ + height_;
	vertices[3].p.z = 0.f;
	vertices[3].p.w = 1.0f;
	vertices[3].color	=	D3DXCOLOR(0,0,0,alpha_);
	//vertices[3].color	+=	color_;
	vertices[3].color += 0x00FFFFFF;
	vertices[3].tu	=	0.f;	
	vertices[3].tv	=	1.f;

	// copy vertices to vertex buffer
	//void* pVertices;
	//if( FAILED( msVB->Lock(0, sizeof(RTLVertex)* 4, (VOID**)&pVertices, D3DLOCK_DISCARD ) ))
	//{
	//	return false;
	//}

	//memcpy( pVertices, vertices, sizeof(RTLVertex) * 4 );

	//if( FAILED( msVB->Unlock() ) )
	//{
	//	return false;
	//}

	if( textureIndex_ >= 0 && msTextures[textureIndex_] != NULL )
	{
		RGetDevice()->SetTexture( 0, msTextures[textureIndex_]->GetTexture() );
	}
	else
	{
		RGetDevice()->SetTexture( 0, NULL );
	}

	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
 	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	//RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );

	RGetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	RGetDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	RGetDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
	
	RGetDevice()->SetFVF( RTLVertexType );
//	RGetDevice()->SetStreamSource( 0, msVB, 0, sizeof( RTLVertex ) );
	//RGetDevice()->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );
	RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(RTLVertex));
//	RGetDevice()->SetStreamSource( 0 , NULL, 0, 0 );

	RGetDevice()->SetTexture( 0, NULL );

	RGetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

	return true;
}	

//////////////////////////////////////////////////////////////////////////
//	초기화
//////////////////////////////////////////////////////////////////////////
bool RLenzFlare::Create( char* filename_ )
{
	//if(FAILED(RGetDevice()->CreateVertexBuffer( sizeof(RTLVertex)*4, 0, RTLVertexType, D3DPOOL_MANAGED, &msVB )))
	//{
	//	return false;
	//}

	// xml을 읽어 플레어의 종류와 순서 그리고 엘레먼트의 수를 정한다
	MXmlDocument	XmlDoc;
	MXmlElement		PNode,Node;

	XmlDoc.Create();

	char Path[256];
	Path[0] = NULL;

	GetPath(filename_,Path);

	char *buffer;
	MZFile mzf;

	if(g_pFileSystem)
	{
		if(!mzf.Open( filename_, g_pFileSystem ))
		{
			if(!mzf.Open( filename_ ))
				return false;
		}
	} 
	else 
	{
		if(!mzf.Open( filename_ ))
			return false;
	}

	buffer = new char[mzf.GetLength()+1];
	buffer[mzf.GetLength()] = 0;

	mzf.Read(buffer,mzf.GetLength());

	if(!XmlDoc.LoadFromMemory(buffer))
		return false;

	delete[] buffer;

	mzf.Close();

	PNode = XmlDoc.GetDocumentElement();

	if( ReadXmlElement( &PNode, Path ) == false ) 
	{
		XmlDoc.Destroy();
		return false;
	}

	XmlDoc.Destroy();

	mbIsReady = true;

	return true;
}

//////////////////////////////////////////////////////////////////////////
//	Destroy
//////////////////////////////////////////////////////////////////////////
bool RLenzFlare::Destroy()
{
	//if( msRef > 0)
	//{
	//	mlog("Object(s) Exist..! Fail to Destroy...\n");
	//	return false;
	//}
	
	//SAFE_RELEASE( msVB );
	for( int i = 0 ; i < MAX_NUMBER_TEXTURE; ++i )
	{
		RDestroyBaseTexture( msTextures[i] );
		msTextures[i] = NULL;
		//SAFE_DELETE( msTextures[i] );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
//	IsReady
//////////////////////////////////////////////////////////////////////////
bool RLenzFlare::IsReady()
{
	return mbIsReady;
}

//////////////////////////////////////////////////////////////////////////
//	생성자 / 소멸자
//////////////////////////////////////////////////////////////////////////
RLenzFlare::RLenzFlare(void)
{
	//++msRef;
//	Initialize();
	miElementOrder=NULL;
}

RLenzFlare::~RLenzFlare(void)
{
	//--msRef;
	SAFE_DELETE_ARRAY( miElementOrder );
}

//////////////////////////////////////////////////////////////////////////
//	HELPERs
//////////////////////////////////////////////////////////////////////////

bool RLenzFlare::ReadXmlElement(MXmlElement* PNode,char* Path)
{
	int i,j;
	int index;
	char texture_file_name[256];
	char buffer[16];
	char NodeName[64];
	
	MXmlElement Node, Leaf;
	
	PNode->GetNodeName(NodeName);
	int nCnt = PNode->GetChildNodeCount();
	for( i = 0 ; i < nCnt; ++i )
	{
		Node = PNode->GetChildNode( i );
		Node.GetTagName( NodeName );
		if (NodeName[0] == '#')
		{
			continue;
		}
		if( !strcmp( NodeName, "TEXTURE" ) )
		{
			index = 0;
			int numTex = Node.GetChildNodeCount();
			for( j = 0 ; j < numTex; ++j )
			{
				Leaf = Node.GetChildNode(j);
				Leaf.GetTagName( NodeName );
				if (NodeName[0] == '#')
				{
					continue;
				}
				if(!Leaf.GetAttribute( texture_file_name, "FILE_NAME" ))
				{
					for( int k = 0 ; k < MAX_NUMBER_TEXTURE; ++k )
					{
						SAFE_DELETE( msTextures[k] );
					}
					return false;
				}
				//D3DXCreateTextureFromFile( RGetDevice(), texture_file_name, &msTextures[index++] );
				msTextures[index++] = RCreateBaseTexture( texture_file_name );
			}
		}
		else if( !strcmp( NodeName, "ELEMENTS" ) )
		{
			index = 0;
			int numElem = Node.GetChildNodeCount();
			for( j = 0 ; j < numElem; ++j )
			{
				Leaf = Node.GetChildNode(j);
				Leaf.GetTagName( NodeName );
				if (NodeName[0] == '#')
				{
					continue;
				}

				if(!Leaf.GetAttribute( buffer, "TYPE" ))
				{
					return false;
				}
				msElements[index].iType = atoi(buffer);

				if(!Leaf.GetAttribute( buffer, "WIDTH" ))
				{
					return false;
				}
				msElements[index].width = atof(buffer);

				if(!Leaf.GetAttribute( buffer, "HEIGHT" ))
				{
					return false;
				}
				msElements[index].height = atof(buffer);


				if(!Leaf.GetAttribute( buffer, "COLOR" ))
				{
					return false;
				}
				msElements[index].color = atol(buffer);

				if(!Leaf.GetAttribute( buffer, "WIDTH" ))
				{
					return false;
				}
				msElements[index].width = atoi(buffer);


				if(!Leaf.GetAttribute( buffer, "TEXTURE_INDEX" ))
				{
					return false;
				}
				msElements[index].iTextureIndex = atoi(buffer);
				++index;
			}
		}
	}
	
	return true;
}

//	렌즈 플레어 기능을 사용하기 위해 호출
bool	RCreateLenzFlare( char* filename_ )
{
	if( RReadyLenzFlare() )	
	{
		return true;	// 이미 사용 준비가 되어 있으면 참 반환
	}

	if( RLenzFlare::Create( filename_ ))
	{
		return true;
	}
	return false;
};

// 더이상 렌즈 플레어 기능을 사용하지 않음을 선언
bool	RDestroyLenzFlare( )
{
	return RLenzFlare::Destroy();
};

// 렌즈 플레어가 사용 준비가 되었는지 쿼리
bool	RReadyLenzFlare( ) 
{
	return RLenzFlare::IsReady();
};

// 렌즈 플레어 객체 얻어오기
//RLenzFlare*	RGetLenzFlare( bool* result )
//{
//	if( !RReadyLenzFlare() )
//	{
//		if(result != NULL )
//		{
//			*result	 = false;
//		}
//		return NULL;
//	}
//	if( result != NULL )
//	{
//		*result = true;
//	}
//	return new RLenzFlare;
//}

RLenzFlare* RGetLenzFlare()
{
	return RLenzFlare::GetInstance();
}

// 엑수엠엘 오픈
bool RLenzFlare::open( const char* pFileName_, MZFileSystem* pfs_ )
{
	//MXmlDocument	aXml;
	//char szXmlFileName[256];
	//sprintf_safe(szXmlFileName, "%s.xml", pFileName_);
	//MZFile mzf;
	//if(!mzf.Open(szXmlFileName,pfs_)) return false;
	//char *buffer;
	//buffer=new char[mzf.GetLength()+1];
	//mzf.Read(buffer,mzf.GetLength());
	//buffer[mzf.GetLength()]=0;
	//aXml.Create();
	//if(!aXml.LoadFromMemory(buffer))
	//{
	//	delete buffer;
	//	return false;
	//}
	//delete buffer;
	//char szBuf[65535];
	//ZeroMemory(szBuf, 65535);
	//MXmlElement aRoot;
	//aRoot = aXml.GetDocumentElement();

	////int nSpawnListCount = aRoot.GetChildNodeCount();

	//for (int i=0; i<nSpawnListCount; i++)
	//{
	//	MXmlElement aSpawnList = aRoot.GetChildNode(i);
	//	aSpawnList.GetTagName(szBuf);
	//	if (szBuf[0] == '#') continue;

	//	if (!_stricmp(szBuf, "DUMMYLIST")) 
	//	{
	//		
	//	}
	//}

	//aXml.Destroy();

	return true;
}

_NAMESPACE_REALSPACE2_END
