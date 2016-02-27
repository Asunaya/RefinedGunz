#include "stdafx.h"
#include "MUtil.h"

#include <windows.h>
#include <mmsystem.h>

const string MGetStrLocalTime( const unsigned short wYear, const unsigned short wMon, const unsigned short wDay, const unsigned short wHour, const unsigned short wMin, const MDateType DateType )
{
	char szBuf[ 128 ] = {0,};
	SYSTEMTIME st;
	GetLocalTime( &st );
	
	GetLocalTime( &st );

	st.wMinute += wMin;
	if( 60 < st.wMinute )
	{
		st.wMinute -= 60;
		st.wHour += 1;
	}
	
	st.wHour += wHour;
	if( 24 < st.wHour )
	{
		st.wHour -= 24;
		st.wDay += 1;
	}

	st.wDay += wDay;
	if( 31 < st.wDay )
	{
		st.wDay -= 31;
		st.wMonth += 1;
	}

	st.wMonth += wMon;
	if( 12 < st.wMonth )
	{
		st.wMonth -= 12;
		st.wYear += 1;
	}

	st.wYear += wYear;

	if( MDT_Y == DateType )
	{
		_snprintf( szBuf, 127, "%u", st.wYear );
	}
	else if( MDT_YM == DateType )
	{
		_snprintf( szBuf, 127, "%u-%u",
			st.wYear, st.wMonth );
	}
	else if( MDT_YMD == DateType )
	{
		_snprintf( szBuf, 127, "%u-%u-%u",
			st.wYear, st.wMonth, st.wDay );
	}
	else if( MDT_YMDH == DateType )
	{
		_snprintf( szBuf, 127, "%u-%u-%u %u",
			st.wYear, st.wMonth, st.wDay, st.wHour );
	}
	else if( MDT_YMDHM == DateType )
	{
		_snprintf( szBuf, 127, "%u-%u-%u %u:%u",
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute );
	}
	
	return string( szBuf );
}
