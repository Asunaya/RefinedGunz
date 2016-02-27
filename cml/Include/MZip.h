#ifndef MZIP_H
#define MZIP_H


/*

	MAIET Entertainment
	Zip Memory Extractor Class

*/

#include <stdio.h>
#include <list>

using namespace std;

#define MZIPREADFLAG_ZIP		1
#define MZIPREADFLAG_MRS		1<<1
#define MZIPREADFLAG_MRS2		1<<2
#define MZIPREADFLAG_FILE		1<<3

enum MZipMode{
	ZMode_Zip = 0,
	ZMode_Mrs,
	ZMode_Mrs2,
	ZMode_End
};

class MZip{
protected:
	struct MZIPDIRHEADER;
	struct MZIPDIRFILEHEADER;
	struct MZIPLOCALHEADER;

protected:
	FILE*						m_fp;			// Refered File Pointer
	char*						m_pDirData;		// Directory Data Block
    const MZIPDIRFILEHEADER**	m_ppDir;		// Directory File Header
	int							m_nDirEntries;	// Number of Directory Entries

	MZipMode					m_nZipMode;
	unsigned long				m_dwReadMode;

public:
	MZip(void);
	virtual ~MZip(void);

	bool Initialize(FILE* fp,unsigned long ReadMode);
	bool Finalize();

	void SetReadMode(unsigned long mode) {	m_dwReadMode = mode; }
	bool isMode(unsigned long mode ) { return (m_dwReadMode & mode) ? true : false ; }
	bool isReadAble(unsigned long mode);

	// Get File Count in Zip
	int GetFileCount(void) const;

	// Get File Name by Index
	void GetFileName(int i, char *szDest) const;

	// Get File Index by Name
	int GetFileIndex(const char* szFileName) const;

	// Get File Index by Index
	int GetFileLength(int i) const;
	int GetFileLength(const char* filename);

	// crc32
	unsigned int GetFileCRC32(int i);
	unsigned int GetFileCRC32(const char* filename);

	// get modified time
	unsigned int GetFileTime(int i);
	unsigned int GetFileTime(const char* filename);

	// Read File Raw Data by Index
	bool ReadFile(int i, void* pBuffer, int nMaxSize);
	bool ReadFile(const char* filename, void* pBuffer, int nMaxSize);

	static bool isVersion1Mrs(FILE* fp);
	static bool isZip(FILE* fp);

	static bool ConvertZip(char* zip_name);
	static bool UpgradeMrs(char* mrs_name);//MrsToMrs2

	static bool RecoveryZip(char* zip_name);
	static bool RecoveryMrs(FILE* fp);
	static bool RecoveryMrs2(FILE* fp);
};

//////////////////////////////////////////////////////////////
// mrs 관련유틸

class FNode {
public:
	FNode();
	void SetName(char* str);

public:

	int	 m_size;
	int	 m_offset;
	char m_name[256];
};

class FFileList :public list<FNode*>
{
public:
	FFileList();
	virtual ~FFileList();

	void Add(FNode* pNode);

	void DelAll();

	void UpgradeMrs();

	void ConvertZip();
	void RecoveryZip();
	void ConvertVtf();

	void ConvertNameMRes2Zip();
	void ConvertNameZip2MRes();
};


bool GetDirList(char* path,	FFileList& pList);
bool GetFileList(char* path,FFileList& pList);
bool GetFileListWin(char* path,FFileList& pList);
bool GetFindFileList(char* path,char* ext,FFileList& pList);
bool GetFindFileListWin(char* path,char* ext,FFileList& pList);


#endif