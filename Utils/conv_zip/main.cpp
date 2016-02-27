#include <windows.h>
//#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>

#include "MZip.h"
#include "MZFileSystem.h"
#include "MDebug.h"

int main(int argc, char *argv[])
{
//	bool bAutoConv = false;

	FFileList dir_list;
	FFileList file_list;

	InitLog( MLOGSTYLE_DEBUGSTRING | MLOGSTYLE_FILE );

//	GetDirList("*.*",dir_list);
//	GetDirList("*.zip",dir_list);
//	GetFileList("*.zip",file_list);

	////////////////////////////////////////////////
/*
	char* pBuffer;

	MZFile mzf;

	mzf.Open("man01.xml","man.mrs");

	int nSize = mzf.GetLength();
	pBuffer = new char[nSize+1];
	pBuffer[nSize] = 0;

	mzf.Read(pBuffer,nSize);

	delete [] pBuffer;

	mzf.Close();

	return true;
*/
	////////////////////////////////////////////////


	bool bRecovery = false;
	bool bUpdate = false;
	bool bPath = false;
	bool bVtf = false;

	int cnt =  argc-1;
	int i = 1;
	char arg_path[1024];

	while(cnt) {
		
		if(strcmp(argv[i],"-r")==0) {
			bRecovery = true;
			cnt--;
			i++;
		}
		else if(strcmp(argv[i],"-u")==0) { // update
			bUpdate = true;
			cnt--;
			i++;
		}
		else if(strcmp(argv[i],"-path")==0) {

			if( argc < i+1 ) {
			
				printf("잘못 사용 하였소이다~\n");
				printf("ex > conv_zip -path [path] \n");
				return 0;
			}

			i++;
			cnt--;

			strcpy( arg_path , argv[i] );

			i++;
			cnt--;

			bPath = true;
		}
		else if(strcmp(argv[i],"-vtf")==0) {
			bVtf = true;
			cnt--;
			i++;
		}
	}

	if(bVtf) {

		GetFindFileListWin("*",".vtf",file_list);
		file_list.ConvertVtf();

		return 0;
	}

	char temp_path[1024];

	if(bPath) {
		sprintf(temp_path,"%s*",arg_path);
	}
	else {
		sprintf(temp_path,"*");
	}

	if(bRecovery) {

		GetFindFileListWin(temp_path,".mrs",file_list);
		file_list.RecoveryZip();
		file_list.ConvertNameMRes2Zip();
	}
	else if(bUpdate) {
		GetFindFileListWin(temp_path,".mrs",file_list);
		file_list.UpgradeMrs();
//		file_list.ConvertNameMRes2Zip();
	}
	else {

		GetFindFileListWin(temp_path,".zip",file_list);
		file_list.ConvertZip();
		file_list.ConvertNameZip2MRes();
	}

	return 0;
}

