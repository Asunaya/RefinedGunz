//#include <windows.h>
#include <afx.h>
#include <crtdbg.h>
#include "MPatchBuilder.h"
#include <time.h>
#include <io.h>
#include <errno.h>
#include <stack>
#include "MZip.h"
#include "FileInfo.h"
using namespace std;
#include "MVersion.h"





//// MPatchBuilder //////////////////////////////////
MPatchBuilder::MPatchBuilder()
{
}

MPatchBuilder::~MPatchBuilder()
{
	Clear();
}

void MPatchBuilder::Clear()
{
	while(m_PatchList.begin() != m_PatchList.end()) {
		delete (*m_PatchList.begin());
		m_PatchList.pop_front();
	}
}

bool MPatchBuilder::GetVersion(char* pszVersion)
{
	DWORD dwMajorVersion = 0;
	DWORD dwMinorVersion = 0;

	if (MGetFileVersion(&dwMajorVersion, &dwMinorVersion, "gunz.exe") == false)
		return false;

	wsprintf(pszVersion, "%u.%u.%u.%u", 
		HIWORD(dwMajorVersion), LOWORD(dwMajorVersion), HIWORD(dwMinorVersion), LOWORD(dwMinorVersion));
	return true;
}

bool MPatchBuilder::Build(const char* pszFileName, const char* pszEncoding)
{
	char szVersion[32];
	if (GetVersion(szVersion) == false) {
		TRACE("gunz.exe Version check failed \n");
		return false;
	}

	ScanDirStack(".");

	for (MPatchList::iterator i=m_PatchList.begin(); i!=m_PatchList.end(); i++) {
		MPatchNode* pNode = *i;
		pNode->MakeChecksum();
	}

	FILE* pFP = fopen(pszFileName, "w+b");
	if (pFP == false) 
		return false;

//	fprintf(pFP, "<?xml version=\"1.0\" encoding=\"%s\"?> \n", pszEncoding);
	fprintf(pFP, "<?xml version=\"1.0\"?> \n");
	fprintf(pFP, "<XML> \n");
	fprintf(pFP, "<PATCHINFO> \n");
	fprintf(pFP, "	<VERSION>%s</VERSION> \n", szVersion);

	for (MPatchList::iterator i=m_PatchList.begin(); i!=m_PatchList.end(); i++) {
		MPatchNode* pNode = *i;
		fprintf(pFP, "	<PATCHNODE file=\"%s\"> \n",		pNode->GetName());
		fprintf(pFP, "		<SIZE>%u</SIZE> \n",			pNode->GetSize());
//		fprintf(pFP, "		<WRITETIMEHIGH>%u</WRITETIMEHIGH> \n",	pNode->GetWriteTime().dwHighDateTime);
//		fprintf(pFP, "		<WRITETIMELOW>%u</WRITETIMELOW> \n",	pNode->GetWriteTime().dwLowDateTime);
		fprintf(pFP, "		<CHECKSUM>%u</CHECKSUM> \n",	pNode->GetChecksum());
		fprintf(pFP, "	</PATCHNODE> \n");
	}

	fprintf(pFP, "</PATCHINFO> \n");
	fprintf(pFP, "</XML> \n");

	fclose(pFP);

	return true;
}

void MPatchBuilder::ScanDirRCS(const char* szDir)	// Recursive Scan Dir
{
	WIN32_FIND_DATA FindData;
	HANDLE hFind;

	char szFind[_MAX_DIR];
	sprintf(szFind, "%s/%s", szDir, "*");

	// Find first .c file in current directory
	if( (hFind = FindFirstFile(szFind, &FindData )) == INVALID_HANDLE_VALUE ) {
		return;
	} else {
		do {
//			TRACEFILE(&c_file);
			if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if ( (strcmp(FindData.cFileName, ".")==0) || (strcmp(FindData.cFileName, "..")==0) ) {
					// Skip
				} else {
					char szPath[_MAX_DIR];
					sprintf(szPath, "%s/%s", szDir, FindData.cFileName);
					ScanDirRCS(szPath);
				}
			} else {
				char szFilePath[_MAX_DIR];
				sprintf(szFilePath, "%s/%s", szDir, FindData.cFileName);
				m_PatchList.push_back(new MPatchNode(szFilePath, FindData.nFileSizeLow, FindData.ftLastWriteTime, 0));
			}
		} while( FindNextFile( hFind, &FindData ) );	// Find the rest files 

		FindClose( hFind );
	}
}

void MPatchBuilder::ScanDirStack(const char* szDir)	// Scan Dir by Stack
{
	WIN32_FIND_DATA FindData;
	HANDLE hFind;

	stack<string>	ScanDir;
	ScanDir.push(szDir);

	while(!ScanDir.empty()) {
		string strDir = ScanDir.top();
		ScanDir.pop();

		char szFind[_MAX_DIR];
		sprintf(szFind, "%s/*", strDir.c_str());

		// Find first .c file in current directory
		if( (hFind = FindFirstFile(szFind, &FindData )) == INVALID_HANDLE_VALUE ) {
			_ASSERT(FALSE);
			continue;
		} else {
			do {
//				TRACEFILE(&c_file);
				if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					if ( (strcmp(FindData.cFileName, ".")==0) || (strcmp(FindData.cFileName, "..")==0) ) {
						// Skip
					} else {
						char szPath[_MAX_DIR];
						sprintf(szPath, "%s/%s", strDir.c_str(), FindData.cFileName);
						ScanDir.push(szPath);
						TRACE("SCANDIR_PUSH : %s \n", szPath);
					}
				} else {
					char szFilePath[_MAX_DIR];
					sprintf(szFilePath, "%s/%s", strDir.c_str(), FindData.cFileName);


					// 파일 확장명
					DWORD dwCRC = 0;
					char szFileExtName[25];
					strcpy( szFileExtName, FindData.cFileName + ( strlen( FindData.cFileName) - 3));


					// *.mrs파일은 파일 헤더를 읽어서 CheckSum을 구한다
					if ( !stricmp( szFileExtName, "mrs"))
					{
						FILE* fp = fopen( szFilePath, "rb");
						if ( fp != NULL)
						{
							MZip zMRSFile;
							unsigned long dwReadMode = MZIPREADFLAG_MRS2;
							if ( zMRSFile.Initialize( fp , dwReadMode ))
							{
								for ( int i = 0;  i < zMRSFile.GetFileCount();  i++)
									dwCRC += zMRSFile.GetFileCRC32( i);
							}
	
							fclose( fp);
						}
					}


					// 나머지 파일은 파일 전체를 읽어서 CheckSum을 구한다
					else
					{
						dwCRC = GetFileCheckSum( szFilePath);
					}


					m_PatchList.push_back(new MPatchNode(szFilePath, FindData.nFileSizeLow, FindData.ftLastWriteTime, dwCRC));
				}
				if (strstr(szFind, "Debug") != 0)
					OutputDebugString("DEBUG \n");
			} while( FindNextFile( hFind, &FindData ) );	// Find the rest files 
			FindClose( hFind );
		}
	}
}

bool MPatchBuilder::TraverseTest()
{
	struct _finddata_t c_file;
	long hFile;

	// Find first .c file in current directory
	if( (hFile = (long)_findfirst( "test/*", &c_file )) == -1L ) {
		TRACE( "No *.c files in current directory!\n" );
	} else {
		do {
			TRACEFILE(&c_file);
		} while( _findnext( hFile, &c_file ) == 0 );	// Find the rest files 

		_findclose( hFile );
	}
	return true;
}

void MPatchBuilder::TRACEFILE(const _finddata_t* pFile, bool bBegin)
{
	if (bBegin) {
		TRACE( "Listing of .c files\n\n" );
		TRACE( "\nDIR RDO HID SYS ARC  FILE         DATE %25c SIZE\n", ' ' );
		TRACE( "--- --- --- --- ---  ----         ---- %25c ----\n", ' ' );
	}

	TRACE( ( pFile->attrib & _A_SUBDIR ) ? " Y  " : " N  " );
	TRACE( ( pFile->attrib & _A_RDONLY ) ? " Y  " : " N  " );
	TRACE( ( pFile->attrib & _A_SYSTEM ) ? " Y  " : " N  " );
	TRACE( ( pFile->attrib & _A_HIDDEN ) ? " Y  " : " N  " );
	TRACE( ( pFile->attrib & _A_ARCH )   ? " Y  " : " N  " );
	TRACE( " %-12s %.24s  %9ld\n",
		pFile->name, ctime( &( pFile->time_write ) ), pFile->size );
}
