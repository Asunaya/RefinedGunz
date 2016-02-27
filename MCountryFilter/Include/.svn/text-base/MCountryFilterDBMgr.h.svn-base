#pragma once


#include "ODBCRecordset.h"
#include "MCountryFilter.h"


class MCountryFilterDBMgr
{
public:
	MCountryFilterDBMgr();
	MCountryFilterDBMgr( MDatabase* pDatabase );
	virtual ~MCountryFilterDBMgr();

	MDatabase* GetDB() { return m_pDB; }

	void SetDB( MDatabase* pDB ) { m_pDB = pDB; }

	bool GetIPContryCode( const string& strIP, 
						  DWORD& dwIPFrom, 
						  DWORD& dwIPTo, 
						  string& strCountryCode );
	bool GetIPtoCountryList( IPtoCountryList& rfIPtoCountryList );
	bool GetBlockCountryCodeList( BlockCountryCodeList& rfBlockCountryCodeList );
	bool GetCustomIP( const string& strIP, DWORD& dwIPFrom, DWORD& dwIPTo, bool& bIsBlock, string& strCountryCode3, string& strComment );
	bool GetCustomIPList( CustomIPList& rfCustomIPList );

	void ReleaseDB();
	
private :
	bool CheckOpen();

private :
	MDatabase* m_pDB;
};