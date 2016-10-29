#define _WINDOWS_
#include "stdafx.h"
#ifdef MSSQL_ENABLED
#undef ASSERT
#undef _WINDOWS_
#include ".\mcountryfilterdbmgr.h"
#include <Windows.h>

#include "mmsystem.h"
#pragma comment( lib, "winmm.lib" )

/*
 Parameter :

 Return :
  @IPFrom bigint
  @IPTo bigint
  @CountryCode3 chr(3)
 */
const TCHAR g_szDB_GetIPtoCountryList[]			= _T("{CALL spIPFltGetIPtoCountryList}");

/*
 Parameter :
  @IP varchar(15)

 Return :
  @IPFrom bigint
  @IPTo bigint
  @CountryCode char(3)
 */
const TCHAR g_szDB_GetIPtoCountry[]				= _T("{CALL spIPFltGetIPtoCountry ('%s')}" );

/*
 Parameter :
  @IP varchar(15)

 Return :
  @CountryCode char(3)
 */
const TCHAR g_szDB_GetIPtoCountryCode[]			= _T("{CALL spIPFltGetIPtoCountryCode ('%s')}");

/*
 Parameter :
  
 Return :
  @CountryCode char(3)
  @IsBlock tinyint
  @RoutingURL varchar(50)
 */
const TCHAR g_szDB_GetBlockCountryCodeList[]	= _T("{CALL spIPFltGetBlockCountryCodeList}");

/*
 Parameter :
 
 Return :
  @IPFrom bigint
  @IPTo bigint
  @IsBlock tinyint
  @Comment varchar(128)
 */
const TCHAR g_szDB_GetCustomIPList[]			= _T("{CALL spIPFltGetCustomIPList}" );

/*
 Parameter :
  @IP varchar(15)

 Return :
  @IPFrom bigint
  @IPTo bigint
  @IsBlock tinyint
  @Comment varchar(128)
 */
const TCHAR g_szDB_GetCustomIP[]					= _T("{CALL spIPFltGetCustomIP ('%s')}" );



MCountryFilterDBMgr::MCountryFilterDBMgr() 
{
	m_pDB = 0;
}


MCountryFilterDBMgr::MCountryFilterDBMgr( MDatabase* pDatabase )
{
	if( 0 != pDatabase )
		m_pDB = pDatabase;
}


MCountryFilterDBMgr::~MCountryFilterDBMgr()
{
}


bool MCountryFilterDBMgr::GetIPCountryCode( const std::string& strIP, 
	uint32_t& dwIPFrom,
	uint32_t& dwIPTo,
	std::string& strCountryCode )
{
	if( CheckOpen() )
	{
		CString strQuery;
		strQuery.Format( g_szDB_GetIPtoCountry, strIP.c_str() );

		CODBCRecordset rs( GetDB() );

		try
		{
			rs.Open( strQuery, CRecordset::forwardOnly, CRecordset::readOnly );
		}
		catch( ... )
		{
			throw;
		}

		if( rs.IsOpen() && !rs.IsEOF() )
		{
			dwIPFrom		= static_cast< DWORD >( rs.Field("IPFrom").AsLong() );
			dwIPTo			= static_cast< DWORD >( rs.Field("IPTo").AsLong() );
			strCountryCode	= rs.Field( "CountryCode3" ).AsString().GetBuffer();
			
			return true;
		}
		else 
			return false;
	}

	return true;
}


bool MCountryFilterDBMgr::GetBlockCountryCodeList( BlockCountryCodeList& rfBlockCountryCodeList )
{
	if( CheckOpen() )
	{
		CODBCRecordset rs( GetDB() );

		try
		{
			rs.Open( g_szDB_GetBlockCountryCodeList, CRecordset::forwardOnly, CRecordset::readOnly );
		}
		catch( ... )
		{
			throw;
		}

		if( rs.IsOpen() )
		{
			BlockCountryCodeInfo* pbcci;

			string strCountryCode3;
			string strRoutingURL;
			bool bIsBlock;

			rfBlockCountryCodeList.clear();

			for( ; !rs.IsEOF(); rs.MoveNext() )
			{
				strCountryCode3	= rs.Field( "CountryCode3" ).AsString().GetBuffer();
				strRoutingURL	= rs.Field( "RoutingURL" ).AsString().GetBuffer();
				bIsBlock		= rs.Field( "IsBlock" ).AsBool();
				
				pbcci = new BlockCountryCodeInfo( strCountryCode3, strRoutingURL, bIsBlock );
				if( 0 == pbcci )
					return false;

				rfBlockCountryCodeList.push_back( pbcci );
			}
		}
	}

	return true;
}


bool MCountryFilterDBMgr::GetIPtoCountryList( IPtoCountryList& rfIPtoCountryList )
{
	if( !CheckOpen() )
		return false;

	CODBCRecordset rs( GetDB() );

	try
	{
		rs.Open(g_szDB_GetIPtoCountryList, CRecordset::forwardOnly, CRecordset::readOnly);
	}
	catch( ... )
	{
		throw;
	}

	if( rs.IsOpen() && !rs.IsEOF() )
	{
		IPtoCountry* pic;

		DWORD	dwIPFrom;
		DWORD	dwIPTo;
		string	strCountryCode3;

		rfIPtoCountryList.clear();
		rfIPtoCountryList.reserve( 100000 );
		
		for( ; !rs.IsEOF(); rs.MoveNext() )
		{
			dwIPFrom		= static_cast< DWORD >( rs.Field( "IPFrom" ).AsLong() );
			dwIPTo			= static_cast< DWORD >( rs.Field( "IPTo" ).AsLong() );
			strCountryCode3	= rs.Field( "CountryCode3" ).AsString().GetBuffer();
			
			pic = new IPtoCountry( dwIPFrom, dwIPTo, strCountryCode3 );
			if( 0 == pic )
				return false;
			rfIPtoCountryList.push_back( pic );
		}
	}

	return true;
}


bool MCountryFilterDBMgr::GetCustomIP( const string& strIP, uint32_t& dwIPFrom, uint32_t& dwIPTo, bool& bIsBlock, string& strCountryCode3, string& strComment )
{
	if( !CheckOpen() ) 
		return false;

	CString strQuery;
	strQuery.Format( g_szDB_GetCustomIP, strIP.c_str() );

	CODBCRecordset rs( GetDB() );

	try
	{
		rs.Open( strQuery, CRecordset::forwardOnly, CRecordset::readOnly );
	}
	catch( ... )
	{
		throw;
	}

	if( rs.IsOpen() && !rs.IsEOF() )
	{
		dwIPFrom		= static_cast< DWORD >( rs.Field("IPFrom").AsLong() );
		dwIPTo			= static_cast< DWORD >( rs.Field("IPTo").AsLong() );
		bIsBlock		= rs.Field( "IsBlock" ).AsBool();
		strCountryCode3 = rs.Field( "CountryCode3" ).AsString().GetBuffer();
		strComment		= rs.Field( "Comment" ).AsString().GetBuffer();
	}
	else 
		return false;

	return true;
}


bool MCountryFilterDBMgr::GetCustomIPList( CustomIPList& rfCustomIPList )
{
	if( !CheckOpen() ) 
		return false;

	rfCustomIPList.clear();
	CODBCRecordset rs( GetDB() );
	try
	{
		rs.Open( g_szDB_GetCustomIPList, CRecordset::forwardOnly, CRecordset::readOnly );
	}
	catch( ... )
	{
		throw;
	}

	if( rs.IsOpen() && !rs.IsEOF() )
	{
		CustomIP* pci;

		DWORD	dwIPFrom;
		DWORD	dwIPTo;
		bool	bIsBlock;
		string	strCountryCode3;
		string	strComment;

		for( ; !rs.IsEOF(); rs.MoveNext() )
		{
			dwIPFrom		= static_cast< DWORD >( rs.Field( "IPFrom" ).AsLong() );
			dwIPTo			= static_cast< DWORD >( rs.Field( "IPTo" ).AsLong() );
			bIsBlock		= rs.Field( "IsBlock" ).AsInt() != 0;
			strCountryCode3	= rs.Field( "CountryCode3" ).AsString().GetBuffer();
			strComment		= rs.Field( "Comment" ).AsString().GetBuffer();

			pci = new CustomIP( dwIPFrom, dwIPTo, bIsBlock, strCountryCode3, strComment );
			if( 0 == pci )
				return false;
			rfCustomIPList.push_back( pci );
		}
	}

	return true;
}


bool MCountryFilterDBMgr::CheckOpen()
{
	return (0 != GetDB()) && GetDB()->CheckOpen();
}


void MCountryFilterDBMgr::ReleaseDB()
{
	if( 0 != m_pDB )
	{
		m_pDB->Disconnect();
		delete m_pDB;
	}
}
#endif