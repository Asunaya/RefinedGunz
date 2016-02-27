#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <tchar.h>

#include "MXml.h"
#include "MCharacter.h"

struct SServConfig
{
	char	ServerName[255];
	char	Location[255];
	char	Desc[255];
	char	AdminName[255];
	char	AdminEMail[255];
};


int InputMenu()
{
	printf("\n\n"\
		   "\t\t1. Sample1 - 항목 읽기\n"\
		   "\t\t2. Sample2 - 항목 읽기 2번째\n"\
		   "\t\t3. Sample3 - XML 저장\n"\
		   "\t\t4. Chracter XML Load\n"\
		   "\t\t0. Quit\n"\
		   "\t==============================\n"\
		   "\t\t? "\
		   );

	int i;
	scanf("%d", &i);
	return i;
}

/// GLOBAL이라는 Element를 찾아서 Child Element의 내용을 읽는다.
void Sample1()
{
	int iCount, i;
	char szBuf[65535];

	MXmlDocument	aXml;
	MXmlElement		aParent, aChild;
	MXmlNode		aNode;

	SServConfig		aConfig;
	memset(&aConfig, 0, sizeof(aConfig));

	aXml.Create();

	if (!aXml.LoadFromFile("sample1.xml")) return;

	aParent = aXml.GetDocumentElement();
	if (aParent.FindChildNode("GLOBAL", &aChild))
	{
		aParent = aChild;

		iCount = aParent.GetChildNodeCount();

		for (i = 0; i < iCount; i++)
		{
			memset(szBuf, 0, sizeof(szBuf));
			aChild = aParent.GetChildNode(i);

			aChild.GetTagName(szBuf);
			if (szBuf[0] == '#') continue;

			if (!strcmp(szBuf, "SERVERNAME"))
			{
				aChild.GetContents(aConfig.ServerName);
			}
			else if (!strcmp(szBuf, "LOCATION"))
			{
				aChild.GetContents(aConfig.Location);
			}
			else if (!strcmp(szBuf, "DESCRIPTION"))
			{
				aChild.GetContents(aConfig.Desc);
			}
			else if (!strcmp(szBuf, "ADMINNAME"))
			{
				aChild.GetContents(aConfig.AdminName);
			}
			else if (!strcmp(szBuf, "ADMINEMAIL"))
			{
				aChild.GetContents(aConfig.AdminEMail);
			}
		}

		printf("ServerName = %s\n", aConfig.ServerName);
		printf("Location = %s\n", aConfig.Location);
		printf("Description = %s\n", aConfig.Desc);
		printf("AdminName = %s\n", aConfig.AdminName);
		printf("AdminEMail = %s\n", aConfig.AdminEMail);
		printf("Done!!!\n\n");
	}

	aXml.Destroy();
}

/// MAP이라는 Element의 ID가 bird인 항목을 찾아 HEIGHT와 WIDTH를
/// 500으로 변환하고 new.xml로 저장한다.
void Sample2()
{
	MXmlDocument	aXml;

	aXml.Create();
	if (!aXml.LoadFromFile("sample1.xml")) return;

	int iCount, i;
	char szBuf1[65535], szBuf2[65535];

	MXmlElement		aParent, aChild;

	aParent = aXml.GetDocumentElement();
	iCount = aParent.GetChildNodeCount();

	for (i = 0; i < iCount; i++)
	{
		aChild = aParent.GetChildNode(i);

		aChild.GetTagName(szBuf1);
		if (szBuf1[0] == '#') continue;

		if (!strcmp(szBuf1, "MAP"))
		{
			aChild.GetAttribute(szBuf2, "ID");
			if (!strcmp(szBuf2, "bird"))
			{
				aParent = aChild;
				if (aParent.FindChildNode("WIDTH", &aChild))
				{
					sprintf(szBuf1, "%d", 500);
					aChild.SetContents(szBuf1);
				}

				if (aParent.FindChildNode("HEIGHT", &aChild))
				{
					sprintf(szBuf1, "%d", 500);
					aChild.SetContents(szBuf1);
				}
			}
		}
	}

	aXml.SaveToFile("new.xml");


	aXml.Destroy();

	printf("Done!!!\n");

}

// xml 파일 만들기 예제
void Sample3()
{
	MXmlDocument aXml;
	aXml.Create();
	aXml.CreateProcessingInstruction();

	MXmlElement		aRootElement;
	MXmlElement		aElement;
	MXmlElement		aChildElement;

	aRootElement = aXml.CreateElement("XML");
	aXml.AppendChild(aRootElement);

	aRootElement = aXml.GetDocumentElement();

	/// text
	aRootElement.AppendText("\n\t");

	aElement = aXml.CreateElement("TEST");
	aRootElement.AppendChild(aElement);

	aElement.AddAttribute("id", "1");
	aElement.AddAttribute("map", "bird");

	/// text
	aElement.AppendText("\n\t\t");

	aChildElement = aXml.CreateElement("CHILD");
	aChildElement.AddAttribute("attr", 500);
	aChildElement.SetContents("자식 노드");
	aElement.AppendChild(aChildElement);

	/// text
	aElement.AppendText("\n\t");

	/// text
	aXml.GetDocumentElement().AppendText("\n");

	aXml.SaveToFile("new2.xml");

	printf("Done!!!\n");
}

void Sample4()
{
	MCHR chr;

	if (chr_LoadFromXmlFile(&chr, "ChrTest.xml", 0))
	{
		printf("Name = %s\n", chr.szName);
		printf("HP = %d\n", chr.iHP);
		printf("EN = %d\n", chr.iEN);

	}
	else
	{
		printf("Can't Load File\n");
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	int t;

	t = InputMenu();
	while (t != 0)
	{
		switch (t)
		{
		case 1:
			Sample1();
			break;
		case 2:

			Sample2();
			break;
		case 3:
			Sample3();
			break;
		case 4:
			Sample3();
			break;

		}	// switch

		t = InputMenu();
	}

	return 0;
}