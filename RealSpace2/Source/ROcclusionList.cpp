#include "stdafx.h"
#include "MXml.h"
#include "ROcclusionList.h"
#include "RToken.h"
#include "RealSpace2.h"

_NAMESPACE_REALSPACE2_BEGIN

ROcclusion::ROcclusion()
{
	nCount=0;
	pVertices=NULL;
	pPlanes=NULL;
}

ROcclusion::~ROcclusion()
{ 
	SAFE_DELETE(pVertices); 
	SAFE_DELETE(pPlanes);
}

ROcclusionList::~ROcclusionList()
{
	for(iterator i=begin();i!=end();i++)
		delete *i;
}

bool ROcclusionList::Open(MXmlElement *pElement)
{
	MXmlElement	aOcclusionNode,aChild;
	int nCount = pElement->GetChildNodeCount();

	char szTagName[256],szContents[256];
	for (int i = 0; i < nCount; i++)
	{
		aOcclusionNode = pElement->GetChildNode(i);
		aOcclusionNode.GetTagName(szTagName);

		if(_stricmp(szTagName,RTOK_OCCLUSION)==0)
		{
			ROcclusion *poc=new ROcclusion;
			aOcclusionNode.GetAttribute(szContents,RTOK_NAME);
			poc->Name=szContents;

			list<rvector> winding;

			int nChildCount=aOcclusionNode.GetChildNodeCount();
			for(int j=0;j<nChildCount;j++)
			{
				aChild = aOcclusionNode.GetChildNode(j);
				aChild.GetTagName(szTagName);
				aChild.GetContents(szContents);

	#define READVECTOR(v) sscanf(szContents,"%f %f %f",&v.x,&v.y,&v.z)

				if(_stricmp(szTagName,RTOK_POSITION)==0)	{
					rvector temp;
					READVECTOR(temp);
					winding.push_back(temp);
				}
			}

			poc->nCount=winding.size();
			poc->pVertices=new rvector[poc->nCount];
			list<rvector>::iterator k=winding.begin();
			for(int j=0;j<poc->nCount;j++)
			{
				poc->pVertices[j]=*k;
				k++;
			}

			push_back(poc);
		}
	}
	return true;
}

bool ROcclusionList::Save(MXmlElement *pElement)
{
	MXmlElement	aOcclusionListElement=pElement->CreateChildElement(RTOK_OCCLUSIONLIST);

	for(ROcclusionList::iterator i=begin();i!=end();i++)
	{
		aOcclusionListElement.AppendText("\n\t\t");

		ROcclusion *poc=*i;
		char buffer[256];

		MXmlElement		aElement,aChild;
		aElement = aOcclusionListElement.CreateChildElement(RTOK_OCCLUSION);

		aElement.AddAttribute(RTOK_NAME,poc->Name.c_str());

		for(int j=0;j<poc->nCount;j++)
		{
			aElement.AppendText("\n\t\t\t");

			aChild=aElement.CreateChildElement(RTOK_POSITION);
			aChild.SetContents(Format(buffer,poc->pVertices[j]));
		}

		aElement.AppendText("\n\t\t");
	}
	aOcclusionListElement.AppendText("\n\t");
	return true;
}

bool ROcclusionList::IsVisible(const rboundingbox &bb) const
{
	for(auto* poc : *this)
	{
		bool bVisible = false;

		for (int j = 0; j < poc->nCount + 1; j++)
		{
			if (isInPlane(&bb, &poc->pPlanes[j]))
			{
				bVisible = true;
				break;
			}
		}

		if (!bVisible)
			return false;
	}

	return true;
}

void ROcclusionList::UpdateCamera(const rmatrix &matWorld, const rvector &cameraPos)
{
	rmatrix invWorld = Inverse(matWorld);
	rvector localCameraPos = cameraPos * invWorld;
	// We need the inverse transpose for proper plane transformations.
	rmatrix trInvWorld = Transpose(invWorld);

	for(auto* poc : *this)
	{
		bool bm_pPositive = DotProduct(poc->plane, localCameraPos) > 0;

		poc->pPlanes[0] *= trInvWorld;

		poc->pPlanes[0] = bm_pPositive ? poc->plane : -poc->plane;
		for(int j=0;j<poc->nCount;j++)
		{
			auto& Plane = poc->pPlanes[j + 1];
			if(bm_pPositive)
				poc->pPlanes[j + 1]= PlaneFromPoints(
					poc->pVertices[j],
					poc->pVertices[(j + 1) % poc->nCount],
					localCameraPos);
			else
				poc->pPlanes[j + 1] = PlaneFromPoints(
					poc->pVertices[(j+1)%poc->nCount],
					poc->pVertices[j],
					localCameraPos);

			Plane *= trInvWorld;
		}
	}
}

_NAMESPACE_REALSPACE2_END
