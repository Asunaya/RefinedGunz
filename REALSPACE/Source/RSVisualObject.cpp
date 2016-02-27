// RSVisualObject : a object that can render as 3d object.. 
// coded by dubble. since 1998

#include "RSVisualObject.h"
#include "rsutils.h"
#include "realspace.h"
#include "RSDebug.h"
#include "RSPointLight.h"
#include "RSMaterialList.h"
#include <d3dx8.h>

#define DELTAZ 0.0001f
#define MAKEALPHABIT(a)		((long)(a) << 24)
#define AMBIENT 0.28f

//#define EL_TEST
//#define EL_TEST_OBJ

RSVisualObject::RSVisualObject()
{
	rsm=NULL;
	nMesh=0;
	nMaterial=0;
	nAnimSet=0;
	meshestoview=NULL;
	Animation=-1;AnimationKey=0.0f;
	AnimationStyle=0;
	CurrentAnimation=NULL;
	bReadZ=TRUE;bWriteZ=TRUE;
	m_bEnvOnly=false;
	m_EnvStyle=RS_ENVIRONMENTSTYLE_WRAP_Z;
	m_AlphaState=RS_ALPHASTYLE_NONE;
	CombineColor=rvector(AMBIENT,AMBIENT,AMBIENT);
	m_hEnvTextureHandle=0;

	/*
	shadowver[0].color=shadowver[1].color=shadowver[2].color=shadowver[3].color=0x80000000;

	shadowface[0].v[0]=&shadowver[0];
	shadowface[0].v[1]=&shadowver[1];
	shadowface[0].v[2]=&shadowver[2];
	shadowface[0].s[0]=0;shadowface[0].t[0]=0;
	shadowface[0].s[1]=1;shadowface[0].t[1]=0;
	shadowface[0].s[2]=1;shadowface[0].t[2]=1;

	shadowface[1].v[0]=&shadowver[0];
	shadowface[1].v[1]=&shadowver[2];
	shadowface[1].v[2]=&shadowver[3];
	shadowface[1].s[0]=0;shadowface[1].t[0]=0;
	shadowface[1].s[1]=1;shadowface[1].t[1]=1;
	shadowface[1].s[2]=0;shadowface[1].t[2]=1;
*/

	isVisualObject=true;
	m_isRender = false;

	materials=NULL;
	m_opacity=255;
	m_FogColor=0xff000000;
}

RSVisualObject::~RSVisualObject()
{
	if(rsm)
		rsm->DecreaseRefferenceCount();
	if(meshestoview) delete []meshestoview;
	if(materials) delete []materials;
}

bool RSVisualObject::Create(const RSObject *r)
{
	_ASSERT(r);
	rsm=(RSObject*)r;
	rsm->IncreaseRefferenceCount();
	nAnimSet=rsm->nAnimSet;
	nMesh=rsm->nMesh;
	meshestoview=new CMesh*[nMesh];
	for(int i=0;i<nMesh;i++)
		meshestoview[i]=r->meshes+i;

	nMaterial=rsm->nMaterial;
	materials=new RSMaterial*[rsm->nMaterial];
	memcpy(materials,rsm->materials,sizeof(RSMaterial*)*nMaterial);

	rboundingbox *b=&rsm->m_bbox;
	memcpy(&m_BoundingBox,b,sizeof(rboundingbox));
	m_BoundingVertices[0]=rvertex(b->Maxx,b->Miny,b->Maxz);
	m_BoundingVertices[1]=rvertex(b->Maxx,b->Maxy,b->Maxz);
	m_BoundingVertices[2]=rvertex(b->Minx,b->Maxy,b->Maxz);
	m_BoundingVertices[3]=rvertex(b->Minx,b->Miny,b->Maxz);
	m_BoundingVertices[4]=rvertex(b->Maxx,b->Miny,b->Minz);
	m_BoundingVertices[5]=rvertex(b->Maxx,b->Maxy,b->Minz);
	m_BoundingVertices[6]=rvertex(b->Minx,b->Maxy,b->Minz);
	m_BoundingVertices[7]=rvertex(b->Minx,b->Miny,b->Minz);

	/*
	shadowver[0].z=shadowver[1].z=shadowver[2].z=shadowver[3].z=rsm->m_bbox.Minz;

	shadowver[0].x=(float)(-(rsm->m_bbox.Maxx-rsm->m_bbox.Minx)*DRAFT_SHAODW_SIZE);
	shadowver[0].y=(float)(-(rsm->m_bbox.Maxy-rsm->m_bbox.Miny)*DRAFT_SHAODW_SIZE);
	shadowver[1].x=-shadowver[0].x;	shadowver[1].y=shadowver[0].y;
	shadowver[2].x=-shadowver[0].x;	shadowver[2].y=-shadowver[0].y;
	shadowver[3].x=shadowver[0].x;	shadowver[3].y=-shadowver[0].y;
*/
	return TRUE;
}

void RSVisualObject::SetHeight(float z)
{
	rvector pos=GetPosition();
	SetPosition(pos.x,pos.y,-ScaleFactorZ*GetAttachedRSM()->m_bbox.Minz+z);	
}

void RSVisualObject::SetCombineColor(rvector cl)
{
	CombineColor=rvector(max(cl.x,AMBIENT),max(cl.y,AMBIENT),max(cl.z,AMBIENT));
}

// 이름으로 인덱스 얻어내기
int RSVisualObject::GetAnimationIndex(char* szName)
{
	if(!szName) return -1;
	RSObject* pObj = GetAttachedRSM();
	for(int i=0; i<pObj->AnimationList.GetCount(); i++){
		RSAnimationInfo* pAIT = pObj->AnimationList.Get(i);
		if(stricmp(pAIT->GetName(), szName)==0) return i;
	}
	//rslog("Animation(%s) not found (Total animation count %d)", szName, pObj->AnimationList.GetCount());
	return -1;
}

void RSVisualObject::SetAnimation(int a)
{
	if((a>=nAnimSet)||(a<-1)) return;
	if(a==-1)
	{
		Animation=-1;
		CurrentAnimation=NULL;
		AnimationKey=0;

	}else
	if(Animation!=a)
	{
		_ASSERT(a<nAnimSet);
		Animation=a;
		CurrentAnimation=GetAttachedRSM()->AnimationList.Get(a);
		AnimationKey=0;
	}
}

void RSVisualObject::ReplaceSubObject(RSObject *obj)
{
	int i,j;
	for(i=0;i<nMesh;i++)
	{
		for(j=0;j<obj->nMesh;j++)
		{
			if(!strcmp(rsm->meshes[i].name,obj->meshes[j].name))
			{
				meshestoview[i]=obj->meshes+j;
				_RPT1(_CRT_WARN,"%s is Replaced.\n",rsm->meshes[i].name);
			}
		}
	}
}

void RSVisualObject::Transform()
{
	static int key,i,j,k,l;
	static rmatrix mm,objmat;
	static rmatrix44 tm;

	objmat=GetMatrix();
	
	CMesh *mesh;
	for (i=0;i<nMesh;i++)
	{
		mesh=meshestoview[i];
		GetSubObjectTM(&mm,&objmat,i);
		static rvector light;
		light=Normalize(TransformNormal(-RSLightDirection,MatrixInverse(mm)));
		tm=MatrixMult(mm,RSViewProjectionViewport);

		static float r,g,b,col;
		rvertex *trv=mesh->ver;
		
		if(CurrentAnimation&&(CurrentAnimation->GetAnimationMethod()==RSANIMATIONMETHOD_VERTEX))
		{
			rvector *dest=CurrentAnimation->GetAnimationVertices(i,(int)AnimationKey);
			for(j=0;j<mesh->nV;j++)
			{
				TransformVertex(*trv,*dest,tm);
				trv++;dest++;
			};
		}
		else
		{
			for(j=0;j<mesh->nV;j++)
			{
				TransformVertexASM(*trv,tm);
				trv++;
			}
		}
		/*
		CFaces *faces=mesh->faceshead;
		for(j=0;j<mesh->nFaces;j++)
		{
			rface *face=faces->face;
			for(k=0;k<faces->nf;k++)
			{
				face->isValid=((((face->v[1]->sx-face->v[0]->sx)*(face->v[2]->sy-face->v[1]->sy)
							-(face->v[2]->sx-face->v[1]->sx)*(face->v[1]->sy-face->v[0]->sy))>0));
				face++;
			}
			faces++;
		}
		*/
	}
}

void RSVisualObject::TransformAndLight(RSPointLightList *PointLightList)
{
	static int key,i,j,l;
	static rmatrix mm,objmat;
	static rmatrix44 tm;

	objmat=GetMatrix();
	
	CMesh *mesh;
	for (i=0;i<nMesh;i++)
	{
		mesh=meshestoview[i];
		GetSubObjectTM(&mm,&objmat,i);
		static rvector light;
		light=Normalize(TransformNormal(-RSLightDirection,MatrixInverse(mm)));
		tm=MatrixMult(mm,RSViewProjectionViewport);

		float col;
		rvertex *trv=mesh->ver;
		
		if(CurrentAnimation&&(CurrentAnimation->GetAnimationMethod()==RSANIMATIONMETHOD_VERTEX))
		{
			rvector *dest=CurrentAnimation->GetAnimationVertices(i,(int)AnimationKey);
			for(j=0;j<mesh->nV;j++)
			{
				TransformVertex(*trv,*dest,tm);
				trv++;dest++;
			};
		}
		else
		{
			for(j=0;j<mesh->nV;j++)
			{
				TransformVertexASM(*trv,tm);
				trv++;
			}
		}
		trv=mesh->ver;
		for(j=0;j<mesh->nV;j++)
		{
			col=max(DotProduct(light,trv->normal),0);
			trv->color_r=max(0,(CombineColor.x+col)*RSLightColor.x);
			trv->color_g=max(0,(CombineColor.y+col)*RSLightColor.y);
			trv->color_b=max(0,(CombineColor.z+col)*RSLightColor.z);
			trv->specular=m_FogColor|(trv->specular&0xffffff);
			trv++;
		}
		/*	// culling
		CFaces *faces=mesh->faceshead;

		for(j=0;j<mesh->nFaces;j++)
		{
			if(materials[faces->iMaterial]->b2Sided)
			{
				int k;
				rface *face=faces->face;
				for(k=0;k<faces->nf;k++)
				{
					face->isValid=1;
					face++;
				}
			}
			else
			{
				int k;
				rface *face=faces->face;
				for(k=0;k<faces->nf;k++)
				{
					face->isValid=((((face->v[1]->sx-face->v[0]->sx)*(face->v[2]->sy-face->v[1]->sy)
								-(face->v[2]->sx-face->v[1]->sx)*(face->v[1]->sy-face->v[0]->sy))>0));
					face++;
				}
			}
			faces++;
		}//*/
	}

	if(PointLightList)
	{
		RSPointLight *pointlight;
		for(l=0;l<PointLightList->GetCount();l++)
		{
			pointlight=PointLightList->Get(l);
			if(BoundingSphereTest(&pointlight->mPosition,pointlight->mRange) && !IS_EQ(pointlight->mRange,0))
			{
				float radius=pointlight->mRange/MaxScaleFactor;
				static rmatrix mm,objmat;
				objmat=GetMatrix();
				CMesh *mesh;
				for (i=0;i<nMesh;i++)
				{
					mesh=meshestoview[i];
					GetSubObjectTM(&mm,&objmat,i);
					static rvector light,pos;

					light=TransformVector(pointlight->mPosition,MatrixInverse(mm));
					rvertex *trv=mesh->ver;
					rvector *dest;
					int deststep=0;
					if(CurrentAnimation&&(CurrentAnimation->GetAnimationMethod()==RSANIMATIONMETHOD_VERTEX))
					{
						dest=CurrentAnimation->GetAnimationVertices(i,(int)AnimationKey);
						deststep=sizeof(rvector);
					}
					else
					{
						dest=(rvector*)&trv->x;
						deststep=sizeof(rvertex);
					}

					for(j=0;j<mesh->nV;j++)
					{
						rvector rel=light-*dest;
						float intensity=max(0,DotProduct(Normalize(rel),trv->normal));
						float distance=rel.GetMagnitude();
						intensity*=max(0,radius-distance)/radius;
						rvector color=intensity*pointlight->mColor;
						trv->color_r+=color.x;
						trv->color_g+=color.y;
						trv->color_b+=color.z;
						trv++;
						dest=(rvector*)((char*)dest+deststep);
					}
				}
			}
		}
	}

	for (i=0;i<nMesh;i++)
	{
		mesh=meshestoview[i];
		rvertex *trv=mesh->ver;
		if(m_AlphaState!=RS_ALPHASTYLE_NONE)
		{
			for(j=0;j<mesh->nV;j++)
			{
				trv->color_r=min(trv->color_r,1);
				trv->color_g=min(trv->color_g,1);
				trv->color_b=min(trv->color_b,1);
				trv->color=FLOAT2RGB24(trv->color_r,trv->color_g,trv->color_b) | (DWORD(m_opacity) << 24);
				trv++;
			}
		}
		else
		{
			for(j=0;j<mesh->nV;j++)
			{
				trv->color_r=min(trv->color_r,1);
				trv->color_g=min(trv->color_g,1);
				trv->color_b=min(trv->color_b,1);
				trv->color=FLOAT2RGB24(trv->color_r,trv->color_g,trv->color_b);
				trv++;
			}
		}
	}
}

void RSVisualObject::AlignPoint(rmatrix &tm,rvector &camera)
{
	float s1,s2,s3;
	rvector a1,a2,a3;
	a1=rvector(tm._11,tm._12,tm._13);s1=a1.GetMagnitude();a1.Normalize();
	a2=rvector(tm._21,tm._22,tm._23);s2=a2.GetMagnitude();a2.Normalize();
	a3=rvector(tm._31,tm._32,tm._33);s3=a3.GetMagnitude();a3.Normalize();

	a2=camera;
	a1=Normalize(CrossProduct(a2,a3));a3=Normalize(CrossProduct(a1,a2));

	tm._11=s1*a1.x;tm._12=s1*a1.y;tm._13=s1*a1.z;
	tm._21=s2*a2.x;tm._22=s2*a2.y;tm._23=s2*a2.z;
	tm._31=s3*a3.x;tm._32=s3*a3.y;tm._33=s3*a3.z;
}

void RSVisualObject::AlignLine(rmatrix &tm,rvector &camera)
{
	float s1,s2,s3;
	rvector a1,a2,a3;
	a1=rvector(tm._11,tm._12,tm._13);s1=a1.GetMagnitude();a1.Normalize();
	a2=rvector(tm._21,tm._22,tm._23);s2=a2.GetMagnitude();a2.Normalize();
	a3=rvector(tm._31,tm._32,tm._33);s3=a3.GetMagnitude();a3.Normalize();

	a1=Normalize(CrossProduct(camera,a3));a2=Normalize(CrossProduct(a3,a1));

	tm._11=s1*a1.x;tm._12=s1*a1.y;tm._13=s1*a1.z;
	tm._21=s2*a2.x;tm._22=s2*a2.y;tm._23=s2*a2.z;
	tm._31=s3*a3.x;tm._32=s3*a3.y;tm._33=s3*a3.z;
}

void RSVisualObject::GetSubObjectTM(rmatrix *tm,rmatrix *model,int i)
{
	rmatrix *temp;
	CMesh *mesh=meshestoview[i];
	if(nAnimSet&&(Animation>=0))
	{
		temp=CurrentAnimation->GetTransformMatrix(i,(int)AnimationKey);
		if(temp)
		{
			switch(mesh->m_align)
			{
				case RS_ALIGNSTYLE_POINT:
				{
					rmatrix mm=mesh->mat;
					rmatrix invmm=MatrixInverse(mm);
					rmatrix mm2=*temp;
					rvector camera=TransformNormal(RSCameraOrientation,MatrixInverse(*model));camera.Normalize();
					AlignPoint(mm2,camera);

					*tm=mm*mm2**model;
				};break;

				case RS_ALIGNSTYLE_LINE:
				{
					rmatrix mm=mesh->mat;
					rmatrix invmm=MatrixInverse(mm);
					rmatrix mm2=*temp;
					rvector camera=TransformNormal(RSCameraOrientation,MatrixInverse(*model));camera.Normalize();
					AlignLine(mm2,camera);

					*tm=mm*mm2**model;
				};break;

				case RS_ALIGNSTYLE_NONE : *tm=mesh->mat*MatrixMult(*temp,*model);break;
			}
			return;
		}
	}

	switch(mesh->m_align)
	{
		case RS_ALIGNSTYLE_POINT:
		{
			rmatrix mm=mesh->mat;
			rmatrix invmm=MatrixInverse(mm);
			rmatrix mm2=IdentityMatrix();
			rvector camera=TransformNormal(RSCameraOrientation,MatrixInverse(invmm**model));camera.Normalize();
			AlignPoint(mm2,camera);

			*tm=mm*mm2*invmm**model;
		};break;

		case RS_ALIGNSTYLE_LINE:
		{
			rmatrix mm=mesh->mat;
			rmatrix invmm=MatrixInverse(mm);
			rmatrix mm2=IdentityMatrix();
			rvector camera=TransformNormal(RSCameraOrientation,MatrixInverse(invmm**model));camera.Normalize();
			AlignLine(mm2,camera);

			*tm=mm*mm2*invmm**model;
		};break;

		case RS_ALIGNSTYLE_NONE : *tm=*model;break;
	}
}

void RSVisualObject::SetupLight(RSPointLightList *PointLightList)
{
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, true );
	int nLight=0;
	RSPointLight *pLights[MAXIMUM_LIGHT];
	int i;
	for(i=0;i<PointLightList->GetCount();i++)
	{
		RSPointLight *pLight=PointLightList->Get(i);
		if(GetPosition().GetDistance(pLight->mPosition)<
			rsm->m_BoundingSphereRadius*GetMaxScaleFactor()+pLight->mRange)
		{
			pLights[nLight]=pLight;
			nLight++;
		}
		if(nLight>=MAXIMUM_LIGHT) break;
	}

	for(i=1;i<MAXIMUM_LIGHT+1;i++)
	{
		g_pd3dDevice->LightEnable( i, false );
	}

	for(i=0;i<nLight;i++)
	{
		RSPointLight *pLight=pLights[i];
		D3DLIGHT8 light;
		ZeroMemory( &light, sizeof(D3DLIGHT8) );
		light.Type       = D3DLIGHT_POINT;
		light.Position.x = pLight->mPosition.x;
		light.Position.y = pLight->mPosition.y;
		light.Position.z = pLight->mPosition.z;
		light.Diffuse.r  = pLight->mColor.x;
		light.Diffuse.g  = pLight->mColor.y;
		light.Diffuse.b  = pLight->mColor.z;
		light.Range       = pLight->mRange;
		light.Attenuation0 = 1.0f;
		light.Attenuation1 = 0.5f;

		g_pd3dDevice->SetLight( i+1, &light );
		g_pd3dDevice->LightEnable( i+1, TRUE);

	}
}

void RSVisualObject::Draw(int iMesh,CFaces *faces)
{
	int k;

	RSSetTexture(materials[faces->iMaterial]->TextureHandle);

	RSSetCullState(
		materials[faces->iMaterial]->b2Sided || meshestoview[iMesh]->m_align!=RS_ALIGNSTYLE_NONE 
		? RS_CULLSTYLE_NONE : RS_CULLSTYLE_CCW);
	
#ifndef EL_TEST_OBJ

	if(CurrentAnimation && CurrentAnimation->GetAnimationMethod()==RSANIMATIONMETHOD_VERTEX)
	{
		rvector *pv0=CurrentAnimation->GetAnimationVertices(iMesh,(int)AnimationKey);
		rvector *pv1=CurrentAnimation->GetAnimationVertices(iMesh,(int)AnimationKey+1);
		float fp=AnimationKey-(int)AnimationKey;
		if(pv0 && pv1)
		{
			VERTEX* pVertices;
			faces->pVertexBuffer[RSGetCurrentDevice()]->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ;
			for(k=0;k<faces->nv;k++)
			{
				rvector *v0=&pv0[faces->indicies_original[k]];
				rvector *v1=&pv1[faces->indicies_original[k]];
				rvector v=*v1*fp+*v0*(1.0f-fp);
				pVertices[k].x=v.x;
				pVertices[k].y=v.y;
				pVertices[k].z=v.z;
			}
			faces->pVertexBuffer[RSGetCurrentDevice()]->Unlock();
		}
	}
	
	g_pd3dDevice->SetStreamSource( 0, faces->pVertexBuffer[RSGetCurrentDevice()], sizeof(VERTEX) );

	if(faces->ni)
	{
		g_pd3dDevice->SetIndices(faces->pIndexBuffer[RSGetCurrentDevice()],0);
		HRESULT hr=g_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,0,faces->nv, 0, faces->ni/3);
		_ASSERT(hr==D3D_OK);
	}
	else
	{
		HRESULT hr=g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0, faces->nv/3);
		_ASSERT(hr==D3D_OK);
	}

#else

	static VERTEX t_vert[10000];

	if(CurrentAnimation && CurrentAnimation->GetAnimationMethod()==RSANIMATIONMETHOD_VERTEX)
	{
		rvector *pv0=CurrentAnimation->GetAnimationVertices(iMesh,(int)AnimationKey);
		rvector *pv1=CurrentAnimation->GetAnimationVertices(iMesh,(int)AnimationKey+1);

		float fp=AnimationKey-(int)AnimationKey;

		if(pv0 && pv1)
		{
			VERTEX* pVertices = faces->tnlvertices;

			for(k=0;k<faces->nv;k++)
			{
				rvector *v0=&pv0[faces->indicies_original[k]];
				rvector *v1=&pv1[faces->indicies_original[k]];
				rvector v=*v1*fp+*v0*(1.0f-fp);
				pVertices[k].x=v.x;
				pVertices[k].y=v.y;
				pVertices[k].z=v.z;
			}
		}

		if(faces->ni)
		{
			int cnt = 0;

			for(int i=0;i<faces->ni;i++)
			{
				memcpy(t_vert+cnt,faces->tnlvertices + *(faces->indicies+i),sizeof(VERTEX));

				cnt++;

				if(cnt/3>=3000)//임시 버텍스 10000개 가진 유닛 노드는 없다~ 그래도 안전하게~
				{
					HRESULT hr = g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST,cnt/3,t_vert,sizeof(VERTEX));
					_ASSERT(hr==D3D_OK);
					cnt=0;
				}
			}

			HRESULT hr = g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST,cnt/3,t_vert,sizeof(VERTEX));
			_ASSERT(hr==D3D_OK);

		}
		else
		{
			HRESULT hr = g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST,faces->nv/3,faces->tnlvertices,sizeof(VERTEX));
			_ASSERT(hr==D3D_OK);
		}
	}
	else
	{
		g_pd3dDevice->SetStreamSource( 0, faces->pVertexBuffer[RSGetCurrentDevice()], sizeof(VERTEX) );

		if(faces->ni)
		{
			g_pd3dDevice->SetIndices(faces->pIndexBuffer[RSGetCurrentDevice()],0);
			HRESULT hr=g_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,0,faces->nv, 0, faces->ni/3);
			_ASSERT(hr==D3D_OK);
		}
		else
		{
			HRESULT hr=g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0, faces->nv/3);
			_ASSERT(hr==D3D_OK);
		}
	}

#endif

}

void RSVisualObject::Draw()
{
//	rslog("%s\n",rsm->name);
	
	RSSetZBufferState(bReadZ,bWriteZ);
//	RSSetAlphaState(m_AlphaState);
	RSSetAlphaState(RS_ALPHASTYLE_NONE);

	unsigned char opacity=255;

	if(m_AlphaState==RS_ALPHASTYLE_ADD||RS_ALPHASTYLE_ALPHA||(m_FogColor!=0xff000000))
		opacity=min(m_opacity,unsigned char(m_FogColor>>24));

	m_isRender = false;

	if(opacity==0) return;

	m_isRender = true;

	g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, VECTOR2RGB24(CombineColor));
	g_pd3dDevice->SetVertexShader(RSFVF);
	g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR , (DWORD)opacity<<24);

	g_pd3dDevice->SetTransform(D3DTS_VIEW, (_D3DMATRIX*)&RSView);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION,(_D3DMATRIX*)&RSProjection);

	if(m_hEnvTextureHandle)
	{
		RSSetTexture(1,m_hEnvTextureHandle);
        // Generate spheremap texture coords, and shift them over
        rmatrix44 mat;
        mat._11 = 0.5f; mat._12 = 0.0f; mat._13 = 0.0f; mat._14 = 0.0f; 
        mat._21 = 0.0f; mat._22 =-0.5f; mat._23 = 0.0f; mat._24 = 0.0f; 
        mat._31 = 0.0f; mat._32 = 0.0f; mat._33 = 1.0f; mat._34 = 0.0f; 
        mat._41 = 0.5f + m_fEnvBiasU; mat._42 = 0.5f + m_fEnvBiasV; mat._43 = 0.0f; mat._44 = 1.0f; 
        g_pd3dDevice->SetTransform( D3DTS_TEXTURE1, (D3DMATRIX*)&mat );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD );
	}
	else
	{
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0);
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU  );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
        g_pd3dDevice->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU  );
        g_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
	}

	int i,j;
	rmatrix mm,objmat;

	objmat=GetMatrix();
	
	CMesh *mesh;
	for(i=0;i<nMesh;i++)
	{
		mesh=meshestoview[i];
		GetSubObjectTM(&mm,&objmat,i);

		rmatrix44 world=MatrixMult(mm,IdentityMatrix44());
		g_pd3dDevice->SetTransform(D3DTS_WORLD, (_D3DMATRIX*)&world);
		
		unsigned char opac=opacity;
		if(CurrentAnimation)
		{
			opac=(unsigned char)
			(CurrentAnimation->GetVisibilityControl(i)->GetVisibility(AnimationKey)*(float)opacity);
		
			g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR , (DWORD)opac << 24 | 0xffffff);
		}

		CFaces *faces=mesh->faceshead;
		for(j=0;j<mesh->nFaces;j++)
		{
			if(faces->pVertexBuffer &&
				!(materials[faces->iMaterial]->ShadeMode!=RSSHADEMODE_NORMAL && m_hEnvTextureHandle) )
			{
				switch(materials[faces->iMaterial]->ShadeMode)
				{
				case RSSHADEMODE_NORMAL :
				{
					RSSetAlphaState(m_AlphaState);
					RSSetZBufferState(bReadZ,bWriteZ);
					g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TFACTOR  );
					g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_SELECTARG1 );
					g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
					g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2 , D3DTA_DIFFUSE );
					g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP , D3DTOP_MODULATE );
				}break;

				case RSSHADEMODE_ALPHAMAP :
					RSSetAlphaState(RS_ALPHASTYLE_ALPHAMAP);
					RSSetZBufferState(bReadZ,bWriteZ);
//					if(opac<255)
					{
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TFACTOR  );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2 , D3DTA_TEXTURE );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_MODULATE );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2 , D3DTA_DIFFUSE );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP , D3DTOP_MODULATE );
					}
/*					else
					{
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_SELECTARG1 );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2 , D3DTA_DIFFUSE );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP , D3DTOP_MODULATE );
					}
*/					break;
				case RSSHADEMODE_ADD :
					{
					RSSetAlphaState(RS_ALPHASTYLE_ADD);
					RSSetZBufferState(bReadZ,false);
					if(opac<255)
					{
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TFACTOR  );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2 , D3DTA_TEXTURE );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_MODULATE );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP , D3DTOP_SELECTARG1 );
					}
					else
					{
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_SELECTARG1 );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP , D3DTOP_SELECTARG1 );
					}
					}break;
				}

				Draw(i,faces);
			}
			faces++;
		}
	}
	if(m_hEnvTextureHandle)
	{
		RSSetTexture(1,0);
        g_pd3dDevice->SetTransform( D3DTS_TEXTURE1, (D3DMATRIX*)&IdentityMatrix44() );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU  );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
	}
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2 , D3DTA_DIFFUSE  );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP , D3DTOP_MODULATE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_DIFFUSE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_SELECTARG1 );
	RSSetCullState(RS_CULLSTYLE_NONE);
}

void RSVisualObject::DrawMono(DWORD dwColor)
{
	int i;
	unsigned char opacity=(unsigned char)(dwColor >> 24);

	RSSetZBufferState(bReadZ,bWriteZ);
	RSSetAlphaState(m_AlphaState);

	g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR , dwColor );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2 , D3DTA_TFACTOR  );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP , D3DTOP_MODULATE );
	
	RSSetTextureWrapState(TRUE);
	g_pd3dDevice->SetVertexShader(RSFVF);
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING ,false);
	
	g_pd3dDevice->SetTransform(D3DTS_VIEW, (_D3DMATRIX*)&RSView);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION,(_D3DMATRIX*)&RSProjection);

	if(m_hEnvTextureHandle)
	{
		RSSetTexture(1,m_hEnvTextureHandle);
        // Generate spheremap texture coords, and shift them over
        rmatrix44 mat;
        mat._11 = 0.5f; mat._12 = 0.0f; mat._13 = 0.0f; mat._14 = 0.0f; 
        mat._21 = 0.0f; mat._22 =-0.5f; mat._23 = 0.0f; mat._24 = 0.0f; 
        mat._31 = 0.0f; mat._32 = 0.0f; mat._33 = 1.0f; mat._34 = 0.0f; 
        mat._41 = 0.5f + m_fEnvBiasU; mat._42 = 0.5f + m_fEnvBiasV; mat._43 = 0.0f; mat._44 = 1.0f; 
        g_pd3dDevice->SetTransform( D3DTS_TEXTURE1, (D3DMATRIX*)&mat );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD );
	}

	static int key,j,k,l;
	static rmatrix mm,objmat;

	objmat=GetMatrix();
	
	CMesh *mesh;
	for(i=0;i<nMesh;i++)
	{
		mesh=meshestoview[i];
		GetSubObjectTM(&mm,&objmat,i);

		rmatrix44 world=MatrixMult(mm,IdentityMatrix44());
		g_pd3dDevice->SetTransform(D3DTS_WORLD, (_D3DMATRIX*)&world);
		
		unsigned char opac=opacity;
		if(CurrentAnimation)
		{
			opac=(unsigned char)(((DWORD)opacity*
				(DWORD)(CurrentAnimation->GetVisibilityControl(i)->GetVisibility(AnimationKey)*255.f))>>8);
			g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR , (DWORD)opac << 24 | ( dwColor & 0xffffff ) );
		}

		CFaces *faces=mesh->faceshead;
		for(int j=0;j<mesh->nFaces;j++)
		{
			if((faces->pVertexBuffer) &&
				!(materials[faces->iMaterial]->ShadeMode!=RSSHADEMODE_NORMAL && m_hEnvTextureHandle) )
			{
				switch(materials[faces->iMaterial]->ShadeMode)
				{
				case RSSHADEMODE_NORMAL :
				{
					RSSetAlphaState(m_AlphaState);
					RSSetZBufferState(bReadZ,bWriteZ);
					g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TFACTOR  );
					g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_SELECTARG1 );
				}break;

				case RSSHADEMODE_ALPHAMAP :
					RSSetAlphaState(RS_ALPHASTYLE_ALPHAMAP);
					RSSetZBufferState(bReadZ,bWriteZ);
					if(opac<255)
					{
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TFACTOR  );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2 , D3DTA_TEXTURE );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_MODULATE );
					}
					else
					{
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_SELECTARG1 );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
					}
					break;
				case RSSHADEMODE_ADD :
					{
					RSSetAlphaState(RS_ALPHASTYLE_ADD);
					RSSetZBufferState(bReadZ,false);
					if(opac<255)
					{
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TFACTOR  );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2 , D3DTA_TEXTURE );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_MODULATE );
					}
					else
					{
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_SELECTARG1 );
						g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
					}
					}break;
				}
				Draw(i,faces);
			}
			faces++;
		}
	}
	if(m_hEnvTextureHandle)
	{
		RSSetTexture(1,0);
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	}
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2 , D3DTA_DIFFUSE  );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP , D3DTOP_MODULATE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_DIFFUSE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_SELECTARG1 );
	RSSetCullState(RS_CULLSTYLE_NONE);
}

bool RSVisualObject::DrawReflect(rvector dir,rvector apoint)
{
	return TRUE;
}

void RSVisualObject::DrawShadow(ASSMap *TargetMap)
{
	switch(nShadowStyle)
	{
	case RS_SHADOWSTYLE_DRAFT:		DrawDraftShadow();break;
	case RS_SHADOWSTYLE_FLAT:		DrawFlatShadow(RSLightDirection);break;
	case RS_SHADOWSTYLE_TEXTUREPROJECTION:			DrawTextureProjectionShadow(TargetMap,RSLightDirection);break;
	case RS_SHADOWSTYLE_DETAILTEXTUREPROJECTION:	DrawDetailTextureProjectionShadow(TargetMap,RSLightDirection);break;
	}
}

void RSVisualObject::DrawDraftShadow()
{
	/*
	RSSetAlphaState(RS_ALPHASTYLE_SHADOW);
	RSSetTexture(ShadowTextureHandle);

	rmatrix44 tm=m_ShadowProjectionMatrix*RSViewProjectionViewport;
	for(int i=0;i<4;i++)
	{
		TransformVertex(shadowver[i],tm);
		shadowver[i].sz-=DELTAZ;
		shadowver[i].color=0xffffff;
	}
	RSDrawFace(&shadowface[0]);
	RSDrawFace(&shadowface[1]);
	*/
}

void RSVisualObject::DrawFlatShadow(rvector &lightdir)
{
	RSSetAlphaState(RS_ALPHASTYLE_ALPHA);

	static int key;
	static rmatrix mm,*temp,obj;
	static rmatrix44 tm;

	obj=GetMatrix();
	// m_ShadowProjectionMatrix는 bbtest때 이미 계산되었다.
	CMesh *mesh;
	for (int i=0;i<nMesh;i++)
	{
		mesh=meshestoview[i];
		GetSubObjectTM(&mm,&m_ShadowProjectionMatrix,i);
		tm=mm * TranslateMatrix44(0,0,0.01f) * RSViewProjectionViewport;

		static rvector light;
		light=Normalize(TransformNormal(-lightdir,MatrixInverse(obj)));

		rvertex *trv=mesh->ver;
		for(int j=0;j<mesh->nV;j++)
		{
			TransformVertexASM(*trv,tm);
			trv->color=0;
			_ASSERT((trv->normal.x!=0)||(trv->normal.y!=0)||(trv->normal.z!=0));
			trv->color=(m_FogColor >> 1 ) & 0xff000000;
			trv++;
		}
		/*
		CFaces *faces=mesh->faceshead;
		for(j=0;j<mesh->nFaces;j++)
		{
			int k;
			rface *face=faces->face;
			for(k=0;k<faces->nf;k++)
			{
				face->isValid=DotProduct(light,face->normal)<0;
				face++;
			}

			faces->Draw();
			faces++;
		}
		*/
		tps+=mesh->nFaces;
	}
}


rmatrix44 SetFrustumMatrix( FLOAT fLeft, FLOAT fRight, FLOAT fTop,
                                     FLOAT fBottom, FLOAT fNearPlane, FLOAT fFarPlane )
{
	rmatrix44 mat;

    float Q = fFarPlane / ( fFarPlane - fNearPlane );

    ZeroMemory( &mat, sizeof(D3DMATRIX) );
    mat._11 = ( 2.0f*fNearPlane )/( fRight - fLeft );
    mat._22 = ( 2.0f*fNearPlane )/( fTop - fBottom );
    mat._31 = ( fRight + fLeft )/ (fRight - fLeft );
    mat._32 = ( fTop + fBottom )/ (fTop - fBottom );
    mat._33 = Q;
    mat._34 = 1.0f;
    mat._43 = -Q*fNearPlane;
	
	return mat;
}

rmatrix44 SetAdjustedProjectionMatrix(FLOAT fFOV, FLOAT fAspect,
                                     FLOAT fNearPlane, FLOAT fFarPlane)
{
	rmatrix44 mat;

    float h =   1.0f  * ( cosf(fFOV/2)/sinf(fFOV/2) );

    float fTop = fNearPlane/h;
    float fBottom = -fTop;
    float fRight = fTop * fAspect;
    float fLeft = -fRight;

    mat=SetFrustumMatrix( fLeft, fRight, fTop, fBottom, fNearPlane, fFarPlane );

    return mat;
}

#define SHADOW_DARKEST_COLOR 0x40

void RSVisualObject::DrawTextureProjectionShadow(ASSMap *TargetMap,rvector &lightdir)
{
	unsigned char opacity=255;

	if((m_AlphaState==RS_ALPHASTYLE_ADD)||
		(m_AlphaState==RS_ALPHASTYLE_ALPHA)||	(m_FogColor!=0xff000000))
		opacity=min(m_opacity,unsigned char(m_FogColor>>24));
	if(opacity==0) return;
	opacity=(unsigned char)(((DWORD)opacity*SHADOW_DARKEST_COLOR)>>8);
	DWORD dwColor=opacity<<16 | opacity<<8 | opacity;

#define EFFECT_TEXTURE_SIZE	0.75f

	float flightdistance=30.0f;
	rvector position=m_vShadowPicked.x>0 ? m_vShadowPicked : GetWorldPosition();
	
	rmatrix44 LightView,LightProjection;

	rvector thisdir=m_dir;
	if(parent)
		thisdir=TransformNormal(thisdir,parent->GetMatrix());
	float fScale=1.2f/((float)(rsm->m_BoundingSphereRadiusXY)*GetMaxScaleFactor());
	LightView=ViewMatrix44(position,rvector(0,0,-1),rvector(thisdir.x,thisdir.y,0));
	LightProjection=ScaleMatrix44(fScale)*IdentityMatrix44();
//*/

	g_pd3dDevice->SetTransform(D3DTS_WORLD, (_D3DMATRIX*)&IdentityMatrix44());
	g_pd3dDevice->SetTransform(D3DTS_VIEW, (_D3DMATRIX*)&RSView);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION,(_D3DMATRIX*)&RSProjection);
	RSSetZBufferState(true,false);
	
	// face 정보
	static rvector normals[6]= { rvector(0,0,1),rvector(-1,0,0),rvector(0,1,0),
								rvector(1,0,0),rvector(0,-1,0),rvector(0,0,-1) };
	
	// face에 속한 버텍스중 하나. ( visibility test를 위함 )
	static int facever[6]= { 0, 3, 2, 1, 0, 4 };
	static bool facevisibility[6];
	
	// line 정보, 0,1은 vertex index 2,3은 인접한 면 index
	static int lines[12][4]= { 
		{0,1,0,3}, {1,2,0,2}, {2,3,0,1}, {3,0,0,4},
		{0,4,4,3}, {1,5,3,2}, {2,6,2,1}, {3,7,1,4},
		{4,5,5,3}, {5,6,5,2}, {6,7,5,1}, {7,4,5,4} };


    rmatrix model = GetMatrix();
	rvector lightposition=position-lightdir*flightdistance;

	int i,nFace=0;
	for(i=0;i<8;i++) TransformVertex(m_BoundingVertices[i],model);
	for(i=0;i<6;i++)
	{
		facevisibility[i]=DotProduct(TransformNormal(normals[i],model),lightdir)
//			*(rvector*)&(m_BoundingVertices[facever[i]].sx)-lightposition)
			>0.f;
		nFace++;
	}

//*/

	rplane planes[12+2];
	int nEdge=0;

	rplueckercoord centerline=rplueckercoord(position,position-lightdir);
	for(i=0;i<12;i++)
	{
		if(facevisibility[lines[i][2]]!=facevisibility[lines[i][3]])	// 한쪽면은 보이고 다른쪽은 안보이면
		{
			int a=lines[i][0],b=lines[i][1];
			if(rplueckercoord(*(rvector*)&m_BoundingVertices[a].sx,*(rvector*)&m_BoundingVertices[b].sx)
				*centerline>0)
				swap(a,b);
			SetupPlane(
				
					*(rvector*)&m_BoundingVertices[a].sx,
					*(rvector*)&m_BoundingVertices[b].sx,
					*(rvector*)&m_BoundingVertices[b].sx+lightdir,
					planes+nEdge);
			nEdge++;
			
			/*// draw selected edge
			{
				rvector *pa=(rvector*)&m_BoundingVertices[a].sx;
				rvector *pb=(rvector*)&m_BoundingVertices[b].sx;

				LITVERTEX v[2];
				v[0].x=pa->x;v[0].y=pa->y;v[0].z=pa->z;
				v[0].Diffuse=0xff0000;
				v[1].x=pb->x;v[1].y=pb->y;v[1].z=pb->z;
				v[1].Diffuse=0xff0000;
				g_pd3dDevice->SetTexture(0,0);
				g_pd3dDevice->SetTransform(D3DTS_WORLD, (_D3DMATRIX*)&IdentityMatrix44());
				g_pd3dDevice->SetVertexShader(RSLFVF);
				g_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST,1,v,sizeof(LITVERTEX));
			}//*/
		}
	}

#define FARZPLANEDIST	20
	SetupPlane(lightdir,RSVisualObject::position,planes+(nEdge++));
	SetupPlane(-lightdir,RSVisualObject::position+lightdir*FARZPLANEDIST,planes+(nEdge++));

	RSCellList cl;
	if(nEdge)
		TargetMap->GetCellListWRTPlanes(&cl,planes,nEdge);

 	/*// test code for show chosen polygons
	{
		ASSCell *cell;
		LITVERTEX verts[4];
		MVERTEX	*pVertices;
		TargetMap->GetVertexBuffer()->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ;
		RSSetTexture(0);
		RSSetAlphaState(RS_ALPHASTYLE_ADD);
		g_pd3dDevice->SetVertexShader(RSLFVF);
		for(i=0;i<cl.GetCount();i++)
		{
			cell=cl.Get(i);
			memcpy(verts+0,pVertices + cell->pind[0],sizeof(LITVERTEX));
			memcpy(verts+1,pVertices + cell->pind[1],sizeof(LITVERTEX));
			memcpy(verts+2,pVertices + cell->pind[2],sizeof(LITVERTEX));
			memcpy(verts+3,pVertices + cell->pind[5],sizeof(LITVERTEX));
			for(int j=0;j<4;j++)
				verts[j].Diffuse=0x40ffffff;
			g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,verts,sizeof(LITVERTEX));
		}
		TargetMap->GetVertexBuffer()->Unlock();
	}
	//*/

	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS,  D3DTTFF_PROJECTED | D3DTTFF_COUNT3 );
	
	
	g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR , dwColor);
	g_pd3dDevice->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_MODULATE);
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2 , D3DTA_TFACTOR);


	rmatrix44 tcm=IdentityMatrix44();
	tcm._11=0.5f;tcm._22=-0.5f;tcm._33=0;
	tcm._41=0.5f;tcm._42=0.5f;tcm._43=1;
	rmatrix44 ttm=MatrixInverse(RSView)*LightView*LightProjection*tcm;
	g_pd3dDevice->SetTransform( D3DTS_TEXTURE0 , (_D3DMATRIX*)&ttm );

	RSSetAlphaState(RS_ALPHASTYLE_MARKS);
	RSSetTexture(rsm->GetShadowTextureHandle());

	RSSetTextureWrapState(false);
	
	g_pd3dDevice->SetVertexShader(RSMFVF);

	if(g_bHardwareTNL)
	{
		int nCount=0;
		WORD *pIndicies=NULL;

		g_pd3dDevice->SetIndices(TargetMap->GetIndexBuffer(),0);
		g_pd3dDevice->SetStreamSource(0,TargetMap->GetVertexBuffer(),sizeof(MVERTEX));

		TargetMap->GetIndexBuffer()->Lock(0, 0, (BYTE**)&pIndicies, D3DLOCK_DISCARD );
		
		for(i=0;i<cl.GetCount();i++)
		{
			ASSCell *cell=cl.Get(i);
			memcpy(pIndicies+(nCount++)*6,cell->pind,sizeof(WORD)*6);
			if(nCount==BUFFERCOUNT)
			{
				nCount=0;
				TargetMap->GetIndexBuffer()->Unlock();
				g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,
					(TargetMap->GetMapSizeX()+1)*(TargetMap->GetMapSizeY()+1)*2,0,nCount*2);
				TargetMap->GetIndexBuffer()->Lock(0, 0, (BYTE**)&pIndicies, D3DLOCK_DISCARD );
			}
		}
		TargetMap->GetIndexBuffer()->Unlock();
		if(nCount)
		g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,
					(TargetMap->GetMapSizeX()+1)*(TargetMap->GetMapSizeY()+1)*2,0,nCount*2);

	}else
	{
		MVERTEX* pVertices;
		TargetMap->GetVertexBuffer()->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ;
		MVERTEX ver[4];
		for(i=0;i<cl.GetCount();i++)
		{
			ASSCell *cell=cl.Get(i);
			memcpy(ver,pVertices+cell->pind[0],sizeof(MVERTEX));
			memcpy(ver+1,pVertices+cell->pind[1],sizeof(MVERTEX));
			memcpy(ver+2,pVertices+cell->pind[2],sizeof(MVERTEX));
			memcpy(ver+3,pVertices+cell->pind[5],sizeof(MVERTEX));
			g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,ver,sizeof(MVERTEX));
		}
		TargetMap->GetVertexBuffer()->Unlock();
	}

	g_pd3dDevice->SetTransform(D3DTS_WORLD, (_D3DMATRIX*)&IdentityMatrix44());
	g_pd3dDevice->SetTransform(D3DTS_VIEW, (_D3DMATRIX*)&RSView);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION,(_D3DMATRIX*)&RSProjection);
		
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2 , D3DTA_DIFFUSE);
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2 , D3DTA_DIFFUSE);

	RSSetAlphaState(RS_ALPHASTYLE_NONE);
	RSSetTextureWrapState(true);
	
	cl.DeleteRecordAll();

}

void RSVisualObject::DrawDetailTextureProjectionShadow(ASSMap *TargetMap,rvector &lightdir)
{
	if(!g_pShadowTexture) return;

	unsigned char opacity=255;

	if((m_AlphaState==RS_ALPHASTYLE_ADD)||
		(m_AlphaState==RS_ALPHASTYLE_ALPHA)||	(m_FogColor!=0xff000000))
		opacity=min(m_opacity,unsigned char(m_FogColor>>24));
	if(opacity==0) return;
	opacity=(unsigned char)(((DWORD)opacity*SHADOW_DARKEST_COLOR)>>8);
	DWORD dwColor=opacity<<16 | opacity<<8 | opacity;

#define EFFECT_TEXTURE_SIZE	0.75f

	float flightdistance=30.0f;
	float angle=(float)atan(rsm->m_BoundingSphereRadius/flightdistance);

	rvector position=GetWorldPosition();
	
	rmatrix44 LightView,LightProjection;

	LightView=ViewMatrix44(position-lightdir*flightdistance,lightdir,rvector(0,0,1));
	rmatrix44 TempProjection=SetAdjustedProjectionMatrix( pi/4.0f, 1.0f, 1.0f, 500.0f);
	rvector right=Normalize(CrossProduct(rvector(0,0,1),lightdir));
	rvector tright=TransformVector(position+right*GetMaxScaleFactor()*rsm->m_BoundingSphereRadius,LightView*TempProjection);
	float fScaleFactor=EFFECT_TEXTURE_SIZE/tright.x;
	rmatrix44 sm=IdentityMatrix44();
	sm._11=sm._22=fScaleFactor;
	LightProjection=TempProjection*sm;

	LPDIRECT3DSURFACE8 pss,backup,backupz;
	g_pShadowTexture->GetSurfaceLevel(0,&pss);
	g_pd3dDevice->GetRenderTarget(&backup);
	g_pd3dDevice->GetDepthStencilSurface(&backupz);
	g_pd3dDevice->SetRenderTarget(pss,NULL);

	g_pd3dDevice->Clear(0,NULL,D3DCLEAR_TARGET,0,1.0f ,0);
	g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0);
	g_pd3dDevice->SetVertexShader(RSFVF);
	RSSetFogState(false,0);
	RSSetZBufferState(false,false);
	RSSetTexture(0);
	
	RSSetCullState(RS_CULLSTYLE_NONE);
	g_pd3dDevice->SetTransform(D3DTS_VIEW, (_D3DMATRIX*)&LightView);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION,(_D3DMATRIX*)&LightProjection);
	g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR , (DWORD)0xffffff);
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TFACTOR  );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP , D3DTOP_SELECTARG1 );

	int i,j;
	rmatrix mm,objmat;

	objmat=GetMatrix();
	
	CMesh *mesh;
	for(i=0;i<nMesh;i++)
	{
		mesh=meshestoview[i];
		GetSubObjectTM(&mm,&objmat,i);

		rmatrix44 world=MatrixMult(mm,IdentityMatrix44());
		g_pd3dDevice->SetTransform(D3DTS_WORLD, (_D3DMATRIX*)&world);
		
		CFaces *faces=mesh->faceshead;
		for(j=0;j<mesh->nFaces;j++)
		{
			if(faces->pVertexBuffer && materials[faces->iMaterial]->ShadeMode!=RSSHADEMODE_ADD)
				Draw(i,faces);
			faces++;
		}
	}

	g_pd3dDevice->SetRenderTarget(backup,backupz);
	backup->Release();
	backupz->Release();
	pss->Release();

	g_pd3dDevice->SetTransform(D3DTS_WORLD, (_D3DMATRIX*)&IdentityMatrix44());
	g_pd3dDevice->SetTransform(D3DTS_VIEW, (_D3DMATRIX*)&RSView);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION,(_D3DMATRIX*)&RSProjection);
	RSSetZBufferState(true,false);
	
	/*// show rendered shadow texture
	{
		if(nShadowStyle==RS_SHADOWSTYLE_TEXTUREPROJECTION)
			RSSetTexture(ShadowTextureHandle);
		else
			g_pd3dDevice->SetTexture(0,g_pShadowTexture);
	
		g_pd3dDevice->SetVertexShader(RSTLFVF);
		RSSetAlphaState(RS_ALPHASTYLE_NONE);
		g_pd3dDevice->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_MODULATE);
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2 , D3DTA_DIFFUSE);
		TRANSLITVERTEX v[4]= { {0.f,0.f,0.f,1.0f,0xffffffff,0,0.f,0.f } ,
			{100.f,0.f,0.f,1.0f,0xffffffff,0,1.f,0.f } ,
			{0.f,100.f,0.f,1.0f,0xffffffff,0,0.f,1.f } ,
		{100.f,100.f,0.f,1.0f,0xffffffff,0,1.f,1.f } };
		g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,v,sizeof(TRANSLITVERTEX));
	}
	//*/

	// face 정보
	static rvector normals[6]= { rvector(0,0,1),rvector(-1,0,0),rvector(0,1,0),
								rvector(1,0,0),rvector(0,-1,0),rvector(0,0,-1) };
	
	// face에 속한 버텍스중 하나. ( visibility test를 위함 )
	static int facever[6]= { 0, 3, 2, 1, 0, 4 };
	static bool facevisibility[6];
	
	// line 정보, 0,1은 vertex index 2,3은 인접한 면 index
	static int lines[12][4]= { 
		{0,1,0,3}, {1,2,0,2}, {2,3,0,1}, {3,0,0,4},
		{0,4,4,3}, {1,5,3,2}, {2,6,2,1}, {3,7,1,4},
		{4,5,5,3}, {5,6,5,2}, {6,7,5,1}, {7,4,5,4} };


    rmatrix model = GetMatrix();
	rvector lightposition=position-lightdir*flightdistance;

	int nFace=0;
	for(i=0;i<8;i++) TransformVertex(m_BoundingVertices[i],model);
	for(i=0;i<6;i++)
	{
		facevisibility[i]=DotProduct(TransformNormal(normals[i],model),
			*(rvector*)&(m_BoundingVertices[facever[i]].sx)-lightposition)>0.f;
		nFace++;
	}

//*/

	rplane planes[12+2];
	int nEdge=0;

	rplueckercoord centerline=rplueckercoord(position,position-lightdir);
	for(i=0;i<12;i++)
	{
		if(facevisibility[lines[i][2]]!=facevisibility[lines[i][3]])	// 한쪽면은 보이고 다른쪽은 안보이면
		{
			int a=lines[i][0],b=lines[i][1];
			if(rplueckercoord(*(rvector*)&m_BoundingVertices[a].sx,*(rvector*)&m_BoundingVertices[b].sx)
				*centerline>0)
				swap(a,b);
			SetupPlane(lightposition,
				
					*(rvector*)&m_BoundingVertices[b].sx,
					*(rvector*)&m_BoundingVertices[a].sx,
					planes+nEdge);
			nEdge++;

			/*// draw selected edge
			{
				rvector *pa=(rvector*)&m_BoundingVertices[a].sx;
				rvector *pb=(rvector*)&m_BoundingVertices[b].sx;

				LITVERTEX v[2];
				v[0].x=pa->x;v[0].y=pa->y;v[0].z=pa->z;
				v[0].Diffuse=0xff0000;
				v[1].x=pb->x;v[1].y=pb->y;v[1].z=pb->z;
				v[1].Diffuse=0xff0000;
				g_pd3dDevice->SetTexture(0,0);
				g_pd3dDevice->SetTransform(D3DTS_WORLD, (_D3DMATRIX*)&IdentityMatrix44());
				g_pd3dDevice->SetVertexShader(RSLFVF);
				g_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST,1,v,sizeof(LITVERTEX));
			}//*/

		}
	}

//#define FARZPLANEDIST	20
	SetupPlane(lightdir,RSVisualObject::position,planes+(nEdge++));
	SetupPlane(-lightdir,RSVisualObject::position+lightdir*FARZPLANEDIST,planes+(nEdge++));

	RSCellList cl;
	if(nEdge)
		TargetMap->GetCellListWRTPlanes(&cl,planes,nEdge);

	/*// test code for show chosen polygons
	{
		ASSCell *cell;
		LITVERTEX verts[4];
		MVERTEX	*pVertices;
		TargetMap->GetVertexBuffer()->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ;
		RSSetTexture(0);
		RSSetAlphaState(RS_ALPHASTYLE_ADD);
		g_pd3dDevice->SetVertexShader(RSLFVF);
		for(i=0;i<cl.GetCount();i++)
		{
			cell=cl.Get(i);
			memcpy(verts+0,pVertices + cell->pind[0],sizeof(LITVERTEX));
			memcpy(verts+1,pVertices + cell->pind[1],sizeof(LITVERTEX));
			memcpy(verts+2,pVertices + cell->pind[2],sizeof(LITVERTEX));
			memcpy(verts+3,pVertices + cell->pind[5],sizeof(LITVERTEX));
			for(int j=0;j<4;j++)
				verts[j].Diffuse=0x40ffffff;
			g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,verts,sizeof(LITVERTEX));
		}
		TargetMap->GetVertexBuffer()->Unlock();
	}
	//*/

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS,  D3DTTFF_PROJECTED | D3DTTFF_COUNT3 );
	
	
	g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR , dwColor);
	g_pd3dDevice->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_MODULATE);
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2 , D3DTA_TFACTOR);


	rmatrix44 tcm=IdentityMatrix44();
	tcm._11=0.5f;tcm._22=-0.5f;tcm._33=0;
	tcm._41=0.5f;tcm._42=0.5f;tcm._43=1;
	rmatrix44 ttm=MatrixInverse(RSView)*LightView*LightProjection*tcm;
	g_pd3dDevice->SetTransform( D3DTS_TEXTURE0 , (_D3DMATRIX*)&ttm );

	RSSetAlphaState(RS_ALPHASTYLE_MARKS);
	g_pd3dDevice->SetTexture(0,g_pShadowTexture);

	RSSetTextureWrapState(false);
	
	g_pd3dDevice->SetVertexShader(RSMFVF);

	if(g_bHardwareTNL)
	{
#ifndef EL_TEST

		int nCount=0;
		WORD *pIndicies=NULL;

		g_pd3dDevice->SetIndices(TargetMap->GetIndexBuffer(),0);
		g_pd3dDevice->SetStreamSource(0,TargetMap->GetVertexBuffer(),sizeof(MVERTEX));

		TargetMap->GetIndexBuffer()->Lock(0, 0, (BYTE**)&pIndicies, D3DLOCK_DISCARD );
		
		for(i=0;i<cl.GetCount();i++)
		{
			ASSCell *cell=cl.Get(i);
			memcpy(pIndicies+(nCount++)*6,cell->pind,sizeof(WORD)*6);
			if(nCount==BUFFERCOUNT)
			{
				nCount=0;
				TargetMap->GetIndexBuffer()->Unlock();
				g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,
					(TargetMap->GetMapSizeX()+1)*(TargetMap->GetMapSizeY()+1)*2,0,nCount*2);
				TargetMap->GetIndexBuffer()->Lock(0, 0, (BYTE**)&pIndicies, D3DLOCK_DISCARD );
			}
		}
		TargetMap->GetIndexBuffer()->Unlock();
		if(nCount)
		g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,
					(TargetMap->GetMapSizeX()+1)*(TargetMap->GetMapSizeY()+1)*2,0,nCount*2);
#else
		// 테스트 임시 코드

		MVERTEX* ver;
		int cnt = 0;

		static MVERTEX	t_vertex[10000];

		MVERTEX* t_point_list = TargetMap->GetVertexList();
		
		for(i=0;i<cl.GetCount();i++)
		{
			ASSCell *cell = cl.Get(i);

			ver = (t_vertex + (cnt*6));

			memcpy(ver  ,t_point_list+cell->pind[0],sizeof(MVERTEX));
			memcpy(ver+1,t_point_list+cell->pind[1],sizeof(MVERTEX));
			memcpy(ver+2,t_point_list+cell->pind[2],sizeof(MVERTEX));
			memcpy(ver+3,t_point_list+cell->pind[3],sizeof(MVERTEX));
			memcpy(ver+4,t_point_list+cell->pind[4],sizeof(MVERTEX));
			memcpy(ver+5,t_point_list+cell->pind[5],sizeof(MVERTEX));

			cnt++;

			if(cnt*6 > 9900)
			{
				g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST,cnt*2,t_vertex,sizeof(MVERTEX));
				cnt = 0;
			}
		}
		
		if(cnt) g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST,cnt*2,t_vertex,sizeof(MVERTEX));

//		TargetMap->GetIndexBuffer()->Unlock();

//		if(nCount)
//			g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,(TargetMap->GetMapSizeX()+1)*(TargetMap->GetMapSizeY()+1)*2,0,nCount*2);

/*
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

#endif

	}else
	{

#ifndef EL_TEST

		MVERTEX* pVertices;
		TargetMap->GetVertexBuffer()->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ;
		MVERTEX ver[4];
		for(i=0;i<cl.GetCount();i++)
		{
			ASSCell *cell=cl.Get(i);
			memcpy(ver,pVertices+cell->pind[0],sizeof(MVERTEX));
			memcpy(ver+1,pVertices+cell->pind[1],sizeof(MVERTEX));
			memcpy(ver+2,pVertices+cell->pind[2],sizeof(MVERTEX));
			memcpy(ver+3,pVertices+cell->pind[5],sizeof(MVERTEX));
			g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,ver,sizeof(MVERTEX));
		}
		TargetMap->GetVertexBuffer()->Unlock();
#else

#endif

	}


	g_pd3dDevice->SetTransform(D3DTS_WORLD, (_D3DMATRIX*)&IdentityMatrix44());
	g_pd3dDevice->SetTransform(D3DTS_VIEW, (_D3DMATRIX*)&RSView);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION,(_D3DMATRIX*)&RSProjection);
		
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2 , D3DTA_DIFFUSE);
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2 , D3DTA_DIFFUSE);

	RSSetAlphaState(RS_ALPHASTYLE_NONE);
	RSSetTextureWrapState(true);
	
	cl.DeleteRecordAll();

}

void RSVisualObject::GetRect(LPFRECT rect,float *z)
{
#define MAXSCREEN 99999999.9f
	static rmatrix44	mm;
	float retz=0;

	rect->x1=rect->y1=MAXSCREEN;rect->x2=rect->y2=-MAXSCREEN;

	mm= MatrixMult( GetMatrix(),RSViewProjectionViewport );
	for(int j=0;j<8;j++)
	{
//		m_BoundingVertices[j]=TransformVector(m_BoundingVertices[j],mm);
		TransformVertex(m_BoundingVertices[j],mm);
		if(rect->x1>m_BoundingVertices[j].sx) rect->x1=m_BoundingVertices[j].sx;
		if(rect->x2<m_BoundingVertices[j].sx) rect->x2=m_BoundingVertices[j].sx;
		if(rect->y1>m_BoundingVertices[j].sy) rect->y1=m_BoundingVertices[j].sy;
		if(rect->y2<m_BoundingVertices[j].sy) rect->y2=m_BoundingVertices[j].sy;
		retz+=m_BoundingVertices[j].sz;
	}
	retz/=8;
	*z = retz;
}

bool RSVisualObject::ReplaceSubObject(const char *TargetName,
									const RSObject *obj,
									const char *SourceName)
{
	int i,j=-1;
	for(i=0;i<nMesh;i++)
	{
		if(!strcmp(rsm->meshes[i].name,TargetName))
			j=i;
	}
	if(j>=0)
	{
		for(i=0;i<obj->nMesh;i++)
		{
			if(!strcmp(SourceName,obj->meshes[j].name))
			{
				meshestoview[j]=obj->meshes+j;
				_RPT1(_CRT_WARN,"%s is Replaced.\n",obj->meshes[j].name);
				return TRUE;
			}
		}
	}
	return FALSE;
}

void RSVisualObject::MoveAnimation(float elapsedtime)
{
	if(!CurrentAnimation) return;
	if(AnimationStyle>=RS_ANIMATIONSTYLE_ROUND)
	{
		AnimationKey+=elapsedtime*0.01f*CurrentAnimation->GetSpeed();
		if(AnimationStyle==RS_ANIMATIONSTYLE_ROUND)
		{
			AnimationKey=
				AnimationKey-CurrentAnimation->GetFrameCount()*
				(int)(AnimationKey/CurrentAnimation->GetFrameCount());
		}
		else
		if(AnimationKey>CurrentAnimation->GetFrameCount())
			AnimationKey=(float)CurrentAnimation->GetFrameCount()-0.0001f;
	}
}

bool RSVisualObject::Pick(const rvector &origin,const rvector &target,rvector *ret)
{
	bool bPickFound;
	rvector vOrigin,vTarget;
	rplueckercoord PickLine;
	float fPickDist;

	// convert world coordinate to model coordinate
	rmatrix model=MatrixInverse(GetMatrix());
	vOrigin=origin;vOrigin=TransformVector(vOrigin,model);
	vTarget=target;vTarget=TransformVector(vTarget,model);

	bPickFound=false;
	PickLine=rplueckercoord(vOrigin,vTarget);

	if(!isLineIntersectBoundingBox(&PickLine,&m_BoundingBox))
		return false;

	int i,j,k;
	CMesh *mesh;

	for(i=0;i<nMesh;i++)
	{
		mesh=meshestoview[i];
		CFaces *faces=mesh->faceshead;
		for(j=0;j<mesh->nFaces;j++)
		{
			bool bVertexAnimation=(faces->indicies_original!=NULL);
			int nCount=(bVertexAnimation ? faces->nv : faces->ni)/3;
			for(k=0;k<nCount;k++)
			{
				rvector *v1,*v2,*v3;
				if(bVertexAnimation)
				{
					WORD ind=k*3;
					v1=(rvector*)&faces->tnlvertices[ind].x;
					v2=(rvector*)&faces->tnlvertices[(ind+1)].x;
					v3=(rvector*)&faces->tnlvertices[(ind+2)].x;
				} else
				{
					WORD *ind;
					ind=faces->indicies+k*3;
					v1=(rvector*)&faces->tnlvertices[*ind].x;
					v2=(rvector*)&faces->tnlvertices[*(ind+1)].x;
					v3=(rvector*)&faces->tnlvertices[*(ind+2)].x;
				}

				if(IsIntersect(PickLine,*v1,*v2,*v3))
				{
					rplane plane;
					SetupPlane(*v1,*v2,*v3,&plane);
					rvector normal=rvector(plane.a,plane.b,plane.c);
					float t=(DotProduct(vOrigin,normal)+plane.d)
							/DotProduct(normal,PickLine.u);
					rvector vColPos=vOrigin-t*PickLine.u;
					float dist=vColPos.GetDistance(vOrigin);
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
			faces++;
		}
	}
	return bPickFound;
}


bool RSVisualObject::isContainThisPoint(int x,int y,float *distance)
{
	rvector from,to;
	from.x=(float)x;from.y=(float)y;from.z=0;
	to.x=(float)x;to.y=(float)y;to.z=1;

	rmatrix44 inv=MatrixInverse(RSViewProjectionViewport);
	from=TransformVector(from,inv);
	to=TransformVector(to,inv);

	rvector ret;
	if(Pick(from,to,&ret))
	{
		*distance=ret.GetDistance(from);
		return true;
	}

	return false;
}

rvector RSVisualObject::GetScreenCoordinate(rvector &coord3d)
{
    return TransformVector(coord3d,MatrixMult(GetMatrix(),RSViewProjectionViewport));
}

rvector RSVisualObject::GetWorldCoordinate(rvector &coord3d)
{
    return TransformVector(coord3d,GetMatrix());
}

// fast object-axis bounding box test.
// first idea was from dubble's. but i think there may be better algorithm.
bool RSVisualObject::BoundingBoxTest(rplane *planes)
{
	/*
	static rplane p;
	static rmatrix m;
	m=MatrixInverse(GetMatrix());
	TransformPlane(m,&p,planes);
	if(!isInPlane(&m_BoundingBox,&p)) return FALSE;
	TransformPlane(m,&p,planes+1);
	if(!isInPlane(&m_BoundingBox,&p)) return FALSE;
	TransformPlane(m,&p,planes+2);
	if(!isInPlane(&m_BoundingBox,&p)) return FALSE;
	TransformPlane(m,&p,planes+3);
	if(!isInPlane(&m_BoundingBox,&p)) return FALSE;
	TransformPlane(m,&p,planes+5);
	if(!isInPlane(&m_BoundingBox,&p)) return FALSE;
	return TRUE;
*/

	/*
	static rplane p;
	static rmatrix m;
	m=MatrixInverse(GetMatrix());
	TransformPlane(m,&p,planes+4);
	float min,max;
	GetDistanceMinMax(m_BoundingBox,p,&min,&max);
	if(min*max<0) return FALSE;
*/
	rmatrix tm=GetMatrix();
	rvector wpos;
	wpos=rvector(tm._41,tm._42,tm._43);
	return isInViewFrustrum(wpos,rsm->m_BoundingSphereRadius*GetMaxScaleFactor(),planes);
}

bool RSVisualObject::BoundingBoxTest()
{
	rplane *planes=RSViewFrustrum;
	rmatrix tm=GetMatrix();
	rvector wpos;
	wpos=rvector(tm._41,tm._42,tm._43);
	return isInViewFrustrum(wpos,rsm->m_BoundingSphereRadius*GetMaxScaleFactor(),planes);
}

bool RSVisualObject::BoundingBoxTest_Shadow(rplane *planes,ASSMap *TargetMap)
{
	rmatrix tm=GetMatrix();

	rvector wpos,target;
	wpos=rvector(tm._41,tm._42,tm._43);

	target=wpos+RSLightDirection;
	if(!TargetMap->Pick(&wpos,&target,&m_vShadowPicked))
		return false;

	return isInViewFrustrum(m_vShadowPicked,rsm->m_BoundingSphereRadius*GetMaxScaleFactor(),planes);
	/*

	// flat shadow를 가정하고 테스트.
	m_ShadowProjectionMatrix = tm*ShadowProjectionMatrix(rvector(0,0,1),
				m_vShadowPicked,
				-RSLightDirection);
	int i;

	for(i=0;i<8;i++)
	{
		TransformVertex(m_BoundingVertices[i],m_ShadowProjectionMatrix);
		if(isInViewFrustrum(*(rvector*)&(m_BoundingVertices[i].sx),RSViewFrustrum))
			return true;
	}
	return false;	
	*/
}

bool RSVisualObject::BoundingSphereTest(rvector *center,float radius)
{
	float dist=(GetWorldCoordinate(rvector(0,0,0))-*center).GetMagnitude();
	if(dist<rsm->m_BoundingSphereRadius*MaxScaleFactor+radius) return true;
	return false;
}

bool RSVisualObject::ReplaceMaterial(const char *MaterialName,RSMaterial *pMaterial)
{
	int i;
	for(i=0;i<nMaterial;i++)
	{
		if(strcmp(materials[i]->Name,MaterialName)==0)
		{
			materials[i]=pMaterial;
			return true;
		}
	}
	return false;
}

void RSVisualObject::DrawNormal(float fLineLength)
{
	static int key,i,j,k;
	static rmatrix mm,objmat;
	static rmatrix44 tm;
	rvertex trv,endv;
	endv.color=0xff0000;

	objmat=GetMatrix();
	
	g_pd3dDevice->SetVertexShader(RSLFVF);
	RSSetAlphaState(RS_ALPHASTYLE_NONE);

	CMesh *mesh;
	for (i=0;i<nMesh;i++)
	{
		mesh=meshestoview[i];
		GetSubObjectTM(&mm,&objmat,i);

		rmatrix44 world=MatrixMult(mm,IdentityMatrix44());
		g_pd3dDevice->SetTransform(D3DTS_WORLD, (_D3DMATRIX*)&world);

		for(j=0;j<mesh->nFaces;j++)
		{
			CFaces *faces=&mesh->faceshead[j];
			for(k=0;k<faces->nv;k++)
			{
				trv.x=faces->tnlvertices[k].x;
				trv.y=faces->tnlvertices[k].y;
				trv.z=faces->tnlvertices[k].z;
				trv.normal.x=faces->tnlvertices[k].nx;
				trv.normal.y=faces->tnlvertices[k].ny;
				trv.normal.z=faces->tnlvertices[k].nz;
				
				endv.x=trv.x+trv.normal.x*fLineLength;
				endv.y=trv.y+trv.normal.y*fLineLength;
				endv.z=trv.z+trv.normal.z*fLineLength;

				LITVERTEX v[2];
				v[0].x=trv.x;v[0].y=trv.y;v[0].z=trv.z;v[0].Diffuse=0xff0000;
				v[1].x=endv.x;v[1].y=endv.y;v[1].z=endv.z;v[1].Diffuse=0xffffff;

				g_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST,1,v,sizeof(LITVERTEX));
			}
		}
	}
}

rvector RSVisualObject::GetRandomPoint()
{
	static rvector rr;
	int is=rand()%GetSubObjectCount();
	CFaces *pfaces=&GetSubObject(is)->faceshead[rand()%GetSubObject(is)->nFaces];
	int np=rand()%pfaces->nv;
	rr=*((rvector*)&(pfaces->tnlvertices[np].x));
	return rr;
}

rvector RSVisualObject::FindNearestVertex(rvector &position)
{
	static rvector ret;
	float mindist=99999999.f;
	CMesh *mesh;
	int i,j,k;
	CFaces *faces;
	for(i=0;i<nMesh;i++)
	{
		mesh=meshestoview[i];
		faces=mesh->faceshead;
		rvector *pv;
		for(j=0;j<mesh->nFaces;j++)
		{
			for(k=0;k<faces->nv;k++)
			{
				pv=(rvector*)&faces->tnlvertices[k].x;
				float dist=position.GetDistance(*pv);
				if(dist<mindist)
				{
					mindist=dist;
					ret=*pv;
				}
			}
			faces++;
		}
	}
	return ret;
}

void RSVisualObject::DrawBoundingBox()
{
	int i;
	rmatrix objmat;
	rmatrix44 tm;

	static ind[8][3]= { {0,0,0},{1,0,0},{1,1,0},{0,1,0}, {0,0,1},{1,0,1},{1,1,1},{0,1,1} };
	static lines[12][2] = { {0,1},{1,5},{5,4},{4,0},{5,6},{1,2},{0,3},{4,7},{7,6},{6,2},{2,3},{3,7} };

	objmat=GetMatrix();
	tm=MatrixMult(objmat,IdentityMatrix44());
	g_pd3dDevice->SetTransform(D3DTS_WORLD, (_D3DMATRIX*)&tm);

	g_pd3dDevice->SetVertexShader(D3DFVF_XYZ);
	rvector vertex[2];

	for(i=0;i<12;i++)
	{
		vertex[0]=*(rvector*)&m_BoundingVertices[lines[i][0]].x;
		vertex[1]=*(rvector*)&m_BoundingVertices[lines[i][1]].x;
		g_pd3dDevice->DrawPrimitiveUP( D3DPT_LINELIST,1,vertex,sizeof(rvector));
	}
	g_pd3dDevice->SetVertexShader(RSTLFVF);
}