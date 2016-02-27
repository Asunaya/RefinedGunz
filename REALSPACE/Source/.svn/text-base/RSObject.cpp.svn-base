#include "RealSpace.h"
#include "RSObject.h"
#include "rutils.h"
#include "stdio.h"
#include "crtdbg.h"
#include "RSMaterialManager.h"
#include "RSDebug.h"
#include "RSAnimationInfo.h"
#include "rtexture.h"
#include "FileInfo.h"

CMesh::CMesh()
{
	ver=NULL;
	name=NULL;
	nV=nFaces=0;
	faceshead=NULL;
	m_align=RS_ALIGNSTYLE_NONE;
}

CMesh::~CMesh()
{
	if (ver) delete []ver;
	if (name) delete []name;
	if (faceshead) delete []faceshead;
}

CFaces::CFaces()
{
	nv=0;ni=0;
	iMaterial=0;
	tnlvertices=NULL;
	indicies=NULL;
	indicies_original=NULL;
	pVertexBuffer=NULL;
	pIndexBuffer=NULL;
}

CFaces::~CFaces()
{
	if(tnlvertices) delete tnlvertices;
	if(indicies) delete indicies;
	if(indicies_original) delete indicies_original;
	if(pVertexBuffer)
	{
		for(int i=0;i<RSGetDeviceCount();i++)
		{
			if(pVertexBuffer[i])
				pVertexBuffer[i]->Release();
		}
		delete pVertexBuffer;
	}
	if(pIndexBuffer)
	{
		for(int i=0;i<RSGetDeviceCount();i++)
		{
			if(pIndexBuffer[i])
				pIndexBuffer[i]->Release();
		}
		delete pIndexBuffer;
	}
}

void CFaces::Draw()
{
/*
	int k;
	//RSDrawFaceArray(faces->face,faces->nf);/*
	rface *f=face;
	for(k=0;k<nf;k++)
	{
		if(f->isValid)
			RSDrawFace(f);
		f++;
	}//*/
}

RSObject::RSObject()
{
	RefferenceCount=0;
	nMesh=0;
	nMaterial=0;
	nAnimSet=0;
	m_BoundingSphereRadius=0;
	m_BoundingSphereRadiusXY=0;
	m_bbox.Minz=m_bbox.Miny=m_bbox.Minx=100000000;
	m_bbox.Maxz=m_bbox.Maxy=m_bbox.Maxx=-100000000;

	materials=NULL;
	meshes=NULL;

	m_hShadowTexture=NULL;
	m_pMM=NULL;
	b_needdelMM=false;
}

RSObject::~RSObject()
{
	Destroy();
}

bool RSObject::Destroy()
{
	if( materials )
	{
		delete []materials;
		materials=NULL;
	}
	if( meshes ) 
	{
		delete []meshes;
		meshes=NULL;
	}
	if(m_hShadowTexture)
	{
		RSDeleteTexture(m_hShadowTexture);
		m_hShadowTexture=NULL;
	}
	if(b_needdelMM && m_pMM)
	{
		delete m_pMM;
		m_pMM=NULL;
		b_needdelMM=false;
	}
	return TRUE;
}

static float g_fTemp;

#define ReadBool(x)		fread(&(x),sizeof(BOOL),1,file)
#define ReadWord(x)		fread(&(x),sizeof(WORD),1,file)
#define ReadInt(x)		fread(&(x),sizeof(int),1,file)
#define ReadFloat(x)	fread(&(x),sizeof(float),1,file)
#define ReadVector(x)	fread(&(x),sizeof(rvector),1,file)
#define ReadString(x)	{int l=fgetc(file);if(l) {x=new char[l+1];x[l]=0;fread(x,l,1,file);}}
#define ReadMatrix(m) 	{ReadFloat(m._11);ReadFloat(m._12);ReadFloat(m._13);ReadFloat(g_fTemp);\
	ReadFloat(m._21);ReadFloat(m._22);ReadFloat(m._23);ReadFloat(g_fTemp);\
	ReadFloat(m._31);ReadFloat(m._32);ReadFloat(m._33);ReadFloat(g_fTemp);\
	ReadFloat(m._41);ReadFloat(m._42);ReadFloat(m._43);ReadFloat(g_fTemp);}

#define IsSameTime(x,y) ((x.dwHighDateTime==y.dwHighDateTime)&&(x.dwLowDateTime==y.dwLowDateTime))

static int g_obj_cnt;

bool RSObject::Load(FILE *file,RSMaterialManager *pMM)
{
	int i,j,k;

	fread(&m_Header,sizeof(m_Header),1,file);
	if((m_Header.RSMID!=HEADER_RSMID)||(m_Header.Build!=HEADER_BUILD))
	{
		rslog("Incorrect file format. rsm build #%d. #%d required.\n",m_Header.Build,HEADER_BUILD);
		return false;
	}
	m_pMM=pMM;

	fread(&m_bbox,sizeof(rboundingbox),1,file);
	m_bbox.Minz=m_bbox.Miny=m_bbox.Minx=100000000;
	m_bbox.Maxz=m_bbox.Maxy=m_bbox.Maxx=-100000000;

	ReadInt(nMaterial);
	materials=new RSMaterial*[nMaterial+1];

//	rslog("[ load object :%d mtrl_num %d ]\n",g_obj_cnt,nMaterial);

	materials[0]=&RSDefaultMaterial;
	for (i=0;i<nMaterial;i++)
	{
		char buf[256];
		{int l=fgetc(file);buf[l]=0;if(l) fread(buf,l,1,file);}
		RSMaterial *pmat=pMM->GetMaterial(buf);
		materials[i+1]=pmat ? pmat : &RSDefaultMaterial;

//		rslog("\t-mtrl :%d name %s \n",i,buf);
	}

	nMaterial++;	// for default material;

	ReadInt(nMesh);
	CMesh *mesh=meshes=new CMesh[nMesh];

//	rslog("\n\t-mesh num :%d \n\n",nMesh);

	for(i=0;i<nMesh;i++)
	{
		ReadString(mesh->name);strlwr(mesh->name);
		ReadMatrix(mesh->mat);
		fread(&mesh->m_bbox,sizeof(rboundingbox),1,file);

		mesh->m_bbox.Minz=mesh->m_bbox.Miny=mesh->m_bbox.Minx=100000000;
		mesh->m_bbox.Maxz=mesh->m_bbox.Maxy=mesh->m_bbox.Maxx=-100000000;

		mesh->center=rvector(0,0,0);

		ReadInt(mesh->nFaces);
		CFaces *faces=mesh->faceshead=mesh->nFaces?new CFaces[mesh->nFaces]:NULL;

//		rslog("\t-mesh face num :%d \n",mesh->nFaces);

		for(j=0;j<mesh->nFaces;j++)
		{
			ReadInt(faces->iMaterial);faces->iMaterial++;		// convert to 1-base index
			ReadInt(faces->nv);
			faces->tnlvertices=new VERTEX[faces->nv];
			fread(faces->tnlvertices,sizeof(VERTEX),faces->nv,file);

			for(k=0;k<faces->nv;k++)
			{
				rvector *pv=(rvector*)(&faces->tnlvertices[k].x);
				m_BoundingSphereRadius=max(m_BoundingSphereRadius,pv->GetMagnitude());
				m_BoundingSphereRadiusXY=max(m_BoundingSphereRadiusXY,rvector(pv->x,pv->y,0).GetMagnitude());
				mesh->m_bbox.Minx=min(mesh->m_bbox.Minx,pv->x);
				mesh->m_bbox.Maxx=max(mesh->m_bbox.Maxx,pv->x);
				mesh->m_bbox.Miny=min(mesh->m_bbox.Miny,pv->y);
				mesh->m_bbox.Maxy=max(mesh->m_bbox.Maxy,pv->y);
				mesh->m_bbox.Minz=min(mesh->m_bbox.Minz,pv->z);
				mesh->m_bbox.Maxz=max(mesh->m_bbox.Maxz,pv->z);
			}

//			rslog("\t\t-mesh face v num :%d \n",faces->nv);

			if(fgetc(file)==1)
			{
				faces->indicies_original=new WORD[faces->nv];
				fread(faces->indicies_original,sizeof(WORD),faces->nv,file);
			}
			else
			{
				ReadInt(faces->ni);
				faces->indicies=new WORD[faces->ni];
				fread(faces->indicies,sizeof(WORD),faces->ni,file);
			}
			faces++;
		}

		if(strstr(mesh->name,ALIGN_POINT_STRING))
			mesh->m_align=RS_ALIGNSTYLE_POINT;
		else
		if(strstr(mesh->name,ALIGN_LINE_STRING))
			mesh->m_align=RS_ALIGNSTYLE_LINE;
		else
			mesh->m_align=RS_ALIGNSTYLE_NONE;

		MergeBoundingBox(&m_bbox,&mesh->m_bbox);
		mesh++;
	}


	m_ShadowTexture.Create(file);

	/*// save test
	{
		char buf[256];
		ReplaceExtension(buf,name,"bmp");
		m_ShadowTexture.SaveAsBMP(buf);
	} //*/
	m_hShadowTexture=RSCreateTexture(
		m_ShadowTexture.GetWidth(),m_ShadowTexture.GetHeight(),
		(char*)m_ShadowTexture.GetData(),NULL,true,"Object:ShadowTexture");

	/*
	rtexture temp;temp.Create(file);
	CreateShadowTexture();
*/

	ReadInt(nAnimSet);
	if(nAnimSet)
	{
		for(i=0;i<nAnimSet;i++)
		{
			RSAnimationInfo *ai=new RSAnimationInfo;
			ai->Load(file);
			AnimationList.Add(ai);
		}
	}
	GenerateVertexBuffers();

//	rslog("\t-mesh ani num :%d \n",nAnimSet);

//	g_obj_cnt++;
	
	return true;
}

void RSObject::CreateShadowTexture()
{
#define EFFECTIVE_SIZE	0.99f
#define SHADOW_COLOR	0xa0a0a0

	rtexture rt;
	rt.New(256,256,RTEXTUREFORMAT_24);
	rt.Fill(0xffffff);

	rmatrix44 tm;

	int i,j,k;
	float f_max=m_BoundingSphereRadiusXY;

	tm=ScaleMatrix44(128.0f*EFFECTIVE_SIZE/f_max)
		*ViewMatrix44(rvector(0,0,0),rvector(1,2,-1),rvector(0,1,0))
		*TranslateMatrix44(128,128,0);

	for(i=0;i<nMesh;i++)
	{
		CMesh *mesh=meshes+i;
		for(j=0;j<mesh->nFaces;j++)
		{
			CFaces *faces=mesh->faceshead+j;
			for(k=0;k<faces->ni;k+=3)
			{
				rvector a=TransformVector(*(rvector*)(&faces->tnlvertices[faces->indicies[k]].x),tm);
				rvector b=TransformVector(*(rvector*)(&faces->tnlvertices[faces->indicies[k+1]].x),tm);
				rvector c=TransformVector(*(rvector*)(&faces->tnlvertices[faces->indicies[k+2]].x),tm);
#define CHECK(x) _ASSERT((x>=0)&&(x<256))

				rt.FillTriangle(a.x,a.y,b.x,b.y,c.x,c.y,SHADOW_COLOR);
				CHECK(a.x);CHECK(a.y);
				CHECK(b.x);CHECK(b.y);
				CHECK(c.x);CHECK(c.y);
			}
		}
	}

	rtexture newrt1;newrt1.CreateAsHalf(&rt);		// 128
	rtexture newrt2;newrt2.CreateAsHalf(&newrt1);	// 64

	newrt2.FillBoundary(0xffffff);
	newrt2.SaveAsBMP("testtest.bmp");

	m_hShadowTexture=RSCreateTexture(
		newrt2.GetWidth(),newrt2.GetHeight(),
		(char*)newrt2.GetData(),NULL,true,"Object:ShadowTexture");
}

bool RSObject::Load(const char *filename,RSMaterialManager *pMM)
{
	strcpy(name,filename);

	FILE *file;
	file=fopen(filename,"rb");

	if(!file) {
		rslog("File Open Failed. : %s \n",filename);
		return false;
	}

	bool ret=Load(file,pMM);
	fclose(file);
	return ret;
}

bool RSObject::Load(const char *filename,const char *RMLName)
{
	m_pMM=new RSMaterialManager;
	if(!m_pMM->Open(RMLName))
		return false;

	b_needdelMM=true;
	return Load(filename,m_pMM);
}

void RSObject::ClearVertexBuffers()
{
	int i,j,k;
	for(i=0;i<nMesh;i++)
	{
		CMesh *mesh=&meshes[i];
		for(j=0;j<mesh->nFaces;j++)
		{
			CFaces *faces=&mesh->faceshead[j];
			if(faces->pVertexBuffer)
			{
				for(k=0;k<RSGetDeviceCount();k++)
					faces->pVertexBuffer[k]->Release();
				delete faces->pVertexBuffer;
				faces->pVertexBuffer=NULL;
			}
			if(faces->pIndexBuffer)
			{
				for(k=0;k<RSGetDeviceCount();k++)
					faces->pIndexBuffer[k]->Release();
				delete faces->pIndexBuffer;
				faces->pIndexBuffer=NULL;
			}
		}
	}
}

void RSObject::GenerateVertexBuffers()
{
	ClearVertexBuffers();
	int i,j,k;
	for(i=0;i<nMesh;i++)
	{
		CMesh *mesh=&meshes[i];
		for(j=0;j<mesh->nFaces;j++)
		{
			CFaces *faces=&mesh->faceshead[j];
//			_ASSERT(faces->nv && faces->ni);

			{
				faces->pVertexBuffer=new LPDIRECT3DVERTEXBUFFER8[RSGetDeviceCount()];
				if(faces->ni)
					faces->pIndexBuffer=new LPDIRECT3DINDEXBUFFER8[RSGetDeviceCount()];

				for(k=0;k<RSGetDeviceCount();k++)
				{
					faces->pVertexBuffer[k]=NULL;
					g_pDevices[k]->EndScene();

					// create vertex buffer
					if( FAILED( g_pDevices[k]->CreateVertexBuffer( sizeof(VERTEX)*faces->nv, 0 , RSFVF,
													  D3DPOOL_MANAGED, &faces->pVertexBuffer[k] ) ) )

					{
						rslog("create vertex buffer failed.\n");
						delete faces->pVertexBuffer;
						delete faces->pIndexBuffer;
						return;
					}

					VOID* pVertices;
					faces->pVertexBuffer[k]->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ;
					memcpy( pVertices, faces->tnlvertices, sizeof(VERTEX)*faces->nv);
					faces->pVertexBuffer[k]->Unlock();

					// create index buffer
					if(faces->pIndexBuffer)
					{
						if( FAILED( g_pDevices[k]->CreateIndexBuffer( sizeof(WORD)*faces->ni, 0 , D3DFMT_INDEX16 ,
														  D3DPOOL_MANAGED, &faces->pIndexBuffer[k] ) ) )

						{
							rslog("create index buffer failed.\n");
							delete faces->pVertexBuffer;
							delete faces->pIndexBuffer;
							return;
						}

						WORD* pIndicies;
						faces->pIndexBuffer[k]->Lock( 0, sizeof(WORD)*faces->ni, (BYTE**)&pIndicies, 0 ) ;
						memcpy( pIndicies,faces->indicies,sizeof(WORD)*faces->ni);
						faces->pIndexBuffer[k]->Unlock();
					}

					g_pDevices[k]->BeginScene();
					//*/
				}
			}
		}
	}
/*// dump model
rslog("%d",faces->nv);
for(k=0;k<faces->nv;k++)
{
	if(k%2==0) rslog("\n");
	VERTEX *pv=&faces->tnlvertices[k];
	rslog("{%3.3ff,%3.3ff,%3.3ff,%x,%x,%3.3ff,%3.3ff},\t",pv->x,pv->y,pv->z,0,0,pv->tu1,pv->tv1);
}
rslog("\n %d ",faces->ni);
int a=0;
for(k=0;k<faces->ni;k++)
{
	if(k%20==0) rslog("\n");
	rslog("%d,",faces->indicies[k]);
	a=max(a,faces->indicies[k]);
}
//*/


}