#include "stdafx.h"
#include "MZFileSystem.h"
#include "MXml.h"
#include <io.h>
#include <crtdbg.h>
#include "MZip.h"
#include "FileInfo.h"
#include "zip/zlib.h"

//#define DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }

void ReplaceBackSlashToSlash(char* szPath)
{
	int nLen = strlen(szPath);
	for(int i=0; i<nLen; i++){
		if(szPath[i]=='\\') szPath[i]='/';
	}
}

void GetRefineFilename(char *szRefine, int maxlen, const char *szSource)
{
	char pBasePath[256];
	GetCurrentDirectory(sizeof(pBasePath),pBasePath);
	strcat_s(pBasePath,"\\");

	GetRelativePath(szRefine, maxlen, pBasePath,szSource);

	ReplaceBackSlashToSlash(szRefine);
}


time_t dos2unixtime(unsigned long dostime)
{
	tm t;         /* argument for mktime() */
	time_t clock = time(NULL);

	localtime_s(&t, &clock);
	t.tm_isdst = -1;     /* let mktime() determine if DST is in effect */
	/* Convert DOS time to UNIX time_t format */
	t.tm_sec = (((int)dostime) << 1) & 0x3e;
	t.tm_min = (((int)dostime) >> 5) & 0x3f;
	t.tm_hour = (((int)dostime) >> 11) & 0x1f;
	t.tm_mday = (int)(dostime >> 16) & 0x1f;
	t.tm_mon = ((int)(dostime >> 21) & 0x0f) - 1;
	t.tm_year = ((int)(dostime >> 25) & 0x7f) + 80;

	return mktime(&t);
}

unsigned long dostime(int y, int n, int d, int h, int m, int s)
/* year, month, day, hour, minute, second */
/*	Convert the date y/n/d and time h:m:s to a four byte DOS date and
time (date in high two bytes, time in low two bytes allowing magnitude
comparison). */
{
	return y < 1980 ? dostime(1980, 1, 1, 0, 0, 0) :
	(((unsigned long)y - 1980) << 25) | ((unsigned long)n << 21) | 
		((unsigned long)d << 16) | ((unsigned long)h << 11) | 
		((unsigned long)m << 5) | ((unsigned long)s >> 1);
}


unsigned long unix2dostime(time_t t)          /* unix time to convert */
/* Return the Unix time t in DOS format, rounded up to the next two
second boundary. */
{
	time_t t_even;
	tm s;         /* result of localtime() */

	t_even = (t + 1) & (~1);     /* Round up to even seconds. */
	localtime_s(&s, &t_even);       /* Use local time since MSDOS does. */
	return dostime(s.tm_year + 1900, s.tm_mon + 1, s.tm_mday,
		s.tm_hour, s.tm_min, s.tm_sec);
}

void MZFileSystem::RemoveFileList(void)
{
	while(1){
		ZFLISTITOR i=m_ZFileList.begin();
		if(i==m_ZFileList.end()) break;
		MZFILEDESC* pZFile = i->second;
		delete pZFile;
		m_ZFileList.erase(i);
	}
}

bool MZFileSystem::AddItem(MZFILEDESC* pDesc)
{
	char key[_MAX_PATH];
	strcpy_safe(key, pDesc->m_szFileName);
	_strlwr_s(key);
	
//	_RPT1(_CRT_WARN,"%s\n",key);

	ZFLISTITOR it=m_ZFileList.find(key);
	if(it!=m_ZFileList.end())
	{
		MZFILEDESC *pOld=it->second;

		// 원래 있던 것보다 몇초뒤의 파일인가 ? 이전파일이면 업데이트 할 필요없다
		double diff=difftime(dos2unixtime(pDesc->m_modTime),dos2unixtime(pOld->m_modTime));
		if(diff<0) // 더 예전의 파일이므로 교체할 필요가 없다
		{
			// 업데이트 번호가 앞서면 실수 ?
			int nOldPkgNum=GetUpdatePackageNumber(pOld->m_szZFileName);
			int nNewPkgNum=GetUpdatePackageNumber(pDesc->m_szZFileName);
			_ASSERT(nOldPkgNum>nNewPkgNum);

			return false;
		}

		// 이전 데이터를 지운다
		delete pOld;
		m_ZFileList.erase(it);
	}

	m_ZFileList.insert(ZFLIST::value_type(string(key), pDesc));
	return true;
}

void MZFileSystem::RefreshFileList(const char* szBasePath)
{
	_ASSERT(szBasePath!=NULL);

	char szFilter[_MAX_PATH];
	sprintf_s(szFilter,"%s*",szBasePath);

    struct _finddata_t c_file;
    long hFile;
	if( (hFile = _findfirst( szFilter, &c_file )) != -1L ){
		do{
			if(c_file.attrib&_A_SUBDIR){

				// Recursive Run if Sub Directory.
				if(strcmp(c_file.name, ".")==0) continue;
				if(strcmp(c_file.name, "..")==0) continue;
				
				char szPath[256];
				sprintf_s(szPath,"%s%s/",szBasePath,c_file.name);
				RefreshFileList(szPath);
			}
			else{

				char szDrive[_MAX_PATH], szDir[_MAX_PATH], szFileName[_MAX_PATH], szExt[_MAX_PATH];
				_splitpath_s(c_file.name, szDrive, szDir, szFileName, szExt);
				
				// zip 파일이면 안의 파일들을 더한다
				if(_stricmp(szExt, "."DEF_EXT)==0 || _stricmp(szExt, ".zip")==0) {

					char szZipFileName[_MAX_PATH],szBaseLocation[_MAX_PATH];
					sprintf_s(szZipFileName,"%s%s",szBasePath,c_file.name);
					char szRelZipFileName[_MAX_PATH];
					GetRelativePath(szRelZipFileName, sizeof(szRelZipFileName), m_szBasePath,szZipFileName);
					ReplaceBackSlashToSlash(szRelZipFileName);

					// base directory 에 있는 업데이트로 예약된 패키지는 파일명을 폴더로 간주하지 않는다
					if(GetUpdatePackageNumber(szRelZipFileName)>0)
					{
						szBaseLocation[0]=0;
					}else
					{
						GetRelativePath(szBaseLocation,m_szBasePath,szBasePath);
						sprintf_s(szBaseLocation,"%s%s/",szBaseLocation,szFileName);
					}

					FILE* fp = nullptr;
					fopen_s(&fp, szZipFileName, "rb");
					if(fp==NULL) continue;

					MZip zf;
//					unsigned long dwReadMode = MZIPREADFLAG_ZIP | MZIPREADFLAG_MRS | MZIPREADFLAG_MRS2 | MZIPREADFLAG_FILE;

					if(zf.Initialize(fp,MZFile::GetReadMode()))
					{
						for(int i=0; i<zf.GetFileCount(); i++)
						{
							char szCurFileName[_MAX_PATH];
							zf.GetFileName(i, szCurFileName);

							// 디렉토리는 더할필요없다
							char lastchar=szCurFileName[strlen(szCurFileName)-1];
							if(lastchar!='\\' && lastchar!='/')
							{
								MZFILEDESC* pDesc = new MZFILEDESC;
								sprintf_s(pDesc->m_szFileName, "%s%s",szBaseLocation,szCurFileName);
								ReplaceBackSlashToSlash(pDesc->m_szFileName);
								strcpy_safe(pDesc->m_szZFileName, szRelZipFileName);
								pDesc->m_iSize = zf.GetFileLength(i);
								pDesc->m_crc32 = zf.GetFileCRC32(i);
								pDesc->m_modTime = zf.GetFileTime(i);

								if(!AddItem(pDesc))
									delete pDesc;
							}
						}
					}
					else
					{
						fclose(fp);
					}
				}
				else
				{
					// Add File Desc
					MZFILEDESC* pDesc = new MZFILEDESC;

					// 절대경로
					char szFullPath[_MAX_PATH];
					sprintf_s(szFullPath, "%s%s", szBasePath, c_file.name);
					GetRelativePath(pDesc->m_szFileName,m_szBasePath,szFullPath);
					ReplaceBackSlashToSlash(pDesc->m_szFileName);
					pDesc->m_szZFileName[0] = NULL;
					pDesc->m_iSize = c_file.size;
					pDesc->m_crc32 = 0;
					pDesc->m_modTime=unix2dostime(c_file.time_write);

					if(!AddItem(pDesc))
						delete pDesc;
				}
			}
		}while( _findnext( hFile, &c_file ) == 0 );

		_findclose( hFile );
	}
}

int MZFileSystem::GetUpdatePackageNumber(const char *szPackageFileName)
{
	if(!szPackageFileName || szPackageFileName[0]==0) return 0;		// 그냥 폴더에 있는 파일이다

	int nLength=strlen(m_szUpdateName);
	if(m_szUpdateName[0] && _strnicmp(szPackageFileName,m_szUpdateName,nLength)==0)
	{
		int nNumber=atoi(szPackageFileName+nLength)+1;
		return nNumber;		// 업데이프 패키지 내에 있는 파일
	}
	return -1;		// 일반적인 패키지 내에 있는 파일
}

unsigned MGetCRC32(const char *data, int nLength)
{
	uLong crc = crc32(0L, Z_NULL, 0);
	crc = crc32(crc, (byte*)data, nLength);
	return crc;
}

unsigned int MZFileSystem::GetCRC32(const char* szFileName)
{
	MZFILEDESC* pDesc = GetFileDesc(szFileName);
	if(!pDesc) return 0;

	// zip 파일 안에 있는것은 이미 crc가 있다
	if(pDesc->m_szZFileName[0]) {
		return pDesc->m_crc32;
	}

	// 그렇지 않으면 읽어서 계산한다

	MZFile mzf;
	if(!mzf.Open(szFileName,this)) return 0;

	int nFileLength=mzf.GetLength();
	char *buffer=new char[nFileLength];
	mzf.Read(buffer,nFileLength);
	mzf.Close();

	unsigned int crc = MGetCRC32(buffer,nFileLength);
	delete buffer;

	return crc;
}

unsigned int MZFileSystem::GetTotalCRC()
{
	unsigned int nCRCTotal = 0;
	for (int i=0; i<GetFileCount(); i++) {
		const MZFILEDESC* pDesc = GetFileDesc(i);
		nCRCTotal += pDesc->m_crc32;
	}
	return nCRCTotal;
}

MZFileSystem::MZFileSystem(void) : m_pCheckList(NULL)
{
	m_szBasePath[0] = 0;
	m_szUpdateName[0] = 0;
}

MZFileSystem::~MZFileSystem(void)
{
	Destroy();
}

void AddSlash(char *szPath, int maxlen)
{
	// 경로뒤에 '/'가 있도록 만든다
	int nLength=strlen(szPath);
	if(nLength>0 && (szPath[nLength-1]!='/' && szPath[nLength-1]!='\\'))
		strcat_s(szPath, maxlen, "/");
}

bool MZFileSystem::Create(const char* szBasePath,const char* szUpdateName)
{
	Destroy();

	strcpy_safe(m_szBasePath, szBasePath);
	AddSlash(m_szBasePath, sizeof(m_szBasePath));

	if(szUpdateName) {
		char szRelative[_MAX_PATH];
		sprintf_s(szRelative, "%s%s", m_szBasePath, szUpdateName);
		GetRelativePath(m_szUpdateName, sizeof(m_szUpdateName), m_szBasePath,szRelative);
	}

	RemoveFileList();
	RefreshFileList(m_szBasePath);

	/*
	// 일괄적으로 '\'을 '/'로 바꾼다.
	for(ZFLISTITOR li=m_ZFileList.begin(); li!=m_ZFileList.end(); li++){
		MZFILEDESC* pDesc = li->second;
		ReplaceBackSlashToSlash(pDesc->m_szFileName);
//		ReplaceBackSlashToSlash(pDesc->m_szLocation);
	}
	*/

	m_nIndex=0;
	m_iterator=m_ZFileList.begin();

	return true;
}

void MZFileSystem::Destroy(void)
{
	m_szBasePath[0] = 0;
	RemoveFileList();
}

int MZFileSystem::GetFileCount(void) const
{
	return m_ZFileList.size();
}

const char* MZFileSystem::GetFileName(int i)
{
	return GetFileDesc(i)->m_szFileName;
}

const MZFILEDESC* MZFileSystem::GetFileDesc(int i)
{
	while(m_nIndex!=i)
	{
		if(m_nIndex<i)
		{
			m_nIndex++;
			m_iterator++;
			if(m_iterator==m_ZFileList.end())
				return NULL;
		}
		else
		{
			if(m_iterator==m_ZFileList.begin())
				return NULL;
			m_nIndex--;
			m_iterator--;
		}
	}

	return m_iterator->second;
}

MZFILEDESC* MZFileSystem::GetFileDesc(const char* szTarget)
{
	char key[_MAX_PATH];
	strcpy_safe(key, szTarget);
	_strlwr_s(key);

	ZFLISTITOR found=m_ZFileList.find(key);
	if(found!=m_ZFileList.end())
		return found->second;;

	return NULL;
}

int MZFileSystem::GetFileLength(int i)
{
	const MZFILEDESC* pDesc = GetFileDesc(i);
	return pDesc->m_iSize;
}

int MZFileSystem::GetFileLength(const char* szFileName)
{
	MZFILEDESC* pDesc = GetFileDesc(szFileName);
	return pDesc->m_iSize;
}

bool MZFileSystem::ReadFile(const char* szFileName, void* pData, int nMaxSize)
{
	MZFILEDESC* pDesc = GetFileDesc(szFileName);
	if(!pDesc) return false;

	/*
	if(pDesc->m_szLocation[0] != NULL)//zip
	{
		FILE* fp = fopen(pDesc->m_szLocation, "rb");

		if(fp==NULL) return false;

		MZip z;

		if(z.Initialize(fp) == false)
		{
			fclose(fp);
			return false;
		}

		bool hr = z.ReadFile(pDesc->m_szFileName, pData, nMaxSize);

		fclose(fp);

		return hr;
	}
	else
	*/
	{
		FILE* fp;
		fopen_s(&fp, pDesc->m_szFileName, "rb");

		if(fp==NULL) return false;

		fread(pData,1,nMaxSize,fp);

		fclose(fp);

		return true;
	}
}

unsigned long MZFile::m_dwReadMode = MZIPREADFLAG_ZIP | MZIPREADFLAG_MRS | MZIPREADFLAG_MRS2 | MZIPREADFLAG_FILE;

MZFile::MZFile(void) : m_nIndexInZip(-1)
{
	m_fp		  = NULL;
	m_IsZipFile	  = false;

	m_pData		 = NULL;
	m_nDataSize	 = 0;

	m_nPos		 = 0;
	m_nFileSize  = 0;

	m_FileName[0]    = 0;
	m_ZipFileName[0] = 0;

//	m_dwReadMode = MZIPREADFLAG_ZIP | MZIPREADFLAG_MRS | MZIPREADFLAG_MRS2 | MZIPREADFLAG_FILE;
}

MZFile::~MZFile(void)
{
	Close();
	DELETE_ARRAY(m_pData);
//	delete [] m_pData;
}

bool MZFile::Create()
{
	return true;
}

#include "MDebug.h"

bool MZFile::Open(const char* szFileName, MZFileSystem* pZFS)
{
	Close();

	if(pZFS==NULL)							// 파일시스템이 없으면 그냥 파일오픈
	{
		if( isMode(MZIPREADFLAG_FILE) == false ) return false; // 그냥 파일 읽기 무시...

		fopen_s(&m_fp, szFileName, "rb");

		if(m_fp==NULL) return false;

		fseek(m_fp, 0, SEEK_END);
		m_nFileSize = ftell(m_fp);
		fseek(m_fp, 0, SEEK_SET);

		strcpy_safe(m_FileName, szFileName);

		m_IsZipFile = false;

		return true;
	}
	else
	{
		MZFILEDESC* pDesc = pZFS->GetFileDesc(szFileName);

		if(pDesc == NULL) 
		{
//			_RPT1(_CRT_WARN,"%s\n",szFileName);
			return false;
		}

		// zip 파일 안에 있는 경우

		if(pDesc->m_szZFileName[0])
		{
			char relativename[_MAX_PATH];

			char *pRelative=pDesc->m_szFileName,*pDest=pDesc->m_szZFileName;
			while(_strnicmp(pRelative,pDest,1)==0)
			{
				pRelative++;
				pDest++;
			}
			if(*pRelative=='/') pRelative++;

			sprintf_s(relativename, "%s", pRelative);

			char szZipFullPath[_MAX_PATH];
			sprintf_s(szZipFullPath, "%s%s", pZFS->GetBasePath(), pDesc->m_szZFileName);

			// 파일체크를 해야하고 zip 파일 안에 있는경우에 crc검사 한다
			bool bFileCheck = false;
			/*if(pZFS->GetFileCheckList())
			{
				unsigned int crc = pZFS->GetFileCheckList()->GetCRC32(szFileName);
				if(crc!=pDesc->m_crc32) {
#ifdef _DEBUG
					char szBuffer[256];
					sprintf_s(szBuffer,"crc error, %s file %u , source %u \n",szFileName, m_crc32, crc);
					OutputDebugString(szBuffer);
#endif
					mlog("crc error file %s file %u, source %u\n", szFileName, m_crc32, crc);
					return false;
				}
				bFileCheck = true;
			}*/

			return Open(relativename,szZipFullPath,bFileCheck,pDesc->m_crc32);
		}

		if( isMode(MZIPREADFLAG_FILE) == false ) return false; // 그냥 파일 읽기 무시...

		// 맨 폴더에 있는경우
		char szFullPath[_MAX_PATH];
		sprintf_s(szFullPath, "%s%s", pZFS->GetBasePath(), szFileName);

		return Open(szFullPath);
	}
}

bool MZFile::Open(const char* szFileName, const char* szZipFileName, bool bFileCheck , unsigned int crc32 )
{
	m_nPos		 = 0;
	m_nFileSize  = 0;

	if(_stricmp(m_ZipFileName,szZipFileName) != 0)
	{
		Close();

		fopen_s(&m_fp, szZipFileName, "rb");

		if(m_fp==NULL) return false;

		if(m_Zip.Initialize(m_fp,m_dwReadMode)==false)
			return false;

		strcpy_safe(m_ZipFileName, szZipFileName);
	}

	if(m_Zip.isReadAble(m_dwReadMode)==false) 
		return false;

	m_nIndexInZip = m_Zip.GetFileIndex(szFileName);
	m_crc32 = m_Zip.GetFileCRC32(m_nIndexInZip);
	if(bFileCheck && m_crc32!=crc32) {
		// filesystem 이 초기화 될때와 현재의 crc가 다르다!
#ifdef _DEBUG
		char szBuffer[256];
		sprintf_s(szBuffer,"crc error, modified after initialize, %s file in %s %u , source %u \n",szFileName, szZipFileName, m_crc32, crc32);
		OutputDebugString(szBuffer);
#endif
		return false;
	}

	int size = m_Zip.GetFileLength(m_nIndexInZip);

	if(size==0) return false;

	DELETE_ARRAY(m_pData);
//	delete []m_pData;
//	m_pData = NULL;

	m_nFileSize = size;

	strcpy_safe(m_FileName, szFileName);

	m_IsZipFile = true;

	return true;
}

void MZFile::Close(void)
{
	if(m_IsZipFile)
	{
		m_Zip.Finalize();
		if(m_fp) {
			fclose(m_fp);
			m_fp = NULL;
		}
		DELETE_ARRAY(m_pData);
		m_IsZipFile = false;
	}
	else {

		if(m_fp) {
			fclose(m_fp);
			m_fp = NULL;
		}
	}

	m_nPos		 = 0;
	m_nFileSize  = 0;

	m_FileName[0]    = 0;
	m_ZipFileName[0] = 0;
}

int MZFile::GetLength(void)
{
	return m_nFileSize;
}

bool MZFile::Seek(long off,int mode)
{
	if(m_IsZipFile) {

		if(mode == begin)
		{
			m_nPos = off;
			return true;
		}
		else if(mode == current)
		{
			m_nPos += off;
			return true;
		}
		else if(mode == end)
		{
			m_nPos = m_nFileSize + off;
			return true;
		}
	}
	else
	{
		if(mode == begin)
		{
			fseek(m_fp,off,SEEK_SET);
			return true;
		}
		else if(mode == current)
		{
			fseek(m_fp,off,SEEK_CUR);
			return true;
		}
		else if(mode == end)
		{
			fseek(m_fp,off,SEEK_END);
			return true;
		}
	}

	return false;
}

bool MZFile::Read(void* pBuffer, int nMaxSize)
{
	if(m_IsZipFile) 
	{
		if( nMaxSize > m_nFileSize - m_nPos ) return false;

		// 한번에 끝까지 읽으려고 하는 시도는 메모리 할당 없이 읽어준다
		if(nMaxSize == GetLength() && (m_nPos==0))
		{
			if(!m_Zip.ReadFile(m_FileName,pBuffer,m_nFileSize))
				return false;
		}else{
			if(m_pData == NULL)//최초로 Read 할때 할당
			{
				m_pData = new char[m_nFileSize+1];
				m_pData[m_nFileSize] = 0;

				if(!m_Zip.ReadFile(m_nIndexInZip,m_pData,m_nFileSize))
					return false;
			}
			
			memcpy(pBuffer,(m_pData + m_nPos),nMaxSize);
		}

		m_nPos += nMaxSize;
	}
	else 
	{
		size_t numread = fread(pBuffer,1,nMaxSize,m_fp);
		if(numread!=nMaxSize) 
			return false;
	}
	return true;
}

/*
// 한번에 바로 읽어온다. 파일내의 seek 위치와는 상관이 없다
bool MZFile::ReadAll(void* pBuffer, int nBufferSize)		
{
	if(nBufferSize<m_nFileSize) return false;	// 버퍼가 작으면 실패

	if(m_IsZipFile) 
	{
		if(!m_Zip.ReadFile(m_FileName,pBuffer,m_nFileSize))
			return false;
	}
	else 
	{
		size_t numread = fread(pBuffer,1,m_nFileSize,m_fp);
		if(numread!=m_nFileSize) 
		{
			Seek(m_nPos,MZFile::begin);
			return false;
		}
	}
	Seek(m_nPos,MZFile::begin);
	return true;
}
*/

bool MZFileCheckList::Open(const char *szFileName, MZFileSystem *pfs)
{
	MZFile mzf;
	if(!mzf.Open(szFileName,pfs))
		return false;

	char *buffer;
	buffer=new char[mzf.GetLength()+1];
	mzf.Read(buffer,mzf.GetLength());
	buffer[mzf.GetLength()]=0;

	MXmlDocument aXml;
	aXml.Create();
	if(!aXml.LoadFromMemory(buffer))
	{
		delete buffer;
		return false;
	}

	m_crc32 = MGetCRC32(buffer,mzf.GetLength());
	delete buffer;

	int iCount, i;
	MXmlElement		aParent, aChild;
	aParent = aXml.GetDocumentElement();
	iCount = aParent.GetChildNodeCount();

	char szTagName[256];
	for (i = 0; i < iCount; i++)
	{
		aChild = aParent.GetChildNode(i);
		aChild.GetTagName(szTagName);
		if(_stricmp(szTagName,"FILE")==0)
		{
			char szContents[256],szCrc32[256];
			aChild.GetAttribute(szContents,"NAME");
			aChild.GetAttribute(szCrc32,"CRC32");

			if(_stricmp(szContents,"config.xml")!=0)
			{
				unsigned int crc32_current;
				sscanf_s(szCrc32, "%x", &crc32_current);

				char szLowerName[256];
				strcpy_safe(szLowerName, szContents);
				_strlwr_s(szLowerName);

				m_fileList.insert(map<string,unsigned int>::value_type(string(szLowerName),crc32_current));
			}
		}
	}
	return true;
}


unsigned int MZFileCheckList::GetCRC32(const char *szFileName)
{
	char szLowerName[256];
	strcpy_safe(szLowerName, szFileName);
	_strlwr_s(szLowerName);

	map<string,unsigned int>::iterator i;
	i = m_fileList.find(string(szLowerName));

	if(i!=m_fileList.end()) {
		return i->second;
	}

	return 0;
}
