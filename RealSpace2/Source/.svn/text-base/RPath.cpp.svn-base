#include <crtdbg.h>
#include "MZFileSystem.h"
#include "RPath.h"
#include "RealSpace2.h"
#include "RVersions.h"

_NAMESPACE_REALSPACE2_BEGIN

RPathList::~RPathList()
{
	DeleteAll();
}

void RPathList::DeleteAll()
{
	while(size())
	{
		delete *begin();
		erase(begin());
	}
}

bool RPathList::Save(const char *filename,int nSourcePolygon)
{
	FILE *file=fopen(filename,"wb+");
	if(!file) return false;

	RHEADER header(R_PAT_ID,R_PAT_VERSION);
	fwrite(&header,sizeof(RHEADER),1,file);

	int nSize=size();
	fwrite(&nSourcePolygon,sizeof(int),1,file);		// source polygon 의 개수로 맞는 파일인지 검사.
	fwrite(&nSize,sizeof(int),1,file);
	fwrite(&m_nGroup,sizeof(int),1,file);
	
	for(iterator i=begin();i!=end();i++){
		RPathNode *pNode=*i;

		fwrite(pNode->plane,sizeof(rplane),1,file);
		fwrite(&pNode->m_nSourceID,sizeof(int),1,file);
		fwrite(&pNode->m_nGroupID,sizeof(int),1,file);
		
		// write vertices
		int nVertices=pNode->vertices.size();
		fwrite(&nVertices,sizeof(int),1,file);
		{
			for(RVERTEXLIST::iterator j=pNode->vertices.begin();j!=pNode->vertices.end();j++)
				fwrite(*j,sizeof(rvector),1,file);
		}

		// write nbh
		int nNbh=pNode->m_Neighborhoods.size();
		fwrite(&nNbh,sizeof(int),1,file);
		{
			for(RPATHLIST::iterator j=pNode->m_Neighborhoods.begin();j!=pNode->m_Neighborhoods.end();j++)
			{
				RPath *ppath=*j;
				fwrite(&ppath->nEdge,sizeof(int),1,file);
				fwrite(&ppath->nIndex,sizeof(int),1,file);
			}
		}
	}
	fclose(file);
	return true;
}

bool RPathList::Open(const char *filename,int nSourcePolygon,MZFileSystem *pfs)
{

	MZFile file;
	if(!file.Open(filename,pfs))
		return false;

	RHEADER header;
	file.Read(&header,sizeof(RHEADER));
	if(header.dwID!=R_PAT_ID || header.dwVersion!=R_PAT_VERSION)
	{
		file.Close();
		return false;
	}

	int nSize;
	file.Read(&nSize,sizeof(int));
	if(nSourcePolygon!=nSize)					// source polygon 의 개수로 맞는 파일인지 검사.
	{
		file.Close();
		return false;
	}

	file.Read(&nSize,sizeof(int));
	file.Read(&m_nGroup,sizeof(int));
	for(int i=0;i<nSize;i++)
	{
		RPathNode *newnode=new RPathNode;
		file.Read(&newnode->plane,sizeof(rplane));
		file.Read(&newnode->m_nSourceID,sizeof(int));
		file.Read(&newnode->m_nGroupID,sizeof(int));
		D3DXPlaneNormalize(&newnode->plane,&newnode->plane);
		

		int nVertices;
		file.Read(&nVertices,sizeof(int));
		for(int j=0;j<nVertices;j++)
		{
			rvector *newvector=new rvector;
			file.Read(newvector,sizeof(rvector));
			newnode->vertices.push_back(newvector);
		}

		int nNbh;
		file.Read(&nNbh,sizeof(int));
		for(j=0;j<nNbh;j++)
		{
			RPath *newpath=new RPath;
			file.Read(&newpath->nEdge,sizeof(int));
			file.Read(&newpath->nIndex,sizeof(int));
			newnode->m_Neighborhoods.push_back(newpath);
		}

		newnode->m_nIndex=i;
		push_back(newnode);
	}
	file.Close();

	return true;
}

bool RPathList::ConstructNeighborhood()
{
	for(iterator i=begin();i!=end();i++)
	{
		RPathNode *node=*i;
		
		RVERTEXLIST::iterator ver=node->vertices.begin();
		rvector *edgea,*edgeb;

		for(size_t j=0;j<node->vertices.size();j++)
		{
			edgea=*ver;
			ver++;
			if(ver==node->vertices.end())
				ver=node->vertices.begin();
			edgeb=*ver;
			

			// 각각의 edge 에 대해서, 다른 node 와 공유하는 edge 가 있는지 찾는다.
			for(iterator k=i+1;k!=end();k++)
			{
				RPathNode *node2=*k;
				
				RVERTEXLIST::iterator ver2=node2->vertices.begin();
				rvector *edge2a,*edge2b;

				for(size_t l=0;l<node2->vertices.size();l++)
				{
					edge2a=*ver2;
					ver2++;
					if(ver2==node2->vertices.end())
						ver2=node2->vertices.begin();
					edge2b=*ver2;

					if((IS_EQ3(*edge2a,*edgea) && IS_EQ3(*edge2b,*edgeb)) ||
						(IS_EQ3(*edge2a,*edgeb) && IS_EQ3(*edge2b,*edgea)))
					{
						RPath *pPath=new RPath;
						pPath->nEdge=j;
						pPath->nIndex=k-begin();
						node->m_Neighborhoods.push_back(pPath);

						pPath=new RPath;
						pPath->nEdge=l;
						pPath->nIndex=i-begin();
						node2->m_Neighborhoods.push_back(pPath);
					}
				}
			}
		}
	}
	
	EliminateInvalid();
	return true;
}

bool RPathList::EliminateInvalid()
{	
	// 한 edge 를 통해서 두개 이상의 길이 생긴것을 제거한다.
	for(iterator i=begin();i!=end();i++)
	{
		size_t j;
		RPathNode *node=*i;

		int *nCounts=new int[node->vertices.size()];
		for(j=0;j<node->vertices.size();j++)
			nCounts[j]=0;

		for(j=0;j<node->m_Neighborhoods.size();j++)
		{
			nCounts[node->m_Neighborhoods[j]->nEdge]++;
		}

		rvector *edgea,*edgeb;
		RVERTEXLIST::iterator ver=node->vertices.begin();
		for(j=0;j<node->vertices.size();j++)
		{
			edgea=*ver;
			ver++;
			if(ver==node->vertices.end())
				ver=node->vertices.begin();
			edgeb=*ver;

			if(nCounts[j]>1){	// 이어지는 곳이 둘 이상이면..
				rvector dir=*edgea-*edgeb;
				Normalize(dir);

				rvector right;
				CrossProduct(&right,rvector(0,0,1),dir);

				_ASSERT(!IS_ZERO(dir.x) || !IS_ZERO(dir.y));		// z축과 평행인 edge는 없다 !
				
				int			leftnode=-1,rightnode=-1;
				float		fLeftMax=-FLT_MAX,fRightMax=FLT_MAX;
				vector<int> CurNodes;

				for(size_t k=0;k<node->m_Neighborhoods.size()+1;k++)
				{
					int nTargetNode;
					if(k==0)
						nTargetNode=i-begin();
					else
					{
						if(node->m_Neighborhoods[k-1]->nEdge!=j) continue;
						nTargetNode=node->m_Neighborhoods[k-1]->nIndex;
					}
					
					CurNodes.push_back(nTargetNode);

					RPathNode *targetnode=(*this)[nTargetNode];
					RVERTEXLIST::iterator theother=targetnode->vertices.begin();
					while(IS_EQ3(**theother,*edgea) || IS_EQ3(**theother,*edgeb))
						theother++;
					rvector last=**theother;
					last=last-*edgea;
					Normalize(last);
					float fDot=DotProduct(last,right);
					
					if(fDot<0)
					{
						if(fDot>fLeftMax)
						{
							fLeftMax=fDot;
							leftnode=nTargetNode;
						}
					}
					else
					{
						if(fDot<fRightMax)
						{
							fRightMax=fDot;
							rightnode=nTargetNode;
						}
					}
				}

				if(leftnode!=-1 && rightnode!=-1)
				{
					size_t k,l;
					for(k=0;k<CurNodes.size();k++)
					{
						for(l=0;l<CurNodes.size();l++)
						{
							if(k==l 
								|| (CurNodes[k]==leftnode && CurNodes[l]==rightnode) 
								|| (CurNodes[k]==rightnode && CurNodes[l]==leftnode))
								continue;

							(*this)[CurNodes[k]]->DeletePath(CurNodes[l]);
						}
					}
				}
			}
		}

		delete nCounts;
	}

	return true;
}

void RPathList::MarkGroupID(RPathNode *pNode,int nGroupID)			// GroupID 를 기록한다.
{
	pNode->m_nGroupID=nGroupID;
	for(RPATHLIST::iterator i=pNode->m_Neighborhoods.begin();i!=pNode->m_Neighborhoods.end();i++)
	{
		RPathNode *pNNode=(*this)[(*i)->nIndex];
		if(pNNode->m_nGroupID==-1)
			MarkGroupID(pNNode,nGroupID);
	}
}

bool RPathList::ConstructGroupIDs()
{
	m_nGroup=0;
	for(iterator i=begin();i!=end();i++)
	{
		RPathNode *node=*i;
		if(node->m_nGroupID==-1)
		{
			MarkGroupID(node,m_nGroup);
			m_nGroup++;
		}
	}
	return true;
}

/*
// 아래는 linear search 임.. 없어져야 할 펑션들..

RPathNode *RPathList::FindNode(rvector &a,rvector &b,rvector &c,rplane &plane)
{
	for(iterator i=begin();i!=end();i++){
		RPathNode *node=*i;
		if(IS_EQ(plane.a,node->plane.a)&&IS_EQ(plane.b,node->plane.b)&&
			IS_EQ(plane.c,node->plane.c)&&IS_EQ(plane.d,node->plane.d))
		{
			rplueckercoord va=rplueckercoord(a,a+rvector(plane.a,plane.b,plane.c));
			rplueckercoord vb=rplueckercoord(b,b+rvector(plane.a,plane.b,plane.c));
			rplueckercoord vc=rplueckercoord(c,c+rvector(plane.a,plane.b,plane.c));

			// 각각의 edge 에 대해 
			RVERTEXLIST::iterator ver=node->vertices.begin();
			rvector *edgea,*edgeb;
			
			bool bFound=true;

			for(size_t j=0;j<node->vertices.size();j++)
			{
				edgea=*ver;
				ver++;
				if(ver==node->vertices.end())
					ver=node->vertices.begin();
				edgeb=*ver;
				rplueckercoord edge=rplueckercoord(*edgea,*edgeb);

				if(edge*va<-1.1f) {bFound=false;break;}
				if(edge*vb<-1.1f) {bFound=false;break;}
				if(edge*vc<-1.1f) {bFound=false;break;}
			}
			if(bFound)
				return *i;
		}
	}

	return NULL;
}

RPathNode *RPathList::FindNode(rvector &pos,rplane &plane)
{
	for(iterator i=begin();i!=end();i++){
		RPathNode *node=*i;
		if(IS_ZERO(D3DXPlaneDotCoord(&node->plane,&pos)))
		{
			rplueckercoord va=rplueckercoord(pos,pos+rvector(node->plane.a,node->plane.b,node->plane.c));

			// 각각의 edge 에 대해 
			RVERTEXLIST::iterator ver=node->vertices.begin();
			rvector *edgea,*edgeb;
			
			bool bFound=true;

			for(size_t j=0;j<node->vertices.size();j++)
			{
				edgea=*ver;
				ver++;
				if(ver==node->vertices.end())
					ver=node->vertices.begin();
				edgeb=*ver;
				rplueckercoord edge=rplueckercoord(*edgea,*edgeb);

				if(edge*va<-0.01f) {bFound=false;break;}
			}
			if(bFound)
				return *i;
		}
	}

	return NULL;
}
*/

RPathNode::RPathNode(void)
{
	m_pUserData = NULL;
}

RPathNode::~RPathNode(void)
{
	{
		for(RPATHLIST::iterator i=m_Neighborhoods.begin();i!=m_Neighborhoods.end();i++)
			delete *i;
	}

	{
		for(RVERTEXLIST::iterator i=vertices.begin();i!=vertices.end();i++)
			delete *i;
	}
}

void RPathNode::DrawWireFrame(DWORD color)
{
	struct LVERTEX {
		float x, y, z;		// world position
		DWORD color;
	} ;

	LVERTEX lines[100]={0,};
	
	RVERTEXLIST::iterator iv=vertices.begin();
	for(int j=0;j<(int)vertices.size()+1;j++)
	{
		lines[j].x=(*iv)->x;
		lines[j].y=(*iv)->y;
		lines[j].z=(*iv)->z;
		lines[j].color=color;
		iv++;
		if(iv==vertices.end())
			iv=vertices.begin();
	}

	RGetDevice()->SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE );
	RGetDevice()->DrawPrimitiveUP(D3DPT_LINESTRIP,vertices.size(),lines,sizeof(LVERTEX));
}

void RPathNode::DeletePath(int nIndex)
{
	RPATHLIST::iterator i=m_Neighborhoods.begin();
	while(i!=m_Neighborhoods.end())
	{
		if((*i)->nIndex==nIndex)
			m_Neighborhoods.erase(i);
		else
			i++;
	}
}

_NAMESPACE_REALSPACE2_END
