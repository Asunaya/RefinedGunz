// RSBspObject.cpp: 99.2.9 by dubble

#include "rutils.h"
#include "RealSpace.h"
#include "RSBspObject.h"
#include "RSObject.h"
#include "RSDebug.h"
#include "float.h"
#include "RSMaterialManager.h"

// for debug variables...
int nsplitcount=0,nleafcount=0;
int nbsppoly=0,nbspcpoly=0;

RSBspNode::RSBspNode()
{
	Positive=Negative=NULL;
	nFace=0;Face=NULL;
}

RSBspNode::~RSBspNode()
{
	if(Negative) delete Negative;
	if(Positive) delete Positive;
	if(Face) delete []Face;
}

RSBspObject::RSBspObject()
{
	BspHead=NULL;
	m_pVertices=NULL;
	pMaterialIndex=NULL;
}
RSBspObject::~RSBspObject()
{
	if(BspHead) delete BspHead;
	if(m_pVertices) delete []m_pVertices;
	if(pMaterialIndex) delete []pMaterialIndex;
}

void RSBspObject::Check(rvertex *v1,rvertex *v2)
{
//#define FPLANE(v) (plane.a*(v)->x+plane.b*(v)->y+plane.c*(v)->z+plane.d)
#define FPLANE(v) (plane->a*(v)->x+plane->b*(v)->y+plane->c*(v)->z+plane->d)
	SIGN a=signof(FPLANE(v1)),b=signof(FPLANE(v2));
	if(a==POSITIVE)
	{
		*v=v1;
		v++;
	}
	if(a*b==-1)
	{
		rvertex *temp=&added[nva];
		*v=temp;
		v++;
		nva++;
	
		float dx=v2->x-v1->x,dy=v2->y-v1->y,dz=v2->z-v1->z;
		float t=-(v1->x*plane->a+v1->y*plane->b+v1->z*plane->c+plane->d)/
				(dx*plane->a+dy*plane->b+dz*plane->c);
		temp->x=v1->x+dx*t;
		temp->y=v1->y+dy*t;
		temp->z=v1->z+dz*t;
		temp->u=v1->u+(v2->u-v1->u)*t;
		temp->v=v1->v+(v2->v-v1->v)*t;
		temp->color=(DWORD((1-t)*float(v1->color&0xff0000)+float(v2->color&0xff0000)*t)&0xff0000)|
					(DWORD((1-t)*float(v1->color&0xff00)+float(v2->color&0xff00)*t)&0xff00)|
					(DWORD((1-t)*float(v1->color&0xff)+float(v2->color&0xff)*t)&0xff);
		temp->specular=0xff000000;
		TransformVertexASM(*temp,RSViewProjectionViewport);
		_ASSERT((0<=t)&&(t<=1));
	}
}

void RSBspObject::DrawFaceWithZClip(rface *face)
{
	nv=0;nva=0;

	face->v[0]->u=face->s[0];face->v[0]->v=face->t[0];
	face->v[1]->u=face->s[1];face->v[1]->v=face->t[1];
	face->v[2]->u=face->s[2];face->v[2]->v=face->t[2];
// pass 1 : plane 1  face->v[1..3] -> verlist[0]
	v=verlist[0];
	Check(face->v[0],face->v[1]);
	Check(face->v[1],face->v[2]);
	Check(face->v[2],face->v[0]);
	if(v==verlist[0]) return;

// pass 2 : Draw.
	rvertex **i,**end;
	i=verlist[0];end=v-2;
	while(i<end)
	{
		i++;
		RSDrawTriangle(verlist[0][0],*i,*(i+1));
	}
}

void RSBspObject::Draw()
{
	RSSetTextureWrapState(true);
	TraverseTreeAndRender(BspHead);
}

void RSBspObject::TraverseTreeAndRender(RSBspNode *bspNode)
{
	if (bspNode == NULL) return;
	if (!isInViewFrustrum(&bspNode->bbTree,RSViewFrustrum)) return;
	if (bspNode->nFace) DrawNodeFaces(bspNode);
	else
		if (bspNode->plane.a*RSCameraPosition.x+
			bspNode->plane.b*RSCameraPosition.y+
			bspNode->plane.c*RSCameraPosition.z+
			bspNode->plane.d>0) // if position is positive side
		{
			TraverseTreeAndRender(bspNode->Negative);
			TraverseTreeAndRender(bspNode->Positive);
		}
		else {
			TraverseTreeAndRender(bspNode->Positive);
			TraverseTreeAndRender(bspNode->Negative);
		}
} /* BSPtraverseTreeAndRender() */

void RSBspObject::DrawNodeFaces(RSBspNode *bspNode)
{
	rface *f=bspNode->Face;
	for(int i=0;i<bspNode->nFace;i++)
	{
		if(DotProduct(f->normal,rvector(f->v[0]->x,f->v[0]->y,f->v[0]->z)-RSCameraPosition)<0)
		{
			TransformVertexASM(*f->v[0],RSViewProjectionViewport);
			TransformVertexASM(*f->v[1],RSViewProjectionViewport);
			TransformVertexASM(*f->v[2],RSViewProjectionViewport);
			RSSetTexture(f->TextureHandle);
			DrawFaceWithZClip(f);
		}
		else
			nbspcpoly++;
		f++;
	}
	nbsppoly+=bspNode->nFace;
}

RSBspNode* RSBspObject::GetLeafNode(rvector *pos,RSBspNode *node)
{
	if(node->nFace) return node;
	if(node->plane.a*pos->x+node->plane.b*pos->y+node->plane.c*pos->z+node->plane.d>0)
		return GetLeafNode(pos,node->Positive);
	else
		return GetLeafNode(pos,node->Negative);
}

#define NEARESTBOUND 0.03f
BOOL RSBspObject::CheckCollision(rvector *pos,rvector *npos,rvector *colpos,rvector *normal)
{
	RSBspNode *posnode=GetLeafNode(pos),*nposnode=GetLeafNode(npos);
	if(posnode==nposnode) return FALSE;
	
	rplueckercoord plu;
	rvector pos2,npos2,dpos2,dpos=*pos-*npos;

	rface *f=posnode->Face;
	for(int i=0;i<posnode->nFace;i++)
	{
		if(DotProduct(f->normal,dpos)<0)
		{
			dpos2=f->normal*NEARESTBOUND;
			pos2=*pos+dpos2;
			npos2=*npos+dpos2;

			plu=rplueckercoord(pos2,npos2);
			if(IsIntersect(&plu,f))
			{
				float t=(f->d+DotProduct(pos2,f->normal))
						/DotProduct(f->normal,pos2-npos2);
				*colpos=*pos-t*dpos;
//				rslog("%8.8f\n",t);
//				*normal=f->normal;
				*normal=*npos+(-f->d-DotProduct(f->normal,*npos)-NEARESTBOUND)*f->normal;
				return TRUE;
			}
		};
		f++;
	}
	return FALSE;	
}

/*
BOOL RSBspObject::GetCollisionFace(rvector *pos,rvector *dir,rface **face)
{
	RSBspNode *posnode=GetLeafNode(pos);
	
	rvector npos=*pos+*dir;
	rplueckercoord plu;
	GetPlueckerCoord(pos,&npos,&plu);

	rface *f=posnode->Face;
	for(int i=0;i<posnode->nFace;i++)
	{
		if((DotProduct(f->normal,*dir)>0)&&(IsIntersect(&plu,f)))
		{
			*face=f;
			return TRUE;
		}
		f++;
	}
	*face=NULL;
	return FALSE;	
}*/
/*
BOOL RSBspObject::isLineIntersectBoundingBox(rplueckercoord *line,rboundingbox *bb)
{
	static int faces[6][4]={0,1,2,3, 6,7,3,2, 1,5,6,2, 0,4,5,1, 7,4,5,6, 3,7,4,0 };
	static rvector normals[6]= { rvector(0,0,1),rvector(-1,0,0),rvector(0,1,0),
								rvector(1,0,0),rvector(0,0,-1),rvector(0,-1,0) };
	static rvector bv[8];
	static rplueckercoord plu;

	bv[0]=rvector(bb->Maxx,bb->Miny,bb->Maxz);
	bv[1]=rvector(bb->Maxx,bb->Maxy,bb->Maxz);
	bv[2]=rvector(bb->Minx,bb->Maxy,bb->Maxz);
	bv[3]=rvector(bb->Minx,bb->Miny,bb->Maxz);
	bv[4]=rvector(bb->Maxx,bb->Miny,bb->Minz);
	bv[5]=rvector(bb->Maxx,bb->Maxy,bb->Minz);
	bv[6]=rvector(bb->Minx,bb->Maxy,bb->Minz);
	bv[7]=rvector(bb->Minx,bb->Miny,bb->Minz);

	for(int i=0;i<6;i++)
	{
		if((DotProduct(normals[i],line->u)>0)&&
			(rplueckercoord(bv[faces[i][0]],bv[faces[i][1]])**line>0)&&
			(rplueckercoord(bv[faces[i][1]],bv[faces[i][2]])**line>0)&&
			(rplueckercoord(bv[faces[i][2]],bv[faces[i][3]])**line>0)&&
			(rplueckercoord(bv[faces[i][3]],bv[faces[i][0]])**line>0))
			return TRUE;
	}
	return FALSE;
}
*/
rface *RSBspObject::GetCollisionFaceR(rvector *origin,rplueckercoord *line,RSBspNode *node)
{
	rface *f;
	if(node->nFace)
	{
		f=node->Face;
		for(int i=0;i<node->nFace;i++)
		{
			if((DotProduct(f->normal,line->u)<0)&&(IsIntersect(line,f)))
				return f;
			f++;
		}
	}
	else
	if(isLineIntersectBoundingBox(line,&node->bbTree))
	{
		if(
			origin->x*node->plane.a+
			origin->y*node->plane.b+
			origin->z*node->plane.c+node->plane.d>0)
		{
			f=GetCollisionFaceR(origin,line,node->Positive);if(f) return f;
			f=GetCollisionFaceR(origin,line,node->Negative);if(f) return f;
		}
		else
		{
			f=GetCollisionFaceR(origin,line,node->Negative);if(f) return f;
			f=GetCollisionFaceR(origin,line,node->Positive);if(f) return f;
		}
	}
	return NULL;
}

rface *RSBspObject::GetCollisionFace(rvector *origin,rvector *direction)
{
	rplueckercoord rcLine;
	rcLine.u=-*direction;
	rcLine.v=CrossProduct(*origin,*origin+*direction);
	return GetCollisionFaceR(origin,&rcLine,BspHead);
}

void RSBspObject::MoveVector(rvector *pos,rvector *npos)
{
	static rvector cp,cn,cp2,cn2;
	if(CheckCollision(pos,npos,&cp,&cn)) 
	{
		*pos=cp;
		if(!CheckCollision(&cp,&cn,&cp2,&cn2))
			*pos=cn;
	}
	else
		*pos=*npos;
}

bool RSBspObject::ReadString(FILE *file,char *buffer,int nBufferSize)
{
	int nCount=0;
	do{
		*buffer=fgetc(file);
		nCount++;
		buffer++;
		if(nCount>=nBufferSize)
			return false;
	}while((*(buffer-1))!=0);
	return true;
}

bool RSBspObject::Open(const char *filename,RSMaterialManager *pmm)
{
	rvector light=Normalize(rvector(1,0,1)); // test code

	FILE *file;
	file=fopen(filename,"rb");
	if(!file) return false;

	// read material indices
	fread(&nMaterial,sizeof(int),1,file);
	pMaterialIndex=new int[nMaterial];
	char buf[256];
	for(int i=0;i<nMaterial;i++)
	{
		if(!ReadString(file,buf,sizeof(buf)))
			return false;
		pMaterialIndex[i]=pmm->Get(buf);
	}
	
	// read vertices
	fread(&nVertices,sizeof(int),1,file);
	m_pVertices=new rvertex[nVertices];
	for(i=0;i<nVertices;i++)
	{
		fread(&m_pVertices[i].x,sizeof(rvector),1,file);
		fread(&m_pVertices[i].normal,sizeof(rvector),1,file);
			float intensity=max(0.1f,DotProduct(m_pVertices[i].normal,light));
/*			vindex[j]->color=FLOAT2RGB24(
							intensity*vindex[j]->material->Diffuse.x,
							intensity*vindex[j]->material->Diffuse.y,
							intensity*vindex[j]->material->Diffuse.z);
*/
			m_pVertices[i].color=0xffffff;//FLOAT2RGB24(intensity,intensity,intensity);
	}

	// read tree information
	BspHead=new RSBspNode;
	Open(BspHead,file,pMaterialIndex);
	
	// set near z plane;
	plane=RSViewFrustrum+4;
	return true;
}

bool RSBspObject::Open(RSBspNode *pNode,FILE *file,int *pTextureHandles)
{
	fread(&pNode->bbTree,sizeof(rboundingbox),1,file);
	fread(&pNode->plane,sizeof(rplane),1,file);

	bool flag;
	fread(&flag,sizeof(bool),1,file);
	if(flag)
	{
		pNode->Positive=new RSBspNode;
		Open(pNode->Positive,file,pTextureHandles);
	}
	fread(&flag,sizeof(bool),1,file);
	if(flag)
	{
		pNode->Negative=new RSBspNode;
		Open(pNode->Negative,file,pTextureHandles);
	}

	fread(&pNode->nFace,sizeof(int),1,file);
	pNode->Face=new rface[pNode->nFace];

	rface *f=pNode->Face;
	for(int i=0;i<pNode->nFace;i++)
	{
		int mat,a,b,c;
		fread(&mat,sizeof(int),1,file);
		fread(&a,sizeof(int),1,file);
		fread(&b,sizeof(int),1,file);
		fread(&c,sizeof(int),1,file);
		fread(&f->s[0],sizeof(float),3,file);
		fread(&f->t[0],sizeof(float),3,file);

		f->v[0]=&m_pVertices[a];
		f->v[1]=&m_pVertices[b];
		f->v[2]=&m_pVertices[c];
		f->TextureHandle=pTextureHandles[mat];
			//(mat==-1) ?0:pTextureHandles[mat];
//calc normal
			rvector c1,c2,c3,nor;
			c1=*(rvector*)&f->v[0]->x;
			c2=*(rvector*)&f->v[1]->x;
			c3=*(rvector*)&f->v[2]->x;
			nor=CrossProduct(c2-c1,c1-c3);
			if((nor.x==0.0f)&&(nor.y==0.0f)&&(nor.z==0.0f))
			{
				static bnoted=FALSE;
				if(!bnoted){rslog("RSBspObject error : invalid face found.\n");bnoted=TRUE;}
				nor=rvector(0,0,1);
			}
			else
				nor=Normalize(nor);
			f->normal=nor;
			f->d=-f->normal.x*(f->v[0]->x)
					-f->normal.y*(f->v[0]->y)
					-f->normal.z*(f->v[0]->z);
		f++;
	}	

	return true;
}
