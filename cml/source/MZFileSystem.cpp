#include "stdafx.h"
#include "MZFileSystem.h"
#include "MXml.h"
#include <io.h>
#include <crtdbg.h>
#include "MZip.h"
#include "FileInfo.h"
#include "zip/zlib.h"
#include "MDebug.h"
#include "MUtil.h"
#include <algorithm>
#include "defer.h"

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

	//DMLog("Adding file desc %s, %s\n", pDesc->m_szFileName, pDesc->m_szZFileName);

	ZFLISTITOR it=m_ZFileList.find(key);
	if(it!=m_ZFileList.end())
	{
		MZFILEDESC *pOld=it->second;

		double diff=difftime(dos2unixtime(pDesc->m_modTime),dos2unixtime(pOld->m_modTime));
		if(diff<0)
		{
			/*int nOldPkgNum=GetUpdatePackageNumber(pOld->m_szZFileName);
			int nNewPkgNum=GetUpdatePackageNumber(pDesc->m_szZFileName);*/
			// NOTE: This is false, idk what's happening here
			//_ASSERT(nOldPkgNum>nNewPkgNum);

			return false;
		}

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
	sprintf_safe(szFilter,"%s*",szBasePath);

    struct _finddata_t c_file;
    long hFile;
	if( (hFile = _findfirst( szFilter, &c_file )) != -1L ){
		do{
			if(c_file.attrib&_A_SUBDIR){

				// Recursive Run if Sub Directory.
				if(strcmp(c_file.name, ".")==0) continue;
				if(strcmp(c_file.name, "..")==0) continue;
				
				char szPath[256];
				sprintf_safe(szPath,"%s%s/",szBasePath,c_file.name);
				RefreshFileList(szPath);
			}
			else{

				char szDrive[_MAX_PATH], szDir[_MAX_PATH], szFileName[_MAX_PATH], szExt[_MAX_PATH];
				_splitpath_s(c_file.name, szDrive, szDir, szFileName, szExt);
				
				if(_stricmp(szExt, "." DEF_EXT)==0 || _stricmp(szExt, ".zip")==0) {

					char szZipFileName[_MAX_PATH],szBaseLocation[_MAX_PATH];
					sprintf_safe(szZipFileName,"%s%s",szBasePath,c_file.name);
					char szRelZipFileName[_MAX_PATH];
					GetRelativePath(szRelZipFileName, sizeof(szRelZipFileName), m_szBasePath,szZipFileName);
					ReplaceBackSlashToSlash(szRelZipFileName);

					// base directory
					if(GetUpdatePackageNumber(szRelZipFileName)>0)
					{
						szBaseLocation[0]=0;
					}else
					{
						GetRelativePath(szBaseLocation,m_szBasePath,szBasePath);
						sprintf_safe(szBaseLocation,"%s%s/",szBaseLocation,szFileName);
					}

					FILE* fp = nullptr;
					fopen_s(&fp, szZipFileName, "rb");
					if(fp==NULL) continue;

					MZip zf;

					if(zf.Initialize(fp, MZFile::GetReadMode()))
					{
						for(int i=0; i<zf.GetFileCount(); i++)
						{
							char szCurFileName[_MAX_PATH];
							zf.GetFileName(i, szCurFileName);

							char lastchar=szCurFileName[strlen(szCurFileName)-1];
							if(lastchar!='\\' && lastchar!='/')
							{
								MZFILEDESC* pDesc = new MZFILEDESC;
								sprintf_safe(pDesc->m_szFileName, "%s%s",szBaseLocation,szCurFileName);
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

					fclose(fp);
				}
				else
				{
					// Add File Desc
					MZFILEDESC* pDesc = new MZFILEDESC;

					char szFullPath[_MAX_PATH];
					sprintf_safe(szFullPath, "%s%s", szBasePath, c_file.name);
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

	if(pDesc->m_szZFileName[0]) {
		return pDesc->m_crc32;
	}

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
		sprintf_safe(szRelative, "%s%s", m_szBasePath, szUpdateName);
		GetRelativePath(m_szUpdateName, sizeof(m_szUpdateName), m_szBasePath,szRelative);
	}

	RemoveFileList();
	RefreshFileList(m_szBasePath);

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

	char *p = key;
	while (p = strchr(p, '\\'))
	{
		*p = '/';
	}

	ZFLISTITOR found = m_ZFileList.find(key);
	if(found!=m_ZFileList.end())
		return found->second;;

	return nullptr;
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
	if (!pDesc) return false;

	FILE* fp;
	fopen_s(&fp, pDesc->m_szFileName, "rb");

	if (fp == NULL) return false;

	fread(pData, 1, nMaxSize, fp);

	fclose(fp);

	return true;
}

int MZFileSystem::CacheArchive(const char * Filename)
{
#ifdef _DEBUG
	if (std::find_if(ArchiveCache.begin(), ArchiveCache.end(),
		[&](auto&& item) { return item.Name == Filename; }) != ArchiveCache.end())
	{
		MLog("Duplicate caching on file %s!\n", Filename);
		return -1;
	}
#endif

	char FilenameWithExtension[64];
	sprintf_safe(FilenameWithExtension, "%s.%s", Filename, DEF_EXT);

#ifdef ARCHIVE_CACHE_MMAP
	ArchiveCache.emplace_back(FilenameWithExtension);
	if (ArchiveCache.back().File.Dead())
	{
		MLog("Failed to load file %s!\n", FilenameWithExtension);
		return -1;
	}
#else
	ArchiveCache.emplace_back();
#endif
	auto&& archive = ArchiveCache.back();
	archive.Index = ArchiveIndexCounter;
	++ArchiveIndexCounter;
#ifdef _DEBUG
	archive.Name = Filename;
#endif

	MZip Zip;
#ifdef ARCHIVE_CACHE_MMAP
	if (!Zip.Initialize(archive.File.GetPointer(), archive.File.GetSize(),
#else
	FILE* fp = nullptr;
	auto ret = fopen_s(&fp, FilenameWithExtension, "rb");
	if (!fp || ret != 0)
	{
		MLog("MZFileSystem::CacheArchive - fopen_s failed on %s\n", Filename);
		return -1;
	}
	DEFER( fclose(fp); );
	if (!Zip.Initialize(fp,
#endif
		MZIPREADFLAG_ZIP | MZIPREADFLAG_MRS | MZIPREADFLAG_MRS2 | MZIPREADFLAG_FILE))
	{
		MLog("MZFileSystem::CacheArchive - MZip::Initialize on %s failed!\n", Filename);
		return -1;
	}
	auto FileCount = Zip.GetFileCount();
	for (int i = 0; i < FileCount; i++)
	{
		char Name[64];
		Zip.GetFileName(i, Name);
		char AbsName[64];
		sprintf_safe(AbsName, "%s/%s", Filename, Name);
		auto Desc = GetFileDesc(AbsName);
		if (!Desc)
		{
			DMLog("Couldn't find file desc for %s\n", AbsName);
			continue;
		}
		auto Size = Zip.GetFileLength(i);
		auto& p = Desc->CachedContents;
		p = new char[Size];
		if (!Zip.ReadFile(i, p, Size))
			MLog("MZFileSystem::CacheArchive - MZip::ReadFile on %s failed!\n", Name);
	}

	DMLog("Cached %d files for archive %s\n", FileCount, Filename);

	return archive.Index;
}

void MZFileSystem::ReleaseArchive(int Index)
{
	auto it = std::find_if(ArchiveCache.begin(), ArchiveCache.end(),
		[&](auto&& item) { return item.Index == Index; });
	if (it == ArchiveCache.end())
		return;

	for (auto* Desc : it->Files)
		SAFE_DELETE_ARRAY(Desc->CachedContents);

	ArchiveCache.erase(it);
}

unsigned long MZFile::m_dwReadMode =
MZIPREADFLAG_ZIP | MZIPREADFLAG_MRS | MZIPREADFLAG_MRS2 | MZIPREADFLAG_FILE;

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
}

MZFile::~MZFile(void)
{
	Close();
	ReleaseData();
}

bool MZFile::Open(const char* szFileName, MZFileSystem* pZFS)
{
	Close();

	if (!pZFS)
	{
		if (isMode(MZIPREADFLAG_FILE) == false)
		{
			MLog("MZFile::Open - Was asked to read non-zipped file, but MZIPREADFLAG_FILE is not enabled\n");
			return false;
		}

		fopen_s(&m_fp, szFileName, "rb");

		if (m_fp == NULL)
		{
			// Big spam
			//DMLog("MZFile::Open - Failed to open non-zipped file %s\n", szFileName);
			return false;
		}

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

		if (pDesc == NULL)
		{
			// the .dds stuff spams this. maybe add it back when fixed.
			//MLog("MZFile::Open - Failed to find file description for %s\n", szFileName);
			return false;
		}

		if (pDesc->CachedContents)
		{
			CachedData = true;
			m_pData = pDesc->CachedContents;
			m_nFileSize = pDesc->m_iSize;
			m_IsZipFile = true;
			return true;
		}

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

			sprintf_safe(relativename, "%s", pRelative);

			char szZipFullPath[_MAX_PATH];
			sprintf_safe(szZipFullPath, "%s%s", pZFS->GetBasePath(), pDesc->m_szZFileName);

			bool bFileCheck = false;

			return Open(relativename,szZipFullPath,bFileCheck,pDesc->m_crc32);
		}

		if (isMode(MZIPREADFLAG_FILE) == false)
		{
			MLog("MZFile::Open - Was asked to read non-zipped file, but MZIPREADFLAG_FILE is not enabled\n");
			return false;
		}

		char szFullPath[_MAX_PATH];
		sprintf_safe(szFullPath, "%s%s", pZFS->GetBasePath(), szFileName);

		return Open(szFullPath);
	}
}

bool MZFile::Open(const char* szFileName, const char* szZipFileName,
	bool bFileCheck, unsigned int crc32)
{
	m_nPos		 = 0;
	m_nFileSize  = 0;

	if(_stricmp(m_ZipFileName,szZipFileName) != 0)
	{
		Close();

		fopen_s(&m_fp, szZipFileName, "rb");

		if (m_fp == NULL)
		{
			MLog("MZFile::Open - fopen failed on %s!\n", szZipFileName);
			return false;
		}

		if (m_Zip.Initialize(m_fp, m_dwReadMode) == false)
		{
			MLog("MZFile::Open - MZip::Initialize failed on %s!\n", szFileName);
			return false;
		}

		strcpy_safe(m_ZipFileName, szZipFileName);
	}

	if (m_Zip.isReadAble(m_dwReadMode) == false)
	{
		MLog("MZFile::Open - MZip::isReadAble(%08X) was false for %s\n",
			m_dwReadMode, szFileName);
		return false;
	}

	m_nIndexInZip = m_Zip.GetFileIndex(szFileName);
	m_crc32 = m_Zip.GetFileCRC32(m_nIndexInZip);
	if(bFileCheck && m_crc32!=crc32) {
#ifdef _DEBUG
		char szBuffer[256];
		sprintf_safe(szBuffer, "crc error, modified after initialize, %s file in %s %u , source %u \n",
			szFileName, szZipFileName, m_crc32, crc32);
		OutputDebugString(szBuffer);
#endif
		return false;
	}

	int size = m_Zip.GetFileLength(m_nIndexInZip);

	if (size == 0)
	{
		MLog("MZFile::Open - Size is 0 for %s\n", szFileName);
		return false;
	}

	ReleaseData();
	m_nFileSize = size;
	strcpy_safe(m_FileName, szFileName);
	m_IsZipFile = true;

	return true;
}

void MZFile::Close()
{
	if(m_IsZipFile)
	{
		m_Zip.Finalize();
		if(m_fp) {
			fclose(m_fp);
			m_fp = NULL;
		}
		ReleaseData();
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
	if (m_IsZipFile)
	{
		if (nMaxSize > m_nFileSize - m_nPos) return false;

		if (!m_pData)
			if (!LoadFile())
				return false;

		memcpy(pBuffer, (m_pData + m_nPos), nMaxSize);

		m_nPos += nMaxSize;
	}
	else
	{
		size_t numread = fread(pBuffer, 1, nMaxSize, m_fp);
		if (numread != nMaxSize)
			return false;
	}

	return true;
}

bool MZFile::LoadFile()
{
	m_pData = new char[m_nFileSize + 1];
	m_pData[m_nFileSize] = 0;

	return m_Zip.ReadFile(m_nIndexInZip, m_pData, m_nFileSize);
}

char* MZFile::Release()
{
	if (!m_pData)
		if (!LoadFile())
			return nullptr;

	auto ret = m_pData;
	m_pData = nullptr;
	return ret;
}

void MZFile::ReleaseData()
{
	if (!CachedData)
		SAFE_DELETE_ARRAY(m_pData);
}

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

				m_fileList.insert({ std::string(szLowerName), crc32_current });
			}
		}
	}
	return true;
}


unsigned int MZFileCheckList::GetCRC32(const char *szFileName) const
{
	char szLowerName[256];
	strcpy_safe(szLowerName, szFileName);
	_strlwr_s(szLowerName);

	auto i = m_fileList.find(string(szLowerName));

	if(i!=m_fileList.end()) {
		return i->second;
	}

	return 0;
}

MMappedFile::MMappedFile(const char * Filename)
{
	auto File = CreateFile(Filename, GENERIC_READ, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (File == INVALID_HANDLE_VALUE)
	{
		MLog("MMappedFile::MMappedFile - CreateFile failed on %s! GetLastError() = %d\n",
			Filename, GetLastError());
		return;
	}

	Size = GetFileSize(File, nullptr);

	char FileMappingName[256];
	sprintf_safe(FileMappingName, "GunzCache_%s", Filename);
	Mapping = CreateFileMapping(File, 0, PAGE_READONLY, 0, 0, FileMappingName);
	if (Mapping == NULL)
		return;

	View = MapViewOfFile(Mapping, FILE_MAP_READ, 0, 0, 0);
	if (View == NULL)
		return;

	bDead = false;
}

MMappedFile::~MMappedFile()
{
	if (View != INVALID_HANDLE_VALUE)
		UnmapViewOfFile(View);
	if (Mapping != INVALID_HANDLE_VALUE)
		CloseHandle(Mapping);
	if (File != INVALID_HANDLE_VALUE)
		CloseHandle(File);
}

MMappedFile::MMappedFile(MMappedFile && src)
	: bDead(src.bDead), View(src.View), Mapping(src.Mapping), File(src.File)
{
	src.bDead = true;
	src.View = INVALID_HANDLE_VALUE;
	src.Mapping = INVALID_HANDLE_VALUE;
	src.File = INVALID_HANDLE_VALUE;
}