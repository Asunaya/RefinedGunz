#include <stdio.h>
#include "RealSpace.h"
#include "ASSMap.h"
#include "ASSSchemeManager.h"
#include "rtexture.h"
#include "RSDebug.h"

#include "MProfiler.h"

//#define _DEBUG_SHOW_LOD_LEVEL
#ifdef _DEBUG_SHOW_LOD_LEVEL
// test
#include "RSMaterialManager.h"
RSMaterialManager *testmaterial;
int hNumberTexture[10];
void loadnumbertextures()
{
	testmaterial=new RSMaterialManager;
	if(!testmaterial->Open("numbers.rml")) return;
	for(int i=0;i<10;i++)
	{
		hNumberTexture[i]=testmaterial->Get(i);
	}
}
// endof test
#endif

#define TEXTURE_PARTITION_SIZE	256

#define ASS_BASE_DIVISION_CONSTANT 8
#define ASS_BASE_MULTIPLY_CONSTANT (1.f/(float)ASS_BASE_DIVISION_CONSTANT)

//#define EL_TEST

ASSVolumeTreeNode::ASSVolumeTreeNode()
{
	m_pCell=NULL;
	m_pLODInfo=NULL;
	m_bExpanded=false;
}

ASSVolumeTreeNode::~ASSVolumeTreeNode()
{
	if(m_pLODInfo) delete m_pLODInfo;
}

ASSMap::ASSMap()
{
#ifdef _DEBUG_SHOW_LOD_LEVEL
	loadnumbertextures();
#endif
	nx=0;ny=0;
	m_nTreeDepth=0;
	m_nTreeNode=0;
	m_pSchemeManager=NULL;
	m_Cells=NULL;
	m_bLODEnable=false;
	m_LODTable=NULL;
	m_nPatches_x=0;
	m_nPatches_y=0;
	m_hMinimapTexture=0;
	m_hLODTextures=NULL;
	m_LodTextures=NULL;
	m_hPattern=0;
	nRefTextures=NULL;
	m_pQueues=NULL;
	nQueueCounts=NULL;
	m_nlodcell_texsize=LODCELL_TEXSIZE;
	m_pTree=NULL;

	m_pCellColor = NULL;

#ifndef EL_TEST

	pVertexBuffer=NULL;
	pIndexBuffer=NULL;

#else

	m_point_list = NULL;
	m_index_list = NULL;

#endif
}

ASSMap::~ASSMap()
{
#ifdef _DEBUG_SHOW_LOD_LEVEL
	delete testmaterial;
#endif
	int i;

	if(m_Cells) delete []m_Cells;
	if(m_Heights) delete []m_Heights;
	if(m_LODTable) delete []m_LODTable;
	if(m_pCellColor) delete [] m_pCellColor;

	if(m_pTree)
	{
		delete []m_pTree;
		m_pTree=NULL;
	}
	if(m_hMinimapTexture)
	{
		RSDeleteTexture(m_hMinimapTexture);
		m_hMinimapTexture=NULL;
	}
	if(m_LodTextures)
	{
		for(i=0;i<m_nPatches_x*m_nPatches_y;i++)
			RSDeleteTexture(m_LodTextures[i]);
		delete []m_LodTextures;
		m_LodTextures=NULL;
	}
	if(nRefTextures)
	{
		delete []nRefTextures;
		nRefTextures=NULL;
	}
	if(m_pQueues)
	{
		for(i=0;i<nQueueCount;i++)
		{
			delete []m_pQueues[i];
			SAFE_RELEASE(ppIndexBuffers[i]);
		}
		delete []m_pQueues;
		delete []ppIndexBuffers;
		m_pQueues=NULL;
	}
	if(nQueueCounts)
	{
		delete []nQueueCounts;
		nQueueCounts=NULL;
	}

#ifndef EL_TEST
	
	if(pVertexBuffer)
		pVertexBuffer->Release();
	if(pIndexBuffer)
		pIndexBuffer->Release();

#else

	if(m_point_list)
		delete [] m_point_list;

	if(m_index_list)
		delete [] m_index_list;

#endif

}

#define COPY_UV(d,f,i) { d.tu1=cell->faces[f].s[i]; d.tv1=cell->faces[f].t[i]; }

DWORD ASSMap::GetFogValue(float z)
{
	float zz=max(min(1.0f-(z-m_FogStartZ)*m_1oDistance,1),0);
	return DWORD(unsigned char(zz*255)) << 24;
}

DWORD AddColor(DWORD ca,DWORD cb)
{
	float r,g,b,a,r2,g2,b2,a2;

	a = (float)((ca & 0xff000000) >> 24);
	r = (float)((ca & 0x00ff0000) >> 16);
	g = (float)((ca & 0x0000ff00) >>  8);
	b = (float)(ca & 0x000000ff);

	a2 = (float)((cb & 0xff000000) >> 24);
	r2 = (float)((cb & 0x00ff0000) >> 16);
	g2 = (float)((cb & 0x0000ff00) >>  8);
	b2 = (float)(cb & 0x000000ff);

	return D3DCOLOR_ARGB(0,((WORD)(r+r2))>>1,((WORD)(g+g2))>>1,((WORD)(b+b2))>>1);
}

DWORD MultiplyColor(DWORD ca,DWORD cb)
{
	float r,g,b,a,r2,g2,b2,a2;

	a = (float)((ca & 0xff000000) >> 24);
	r = (float)((ca & 0x00ff0000) >> 16);
	g = (float)((ca & 0x0000ff00) >>  8);
	b = (float)(ca & 0x000000ff);

	a2 = (float)((cb & 0xff000000) >> 24);
	r2 = (float)((cb & 0x00ff0000) >> 16);
	g2 = (float)((cb & 0x0000ff00) >>  8);
	b2 = (float)(cb & 0x000000ff);

	return D3DCOLOR_ARGB(0,((WORD)(255*r*r2/65535)),((WORD)(255*g*g2/65535)),((WORD)(255*b*b2/65535)));
}

void ASSMap::CalcColor(int index)
{
	ASSCell *cell=m_Cells+index;
	DWORD add_color = m_pCellColor[index];

	float	cl;

#define MAP_AMBIENT 0.1f
	cl=min(max(DotProduct(cell->normal,-RSLightDirection),MAP_AMBIENT),1.0f);
	//cl = pow(cl, 2);
	//cl = (sin(pi*cl-pi/2)+1.0f)/2.0f;

	if(cell->m_nLitFrame==RSFrameCount)
	{
		/*
		cell->m_color.x=min(cell->m_color.x+cl*RSLightColor.x,1.0f);
		cell->m_color.y=min(cell->m_color.y+cl*RSLightColor.y,1.0f);
		cell->m_color.z=min(cell->m_color.z+cl*RSLightColor.z,1.0f);
		*/
		cell->m_color+=cl*RSLightColor;
		cell->m_color.x=min(cell->m_color.x,1.0f);
		cell->m_color.y=min(cell->m_color.y,1.0f);
		cell->m_color.z=min(cell->m_color.z,1.0f);
		cell->m_dwColor=VECTOR2RGB24(cell->m_color);
	}
	else
		cell->m_dwColor=FLOAT2RGB24(cl*RSLightColor.x,cl*RSLightColor.y,cl*RSLightColor.z);
//		cell->m_dwColor=0x9f2f2f2f;//FLOAT2RGB24(cl*RSLightColor.x,cl*RSLightColor.y,cl*RSLightColor.z);
/*
	{
		cell->m_color+=cl*RSLightColor;
		cell->m_color.x=min(cell->m_color.x,1.0f);
		cell->m_color.y=min(cell->m_color.y,1.0f);
		cell->m_color.z=min(cell->m_color.z,1.0f);
		cell->m_dwColor=VECTOR2RGB24(cell->m_color);
	}
*/
/*
	{
		rvector color;

		color.x = 0.7f;
		color.y = 0.7f;
		color.z = 0.7f;

		DWORD dwcolor = FLOAT2RGB24(cl*color.x,cl*color.y,cl*color.z);
		cell->m_dwColor=dwcolor;

	}
*/	
#ifndef EL_TEST

	pVertices[index+nx*ny].Diffuse=	
//	pVertices[index].Diffuse=cell->m_dwColor;
	//pVertices[index].Diffuse = AddColor(add_color,cell->m_dwColor);
	pVertices[index].Diffuse = MultiplyColor(add_color,cell->m_dwColor);

#else

	m_point_list[index+nx*ny].Diffuse=	
	m_point_list[index].Diffuse=cell->m_dwColor;

#endif
}

void ASSMap::Transform(int index)
{
	ASSCell *cell=m_Cells+index;
	if(cell->m_nTransformedFrame==RSFrameCount) return;
	cell->m_nTransformedFrame=RSFrameCount;
	float x=float(index % nx),y=float(index/nx),z=m_Heights[index];

	float sz=RSView._13*x+RSView._23*y+RSView._33*z+RSView._43;
	DWORD fow=DWORD(cell->m_fFow*255.f)<<24;
	DWORD fog=min(fow,GetFogValue(sz));

#ifndef EL_TEST

	pVertices[index+nx*ny].Specular=pVertices[index].Specular=fog;

#else

	m_point_list[index+nx*ny].Specular = m_point_list[index].Specular = fog;

#endif

	CalcColor(index);
}

// child 의 view-frustrum-culling states를 갱신한다.
bool ASSMap::UpdateInOutStates(ASSVolumeTreeNode *parent,ASSVolumeTreeNode *child)
{
	int i,nin=0;
	for(i=0;i<6;i++)
	{
		if(parent->states[i]==IS_IN) child->states[i]=IS_IN;
		else
		{
			float min,max;
			GetDistanceMinMax(child->m_bbTree,*(RSViewFrustrum+i),&min,&max);
			if(max<0) return false;
			if(min>0) { child->states[i]=IS_IN;nin++; } else child->states[i]=IS_DONTKNOW;
		}
	}
	child->m_bAllIn=(nin==6);
	return true;
}

void ASSMap::ReadChunk_Size(FILE *file)
{
	fread(&nx,sizeof(int),1,file);
	fread(&ny,sizeof(int),1,file);
	m_Cells=new ASSCell[nx*ny];
	m_Heights=new float[nx*ny];
	m_pCellColor = new DWORD[nx*ny];

	for(int i=0;i<nx*ny;i++)
	{
		m_pCellColor[i] = 0xffffffff;
	}
}

void ASSMap::ReadChunk_Color(FILE *file)
{
	fread(m_pCellColor,sizeof(DWORD)*nx*ny,1,file);
}

void ASSMap::ReadChunk_Geometry(FILE *file)
{
	int i;
	ASSCell *cell;
	ASSCELLFLAGS flags;
	for(i=0;i<nx*ny;i++)
	{
		cell=&m_Cells[i];
		cell->normal=rvector(0,0,1);
		for(int j=0;j<8;j++)
			cell->planes[j/4].m[j%4]=0;
		fread(&m_Heights[i],sizeof(float),1,file);
		fread(&flags,sizeof(flags),1,file);
		cell->m_nTextureDir=flags.nTextureDir;
		cell->m_nTextureIndex=flags.nTextureIndex;
		cell->m_nTileSet=flags.nTileSet;
	}
}

void ASSMap::ReadChunk_LODTexture(FILE *file)
{
	rtexture lodtexture;
	lodtexture.New((nx-1)*2,(ny-1)*2,RTEXTUREFORMAT_24);
	
	int tx=(nx-1)*2,ty=(ny-1)*2;
	fread(lodtexture.GetData(),tx*ty*3,1,file);

	m_nPatches_x=((tx-1)/TEXTURE_PARTITION_SIZE)+1;
	m_nPatches_y=((ty-1)/TEXTURE_PARTITION_SIZE)+1;

	m_LodTextures=new int[m_nPatches_x*m_nPatches_y];
	int ix,iy;
	for(ix=0;ix<m_nPatches_x;ix++)
	{
		for(iy=0;iy<m_nPatches_y;iy++)
		{
			rtexture temptexture;
			temptexture.CreateAsCopy(&lodtexture,ix*TEXTURE_PARTITION_SIZE,iy*TEXTURE_PARTITION_SIZE
				,(ix==m_nPatches_x-1)?((tx-1)%TEXTURE_PARTITION_SIZE+1):TEXTURE_PARTITION_SIZE
				,(iy==m_nPatches_y-1)?((ty-1)%TEXTURE_PARTITION_SIZE+1):TEXTURE_PARTITION_SIZE);

			m_LodTextures[m_nPatches_x*iy+ix]=
				RSCreateTexture(temptexture.GetWidth(),temptexture.GetHeight(),
				(char*)temptexture.GetData(),NULL,true,"Map Created");
		}
	}

	rtexture *pMinimap=new rtexture;
	pMinimap->Create(&lodtexture);
	while((pMinimap->GetWidth()>TEXTURE_PARTITION_SIZE) || (pMinimap->GetHeight()>TEXTURE_PARTITION_SIZE))
	{
		rtexture *ptemp=new rtexture;
		ptemp->CreateAsHalf(pMinimap);
		delete pMinimap;
		pMinimap=ptemp;
	}
	m_hMinimapTexture=RSCreateTexture(pMinimap->GetWidth(),pMinimap->GetHeight(),(char*)pMinimap->GetData(),NULL,true,
		"map Mini mapTexture");
	delete pMinimap;
	m_nlodcell_texsize=2;
}

void ASSMap::ReadChunk_LODTexture2(FILE *file)
{
	rtexture lodtexture;
	lodtexture.Create(file);

/*
//	lodtexture.SaveAsBMP("testlod.bmp");
//	testcode
//		lodtexture.CreateFromBMP("C:\\Down\\incoming\\test.bmp");
*/

	int tx=lodtexture.GetWidth(),ty=lodtexture.GetHeight();

	m_nPatches_x=((tx-1)/TEXTURE_PARTITION_SIZE)+1;
	m_nPatches_y=((ty-1)/TEXTURE_PARTITION_SIZE)+1;

	m_LodTextures=new int[m_nPatches_x*m_nPatches_y];
	int ix,iy;
	for(ix=0;ix<m_nPatches_x;ix++)
	{
		for(iy=0;iy<m_nPatches_y;iy++)
		{
			rtexture temptexture;
			temptexture.CreateAsCopy(&lodtexture,ix*TEXTURE_PARTITION_SIZE,iy*TEXTURE_PARTITION_SIZE
				,(ix==m_nPatches_x-1)?((tx-1)%TEXTURE_PARTITION_SIZE+1):TEXTURE_PARTITION_SIZE
				,(iy==m_nPatches_y-1)?((ty-1)%TEXTURE_PARTITION_SIZE+1):TEXTURE_PARTITION_SIZE,
				(ix % 2) == 1, (iy % 2) == 1);

			m_LodTextures[m_nPatches_x*iy+ix]=
				RSCreateTexture(temptexture.GetWidth(),temptexture.GetHeight(),
				(char*)temptexture.GetData(),NULL,true,"Map Created");
		}
	}
/*
	rtexture *pMinimap=new rtexture;
	pMinimap->Create(&lodtexture);
	while((pMinimap->GetWidth()>TEXTURE_PARTITION_SIZE) || (pMinimap->GetHeight()>TEXTURE_PARTITION_SIZE))
	{
		rtexture *ptemp=new rtexture;
		ptemp->CreateAsHalf(pMinimap);
		delete pMinimap;
		pMinimap=ptemp;
	}
	m_hMinimapTexture=RSCreateTexture(pMinimap->GetWidth(),pMinimap->GetHeight(),(char*)pMinimap->GetData(),NULL,true,
		"map Mini mapTexture");
	delete pMinimap;
	*/
	m_nlodcell_texsize=lodtexture.GetWidth()/(nx-1);
}

void ASSMap::ReadChunk_MinimapTexture(FILE *file)
{
	rtexture minimaptexture;
	minimaptexture.Create(file);

	int tx=minimaptexture.GetWidth(),ty=minimaptexture.GetHeight();

	m_hMinimapTexture=RSCreateTexture(minimaptexture.GetWidth(),minimaptexture.GetHeight(),
		(char*)minimaptexture.GetData(),NULL,true,"map Mini mapTexture");
}


bool ASSMap::Open(FILE *file,ASSSchemeManager *pSchemeManager)
{
	m_pSchemeManager=pSchemeManager;

	RSCHUNKHEADER ch;
	while((fread(&ch,RSCHUNKHEADERSIZE,1,file)) 
		&& ( ch.ChunkID != AMCHUNKHEADERID_END ) )
	{
		switch(ch.ChunkID)
		{
		case AMCHUNKHEADERID_SIZE		: ReadChunk_Size(file);break;
		case AMCHUNKHEADERID_GEOMETRY	: ReadChunk_Geometry(file);break;
		case AMCHUNKHEADERID_LODTEXTURE : ReadChunk_LODTexture(file);break;
		case AMCHUNKHEADERID_LODTEXTURE2 : ReadChunk_LODTexture2(file);break;
		case AMCHUNKHEADERID_MINIMAPTEXTURE : ReadChunk_MinimapTexture(file);break;
		case AMCHUNKHEADERID_COLOR		: ReadChunk_Color(file); break;
		default : fseek(file,ch.ChunkSize,SEEK_CUR);break;
		}
	}
	if(!nx || !ny || !m_Cells )
		return false;

	CreateSchemeRefTextures();
	CreateVertexBuffer();
	ConstructVolumeTree();
	UpdateGeometry(0,0,nx-1,ny-1);

	SetLODState(true);
	
	return true;
}

bool ASSMap::Open(const char* filename,ASSSchemeManager *pSchemeManager)
{
	FILE *file=NULL;
	file=fopen(filename,"rb");

	if(!file) return false;
	bool ret=Open(file,pSchemeManager);

	fclose(file);
	return ret;
}

static rvector vPickOrigin;		// PickLine.u is Pick Direction
static rplueckercoord PickLine;
static float	fPickDist=0;
static bool		bPickFound;

//now checking whole the intersect face ... need optimize. need 2 planes for each node;
bool ASSMap::Pick(ASSVolumeTreeNode *node,rvector *ret)
{
	int i;
	if(node->m_pCell)
	{
//		rface *f;
		for(i=0;i<2;i++)
		{
//			f=&node->m_pCell->faces[i];
			rplane *pplane=&node->m_pCell->planes[i];
			
			if((DotProduct(pplane->normal,PickLine.u)>0)&&
				(IsIntersect(PickLine,GetVector(node->ind[0+3*i]),GetVector(node->ind[1+3*i]),GetVector(node->ind[2+3*i]))))
			{
				float t=(DotProduct(vPickOrigin,pplane->normal)+pplane->d)
						/DotProduct(pplane->normal,PickLine.u);
				if(t>=-0.001f)
				{
					rvector vColPos=vPickOrigin-t*PickLine.u;
					float dist=vColPos.GetDistance(vPickOrigin);
					if(!bPickFound)
					{
						fPickDist=dist;
						*ret=vColPos;
						bPickFound=true;
					}
					else
					{
						if(fPickDist>dist)
						{
							fPickDist=dist;
							*ret=vColPos;
						}
					}
				}
			}
		}
	}
	else
//	if(isLineIntersectBoundingBox(&PickLine,&node->m_bbTree))
	if(GetDistance(node->m_Center,vPickOrigin,PickLine.u)<node->m_fRadius)
	{
		
		for(i=0;i<4;i++)
		{
			if(node->m_nChildren[i])
				Pick(&m_pTree[node->m_nChildren[i]],ret);
		}
		
	}
	return bPickFound;
}

bool ASSMap::Pick(const rvector *origin,const rvector *target,rvector *ret)
{
	vPickOrigin=*origin;bPickFound=false;
	PickLine=rplueckercoord(*origin,*target);
	return Pick(m_pTree,ret);
}

bool ASSMap::Pick(rvector &origin,rvector &target,rvector *ret)
{
	vPickOrigin=origin;bPickFound=false;
	PickLine=rplueckercoord(origin,target);
	return Pick(m_pTree,ret);
}

bool ASSMap::Pick(int x, int y, rvector *ret)
{
	rvector scrpoint=rvector((float)x,(float)y,0.1f);
	scrpoint=TransformVector(scrpoint,MatrixInverse(RSViewProjectionViewport));
	vPickOrigin=RSCameraPosition;bPickFound=false;
	PickLine=rplueckercoord(RSCameraPosition,scrpoint);
	return Pick(m_pTree,ret);
}

/*
rface *ASSMap::GetFace(float x,float y)
{
	if((x>nx-1)||(x<0)||(y>ny-1)||(y<0)) return NULL;
	int index=(int)x+(int)y*nx;
	return (GetCase(x,y)%2) ? &m_Cells[index].faces[1] : &m_Cells[index].faces[0];
}

rface *ASSMap::GetFace(int x,int y,int iface)
{
	if((x>nx-1)||(x<0)||(y>ny-1)||(y<0)) return NULL;
	int index=(int)x+(int)y*nx;
	return &m_Cells[index].faces[iface];
}
*/

rvector ASSMap::GetNormal(float x, float y)
{
	/*
	rface *targetface=GetFace(x,y);
	return (targetface)?targetface->normal:rvector(0,0,1);
	*/
	ASSCell *pCell=GetCell(x,y);
	
	if(!pCell) 
		return rvector(0,0,1);
	return pCell->planes[GetCase(x,y)%2].normal;
}

rvector ASSMap::GetInterpolatedNormal(float x, float y)
{
	if((x<0)||(y<0)||(x>=nx-1)||(y>=ny-1))
		return rvector(0,0,1);
/*	rvertex *a,*b,*c;

	rface *f=GetFace(x,y);
	a=f->v[0];b=f->v[1];c=f->v[2];
*/
	ASSCell *pcell=GetCell(x,y);
	int nCase=GetCase(x,y)%2;
	rvector a=GetVector(pcell->pind[nCase*3+0]),
		b=GetVector(pcell->pind[nCase*3+1]),
		c=GetVector(pcell->pind[nCase*3+2]);
	rvector na=GetCell(pcell->pind[nCase*3+0])->normal,
		nb=GetCell(pcell->pind[nCase*3+1])->normal,
		nc=GetCell(pcell->pind[nCase*3+2])->normal;
/*	rvector s,t;
	float sx,tx;

	sx=float(a->x*fabs(b->y-y)+b->x*fabs(y-a->y));-
		s=a->normal*(float)fabs(b->y-y)+b->normal*(float)fabs(y-a->y);
	tx=float(a->x*fabs(c->y-y)+c->x*fabs(y-a->y));
		t=a->normal*(float)fabs(c->y-y)+c->normal*(float)fabs(y-a->y);
	return (s*(float)fabs(tx-x)+t*(float)fabs(x-sx))*float(1/fabs(tx-sx));*/

	rvector tem;
	float dx,dy,ex,ey,s,ibcx,ibcy;
	dx=x-a.x;dy=y-a.y;
	ex=c.x-b.x;ey=c.y-b.y;
	if(IS_EQ(dy*ex-dx*ey,0)) return na;
	s=(dy*(a.x-b.x)+dx*(b.y-a.y))/(dy*ex-dx*ey);
	tem=InterpolatedVector(nb,nc,s);
	ibcx=b.x+s*(c.x-b.x);ibcy=b.y+s*(c.y-b.y);
	dx=ibcx-a.x;dy=ibcy-a.y;
	s=(fabs(dy)>fabs(dx))?(y-a.y)/(ibcy-a.y):(x-a.x)/(ibcx-a.x);
	return InterpolatedVector(na,tem,s);
}

float ASSMap::GetHeight(float x,float y)
{
	if((x<0)||(y<0)||(x>nx-1)||(y>ny-1)) return 0;
//	rface *targetface=GetFace(x,y);

	if(x==(float)(nx-1)) x=(float)(x-TOLER);
	if(y==(float)(ny-1)) y=(float)(y-TOLER);

	int nCase=GetCase(x,y) % 2;
	ASSCell *pCell=GetCell(x,y);
	rvector v=GetVector(pCell->pind[3*nCase]),normal=pCell->planes[nCase].normal;
	
	float ret=(((v.x-x)*normal.x)+(v.y-y)*normal.y)/normal.z+v.z;
	_ASSERT(_finite(ret));
	return ret;	
	
}

float ASSMap::GetHeight(int x,int y)
{
	if((x<0)||(y<0)||(x>nx-1)||(y>ny-1)) return 0;
	return m_Heights[x+y*nx];
}

void ASSMap::SetHeight(int x,int y,float z)
{
	if((x<0)||(y<0)||(x>nx-1)||(y>ny-1)) return ;
//	m_Cells[x+y*nx].v.z=z;
	m_Heights[x+y*nx]=z;
}

void ASSMap::UpdateGeometry(int x1,int y1,int x2,int y2)
{
#ifndef EL_TEST

	pVertexBuffer->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ;
	
	x1=max(x1,0);y1=max(y1,0);
	x2=min(x2,nx-1);y2=min(y2,ny-1);
	for(int i=y1;i<=y2;i++)
	{
		for(int j=x1;j<=x2;j++)
		{
			int index=j+i*nx;
			pVertices[index].z=m_Heights[index];
		}
	}

	UpdateNormals(x1,y1,x2,y2);
	UpdateBoundingVolumes(m_pTree,x1,y1,x2,y2);
	UpdateColors(m_pTree,x1,y1,x2,y2);

	pVertexBuffer->Unlock();

#else

	int i,j,index;

	x1=max(x1,0);
	y1=max(y1,0);
	x2=min(x2,nx-1);
	y2=min(y2,ny-1);

	for(i=y1;i<=y2;i++)
	{
		for(j=x1;j<=x2;j++)
		{
			index = j+i*nx;
			m_point_list[index].z = m_Heights[index];
		}
	}

	UpdateNormals(x1,y1,x2,y2);
	UpdateBoundingVolumes(m_pTree,x1,y1,x2,y2);
	UpdateColors(m_pTree,x1,y1,x2,y2);

#endif

}

void ASSMap::UpdateNormals(int x1,int y1,int x2,int y2)
{
	int i,j,k,index;
	for(i=y1;i<y2;i++)
	{
		for(j=x1;j<x2;j++)
		{
			index=j+i*nx;
			ASSCell *pCell=GetCell(index);
			for(k=0;k<2;k++)
			{
				rvector c1,c2,c3;
				c1=GetVector(pCell->pind[0+3*k]);
				c2=GetVector(pCell->pind[1+3*k]);
				c3=GetVector(pCell->pind[2+3*k]);

				SetupPlane(c1,c2,c3,&pCell->planes[k]);
			}
			/*
			CalcNormal(&m_Cells[index].faces[0]);
			CalcNormal(&m_Cells[index].faces[1]);
			*/
		}
	}

//	rvertex *ver;
//	rface *face;
	for(i=y1;i<=y2;i++)
	{
		for(j=x1;j<=x2;j++)
		{
			index=j+i*nx;
			rvector *pnormal=&(GetCell(index)->normal);
			*pnormal=rvector(0,0,0);

			ASSCell *pCell;

			if(GetCase(j,i))
			{
#define ADD_NORMAL(x,y,ind) { pCell=GetCell(x,y);if(pCell) (*pnormal)+=pCell->planes[ind].normal; }
				ADD_NORMAL(j-1,i-1,1);
				ADD_NORMAL(j,i-1,0);
				ADD_NORMAL(j-1,i,1);
				ADD_NORMAL(j,i,0);
			}
			else
			{
				ADD_NORMAL(j-1,i-1,0);
				ADD_NORMAL(j-1,i-1,1);
				ADD_NORMAL(j,i-1,0);
				ADD_NORMAL(j,i-1,1);
				ADD_NORMAL(j-1,i,0);
				ADD_NORMAL(j-1,i,1);
				ADD_NORMAL(j,i,0);
				ADD_NORMAL(j,i,1);
			}
			pnormal->Normalize();
			CalcColor(index);
		}
	}
}

void ASSMap::UpdateBoundingVolumes(ASSVolumeTreeNode *node,int x1,int y1,int x2,int y2)
{
	if(node->m_pCell)
	{
		ASSCell *cell=node->m_pCell;

		// recalculate Bounding Box
		node->m_bbTree.m[2][0]=min(GetHeight(node->ind[0]),min(GetHeight(node->ind[1]),
								min(GetHeight(node->ind[2]),GetHeight(node->ind[5]))));
		node->m_bbTree.m[2][1]=max(GetHeight(node->ind[0]),max(GetHeight(node->ind[1]),
								max(GetHeight(node->ind[2]),GetHeight(node->ind[5]))));
		node->m_Center=rvector((node->m_bbTree.Maxx+node->m_bbTree.Minx)*.5f,
								(node->m_bbTree.Maxy+node->m_bbTree.Miny)*.5f,
								(node->m_bbTree.Maxz+node->m_bbTree.Minz)*.5f);
		node->m_fVariance=node->m_bbTree.Maxz-node->m_bbTree.Minz;
		node->m_fRadius=sqrtf(.5f+.25f*node->m_fVariance*node->m_fVariance);
	}
	else
	{
		if((max(node->m_bbTree.m[0][0],x1)<min(node->m_bbTree.m[0][1],x2))&&
			(max(node->m_bbTree.m[1][0],y1)<min(node->m_bbTree.m[1][1],y2)))
		{
			int i;
			for(i=0;i<4;i++)
			{
				if(node->m_nChildren[i])
				{
					UpdateBoundingVolumes(m_pTree+node->m_nChildren[i],x1,y1,x2,y2);
					_ASSERT(node->m_bbTree.Minx<=(m_pTree+node->m_nChildren[i])->m_bbTree.Minx);
					_ASSERT(node->m_bbTree.Maxx>=(m_pTree+node->m_nChildren[i])->m_bbTree.Maxx);
					_ASSERT(node->m_bbTree.Miny<=(m_pTree+node->m_nChildren[i])->m_bbTree.Miny);
					_ASSERT(node->m_bbTree.Maxy>=(m_pTree+node->m_nChildren[i])->m_bbTree.Maxy);
				}
			}
			node->m_bbTree=m_pTree[node->m_nChildren[0]].m_bbTree;
			for(i=1;i<4;i++)
			{
				if(node->m_nChildren[i])
					MergeBoundingBox(&node->m_bbTree,&(m_pTree[node->m_nChildren[i]].m_bbTree));
			}
			node->m_Center=rvector((node->m_bbTree.Maxx+node->m_bbTree.Minx)*.5f,
									(node->m_bbTree.Maxy+node->m_bbTree.Miny)*.5f,
									(node->m_bbTree.Maxz+node->m_bbTree.Minz)*.5f);
			node->m_fVariance=node->m_bbTree.Maxz-node->m_bbTree.Minz;
#define SQR(x) ((x)*(x))
			node->m_fRadius=sqrtf(.25f*(
				SQR(node->m_bbTree.Maxx-node->m_bbTree.Minx)+
				SQR(node->m_bbTree.Maxy-node->m_bbTree.Miny)+
				+SQR(node->m_fVariance)));
		}
	}
}

void ASSMap::SetTileSet(int x,int y,int nTileSet)
{
	m_Cells[y*nx+x].m_nTileSet=nTileSet;
}

void ASSMap::UpdateMapping_Recurse(ASSVolumeTreeNode *node,int x1,int y1,int x2,int y2)
{
	if(node->m_pCell)
	{
		ASSCell *cell=node->m_pCell;
		node->nRefTexture=cell->m_nTextureIndex*4+cell->m_nTextureDir;
	}
	else
	{
		if((max(node->m_bbTree.m[0][0],x1)<min(node->m_bbTree.m[0][1],x2))&&
			(max(node->m_bbTree.m[1][0],y1)<min(node->m_bbTree.m[1][1],y2)))
		{
			int i;
			for(i=0;i<4;i++)
			{
				if(node->m_nChildren[i])
					UpdateMapping_Recurse(m_pTree+node->m_nChildren[i],x1,y1,x2,y2);
			}
		}
	}
}

void ASSMap::UpdateMapping(int x1,int y1,int x2,int y2)
{
	static int tilestyletable[16] = { -1,1,1,2,1,4,2,3,1,2,4,3,2,3,3,0 };
	static int tiledirtable[16] = { 0,3,2,2,1,1,1,2,0,3,0,3,0,0,1,0 };

	x1=max(x1,0);y1=max(y1,0);
	x2=min(x2,nx-1);y2=min(y2,ny-1);

	int i,j,k,index;
	int nts[4],nts2[4];
	ASSCell *cell;

	for(i=y1;i<y2;i++)
		for(j=x1;j<x2;j++)
		{
			index=i*nx+j;
			cell= &m_Cells[index];

			nts[0]=cell->m_nTileSet;
			nts[1]=(cell+1)->m_nTileSet;
			nts[2]=(cell+nx+1)->m_nTileSet;
			nts[3]=(cell+nx)->m_nTileSet;
			
			int nTargetTS=-1;
			for(k=0;k<4;k++)
			{
				nts2[k]=(nts[k]!=0);
				if(nts[k])
				{
					if(nTargetTS==-1)
						nTargetTS=nts[k];
					else
						if(nTargetTS!=nts[k])
						{
							cell->m_nTileSet=(cell+1)->m_nTileSet=
							(cell+nx+1)->m_nTileSet=(cell+nx)->m_nTileSet=0;
							nts2[0]=nts2[1]=nts2[2]=nts2[3]=0;
							UpdateMapping(j-1,i-1,j+2,i+2);
							break;
						}
				}
			}
			// tileset 종류를 하나로 만들었고..

			int nCode=(nts2[0]<<3)+(nts2[1]<<2)+(nts2[2]<<1)+nts2[3];
			int nStyle=tilestyletable[nCode],nDir;
			if((nStyle==-1)||(nStyle==0)) 
				nDir=rand()%4;						// 임의의 방향으로 돌림
			else nDir=tiledirtable[nCode];
			cell->m_nTextureDir=nDir;
			int hTexture;
			
			if(nStyle==-1)
			{
				int nCount=m_pSchemeManager->GetBaseCount();
				int t=rand()%nCount;
				cell->m_nTextureIndex=m_pSchemeManager->GetBaseTile(t)->nTextureIndex;
				hTexture=m_pSchemeManager->GetBaseTile(t)->nTextureHandle[nDir];
			}
			else
			{
				int nCount=m_pSchemeManager->Get(nTargetTS-1)->Get(nStyle)->GetCount();
				int t=rand()%nCount;
				cell->m_nTextureIndex=
					m_pSchemeManager->Get(nTargetTS-1)->Get(nStyle)->Get(t)->nTextureIndex;
				_ASSERT(cell->m_nTextureIndex);

				int newdir=nDir;
				int ufliptable[5][4]={{0,0,0,0},{1,0,3,2},{0,3,2,1},{1,0,3,2},{1,0,3,2}};
				int vfliptable[5][4]={{0,0,0,0},{3,2,1,0},{2,1,0,3},{3,2,1,0},{1,0,3,2}};
				if(j%2) newdir=ufliptable[nStyle][newdir];
				if(i%2) newdir=vfliptable[nStyle][newdir];

				hTexture=m_pSchemeManager->Get(nTargetTS-1)->Get(nStyle)->Get(t)->nTextureHandle[newdir];
				cell->m_nTextureDir=newdir;
			}
		}
	if(m_pTree)
		UpdateMapping_Recurse(m_pTree,x1,y1,x2,y2);
}

void ASSMap::SetFOW(int x,int y,float intensity)
{
	if((x<0)||(y<0)||(x>nx-1)||(y>ny-1)) return;
	m_Cells[y*nx+x].m_fFow=intensity;
//	CalcColor(&m_Cells[y*nx+x]);
}

float ASSMap::GetFOW(float x,float y)
{
	if((x<0)||(y<0)||(x>=nx-1)||(y>=ny-1))
		return 1.0f;
	int index=(int)x+(int)y*nx;
//	rface *targetface=NULL;
	rvector normal,c1,c2,c3;

	WORD *ind;
	switch(GetCase(x,y))
	{
#define SETC123(a,b,c) { c1.z=m_Cells[a].m_fFow;c2.z=m_Cells[b].m_fFow;c3.z=m_Cells[c].m_fFow; }
	case 0:
//			targetface=&m_Cells[index].faces[0];
			ind=GetCell(x,y)->pind;
			SETC123(index,index+1,index+nx);
			break;
	case 1:
			ind=GetCell(x,y)->pind+3;
//			targetface=&m_Cells[index].faces[1];
			SETC123(index+nx+1,index+nx,index+1);
			break;
	case 2:
			ind=GetCell(x,y)->pind;
//			targetface=&m_Cells[index].faces[0];
			SETC123(index+nx,index,index+nx+1);
			break;
	case 3:
			ind=GetCell(x,y)->pind+3;
//			targetface=&m_Cells[index].faces[1];
			SETC123(index+1,index+nx+1,index);
			break;
	}


	rvector targetv0=GetVector(ind[0]),targetv1=GetVector(ind[1]),targetv2=GetVector(ind[3]);

	c1.x=targetv0.x;c1.y=targetv0.y;
	c2.x=targetv1.x;c2.y=targetv1.y;
	c3.x=targetv2.x;c3.y=targetv2.y;

	normal=Normalize(CrossProduct(c2-c1,c1-c3));
	return
		(((targetv0.x-x)*normal.x)
		+(targetv0.y-y)*normal.y)/normal.z
		+c1.z;	

}
/*
bool ASSMap::IsInScreen(int x,int y)
{
	if((x<0)||(y<0)||(x>=nx-1)||(y>=ny-1))
		return 0;
	else
		return m_Cells[y*nx+x].m_nDrawedFrame==RSFrameCount;
}
*/

void ASSMap::DrawLightMap(rface *f,RSPointLight *light)
{
/*
	rvector zero=rvector(0,0,0);
	_ASSERT(!IS_EQ3(f->v[0]->normal,zero));
	_ASSERT(!IS_EQ3(f->v[1]->normal,zero));
	_ASSERT(!IS_EQ3(f->v[2]->normal,zero));
	_ASSERT(!IS_EQ3(f->normal,zero));

#define distance(v) float(sqrt((((v).px)*((v).px)+((v).py)*((v).py)+((v).pz)*((v).pz))))
	static rmatrix mm;
	static float s[3],t[3],sf;
	float r;

	rvector p=rvector(f->v[0]->x,f->v[0]->y,f->v[0]->z)-light->mPosition;
	float d=-f->normal.x*f->v[0]->x-f->normal.y*f->v[0]->y-f->normal.z*f->v[0]->z;
	float dist=(float)fabs(d+DotProduct(light->mPosition,f->normal));
	if(dist>light->mRange) return ;
	r=1/(float)sqrt(light->mRange*light->mRange-dist*dist);
	mm=ViewMatrix(light->mPosition,f->normal);
	for(int i=0;i<3;i++)
	{
		TransformVertexP(*f->v[i],mm);
		float intensity=max((light->mRange-distance(*f->v[i])),0)/light->mRange;
		f->v[i]->u=min(max(f->v[i]->px*r*0.5f+0.5f,0),1.0f);
		f->v[i]->v=min(max(f->v[i]->py*r*0.5f+0.5f,0),1.0f);
		f->v[i]->color=FLOAT2RGB24(intensity*light->mColor.x,intensity*light->mColor.y,intensity*light->mColor.z);
		f->v[i]->specular=0xff000000;
	}
	RSDrawTriangle(f->v[0],f->v[1],f->v[2]);
*/
}

void ASSMap::DrawLightMap(RSPointLight *light)
{
	/*
	static int i,j,k;
	static int x1,y1,x2,y2;
	static rface *f;
	x1=(int)max(0,light->mPosition.x-light->mRange);
	x2=(int)min(nx-1,light->mPosition.x+light->mRange+1);
	y1=(int)max(0,light->mPosition.y-light->mRange);
	y2=(int)min(ny-1,light->mPosition.y+light->mRange+1);
	ASSCell *cell;

	for(i=y1;i<y2;i++)
	{
		for(j=x1;j<x2;j++)
		{
			if(IsInScreen(j,i))		// 이건 필요없을듯한데 ... 
			{
				cell=&m_Cells[i*nx+j];
				TransformVertexASM(*cell->faces[0].v[0],RSViewProjectionViewport);
				TransformVertexASM(*cell->faces[0].v[1],RSViewProjectionViewport);
				TransformVertexASM(*cell->faces[0].v[2],RSViewProjectionViewport);
				TransformVertexASM(*cell->faces[1].v[0],RSViewProjectionViewport);
				DrawLightMap(&cell->faces[0],light);
				DrawLightMap(&cell->faces[1],light);
			}
		}
	}
	*/
}

void ASSMap::DrawBlackMark(float xPos,float yPos,float Radius,DWORD color,DWORD hTexture)
{
	static int i,j,k;
	static int x1,y1,x2,y2;
	static rface *f;
	x1=(int)max(0,xPos-Radius);
	x2=(int)min(nx-1,xPos+Radius+1);
	y1=(int)max(0,yPos-Radius);
	y2=(int)min(ny-1,yPos+Radius+1);
	ASSCell *cell;

	RSSetAlphaState(RS_ALPHASTYLE_MARKS);
	RSSetTexture(hTexture);
	RSSetTextureWrapState(false);

	rmatrix trm;


	rvector dir=rvector(0,0,-1);
	rvector right=Normalize(CrossProduct(dir,rvector(1,0,0)));
	rvector up=Normalize(CrossProduct(dir,right));

	rvector wpos=rvector(xPos,yPos,0);

#define DEFAULT_SCALE_FACTOR	1.2f

	float fScaleX=DEFAULT_SCALE_FACTOR/Radius;
	float fScaleY=fScaleX;

	trm._11 = right.x*fScaleX;
	trm._21 = right.y*fScaleX;
	trm._31 = right.z*fScaleX;
	trm._12 = up.x*fScaleY;
	trm._22 = up.y*fScaleY;
	trm._32 = up.z*fScaleY;
	trm._13 = dir.x;
	trm._23 = dir.y;
	trm._33 = dir.z;
	
	trm._41 = -DotProduct(right, wpos)*fScaleX+.5f;
	trm._42 = -DotProduct(up, wpos)*fScaleY+.5f;	// for adjust uv coord.
	trm._43 = -DotProduct(dir, wpos);

	g_pd3dDevice->SetVertexShader(RSLFVF);
	
#ifndef EL_TEST

	for(i=y1;i<y2;i++)
	{
		for(j=x1;j<x2;j++)
		{
			cell=&m_Cells[i*nx+j];

			pVertexBuffer->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ;

			LITVERTEX verts[4];
			memcpy(verts+0,pVertices + cell->pind[0],sizeof(LITVERTEX));
			memcpy(verts+1,pVertices + cell->pind[1],sizeof(LITVERTEX));
			memcpy(verts+2,pVertices + cell->pind[2],sizeof(LITVERTEX));
			memcpy(verts+3,pVertices + cell->pind[5],sizeof(LITVERTEX));

			for(k=0;k<4;k++)
			{
				rvector uvs=TransformVector(*((rvector*)&verts[k].x),trm);
				verts[k].tu1=uvs.x;
				verts[k].tv1=uvs.y;
				verts[k].Diffuse=color;
			}

			pVertexBuffer->Unlock();

			g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,verts,sizeof(LITVERTEX));
		}
	}

#else
	// 임시 모아 찍기 구현
/*
	MVERTEX* ver;
	int cnt = 0;

	static MVERTEX t_vertex[10000];

	for(i=0;i<nQueueCounts[nRefTexture];i++)
	{
		ASSVolumeTreeNode *pNode=m_pQueues[nRefTexture][i];

		ver = (t_vertex + (cnt*6));

		memcpy(ver  ,m_point_list+pNode->ind[0],sizeof(MVERTEX));
		memcpy(ver+1,m_point_list+pNode->ind[1],sizeof(MVERTEX));
		memcpy(ver+2,m_point_list+pNode->ind[2],sizeof(MVERTEX));
		memcpy(ver+3,m_point_list+pNode->ind[3],sizeof(MVERTEX));
		memcpy(ver+4,m_point_list+pNode->ind[4],sizeof(MVERTEX));
		memcpy(ver+5,m_point_list+pNode->ind[5],sizeof(MVERTEX));
		cnt++;

		if(cnt*6 > 9900)
		{
			g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST,cnt*2,t_vertex,sizeof(MVERTEX));
			cnt = 0;
		}
	}

	g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST,cnt*2,t_vertex,sizeof(MVERTEX));

*/
	for(i=y1;i<y2;i++)
	{
		for(j=x1;j<x2;j++)
		{
			cell=&m_Cells[i*nx+j];

			LITVERTEX verts[4];

			memcpy(verts+0,m_point_list + cell->pind[0],sizeof(LITVERTEX));
			memcpy(verts+1,m_point_list + cell->pind[1],sizeof(LITVERTEX));
			memcpy(verts+2,m_point_list + cell->pind[2],sizeof(LITVERTEX));
			memcpy(verts+3,m_point_list + cell->pind[5],sizeof(LITVERTEX));

			for(k=0;k<4;k++)
			{
				rvector uvs=TransformVector(*((rvector*)&verts[k].x),trm);
				verts[k].tu1=uvs.x;
				verts[k].tv1=uvs.y;
				verts[k].Diffuse=color;
			}

			g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,verts,sizeof(LITVERTEX));
		}
	}


#endif

	RSSetTexture(NULL);
	RSSetAlphaState(RS_ALPHASTYLE_NONE);
}

void ASSMap::SetConstantFOW( float intensity )
{
	int cnt = nx * ny;
	for( int i = 0; i < nx*ny; i++ ){
		m_Cells[i].m_fFow = intensity;
//		CalcColor(&m_Cells[i]);
	}
}

static RSCellList *g_CellList_for_Get;
static rplane	*g_Planes_for_Get;
static int	g_nPlanes_for_Get;

void ASSMap::GetCellListWRTPlanes(RSCellList *fl,rplane *planes,int nplanes)
{
	g_CellList_for_Get=fl;
	g_Planes_for_Get=planes;
	g_nPlanes_for_Get=nplanes;
	TraverseTreeAndChoose(m_pTree);
}

void ASSMap::TraverseTreeAndChoose(ASSVolumeTreeNode *node)
{
	if(!isInViewFrustrumwrtnPlanes(&node->m_bbTree,g_Planes_for_Get,g_nPlanes_for_Get)) return;
	if(node->m_pCell)
	{
		g_CellList_for_Get->Add(node->m_pCell);
	}
	else
	{
		for(int i=0;i<4;i++)
		{
			if(node->m_nChildren[i])
				TraverseTreeAndChoose(m_pTree+node->m_nChildren[i]);
		}

	}
}

void ASSMap::SetLODState(bool bEnable,float fLODBias)
{
	m_bLODEnable=bEnable;
	if(m_LODTable) delete []m_LODTable;
	m_LODTable=new float[m_nTreeDepth];
	for(int i=0;i<m_nTreeDepth;i++)
	{
		m_LODTable[i]=(float)(30.f*fLODBias*pow(1.5,(m_nTreeDepth-i-1)));
	}
}

void ASSMap::ProcessVertexLight(RSPointLight *light)
{
	if(light->GetPosition().GetDistance(RSCameraPosition)-light->GetRange()>m_FogEndZ)
		return;

	static int i,j,k;
	static int x1,y1,x2,y2;
	static rface *f;
	x1=(int)max(0,light->mPosition.x-light->mRange);
	x2=(int)min(nx,light->mPosition.x+light->mRange+1);
	y1=(int)max(0,light->mPosition.y-light->mRange);
	y2=(int)min(ny,light->mPosition.y+light->mRange+1);
	ASSCell *cell;

	for(i=y1;i<y2;i++)
	{
		for(j=x1;j<x2;j++)
		{
			cell=m_Cells+i*nx+j;
			float dist=(rvector((float)j,(float)i,GetHeight(j,i))-light->GetPosition()).GetMagnitude();
			float intensity=max(min(1.0f-dist/light->GetRange(),1.0f),0);
			if(cell->m_nLitFrame==RSFrameCount)
				cell->m_color+=intensity*light->GetColor();
			else
			{
				cell->m_color=intensity*light->GetColor();
//				cell->m_color+=intensity*light->GetColor();
				cell->m_nLitFrame=RSFrameCount;
			}
		}
	}
}

void ASSMap::Merge(int index)
{

#ifndef EL_TEST

	if(index>=m_nTreeNode) return;
	ASSVolumeTreeNode *pNode=m_pTree+index;
	int i;
	pNode->m_bExpanded=false;

	ASSLODInfo *li=pNode->m_pLODInfo;

	if(li)
	{
		for(i=0;i<4;i++)
		{
/*
			ASSCell *pc=&m_Cells[li->CrackPoint[i]];
			pc->v.z=
				(m_Heights[li->pCells[i]-m_Cells]+
				m_Heights[li->pCells[(i+1)%4]-m_Cells])/2.f;
*/
			if(li->CrackPoint[i])
			{
				pVertices[li->CrackPoint[i]+nx*ny].z=
				pVertices[li->CrackPoint[i]].z=
					(m_Heights[li->pCells[i]-m_Cells]+
					m_Heights[li->pCells[(i+1)%4]-m_Cells])/2.f;
			}
		}
	}

#else

	if(index>=m_nTreeNode) return;

	ASSVolumeTreeNode *pNode = m_pTree+index;

	int i;

	pNode->m_bExpanded = false;

	ASSLODInfo *li = pNode->m_pLODInfo;

	if(li)
	{
		for(i=0;i<4;i++)
		{
			m_point_list[li->CrackPoint[i]+nx*ny].z=
			m_point_list[li->CrackPoint[i]].z=	
				(m_Heights[li->pCells[i]-m_Cells] + m_Heights[li->pCells[(i+1)%4]-m_Cells])/2.f;
		}
	}

#endif

}

/*
ASSVolumeTreeNode *ASSMap::GetAdjacency(int index,int nAdj)
{
	if((index==0)||(index>=m_nTreeNode)) return NULL;
	ASSVolumeTreeNode *pNode=m_pTree+index;
	int nLevel=pNode->nLevel;
//		GetLevel(index);
	int nWidth=1<<nLevel;
	int nLevelBase=((1<<(nLevel*2))-1)/3;
	int nx=(index-nLevelBase) % nWidth;
	int ny=(index-nLevelBase) / nWidth;

	int s[4][2]={{0,-1},{1,0},{0,1},{-1,0}};
	int tx=nx+s[nAdj][0],ty=ny+s[nAdj][1];
	if(tx<0 || ty<0 || tx>=nWidth || ty>=nWidth)
		return NULL;
	return m_pTree+nLevelBase+nWidth*ty+tx;
}
*/

void ASSMap::Split(int index)
{
#ifndef EL_TEST

	if(index>=m_nTreeNode) return;
	ASSVolumeTreeNode *pNode=m_pTree+index;

	int i;
	ASSLODInfo *lodinfo=pNode->m_pLODInfo;
	if(!lodinfo) return;

	for(i=0;i<4;i++)
	{
		if(pNode->m_nChildren[i])
			Merge(pNode->m_nChildren[i]);
	}

	int *k=lodinfo->CrackPoint;
	for(i=0;i<4;i++)
	{
		if(!(lodinfo->m_pAdjacencies[i]) || lodinfo->m_pAdjacencies[i]->m_bExpanded)
			pVertices[*k+nx*ny].z=pVertices[*k].z=m_Heights[*k];
		k++;
	}
	pNode->m_bExpanded=true;

#else
	
	if(index>=m_nTreeNode) return;

	ASSVolumeTreeNode *pNode = m_pTree+index;

	int i;

	if(!(pNode->m_pLODInfo)) return;

	for(i=0;i<4;i++)
	{
		if(pNode->m_nChildren[i])
			Merge(GetChild(index,i));
	}

	int* k = pNode->m_pLODInfo->CrackPoint;

	for(i=0;i<4;i++)
	{
		ASSVolumeTreeNode *pAdj = GetAdjacency(index,i);

		if(!pAdj || pAdj->m_bExpanded)
			m_point_list[*k+nx*ny].z = m_point_list[*k].z = m_Heights[*k];

		k++;
	}

	pNode->m_bExpanded=true;

#endif

}

void ASSMap::Draw(int index)
{
//	_ASSERT(index!=18);
	if(index>=m_nTreeNode) return;
	ASSVolumeTreeNode *pNode=m_pTree+index;
	float mindist=GetDistance(&pNode->m_bbTree,RSViewFrustrum+4);
	// mindist is distance bounding box between near z plane

	if(pNode->m_pCell ||
//		GetLevel(index)==m_nTreeDepth-1 ||
		(m_bLODEnable&&(mindist>m_LODTable[pNode->nLevel])))		// 이 레벨을 그려야한다.
	{
		if(pNode->m_bExpanded)		// 합쳐줘야한다. 또 crackpoints 를 복구해야 한다.
			Merge(index);
//		Draw(pNode);
		queue[nQ++]=pNode;
		_ASSERT(nQ<QUEUECOUNT);
	}
	else
	{
		if(!pNode->m_bExpanded)		// 쪼개주자.
			Split(index);
		if(!pNode->m_pCell)
		{
			for(int i=0;i<4;i++)
			{
				int iChild=pNode->m_nChildren[i];
				if(iChild && (pNode->m_bAllIn || UpdateInOutStates(pNode,m_pTree+iChild)))
					Draw(iChild);
			}
		}
	}
}

void ASSMap::Draw()
{
	_BP("Choose polygons");
	
#ifndef EL_TEST

	pVertexBuffer->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ;

	g_pd3dDevice->SetVertexShader(RSLFVF);
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, false );		//*/
	g_pd3dDevice->SetTransform(D3DTS_VIEW, (_D3DMATRIX*)&RSView);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION,(_D3DMATRIX*)&RSProjection);
	g_pd3dDevice->SetTransform(D3DTS_WORLD, (_D3DMATRIX*)&IdentityMatrix44());
	RSSetZBufferState(true,true);
	RSSetTextureWrapState(false);
	RSSetCullState(RS_CULLSTYLE_CCW);
	RSSetAlphaState(RS_ALPHASTYLE_NONE);
	RSSetTexture(0);

	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	if(m_hPattern)
	{
		RSSetTexture(1,m_hPattern);
	
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

		g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
		g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
		g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

		g_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		g_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP,   D3DTOP_MODULATE);
		g_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );
		g_pd3dDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
		
		g_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX , 1);
	}
	
	int i;
	for(i=0;i<6;i++)
		m_pTree->states[i]=IS_DONTKNOW;

	m_pTree->m_bAllIn=false;
	
	nQ=0;
	Draw(0);
	pVertexBuffer->Unlock();

	_EP("Choose polygons");

	_BP("Draw polygons");

	for(i=0;i<nQ;i++)
		Draw(queue[i]);

	for(i=0;i<nQueueCount;i++)
	{
		if(nQueueCounts[i])
			Flush(i);
	}

	_EP("Draw polygons");

	if(m_hPattern)
	{
		RSSetTexture(1,0);
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
		g_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	}

#else

	g_pd3dDevice->SetVertexShader(RSLFVF);
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, false );		//*/
	g_pd3dDevice->SetTransform(D3DTS_VIEW, (_D3DMATRIX*)&RSView);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION,(_D3DMATRIX*)&RSProjection);
	g_pd3dDevice->SetTransform(D3DTS_WORLD, (_D3DMATRIX*)&IdentityMatrix44());

	RSSetZBufferState(true,true);
	RSSetTextureWrapState(false);
	RSSetCullState(RS_CULLSTYLE_CCW);
	RSSetAlphaState(RS_ALPHASTYLE_NONE);

	RSSetTexture(0);

	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	if(m_hPattern)
	{
		RSSetTexture(1,m_hPattern);
	
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

		g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
		g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
		g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

		g_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		g_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP,   D3DTOP_MODULATE);
		g_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );
		g_pd3dDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
		
		g_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX , 1);
	}
	
	int i;
	for(i=0;i<6;i++)
		m_pTree->states[i]=IS_DONTKNOW;
	m_pTree->m_bAllIn=false;
	
	nQ=0;

	Draw(0);

	_EP("Choose polygons");

	_BP("Draw polygons");

	for(i=0;i<nQ;i++)
		Draw(queue[i]);

	for(i=0;i<nQueueCount;i++)
	{
		if(nQueueCounts[i])
			Flush(i);
	}

	_EP("Draw polygons");

	if(m_hPattern)
	{
		RSSetTexture(1,0);
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
		g_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	}

#endif

}

void ASSMap::DrawLine()
{
	g_pd3dDevice->SetRenderState( D3DRS_FILLMODE ,  D3DFILL_WIREFRAME );
	Draw();
	g_pd3dDevice->SetRenderState( D3DRS_FILLMODE ,  D3DFILL_SOLID );
}


void ASSMap::UpdateColors(ASSVolumeTreeNode *node,int x1,int y1,int x2,int y2)
{
	ASSLODInfo *li=node->m_pLODInfo;
	if(!li)
	{
		CalcColor(node->m_pCell-m_Cells);
	} else
	{
		if((max(node->m_bbTree.m[0][0],x1)<min(node->m_bbTree.m[0][1],x2))&&
			(max(node->m_bbTree.m[1][0],y1)<min(node->m_bbTree.m[1][1],y2)))
		{
			int i;
			for(i=0;i<4;i++)
			{
				if(node->m_nChildren[i])
					UpdateColors(m_pTree+node->m_nChildren[i],x1,y1,x2,y2);
			}
		}
	}
}


void ASSMap::Draw(ASSVolumeTreeNode *pNode)
{
	if(pNode->m_pCell)
	{
		Transform(pNode->ind[0]);
		Transform(pNode->ind[1]);
		Transform(pNode->ind[2]);
		Transform(pNode->ind[5]);
	}else
	{
		Transform(pNode->ind[0]-nx*ny);
		Transform(pNode->ind[1]-nx*ny);
		Transform(pNode->ind[2]-nx*ny);
		Transform(pNode->ind[5]-nx*ny);
	}

	Enque(pNode);
}

static int g_nLevelBase[10]={0,};

void ASSMap::ConstructVolumeTree()
{
	m_nTreeNode=0;m_nTreeDepth=-1;
	int x=nx-1,y=ny-1;
	int levelx,levely,i,j;
	while(x>1 || y>1)
	{
		levelx=(nx-1)/x;
		levely=(ny-1)/y;
		m_nTreeNode+=levelx*levely;
		if(x>1) x>>=1;
		if(y>1) y>>=1;
		m_nTreeDepth++;
		g_nLevelBase[m_nTreeDepth+1]=m_nTreeNode;
	}

	m_nTreeDepth++;
	m_nTreeNode+=(nx-1)*(ny-1);
	g_nLevelBase[m_nTreeDepth+1]=m_nTreeNode;
	m_pTree=new ASSVolumeTreeNode[m_nTreeNode];
//	ConstructVolumeTree(0,0,0,nx-2,ny-2);

	x=nx-1;
	y=ny-1;

//{{0,-1},{1,0},{0,1},{-1,0}};
	int level=0;
	while(x>1 || y>1)
	{
		int nextlevelbase=g_nLevelBase[level+1];
		levelx=(nx-1)/x;
		levely=(ny-1)/y;

		for(i=0;i<levely;i++)
		{
			for(j=0;j<levelx;j++)
			{
#define CELL(xx,yy)  (m_pTree+g_nLevelBase[level]+(yy)*levelx+xx)
				ASSVolumeTreeNode *node=CELL(j,i);

				if(x==1)
				{
					node->m_nChildren[0]=nextlevelbase+i*2*levelx+j;
					node->m_nChildren[1]=nextlevelbase+(i*2+1)*levelx+j;
					node->m_nChildren[2]=0;
					node->m_nChildren[3]=0;
				}else
				if(y==1)
				{
					node->m_nChildren[0]=nextlevelbase+i*levelx*2+j*2;
					node->m_nChildren[1]=nextlevelbase+i*levelx*2+j*2+1;
					node->m_nChildren[2]=0;
					node->m_nChildren[3]=0;
				}else
				{
					node->m_nChildren[0]=nextlevelbase+(i*2)*levelx*2+j*2;
					node->m_nChildren[1]=nextlevelbase+(i*2)*levelx*2+j*2+1;
					node->m_nChildren[2]=nextlevelbase+(i*2+1)*levelx*2+j*2;
					node->m_nChildren[3]=nextlevelbase+(i*2+1)*levelx*2+j*2+1;
				}

				
				int x1=j*x,y1=i*y,x2=(j+1)*x,y2=(i+1)*y;
				
				node->m_bbTree.m[0][0]=(float)x1;node->m_bbTree.m[0][1]=(float)x2;
				node->m_bbTree.m[1][0]=(float)y1;node->m_bbTree.m[1][1]=(float)y2;
				node->m_bbTree.m[2][0]=FLT_MAX;
				node->m_bbTree.m[2][1]=FLT_MIN;

				// generate LOD Infomation
				ASSLODInfo *lodinfo=node->m_pLODInfo=new ASSLODInfo;
				lodinfo->pCells[0]=&m_Cells[y1*nx+x1];
				lodinfo->pCells[1]=&m_Cells[y1*nx+x2];
				lodinfo->pCells[2]=&m_Cells[y2*nx+x2];
				lodinfo->pCells[3]=&m_Cells[y2*nx+x1];

				lodinfo->m_pAdjacencies[0]=NULL;
				lodinfo->m_pAdjacencies[1]=NULL;
				lodinfo->m_pAdjacencies[2]=NULL;
				lodinfo->m_pAdjacencies[3]=NULL;
				
				// create Adjacency table
				if(x>1)
				{
					if(i>0)				// 위쪽
						node->m_pLODInfo->m_pAdjacencies[0]=CELL(j,i-1);
					if(i<levely-1)		// 아래쪽
						node->m_pLODInfo->m_pAdjacencies[2]=CELL(j,i+1);
				}
				if(y>1)
				{
					if(j>0)				// 왼쪽
						node->m_pLODInfo->m_pAdjacencies[3]=CELL(j-1,i);
					if(j<levelx-1)		// 오른쪽
						node->m_pLODInfo->m_pAdjacencies[1]=CELL(j+1,i);
				}

				int xm=(x1+x2)/2,ym=(y1+y2)/2;
				
				if(x>1)
				{
					lodinfo->CrackPoint[0]=y1*nx+xm;
					lodinfo->CrackPoint[2]=y2*nx+xm;
				}else
					lodinfo->CrackPoint[0]=lodinfo->CrackPoint[2]=0;

				if(y>1)
				{
					lodinfo->CrackPoint[1]=ym*nx+x2;
					lodinfo->CrackPoint[3]=ym*nx+x1;
				}else
					lodinfo->CrackPoint[1]=lodinfo->CrackPoint[3]=0;
			
				int test[6];

				test[0]=(y2*nx+x1+nx*ny);
				test[1]=(y1*nx+x1+nx*ny);
				test[2]=(y2*nx+x2+nx*ny);
				test[3]=(y2*nx+x2+nx*ny);
				test[4]=(y1*nx+x1+nx*ny);
				test[5]=(y1*nx+x2+nx*ny);
				for(int qqqq=0;qqqq<6;qqqq++)
					node->ind[qqqq]=(WORD)test[qqqq];
				// 이거 컴파일러 버그라고 생각됨. node->ind[0~5] 에 직접 값을 써넣으면 release / debug 값이 틀림.

				node->nLevel=level;
				node->nRefTexture=m_LodTextures?
						m_nSchemeTextures+(y1*m_nlodcell_texsize/TEXTURE_PARTITION_SIZE)*m_nPatches_x
						+(x1*m_nlodcell_texsize/TEXTURE_PARTITION_SIZE)
					:nQueueCount-1;

			}
		}

		if(x>1) x>>=1;
		if(y>1) y>>=1;
		level++;
	}

	x=nx-1;
	y=ny-1;
	levelx=nx-1;
	for(i=0;i<y;i++)
		for(j=0;j<x;j++)
		{
			ASSVolumeTreeNode *node=CELL(j,i);
			int x1=j,y1=i;//x2=(j+1),y2=(i+1);
			ASSCell *cell=&m_Cells[y1*nx+x1];
			cell->pind=node->ind;
			node->m_pCell=cell;
			node->nRefTexture=cell->m_nTextureIndex*4+cell->m_nTextureDir;
			if(node->nRefTexture>=m_nSchemeTextures)
			{
//			_ASSERT(0);			// maybe scheme not match
			node->nRefTexture=nQueueCount-1;
			}

			// recalculate Bounding Box
			node->m_bbTree.m[0][0]=float(x1);node->m_bbTree.m[0][1]=float(x1+1);
			node->m_bbTree.m[1][0]=float(y1);node->m_bbTree.m[1][1]=float(y1+1);
			node->m_bbTree.m[2][0]=min(GetHeight(x1,y1),min(GetHeight(x1+1,y1),
									min(GetHeight(x1,y1+1),GetHeight(x1+1,y1+1))));
			node->m_bbTree.m[2][1]=max(GetHeight(x1,y1),max(GetHeight(x1+1,y1),
									max(GetHeight(x1,y1+1),GetHeight(x1+1,y1+1))));

			node->m_nChildren[0]=0;
			node->m_nChildren[1]=0;
			node->m_nChildren[2]=0;
			node->m_nChildren[3]=0;

#define INDCELL(xx,yy) ((yy)*nx+xx)
			if(GetCase(x1,y1))
			{
				node->ind[0]=INDCELL(x1,y1+1);
				node->ind[1]=INDCELL(x1,y1);
				node->ind[2]=INDCELL(x1+1,y1+1);
				node->ind[3]=INDCELL(x1+1,y1+1);
				node->ind[4]=INDCELL(x1,y1);
				node->ind[5]=INDCELL(x1+1,y1);
			}else
			{
				node->ind[0]=INDCELL(x1,y1);
				node->ind[1]=INDCELL(x1+1,y1);
				node->ind[2]=INDCELL(x1,y1+1);
				node->ind[3]=INDCELL(x1,y1+1);
				node->ind[4]=INDCELL(x1+1,y1);
				node->ind[5]=INDCELL(x1+1,y1+1);
			}
				{
					for(int i=0;i<6;i++)
					{
						_ASSERT(node->ind[i]<nx*ny);
					}
				}
			node->nLevel=level;
		}

	UpdateBoundingVolumes(m_pTree,0,0,nx-1,ny-1);
}

/*
int GetChild(int nLevel, int index,int nChild) 
{
	int nWidth=1<<nLevel;
	int nNextWidth=1<<(nLevel+1);
	int nLevelBase=((1<<(nLevel*2))-1)/3;
	int nNextLevelBase=((1<<((nLevel+1)*2))-1)/3;
	int nx=(index-nLevelBase) % nWidth;
	int ny=(index-nLevelBase) / nWidth;
	return nNextLevelBase+(ny*2+nChild/2)*nNextWidth+nx*2+nChild%2; 
}

void ASSMap::ConstructVolumeTree(int index,int x1,int y1,int x2,int y2)
{
static int nLevel=-1;

	nLevel++;

	ASSVolumeTreeNode *node=m_pTree+index;
	if((x1==x2)&&(y1==y2))
	{
		ASSCell *cell=&m_Cells[y1*nx+x1];
//		cell->litvertex=node->litvertex;
		cell->pind=node->ind;
		node->m_pCell=cell;
		node->nRefTexture=cell->m_nTextureIndex*4+cell->m_nTextureDir;
		if(node->nRefTexture>=m_nSchemeTextures)
		{
//			_ASSERT(0);			// maybe scheme not match
			node->nRefTexture=nQueueCount-1;
		}

		// recalculate Bounding Box
		node->m_bbTree.m[0][0]=float(x1);node->m_bbTree.m[0][1]=float(x1+1);
		node->m_bbTree.m[1][0]=float(y1);node->m_bbTree.m[1][1]=float(y1+1);
		node->m_bbTree.m[2][0]=min(GetHeight(x1,y1),min(GetHeight(x1+1,y1),
								min(GetHeight(x1,y1+1),GetHeight(x1+1,y1+1))));
		node->m_bbTree.m[2][1]=max(GetHeight(x1,y1),max(GetHeight(x1+1,y1),
								max(GetHeight(x1,y1+1),GetHeight(x1+1,y1+1))));

		node->m_nChildren[0]=0;
		node->m_nChildren[1]=0;
		node->m_nChildren[2]=0;
		node->m_nChildren[3]=0;

#define INDCELL(x,y) ((y)*nx+x)
		if(GetCase(x1,y1))
		{
			node->ind[0]=INDCELL(x1,y1+1);
			node->ind[1]=INDCELL(x1,y1);
			node->ind[2]=INDCELL(x1+1,y1+1);
			node->ind[3]=INDCELL(x1+1,y1+1);
			node->ind[4]=INDCELL(x1,y1);
			node->ind[5]=INDCELL(x1+1,y1);
		}else
		{
			node->ind[0]=INDCELL(x1,y1);
			node->ind[1]=INDCELL(x1+1,y1);
			node->ind[2]=INDCELL(x1,y1+1);
			node->ind[3]=INDCELL(x1,y1+1);
			node->ind[4]=INDCELL(x1+1,y1);
			node->ind[5]=INDCELL(x1+1,y1+1);
		}
		node->nLevel=nLevel;
		
	}
	else
	{
		int xm,ym;
		xm=(x1+x2)/2;ym=(y1+y2)/2;
		int nCurrInd = index-g_nLevelBase[nLevel];
		_ASSERT(nCurrInd>=0);

		if(y1==y2)
		{
			node->m_nChildren[0]=g_nLevelBase[nLevel+1]+nCurrInd*2;
			node->m_nChildren[1]=g_nLevelBase[nLevel+1]+nCurrInd*2+1;
			node->m_nChildren[2]=0;
			node->m_nChildren[3]=0;
			ConstructVolumeTree(node->m_nChildren[0],x1,y1,xm,y1);
			ConstructVolumeTree(node->m_nChildren[1],xm+1,y1,x2,y1);
		}
		else
		if(x1==x2)
		{
			node->m_nChildren[0]=g_nLevelBase[nLevel+1]+nCurrInd*2;
			node->m_nChildren[1]=g_nLevelBase[nLevel+1]+nCurrInd*2+1;
			node->m_nChildren[2]=0;
			node->m_nChildren[3]=0;
			ConstructVolumeTree(node->m_nChildren[0],x1,y1,x1,ym);
			ConstructVolumeTree(node->m_nChildren[1],x1,ym+1,x1,y2);
		}
		else
		{
			node->m_nChildren[0]=g_nLevelBase[nLevel+1]+nCurrInd*4;
			node->m_nChildren[1]=g_nLevelBase[nLevel+1]+nCurrInd*4+1;
			node->m_nChildren[2]=g_nLevelBase[nLevel+1]+nCurrInd*4+2;
			node->m_nChildren[3]=g_nLevelBase[nLevel+1]+nCurrInd*4+3;
			ConstructVolumeTree(node->m_nChildren[0],x1	,y1		,xm	,ym);
			ConstructVolumeTree(node->m_nChildren[1],xm+1	,y1		,x2	,ym);
			ConstructVolumeTree(node->m_nChildren[2],x1	,ym+1	,xm	,y2);
			ConstructVolumeTree(node->m_nChildren[3],xm+1	,ym+1	,x2	,y2);
		}
		node->m_bbTree=(m_pTree+node->m_nChildren[0])->m_bbTree;
		for(int i=1;i<4;i++)
		{
			if(node->m_nChildren[i])
				MergeBoundingBox(&node->m_bbTree,&(m_pTree+node->m_nChildren[i])->m_bbTree);
		}

		// generate LOD Infomation
		ASSLODInfo *lodinfo=node->m_pLODInfo=new ASSLODInfo;
		lodinfo->pCells[0]=&m_Cells[y1*nx+x1];
		lodinfo->pCells[1]=&m_Cells[y1*nx+x2+1];
		lodinfo->pCells[2]=&m_Cells[(y2+1)*nx+x2+1];
		lodinfo->pCells[3]=&m_Cells[(y2+1)*nx+x1];

		xm++;ym++;
		
		if(x1!=x2)
		{
			lodinfo->CrackPoint[0]=y1*nx+xm;
			lodinfo->CrackPoint[2]=(y2+1)*nx+xm;
		}else
			lodinfo->CrackPoint[0]=lodinfo->CrackPoint[2]=0;

		if(x1!=x2)
		{
			lodinfo->CrackPoint[1]=ym*nx+x2+1;
			lodinfo->CrackPoint[3]=ym*nx+x1;
		}else
			lodinfo->CrackPoint[1]=lodinfo->CrackPoint[3]=0;
		
		lodinfo->m_pAdjacencies[0]=NULL;
		lodinfo->m_pAdjacencies[1]=NULL;
		lodinfo->m_pAdjacencies[2]=NULL;
		lodinfo->m_pAdjacencies[3]=NULL;

#define INDCELL2(x,y) ((y)*nx+x+nx*ny)
		node->ind[0]=INDCELL2(x1,y2+1);
		node->ind[1]=INDCELL2(x1,y1);
		node->ind[2]=INDCELL2(x2+1,y2+1);
		node->ind[3]=INDCELL2(x2+1,y2+1);
		node->ind[4]=INDCELL2(x1,y1);
		node->ind[5]=INDCELL2(x2+1,y1);
		
		node->nLevel=nLevel;
		node->nRefTexture=m_LodTextures?
				m_nSchemeTextures+(y1*m_nlodcell_texsize/TEXTURE_PARTITION_SIZE)*m_nPatches_x
				+(x1*m_nlodcell_texsize/TEXTURE_PARTITION_SIZE)
			:nQueueCount-1;
	}

	nLevel--;
}
*/


void ASSMap::Flush(int nRefTexture)
{
	int i;

	RSSetTexture(nRefTextures[nRefTexture]);
	g_pd3dDevice->SetVertexShader(RSMFVF);

#ifndef EL_TEST

	if(g_bHardwareTNL)
	{
		
#ifdef DUBBLE_TEST		
		WORD *pIndicies=NULL;
		pIndexBuffer->Lock(0, sizeof(WORD)*nQueueCounts[nRefTexture]*6, (BYTE**)&pIndicies, D3DLOCK_DISCARD );
		for(i=0;i<nQueueCounts[nRefTexture];i++)
		{
			ASSVolumeTreeNode *pNode=m_pQueues[nRefTexture][i];
			memcpy(pIndicies+i*6,pNode->ind,sizeof(WORD)*6);
		}
		pIndexBuffer->Unlock();

		g_pd3dDevice->SetStreamSource(0,pVertexBuffer,sizeof(MVERTEX));
		g_pd3dDevice->SetIndices(pIndexBuffer,0);
		g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,nx*ny*2,0,nQueueCounts[nRefTexture]*2);
#else
		_ASSERT(nQueueCounts[nRefTexture]<BUFFERCOUNT);
		WORD *pIndicies=NULL;
		ppIndexBuffers[nRefTexture]->Lock(0, sizeof(WORD)*nQueueCounts[nRefTexture]*6, (BYTE**)&pIndicies, D3DLOCK_DISCARD );
		for(i=0;i<nQueueCounts[nRefTexture];i++)
		{
			ASSVolumeTreeNode *pNode=m_pQueues[nRefTexture][i];
			memcpy(pIndicies+i*6,pNode->ind,sizeof(WORD)*6);
		}
		ppIndexBuffers[nRefTexture]->Unlock();

		g_pd3dDevice->SetStreamSource(0,pVertexBuffer,sizeof(MVERTEX));
		g_pd3dDevice->SetIndices(ppIndexBuffers[nRefTexture],0);
		g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,nx*ny*2,0,nQueueCounts[nRefTexture]*2);
#endif

	}
	else
	{
		pVertexBuffer->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ;
		MVERTEX ver[4];
		for(i=0;i<nQueueCounts[nRefTexture];i++)
		{
			ASSVolumeTreeNode *pNode=m_pQueues[nRefTexture][i];
			memcpy(ver,pVertices+pNode->ind[0],sizeof(MVERTEX));
			memcpy(ver+1,pVertices+pNode->ind[1],sizeof(MVERTEX));
			memcpy(ver+2,pVertices+pNode->ind[2],sizeof(MVERTEX));
			memcpy(ver+3,pVertices+pNode->ind[5],sizeof(MVERTEX));
			g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,ver,sizeof(MVERTEX));
		}
		pVertexBuffer->Unlock();
	}

#else

//	RSSetCullState(RS_CULLSTYLE_NONE);
	MVERTEX* ver;
	int cnt = 0;

	static MVERTEX t_vertex[10000];

	if(g_bHardwareTNL)
	{
/*
		WORD *pIndicies = m_index_list;
//		WORD *pIndicies = NULL;

		for(i=0;i<nQueueCounts[nRefTexture];i++)
		{
			ASSVolumeTreeNode *pNode = m_pQueues[nRefTexture][i];
			memcpy(pIndicies+i*6,pNode->ind,sizeof(WORD)*6);
		}

		g_pd3dDevice->SetStreamSource(0,NULL,0);
		g_pd3dDevice->SetIndices(NULL,0);

//		HRESULT hr = g_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,nx*ny*2,nQueueCounts[nRefTexture]*2
//					,m_index_list,D3DFMT_INDEX16,m_point_list,sizeof(MVERTEX));

		HRESULT hr = g_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,nx*ny*2,( nQueueCounts[nRefTexture]*2)/3,
			m_index_list,D3DFMT_INDEX16,m_point_list,sizeof(MVERTEX));
*/
		// 테스트 임시 코드

		for(i=0;i<nQueueCounts[nRefTexture];i++)
		{
			ASSVolumeTreeNode *pNode=m_pQueues[nRefTexture][i];

			ver = (t_vertex + (cnt*6));

			memcpy(ver  ,m_point_list+pNode->ind[0],sizeof(MVERTEX));
			memcpy(ver+1,m_point_list+pNode->ind[1],sizeof(MVERTEX));
			memcpy(ver+2,m_point_list+pNode->ind[2],sizeof(MVERTEX));
			memcpy(ver+3,m_point_list+pNode->ind[3],sizeof(MVERTEX));
			memcpy(ver+4,m_point_list+pNode->ind[4],sizeof(MVERTEX));
			memcpy(ver+5,m_point_list+pNode->ind[5],sizeof(MVERTEX));
			cnt++;

			if(cnt*6 > 9900)
			{
				g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST,cnt*2,t_vertex,sizeof(MVERTEX));
				cnt = 0;
			}
		}

		g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST,cnt*2,t_vertex,sizeof(MVERTEX));

	}
	else
	{
//		g_pd3dDevice->SetStreamSource(0,NULL,0);
//		g_pd3dDevice->SetIndices(NULL,0);

//		RSSetCullState(RS_CULLSTYLE_NONE);

/*
		WORD	index[10000];

		for(i=0;i<nQueueCounts[nRefTexture];i++)
		{
			ASSVolumeTreeNode *pNode=m_pQueues[nRefTexture][i];

			memcpy(index+i*6 ,pNode->ind,sizeof(WORD)*6);

//			g_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLESTRIP,0,4,2,index,D3DFMT_INDEX16,m_point_list,sizeof(MVERTEX));
//			HRESULT hr=g_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST ,0,nx*ny*2,2,index,D3DFMT_INDEX16,m_point_list,sizeof(MVERTEX));
//			_ASSERT(hr==D3D_OK);

//			g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,ver,sizeof(MVERTEX));
		}

		HRESULT hr=g_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST ,0,nx*ny*2,i*2,index,D3DFMT_INDEX16,m_point_list,sizeof(MVERTEX));
		_ASSERT(hr==D3D_OK);
*/
/*
		MVERTEX* ver[6];

		for(i=0;i<nQueueCounts[nRefTexture];i++)
		{
			ASSVolumeTreeNode *pNode=m_pQueues[nRefTexture][i];

			memcpy(ver  ,m_point_list+pNode->ind[0],sizeof(MVERTEX));
			memcpy(ver+1,m_point_list+pNode->ind[1],sizeof(MVERTEX));
			memcpy(ver+2,m_point_list+pNode->ind[2],sizeof(MVERTEX));
			memcpy(ver+3,m_point_list+pNode->ind[3],sizeof(MVERTEX));
			memcpy(ver+4,m_point_list+pNode->ind[4],sizeof(MVERTEX));
			memcpy(ver+5,m_point_list+pNode->ind[5],sizeof(MVERTEX));

			g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST,2,ver,sizeof(MVERTEX));
		}
*/

		// 테스트 임시 코드

		for(i=0;i<nQueueCounts[nRefTexture];i++)
		{
			ASSVolumeTreeNode *pNode=m_pQueues[nRefTexture][i];

			ver = (t_vertex + (cnt*6));

			memcpy(ver  ,m_point_list+pNode->ind[0],sizeof(MVERTEX));
			memcpy(ver+1,m_point_list+pNode->ind[1],sizeof(MVERTEX));
			memcpy(ver+2,m_point_list+pNode->ind[2],sizeof(MVERTEX));
			memcpy(ver+3,m_point_list+pNode->ind[3],sizeof(MVERTEX));
			memcpy(ver+4,m_point_list+pNode->ind[4],sizeof(MVERTEX));
			memcpy(ver+5,m_point_list+pNode->ind[5],sizeof(MVERTEX));
			cnt++;

			if(cnt*6 > 9900)
			{
				g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST,cnt*2,t_vertex,sizeof(MVERTEX));
				cnt = 0;
			}
		}

		g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST,cnt*2,t_vertex,sizeof(MVERTEX));


		///////////////////////////////////////////////////

/*
#define RSMFVF2	( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1 )

typedef struct MVERTEX2 {
	float x, y, z;		// world position
	DWORD Diffuse;   // diffuse color    
	DWORD Specular;  // specular color    
	float tu1, tv1;  // texture coordinates
} MVERTEX2, *LPMVERTEX2; 

		g_pd3dDevice->SetVertexShader(RSMFVF2);

		MVERTEX2 vv[4];

		vv[0].x			= 0;
		vv[0].y			= 0;
		vv[0].z			= 0;
		vv[0].Diffuse	= 0xffffffff;
		vv[0].Specular	= 0xffffffff;
		vv[0].tu1	= 0.f;
		vv[0].tv1	= 0.f;
//		vv[0].tu2	= 0.f;
//		vv[0].tv2	= 0.f;

		vv[1].x			= 0;
		vv[1].y			= 100;
		vv[1].z			= 0;
		vv[1].Diffuse	= 0xffffffff;
		vv[1].Specular	= 0xffffffff;
		vv[1].tu1	= 0.f;
		vv[1].tv1	= 0.f;
//		vv[1].tu2	= 0.f;
//		vv[1].tv2	= 0.f;

		vv[2].x			= 100;
		vv[2].y			= 0;
		vv[2].z			= 0;
		vv[2].Diffuse	= 0xffffffff;
		vv[2].Specular	= 0xffffffff;
		vv[2].tu1	= 0.f;
		vv[2].tv1	= 0.f;
//		vv[2].tu2	= 0.f;
//		vv[2].tv2	= 0.f;

		vv[3].x			= 100;
		vv[3].y			= 100;
		vv[3].z			= 0;
		vv[3].Diffuse	= 0xffffffff;
		vv[3].Specular	= 0xffffffff;
		vv[3].tu1	= 0.f;
		vv[3].tv1	= 0.f;
//		vv[3].tu2	= 0.f;
//		vv[3].tv2	= 0.f;

		WORD index[6] = {0,1,2,1,3,2};

		g_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,6,2,index,D3DFMT_INDEX16,vv,sizeof(MVERTEX2));
*/

/*
		MVERTEX* ver[6];
		WORD	 index[6];

		g_pd3dDevice->SetStreamSource(0,NULL,0);
		g_pd3dDevice->SetIndices(NULL,0);

		g_pd3dDevice->SetVertexShader(RSMFVF);

		for(i=0;i<nQueueCounts[nRefTexture];i++)
		{
			ASSVolumeTreeNode *pNode=m_pQueues[nRefTexture][i];

//			memcpy(index,&pNode->ind[0],sizeof(WORD)*6);
			for(int k=0;k<6;k++)
				index[k] = pNode->ind[k];

//			g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST,2,ver,sizeof(MVERTEX));
			g_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,6,2,index,D3DFMT_INDEX16,m_point_list,sizeof(MVERTEX));
		}
*/
	}

#endif

	#ifdef _DEBUG_SHOW_LOD_LEVEL

	RSSetAlphaState(RS_ALPHASTYLE_ADD);
	for(i=0;i<nQueueCounts[nRefTexture];i++)
	{
		ASSVolumeTreeNode *pNode=m_pQueues[nRefTexture][i];
		// test code show lod level

		MVERTEX ver[4];
		memcpy(ver,pVertices+pNode->ind[0],sizeof(MVERTEX));
		memcpy(ver+1,pVertices+pNode->ind[1],sizeof(MVERTEX));
		memcpy(ver+2,pVertices+pNode->ind[2],sizeof(MVERTEX));
		memcpy(ver+3,pVertices+pNode->ind[5],sizeof(MVERTEX));

		ver[0].tu1=1;ver[0].tv1=0;ver[0].Diffuse=0xffffffff;
		ver[1].tu1=0;ver[1].tv1=0;ver[1].Diffuse=0xffffffff;
		ver[2].tu1=1;ver[2].tv1=1;ver[2].Diffuse=0xffffffff;
		ver[3].tu1=0;ver[3].tv1=1;ver[3].Diffuse=0xffffffff;
		RSSetTexture(hNumberTexture[pNode->nLevel]);

		g_pd3dDevice->SetVertexShader(RSLFVF);
		HRESULT result=g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,ver,sizeof(MVERTEX));
		_ASSERT(result==D3D_OK);
	}
	RSSetAlphaState(RS_ALPHASTYLE_NONE);

	#endif

	#ifdef _DEBUG
		tps+=nQueueCounts[nRefTexture]*2;
	#endif

	nQueueCounts[nRefTexture]=0;
}

void ASSMap::Enque(ASSVolumeTreeNode *pNode)
{
	int nRefTexture=pNode->nRefTexture;
/*
	if(pNode->litvertex[0].Specular<0x1000000 &&
		pNode->litvertex[1].Specular<0x1000000 &&
		pNode->litvertex[2].Specular<0x1000000 &&
		pNode->litvertex[3].Specular<0x1000000 	) nRefTexture=nQueueCount-1;
*/
	
	m_pQueues[nRefTexture][nQueueCounts[nRefTexture]++]=pNode;
	if(nQueueCounts[nRefTexture]==BUFFERCOUNT)
		Flush(nRefTexture);
}

void ASSMap::CreateVertexBuffer()
{
#ifndef EL_TEST

	if( FAILED( g_pd3dDevice->CreateVertexBuffer( sizeof(MVERTEX)*nx*ny*2 , 0 , RSMFVF,D3DPOOL_MANAGED  , &pVertexBuffer ) ) )
//	if( FAILED( g_pd3dDevice->CreateVertexBuffer( sizeof(MVERTEX)*nx*ny*2 , 0 , RSMFVF,D3DPOOL_SYSTEMMEM, &pVertexBuffer ) ) )
		return;

	pVertexBuffer->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ;
	
	int i;
	for(i=0;i<nx*ny;i++)
	{
		MVERTEX *pv=pVertices+i;
		pv->x=float(i%nx);
		pv->y=float(i/nx);
		pv->z=m_Heights[i];
		pv->tu1=float(i % nx % 2);//1.0f-(float)fabs((float)fmod(pv->x,2)-1);
		pv->tv1=float(i / nx % 2);//1.0f-(float)fabs((float)fmod(pv->y,2)-1);
		pv->tu2=pv->x;
		pv->tv2=pv->y;
		pv->Diffuse=0xffffffff;
		pv->Specular=0xff000000;
	}

	memcpy(pVertices+nx*ny,pVertices,sizeof(MVERTEX)*nx*ny);

	float patchwidthx=(float)min((nx-1)*m_nlodcell_texsize,TEXTURE_PARTITION_SIZE);
	float patchwidthy=(float)min((ny-1)*m_nlodcell_texsize,TEXTURE_PARTITION_SIZE);

	for(i=0;i<nx*ny;i++)
	{
		MVERTEX *pv=pVertices+i+nx*ny;
		pv->tu1=1.0f-(float)fabs((float)fmod(pv->x,patchwidthx)/patchwidthx*2-1);
		pv->tv1=1.0f-(float)fabs((float)fmod(pv->y,patchwidthy)/patchwidthy*2-1);
	}

	Restore();	// for create indexbuffer

	pVertexBuffer->Unlock();

#else
	
	m_point_list = new MVERTEX [nx*ny*2];

	float patchwidth=(float)(TEXTURE_PARTITION_SIZE/m_nlodcell_texsize)*2;
	
	int i;

	for(i=0;i<nx*ny;i++)
	{
		MVERTEX *pv = m_point_list+i;

		pv->x=float(i%nx);
		pv->y=float(i/nx);
		pv->z=m_Heights[i];

		pv->tu1=float(i % nx % 2);//1.0f-(float)fabs((float)fmod(pv->x,2)-1);
		pv->tv1=float(i / nx % 2);//1.0f-(float)fabs((float)fmod(pv->y,2)-1);

		pv->tu2=pv->x;
		pv->tv2=pv->y;

		pv->Diffuse=0xffffffff;
		pv->Specular=0xff000000;
	}

	memcpy(m_point_list+nx*ny,m_point_list , sizeof(MVERTEX)*nx*ny);

	for(i=0;i<nx*ny;i++)
	{
		MVERTEX *pv = m_point_list + i + nx*ny;
		pv->tu1 = 1.0f-(float)fabs((float)fmod(pv->x,patchwidth)/patchwidth*2-1);
		pv->tv1 = 1.0f-(float)fabs((float)fmod(pv->y,patchwidth)/patchwidth*2-1);
	}

	m_index_list = new WORD [BUFFERCOUNT*6];

#endif

}

void ASSMap::CreateSchemeRefTextures()
{
	int i;
	
	if(m_pQueues)
	{
		for(i=0;i<nQueueCount;i++)
		{
			delete []m_pQueues[i];
			SAFE_RELEASE(ppIndexBuffers[i]);
		}
		delete []m_pQueues;
		m_pQueues=NULL;
	}
	if(nQueueCounts)
	{
		delete []nQueueCounts;
		nQueueCounts=NULL;
	}
	
	m_hPattern=m_pSchemeManager->GetMaterialManager()->Get("pattern");
	m_nSchemeTextures=m_pSchemeManager->GetMaterialManager()->GetCount()*4;

	nQueueCount=m_nSchemeTextures+m_nPatches_x*m_nPatches_y+2;
	nQueueCounts=new int[nQueueCount];
	m_pQueues=new ASSVolumeTreeNode**[nQueueCount];

	ppIndexBuffers=new LPDIRECT3DINDEXBUFFER8[nQueueCount];
	for(i=0;i<nQueueCount;i++)
	{
		m_pQueues[i]=new ASSVolumeTreeNode*[BUFFERCOUNT];
		nQueueCounts[i]=0;
		ppIndexBuffers[i]=NULL;
	}

	if(nRefTextures)
		delete []nRefTextures;
	nRefTextures=new int[nQueueCount];
	for(i=0;i<nQueueCount;i++)
		nRefTextures[i]=0;

	for(i=0;i<m_pSchemeManager->GetCount();i++)
	{
		ASSTileSet *pTileSet=m_pSchemeManager->Get(i);
		for(int j=0;j<5;j++)
		{
			ASSTileAppearance *pTAP=pTileSet->Get(j);
			for(int k=0;k<pTAP->GetCount();k++)
			{
				ASSTile *pT=pTAP->Get(k);
				for(int l=0;l<4;l++)
					nRefTextures[pT->nTextureIndex*4+l]=pT->nTextureHandle[l];
			}
		}
	}
	for(i=0;i<m_pSchemeManager->GetBaseCount();i++)
	{
		ASSTile *pT=m_pSchemeManager->GetBaseTile(i);
		for(int l=0;l<4;l++)
			nRefTextures[pT->nTextureIndex*4+l]=pT->nTextureHandle[l];
	}

	for(i=0;i<m_nPatches_x*m_nPatches_y;i++)
		nRefTextures[i+m_nSchemeTextures]=m_LodTextures[i];
	nRefTextures[nQueueCount-2]=m_hMinimapTexture;
	nRefTextures[nQueueCount-1]=0;
}

void ASSMap::Invalidate()
{
	SAFE_RELEASE(pIndexBuffer);
	for(int i=0;i<nQueueCount;i++)
		SAFE_RELEASE(ppIndexBuffers[i]);
}

void ASSMap::Restore()
{
	if(pIndexBuffer)
		Invalidate();

	HRESULT hr = g_pd3dDevice->CreateIndexBuffer( sizeof(WORD)*BUFFERCOUNT*6, 
										D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY , D3DFMT_INDEX16,
										D3DPOOL_DEFAULT , &pIndexBuffer )  ;
	for(int i=0;i<nQueueCount;i++)
	{
	HRESULT hr = g_pd3dDevice->CreateIndexBuffer( sizeof(WORD)*BUFFERCOUNT*6 ,
									D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY , D3DFMT_INDEX16,
									D3DPOOL_DEFAULT , &ppIndexBuffers[i] )  ;
	}
	_ASSERT(hr==D3D_OK);	
}
