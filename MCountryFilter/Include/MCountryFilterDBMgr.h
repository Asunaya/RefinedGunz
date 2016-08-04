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

	bool GetIPCountryCode( const std::string& strIP, 
		uint32_t& dwIPFrom,
		uint32_t& dwIPTo,
		std::string& strCountryCode );
	bool GetIPtoCountryList( IPtoCountryList& rfIPtoCountryList );
	bool GetBlockCountryCodeList( BlockCountryCodeList& rfBlockCountryCodeList );
	bool GetCustomIP( const string& strIP, uint32_t& dwIPFrom, uint32_t& dwIPTo, bool& bIsBlock, string& strCountryCode3, string& strComment );
	bool GetCustomIPList( CustomIPList& rfCustomIPList );

	void ReleaseDB();
	
private :
	bool CheckOpen();

private :
	MDatabase* m_pDB;
};