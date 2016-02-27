/******************************************************************
   
   ZUpdate.h

     Corperation : MAIET entertainment
     Programmer  : Lim Dong Hwan
	 Date        : 22.June.2005

*******************************************************************/


#ifndef _ZUPDATE
#define _ZUPDATE


#include "ZFileTransfer.h"
#include <windows.h>
#include <list>
using namespace std;


// Update patch file node
class ZUpdatePatchNode
{
protected:
	char				m_szFileName[ _MAX_DIR];
	unsigned long		m_nSize;
	unsigned long		m_nCheckSum;
	bool				m_bValidate;


public:
	ZUpdatePatchNode( const char* pszName, unsigned long nSize, unsigned long nChecksum);
	virtual ~ZUpdatePatchNode();

	const char* GetFileName()			{ return m_szFileName; }
	unsigned long GetSize()				{ return m_nSize; }
	unsigned long GetChecksum()			{ return m_nCheckSum; }
	bool IsValid()						{ return m_bValidate; }
	void SetValid( bool bValid)			{ m_bValidate = bValid; }
	bool CheckValid();
};
class ZUpdatePatchList : public list<ZUpdatePatchNode*>{};


// Update UI infomation
class ZUpdateUIInfo
{
protected:
	ZFileTransferUI*	m_pFileTransferUI;
	unsigned long		m_nTotalPatchFileSize;
	unsigned long		m_nCurrPatchedFileSize;
	unsigned long		m_nTotalPatchFileCount;
	unsigned long		m_nCurrPatchedFileCount;
	int					m_nPatchFailedCount;


public:
	ZUpdateUIInfo()
	{
		m_nTotalPatchFileSize = 0;
		m_nCurrPatchedFileSize = 0;
		m_nTotalPatchFileCount = 0;
		m_nCurrPatchedFileCount = 0;
		m_nPatchFailedCount = 0;
	};

	void SetTransferUI( ZFileTransferUI* pUI)					{ m_pFileTransferUI = pUI; }

	const char* GetCurrPatchFileName()							{ return m_pFileTransferUI->GetDownloadFileName(); }
	unsigned long GetTotalDownloadSize()						{ return m_pFileTransferUI->GetTotalDownloadSize(); }
	unsigned long GetCurrDownloadSize()							{ return m_pFileTransferUI->GetCurrDownloadSize(); }

	unsigned long GetTotalPatchFileSize()						{ return m_nTotalPatchFileSize; }
	void SetTotalPatchFileSize( unsigned long nSize)			{ m_nTotalPatchFileSize = nSize; }
	void AddTotalPatchFileSize( unsigned long nSize)			{ m_nTotalPatchFileSize += nSize; }
	void ClearTotalPatchFileSize()								{ m_nTotalPatchFileSize = 0; }

	unsigned long GetCurrPatchedFileSize()						{ return m_nCurrPatchedFileSize; }
	void SetCurrPatchedFileSize( unsigned long nSize)			{ m_nCurrPatchedFileSize = nSize; }
	void AddCurrPatchedFileSize( unsigned long nSize)			{ m_nCurrPatchedFileSize += nSize; }
	void ClearCurrPatchedFileSize()								{ m_nCurrPatchedFileSize = 0; }

	unsigned long GetTotalPatchFileCount()						{ return m_nTotalPatchFileCount; }
	void SetTotalPatchFileCount( unsigned long nCount)			{ m_nTotalPatchFileCount = nCount; }
	void AddTotalPatchFileCount( unsigned long nCount)			{ m_nTotalPatchFileCount += nCount; }
	void ClearTotalPatchFileCount()								{ m_nTotalPatchFileCount = 0; }

	unsigned long GetCurrPatchedFileCount()						{ return m_nCurrPatchedFileCount; }
	void SetCurrPatchedFileCount( unsigned long nCount)			{ m_nCurrPatchedFileCount = nCount; }
	void AddCurrPatchedFileCount( unsigned long nCount)			{ m_nCurrPatchedFileCount += nCount; }
	void ClearCurrPatchedFileCount()							{ m_nCurrPatchedFileCount = 0; }

	int GetPatchFailedCount()									{ return m_nPatchFailedCount; }
	void SetPatchFailedCount( int nCount)						{ m_nPatchFailedCount = nCount; }
	void AddPatchFailedCount( int nCount)						{ m_nPatchFailedCount += nCount; }
	void ClearPatchFailedCount()								{ m_nPatchFailedCount = 0; }
};


// class ZUpdate
class ZUpdate
{
protected:
	ZUpdatePatchList	m_pUpdatePatchList;

	ZFileTransfer		m_FileTransfer;

	ZUpdateUIInfo		m_UpdateInfo;

	bool				m_bInitialize;
	char				m_szAddress[ 256];
	unsigned long		m_nPort;
	char				m_szID[ 256];
	char				m_szPassword[ 256];
	bool				m_bStopUpdate;
	bool				m_bPatchComplete;
	char				m_szLastError[ 512];


public:
	ZUpdate();
	virtual ~ZUpdate();

	bool Create( const char* pszAddress, unsigned long nPort, const char* pszDefDirectory, const char* pszID, const char* pszPassword);
	bool Destroy();

	bool StartUpdate( const char* pszPatchFileName);
	bool StopUpdate();

	ZUpdateUIInfo GetUpdateInfo()			{ return m_UpdateInfo;}
	bool IsPatchComplete()					{ return m_bPatchComplete; }

	char* GetLastError()					{ return m_szLastError; }


protected:
	bool GetUpdateInfo( const char* pszPatchFileName);
	bool CheckValidFromPatchList();
	bool PatchFiles();

};

#endif
