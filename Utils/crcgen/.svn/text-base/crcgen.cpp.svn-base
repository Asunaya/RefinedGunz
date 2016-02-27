// crcgen.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "MXml.h"
#include "MZFileSystem.h"

bool buildfilelist(const char *szPath)
{
	MZFileSystem newfs;
	newfs.Create(szPath,"update");

	// 파일목록 XML 을 만든다
	MXmlDocument	aXml;
	aXml.Create();
	aXml.CreateProcessingInstruction();

	MXmlElement		aRootElement;

	aRootElement=aXml.CreateElement("XML");

	aXml.AppendChild(aRootElement);

	for(int i=0;i<newfs.GetFileCount();i++)
	{
		aRootElement.AppendText("\n\t");
		const char *filename=newfs.GetFileName(i);

		MXmlElement	anElement=aRootElement.CreateChildElement("FILE");
		anElement.AddAttribute("NAME",filename);
		char szChecksum[256];
		sprintf(szChecksum,"%x",newfs.GetCRC32(filename));
		anElement.AddAttribute("CRC32",szChecksum);
	}
	aRootElement.AppendText("\n");

	char filename[256];
	sprintf(filename,"%sfilelist.xml",szPath);
	aXml.SaveToFile(filename);

	MZFile mzf;
	mzf.Open(filename);
	char *buffer = new char[mzf.GetLength()];
    mzf.Read(buffer,mzf.GetLength());

	unsigned int crc32 = MGetCRC32(buffer,mzf.GetLength());

	sprintf(filename,"%sfilelistcrc.txt",szPath);
	FILE *file = fopen(filename,"w+");
	fprintf(file,"%u",crc32);
	fclose(file);

	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	char szPath[MAX_PATH]="./";

	if(argc>1)
	{
		for(int i=1;i<argc;i++)
		{
			if(stricmp("-path",argv[i])==0)
			{
				if(argc<=i+1)
					goto usage;
				strcpy(szPath,argv[i+1]);
				i++;
			}else
				goto usage;
		}
	}

	buildfilelist(szPath);

	return 0;

usage:
	printf(" usage : crcgen [-path TARGETPATH]\n :) thanx");
	return 0;
}

