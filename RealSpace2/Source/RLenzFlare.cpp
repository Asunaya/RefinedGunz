#include "stdafx.h"
#include "MXml.h"
#include "MDebug.h"
#include "RealSpace2.h"
#include "RMeshUtil.h"

#include "RBspObject.h"
#include "RLenzFlare.h"

#define	NUM_ELEMENT	10
#define MAX_FLARE_ELEMENT_WIDTH		1000
#define MAX_FLARE_ELEMENT_HEIGHT	1000
#define MAX_ALPHA	0.5

_USING_NAMESPACE_REALSPACE2

_NAMESPACE_REALSPACE2_BEGIN

RBaseTexture*			RLenzFlare::msTextures[MAX_NUMBER_TEXTURE];
sFlareElement			RLenzFlare::msElements[MAX_NUMBER_ELEMENT];
bool					RLenzFlare::mbIsReady = FALSE;
RLenzFlare				RLenzFlare::msInstance;

bool RLenzFlare::Render( rvector& light_pos_, rvector& centre_, RBspObject* pbsp_  ) 
{
	rvector rDir	= light_pos_ - centre_;
	rvector cDir	= RCameraDirection;
	if( DotProduct( rDir, cDir )  < 0 )
	{
		return false;
	}

	auto view = RGetTransform(D3DTS_VIEW);
	auto proj = RGetTransform(D3DTS_PROJECTION);
	
	rvector pos = ( light_pos_ * view * proj ) ;
	pos.z = 0.0f;

	float alpha = max(min(((1 - pos.x)*(1 + pos.x)+(1-pos.y)*(1+pos.y)-1.0f)*0.5f, 0.6f ), 0.0f);

	RBSPPICKINFO info;
	rvector	dir;
	float distance;
	dir = light_pos_ - RCameraPosition;
	distance	= MagnitudeSq( dir );
	Normalize(dir);
	if( pbsp_->Pick( RCameraPosition, dir, &info,  RM_FLAG_ADDITIVE) )
	{
		if( distance > MagnitudeSq(RCameraPosition - info.PickPos) )
		{
			return false;
		}
	}
	
	
    rvector centre = ( centre_ * view * proj );
	centre.z = 0.0f;

	{
		centre.x = 0;
		centre.y = 0;
	}

	float dist = Magnitude(pos - centre);
	float scale_factor = 1/dist;

	pos.x = (pos.x + 1) * 0.5 * RGetScreenWidth();
	pos.y = (-pos.y + 1) * 0.5 * RGetScreenHeight();
	centre.x = (centre.x + 1) * 0.5 * RGetScreenWidth();
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

	draw( 0, 0, RGetScreenWidth(), RGetScreenHeight(), alpha, 0xFFFFFFFF, -1 );

    return true;
}

bool	RLenzFlare::Render( rvector& centre_, RBspObject* pbsp_  )
{
	for( int i = 0 ; i < miNumLight; ++i )
	{
		Render( mLightList[i], centre_, pbsp_ ) ;
	}
	return true;
}

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

void RLenzFlare::Initialize()
{
	_ASSERT( miElementOrder==NULL );
	SAFE_DELETE_ARRAY( miElementOrder );

	miElementOrder = new int[NUM_ELEMENT];
	miNumFlareElement = NUM_ELEMENT;
	for( int i = 0 ; i < miNumFlareElement; ++i )
	{
		miElementOrder[i] = i;
	}
}

bool RLenzFlare::draw( float x_, float y_,  float width_, float height_,  
					  float alpha_,  DWORD color_, int textureIndex_ )
{
	RTLVertex vertices[4];

	if( color_ > 0x00ffffff )
	{
		DWORD alpha_value = color_ & 0xff000000 ;
		color_ = color_ - alpha_value;
	}

	vertices[0].p.x = x_; 
	vertices[0].p.y = y_;
	vertices[0].p.z = 0.f;
	vertices[0].p.w = 1.0f;
	vertices[0].color = RGBA(0, 0, 0, alpha_);
	vertices[0].color += 0x00FFFFFF;
	vertices[0].tu	=	0.f;	
	vertices[0].tv	=	0.f;

	vertices[1].p.x = x_ + width_; 
	vertices[1].p.y = y_;
	vertices[1].p.z = 0.f;
	vertices[1].p.w = 1.0f;
	vertices[1].color = RGBA(0, 0, 0, alpha_);
	vertices[1].color += 0x00FFFFFF;
	vertices[1].tu	=	1.f;	
	vertices[1].tv	=	0.f;

	vertices[2].p.x = x_ + width_; 
	vertices[2].p.y = y_ + height_;
	vertices[2].p.z = 0.f;
	vertices[2].p.w = 1.0f;
	vertices[2].color = RGBA(0, 0, 0, alpha_);
	vertices[2].color += 0x00FFFFFF;
	vertices[2].tu	=	1.f;	
	vertices[2].tv	=	1.f;

	vertices[3].p.x = x_; 
	vertices[3].p.y = y_ + height_;
	vertices[3].p.z = 0.f;
	vertices[3].p.w = 1.0f;
	vertices[3].color = RGBA(0, 0, 0, alpha_);;
	vertices[3].color += 0x00FFFFFF;
	vertices[3].tu	=	0.f;	
	vertices[3].tv	=	1.f;

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
	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );

	RGetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	RGetDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	RGetDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
	
	RGetDevice()->SetFVF( RTLVertexType );
	RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(RTLVertex));

	RGetDevice()->SetTexture( 0, NULL );

	RGetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

	return true;
}	

bool RLenzFlare::Create( char* filename_ )
{
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

bool RLenzFlare::Destroy()
{
	for( int i = 0 ; i < MAX_NUMBER_TEXTURE; ++i )
	{
		RDestroyBaseTexture( msTextures[i] );
		msTextures[i] = NULL;
	}

	return true;
}

bool RLenzFlare::IsReady()
{
	return mbIsReady;
}

RLenzFlare::RLenzFlare()
{
	miElementOrder=NULL;
}

RLenzFlare::~RLenzFlare()
{
	SAFE_DELETE_ARRAY( miElementOrder );
}

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
				if (!Leaf.GetAttribute(texture_file_name, "FILE_NAME"))
				{
					for (int k = 0; k < MAX_NUMBER_TEXTURE; ++k)
					{
						RDestroyBaseTexture(msTextures[k]);
					}
					return false;
				}
				msTextures[index++] = RCreateBaseTexture(texture_file_name);
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

bool	RCreateLenzFlare( char* filename_ )
{
	if( RReadyLenzFlare() )	
	{
		return true;
	}

	if( RLenzFlare::Create( filename_ ))
	{
		return true;
	}
	return false;
};

bool	RDestroyLenzFlare( )
{
	return RLenzFlare::Destroy();
};

bool	RReadyLenzFlare( ) 
{
	return RLenzFlare::IsReady();
};


RLenzFlare* RGetLenzFlare()
{
	return RLenzFlare::GetInstance();
}

bool RLenzFlare::open( const char* pFileName_, MZFileSystem* pfs_ )
{
	return true;
}

_NAMESPACE_REALSPACE2_END
