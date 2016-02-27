#ifndef MZFileSYSTEM_H
#define MZFileSYSTEM_H

#include <list>
#include <map>
#include <string>
#include "MZip.h"

using namespace std;

#define DEF_EXT	"mrs"

// File Descrition ( by Zip File System )
struct MZFILEDESC{
	char	m_szFileName[_MAX_PATH];		// 파일명 ( 실제 파일인 경우도 있고, zip 파일안에 속해 있는 경우도 있다. )
	char	m_szZFileName[_MAX_PATH];		// "" 이면 실제 파일을 의미한다.
	int		m_iSize;
	unsigned int	m_crc32;
	unsigned long	m_modTime;	// zip 파일 안에 있는 파일들을 위해 dos time 으로 저장한다
};

typedef map<string,MZFILEDESC*>	ZFLIST;
typedef ZFLIST::iterator				ZFLISTITOR;

// iterator 를 가지고있으므로 create 된 이후에 파일목록이 추가되면 곤란하다. 그런경우 코드 수정이 필요

template<size_t size> void GetRefineFilename(char(&szRefine)[size], const char *szSource) {
	GetRefineFilename(szRefine, size, szSource);
}
void GetRefineFilename(char *szRefine, int maxlen, const char *szSource);
unsigned MGetCRC32(const char *data, int nLength);

class MZFileCheckList;

class MZFileSystem{
	char		m_szBasePath[256];
	ZFLIST		m_ZFileList;
	ZFLISTITOR	m_iterator;
	int			m_nIndex;

	char		m_szUpdateName[256];

	MZFileCheckList *m_pCheckList;

protected:
	bool AddItem(MZFILEDESC*);

	void RemoveFileList(void);
	// RefreshFileList Pure File List
	void RefreshFileList(const char* szBasePath);
	// Expand Z File List From File List
//	void ExpandZFileList(ZFLIST* pZFList);
	// '\' -> '/'

	int GetUpdatePackageNumber(const char *szPackageFileName);

public:
	MZFileSystem(void);
	virtual ~MZFileSystem(void);

	bool Create(const char* szBasePath,const char* szUpdateName=NULL);
	void Destroy(void);

	int GetFileCount(void) const;
	const char* GetFileName(int i);
	const MZFILEDESC* GetFileDesc(int i);

	const char *GetBasePath() { return m_szBasePath; }

	MZFILEDESC* GetFileDesc(const char* szFileName);

	unsigned int GetCRC32(const char* szFileName);
	unsigned int GetTotalCRC();

	int GetFileLength(const char* szFileName);
	int GetFileLength(int i);

	bool IsZipFile(const char* szFileName);

	bool ReadFile(const char* szFileName, void* pData, int nMaxSize);

	void SetFileCheckList(MZFileCheckList *pCheckList) { m_pCheckList = pCheckList; }
	MZFileCheckList *GetFileCheckList()	{ return m_pCheckList; }
};


class MZFile{
public:
	enum SeekPos { begin = 0x0, current = 0x1, end = 0x2 };

protected:
	FILE*	m_fp;
	bool	m_IsZipFile;

	MZip	m_Zip;

	char*	m_pData;
	int		m_nDataSize;

	int		m_nIndexInZip;
	unsigned int	m_crc32;

	int		m_nPos;
	int		m_nFileSize;

	char	m_FileName[256];
	char	m_ZipFileName[256];

	static  unsigned long m_dwReadMode;

public:

	MZFile(void);
	virtual ~MZFile(void);

	bool Create();

//	bool Open(const char* szFileName);		// 이 방법은 그냥 파일에서 오픈 하는 방법으로 아래 펑션에 합친다.
	bool Open(const char* szFileName, MZFileSystem* pZFS = NULL);
	bool Open(const char* szFileName, const char* szZipFileName, bool bFileCheck = false , unsigned int crc32 = 0);

	bool Seek(long off,int mode);

	void Close(void);

	static void SetReadMode(unsigned long mode) {	m_dwReadMode = mode; }
	static unsigned long GetReadMode(void)  { return m_dwReadMode; }
	static bool isMode(unsigned long mode ) { return (m_dwReadMode & mode) ? true : false ; }

//	bool OpenZip(char* szZipFileName);
//	void CloseZip(void);

	int GetLength(void);
	bool Read(void* pBuffer, int nMaxSize);

	// 한번에 바로 읽어온다. 파일내의 seek 위치와는 상관이 없다
//	bool ReadAll(void* pBuffer, int nBufferSize);
};

class MZFileCheckList
{
	unsigned int				m_crc32;		// 파일 자체의 crc32
	map<string,unsigned int>	m_fileList;

public:
	bool Open(const char *szFileName, MZFileSystem *pfs = NULL);

	unsigned int GetCRC32(const char *szFileName);	// 특정파일의 crc32를 리턴
	unsigned int GetCRC32()	{ return m_crc32; }		// 스스로의 crc32를 리턴
};

#endif

/*
	////	간단 사용법		////

	MZFileSystem fs;

	fs.Create("./");

	char buffer[1000];

	MZFile mzf;

	mzf.Open("6.txt");

	int size = mzf.GetLength();
	mzf.Read(buffer,size);

	mzf.Close();

	mzf.Open("5.txt","test2.zip");
//	mzf.Read(buffer,mzf.GetLength());
	mzf.Read(buffer,4);
	mzf.Read((buffer+4),6);

	mzf.Seek(10,MZFile::begin);
	mzf.Read(buffer,4);

	mzf.Seek(2,MZFile::current);
	mzf.Read(buffer,4);

	mzf.Seek(-2,MZFile::end);
	mzf.Read(buffer,4);

	mzf.Open("4.txt","test2.zip");
	mzf.Read(buffer,mzf.GetLength());

	mzf.Close();

	mzf.Open("5.txt",&fs);
	mzf.Read(buffer,mzf.GetLength());

	mzf.Open("4.txt",&fs);
	mzf.Read(buffer,mzf.GetLength());

	mzf.Open("6.txt",&fs);
	mzf.Read(buffer,mzf.GetLength());

	mzf.Close();

	fs.Destroy();
	*/