#include "stdafx.h"
#include "MXml.h"
#include "RLightList.h"
#include "RToken.h"
#include "MXml.h"

_NAMESPACE_REALSPACE2_BEGIN

bool RLightList::Open(MXmlElement *pElement)
{
	MXmlElement	aLightNode,aChild;
	int nCount = pElement->GetChildNodeCount();

	char szTagName[256],szContents[256];
	for (int i = 0; i < nCount; i++)
	{
		aLightNode = pElement->GetChildNode(i);
		aLightNode.GetTagName(szTagName);

		if(_stricmp(szTagName,RTOK_LIGHT)==0)
		{
			RLIGHT Light;
			aLightNode.GetAttribute(szContents,RTOK_NAME);
			Light.Name=szContents;
			Light.dwFlags=0;

			int nChildCount=aLightNode.GetChildNodeCount();
			for(int j=0;j<nChildCount;j++)
			{
				aChild = aLightNode.GetChildNode(j);
				aChild.GetTagName(szTagName);
				aChild.GetContents(szContents);

	#define READVECTOR(v) sscanf(szContents,"%f %f %f",&v.x,&v.y,&v.z)

				if(_stricmp(szTagName,RTOK_POSITION)==0)		READVECTOR(Light.Position); else
				if(_stricmp(szTagName,RTOK_COLOR)==0)		READVECTOR(Light.Color); else
				if(_stricmp(szTagName,RTOK_INTENSITY)==0)	sscanf(szContents,"%f",&Light.fIntensity); else
				if(_stricmp(szTagName,RTOK_ATTNSTART)==0)	sscanf(szContents,"%f",&Light.fAttnStart); else
				if(_stricmp(szTagName,RTOK_ATTNEND)==0)		sscanf(szContents,"%f",&Light.fAttnEnd); else
				if(_stricmp(szTagName,RTOK_CASTSHADOW)==0)	Light.dwFlags|=RM_FLAG_CASTSHADOW;
			}

			push_back(Light);
		}
	}
	return true;
}

bool RLightList::Save(MXmlElement *pElement)
{
	MXmlElement	aLightListElement=pElement->CreateChildElement(RTOK_LIGHTLIST);

	for(auto& Light : *this)
	{
		aLightListElement.AppendText("\n\t\t");

		char buffer[256];

		MXmlElement		aElement,aChild;
		aElement = aLightListElement.CreateChildElement(RTOK_LIGHT);

		aElement.AddAttribute(RTOK_NAME,Light.Name.c_str());

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_POSITION);
		aChild.SetContents(Format(buffer,Light.Position));

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_COLOR);
		aChild.SetContents(Format(buffer,Light.Color));

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_INTENSITY);
		aChild.SetContents(Format(buffer,Light.fIntensity));

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_ATTNSTART);
		aChild.SetContents(Format(buffer,Light.fAttnStart));

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_ATTNEND);
		aChild.SetContents(Format(buffer,Light.fAttnEnd));

		{
			MXmlElement aFlagElement;

			if((Light.dwFlags & RM_FLAG_CASTSHADOW) !=0)
			{
				aElement.AppendText("\n\t\t\t");
				aElement.CreateChildElement(RTOK_CASTSHADOW);
			}
		}
		aElement.AppendText("\n\t\t");
	}
	aLightListElement.AppendText("\n\t");
	return true;
}

_NAMESPACE_REALSPACE2_END
