#include "stdafx.h"


// disable warning. by dubble
#pragma warning( disable : 4311 4312 )

/*************************************************************************
 FILE       : ODBCRecordset.cpp

 Author :	  Stefan Tchekanov  (stefant@iname.com)

 Description: CODBCRecordset and CDBField classes implementation

 Created By : Stefan Tchekanov


Copyright(c) 1998,1999,2000
Stefan Tchekanov (stefant@iname.com)

This code may be used in compiled form in any way you desire. This
file may be redistributed unmodified by any means PROVIDING it is 
not sold for profit without the authors written consent, and 
providing that this notice and the authors name is included. If 
the source code in this file is used in any commercial application 
then a simple email to the author would be nice.

This file is provided "as is" with no expressed or implied warranty.
The author accepts no liability if it causes any damage.

*************************************************************************/
/* #    Revisions    # */

/*************************************************************************
  REVISION ON 12.09.2000 15:27:22  By Stefan Tchekanov
 
  Comments  : The field names in DoFieldExchange() are made to be quoted
			  in square brackets - [FieldName]. This is the way the class 
			  wizard passes field names in DoFieldExchange() implementations.
 
 *************************************************************************/


/*************************************************************************
  REVISION ON 09.06.2000 21:46:00  By Stefan Tchekanov
 
  Comments  : 1. Added CDBField::operator =( const char );
			  2. Removed (!IsEOF()) && (!IsBOF()) from the ASSERT in
				 CODBCRecordset::Field( int nField ) to be able to
				 add records on empty recordsets
 
 *************************************************************************/



////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "ODBCRecordset.h"

////////////////////////////////////////////////////////////////
//
//	CODBCRecordset class
//
////////////////////////////////////////////////////////////////
/*
CODBCRecordset::CODBCRecordset( CDatabase* pDatabase  ) : 
				CRecordset( pDatabase )
{
	m_fields = NULL;
	m_AllocatedFields = 0;
}
*/
////////////////////////////////////////////////////////////////

CODBCRecordset::CODBCRecordset( MDatabase* pDatabase ) :
	CRecordset( pDatabase->GetDatabase() )
{
	m_fields = NULL;
	m_AllocatedFields = 0;
}

////////////////////////////////////////////////////////////////

CODBCRecordset::~CODBCRecordset() {
	Clear();
}
////////////////////////////////////////////////////////////////

//	Frees all the allocated memory
void	CODBCRecordset::Clear() {
	if( m_fields != NULL )
		delete[]	m_fields;
	m_fields = NULL;
	m_AllocatedFields = 0;
}
////////////////////////////////////////////////////////////////

//	Open the recordset
//	lpszSQL is a SQL statement that returns recordset
//	e.g. SELECT * FROM tablename
//	nOpentype is CRecordset open type, see CRecordset::Open()
//	dwOptions is CRecordset options, see CRecordset::Open()
BOOL	CODBCRecordset::Open( LPCTSTR lpszSQL, 
							  UINT nOpenType, /*=AFX_DB_USE_DEFAULT_TYPE*/
							  DWORD dwOptions /*= 0*/ )
{
	//	Allocate the maximum possible field info storage
	//	This is managed by CRecordset class
	m_nFields = 255;
	m_bNotLoadedFieldsMap = true;
	BOOL	nRes = CRecordset::Open( 
				nOpenType, 
				lpszSQL, 
				dwOptions );
	
	return	nRes;
}
////////////////////////////////////////////////////////////////

//	Called by Move() to load info about all the fields
void	CODBCRecordset::LoadFieldNamesMap()
{
	m_mapNameIdx.RemoveAll();

	int	nFields = m_nFields = GetODBCFieldCount();

	//	Smart storage reallocation for the fields buffer
	if( m_AllocatedFields < nFields ) {
		Clear();
		m_fields = new CDBField[ m_nFields ];
		m_AllocatedFields = m_nFields;
	}

	//	Load field names map
	CODBCFieldInfo	fi;
	CString			cName;
	for( int i = 0; i < nFields; i++ ) {
		//	Clear the previously allocated storage object
		m_fields[i].Clear();

		// Determine the field type and initialize the data buffer
		GetODBCFieldInfo( i, fi );
		AllocDataBuffer( m_fields[i], fi );
		
		//	Set the field name
		fi.m_strName.MakeUpper();
		cName = fi.m_strName;

		//	Make different field names for the fields with
		//	equal names.
		int	fldCount = 1;
		while( GetFieldID( cName ) != -1 ) {
			fldCount++;
			cName.Format( "%s%d", fi.m_strName, fldCount );
		}
		m_fields[i].m_cName = cName;
		m_mapNameIdx.SetAt( cName, (void*)i );
	}
}
////////////////////////////////////////////////////////////////

//	Overloaded so all the fields be loaded when needed
void	CODBCRecordset::Move( long nRows, WORD wFetchType /*= SQL_FETCH_RELATIVE*/ )
{
	if( m_bNotLoadedFieldsMap )
	{
		LoadFieldNamesMap();
		m_bNotLoadedFieldsMap = false;
	}
	CRecordset::Move( nRows, wFetchType );
}
////////////////////////////////////////////////////////////////

//	Get the field ID by name
//	GetFieldIndexByName() works, but is case sensitive
int	CODBCRecordset::GetFieldID( LPCTSTR szName )
{
	void*	idx = (void*)-1;
	CString	cName( szName );
	cName.MakeUpper();
	if( ! m_mapNameIdx.Lookup( cName, idx ) )
		return	-1;
	return	(int)idx;
}

CString	CODBCRecordset::GetFieldName( int nField )
{
	//	The field ID is invalid
	ASSERT( nField >= 0 );
	ASSERT( nField < GetODBCFieldCount() );

	CString		cName;
	void*		idx;
	POSITION	pos = m_mapNameIdx.GetStartPosition();
	while( pos != NULL ) {
		m_mapNameIdx.GetNextAssoc( pos, cName, idx );
		if( (int)idx == nField )
			return	cName;
	}
	cName.Empty();
	return	cName;
}
////////////////////////////////////////////////////////////////

void CODBCRecordset::DoFieldExchange( CFieldExchange* pFX )
{
	pFX->SetFieldType( CFieldExchange::outputColumn );

	
	CString		cFieldName;
	for( UINT i = 0; i < m_nFields; i++ )
	{
		cFieldName.Format( "[%s]", GetFieldName(i) );
		switch( m_fields[i].m_dwType )
		{
		case	DBVT_NULL:
					break;
		case	DBVT_BOOL:
					RFX_Bool( pFX, cFieldName, m_fields[i].m_boolVal );
					break;
		case	DBVT_UCHAR:
					RFX_Byte( pFX, cFieldName, m_fields[i].m_chVal );
					break;
		case	DBVT_SHORT:
					//	CDBVariant::m_iVal is of type short
					//	RFX_Int() requires parameter of type int.
					//	Class wizard maps int variable in this case
					//	but CDBVariand does not have int member.
					m_fields[i].m_dwType = DBVT_LONG;
					RFX_Long( pFX, cFieldName, m_fields[i].m_lVal );
					break;
		case	DBVT_LONG:
					RFX_Long( pFX, cFieldName, m_fields[i].m_lVal );
					break;
		case	DBVT_SINGLE:
					RFX_Single( pFX, cFieldName, m_fields[i].m_fltVal );
					break;
		case	DBVT_DOUBLE:
					RFX_Double( pFX, cFieldName, m_fields[i].m_dblVal );
					break;
		case	DBVT_DATE:
					RFX_Date( pFX, cFieldName, *m_fields[i].m_pdate );
					break;
		case	DBVT_STRING:
				{
					CODBCFieldInfo	fi;
					GetODBCFieldInfo( i, fi );
					RFX_Text( pFX, cFieldName, *m_fields[i].m_pstring, fi.m_nPrecision );
					break;
				}
		case	DBVT_BINARY:
					RFX_LongBinary( pFX, cFieldName, *(m_fields[i].m_pbinary) );
					break;
		default:
			//	Unknown datatype
			ASSERT( FALSE );
		}
		m_fields[i].SetNull( FALSE != IsFieldStatusNull( i ) );
	}
}
////////////////////////////////////////////////////////////////

short CODBCRecordset::GetCFieldType( short nSQLType )
{
	short nFieldType = 0;

	switch( nSQLType )
	{
	case SQL_BIT:
		nFieldType = SQL_C_BIT;
		break;

	case SQL_TINYINT:
		nFieldType = SQL_C_UTINYINT;
		break;

	case SQL_SMALLINT:
		nFieldType = SQL_C_SSHORT;
		break;

	case SQL_INTEGER:
		nFieldType = SQL_C_SLONG;
		break;

	case SQL_REAL:
		nFieldType = SQL_C_FLOAT;
		break;

	case SQL_FLOAT:
	case SQL_DOUBLE:
		nFieldType = SQL_C_DOUBLE;
		break;

	case SQL_DATE:
	case SQL_TIME:
	case SQL_TIMESTAMP:
		nFieldType = SQL_C_TIMESTAMP;
		break;

	case SQL_NUMERIC:
	case SQL_DECIMAL:
	case SQL_BIGINT:
	case SQL_CHAR:
	case SQL_VARCHAR:
		nFieldType = SQL_C_CHAR;
		break;

	//	SQL_LONGVARCHAR moved here. 
	//	Its default value is SQL_C_CHAR
	case SQL_LONGVARCHAR:
	case SQL_BINARY:
	case SQL_VARBINARY:
	case SQL_LONGVARBINARY:
		nFieldType = SQL_C_BINARY;
		break;

	default:
		//	Unknown data type
		ASSERT( FALSE );
	}

	return	nFieldType;
}

bool CODBCRecordset::InsertBinary( CString strQuery, unsigned char* pData, const int nSize )
{
	if( !m_pDatabase->IsOpen() )
	{
		ASSERT( 0 );
		return false;
	}

	if( (0 == strQuery.GetLength()) || 0 == pData )
		return false;

	if( 8000 < nSize )
		return false;

	HSTMT		hStmt;
	SQLTCHAR*	pWriteBuff;
	SQLINTEGER  ind			= SQL_DATA_AT_EXEC;

	// 이부분을 서버에서 사용하고있는 DB객체로 바꿔줘야 함!!
	if( SQL_ERROR == SQLAllocHandle(SQL_HANDLE_STMT, m_pDatabase->m_hdbc, &hStmt) )
	{
		return false;
	}
	//

	if( SQL_ERROR == SQLPrepare(hStmt,(SQLCHAR*)strQuery.GetBuffer(), SQL_NTS) )
	{
		return false;
	}

	if( SQL_ERROR == SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT,
									  SQL_C_BINARY, SQL_BINARY,
									  nSize, 0, (SQLPOINTER)pData, nSize, &ind) )
	{
		return false;
	}

	if( SQL_ERROR == SQLExecute(hStmt) )
	{
		return false;
	}
	
	if( SQL_ERROR == SQLParamData(hStmt, (SQLPOINTER*)&pWriteBuff) )
	{
		return false;
	}

	int			nWrSize;
	int			len;
	SQLRETURN	sqlRet;
	
	// 나누어서 데이터 전송.
	for( nWrSize = 0; nWrSize < nSize; nWrSize += BINARY_CHUNK_SIZE, pWriteBuff += BINARY_CHUNK_SIZE )
	{
		if( nWrSize + BINARY_CHUNK_SIZE < nSize )
			len = BINARY_CHUNK_SIZE;
		else
			len = nSize - nWrSize;

		sqlRet =  SQLPutData( hStmt, (SQLPOINTER)pWriteBuff, len );
	}

	// 데이터 전송이 성공적으로 처리되었는지 검사.
	if( SQL_ERROR == sqlRet )
	{
		return false;
	}

	if( SQL_ERROR == SQLParamData(hStmt, (SQLPOINTER*)&pWriteBuff) )
	{
		return false;
	}

	SQLFreeHandle( SQL_HANDLE_STMT, hStmt );
	
	return true;
}

CDBBinary CODBCRecordset::SelectBinary( CString strQuery )
{
	CDBBinary DBBinary( 5, 5 );

	// 데이터를 가져오기전에 이전에 저장되어있던 데이터를 모두 제거함.
	DBBinary.Clear();

	// 작업이 정상적으로 처리되면 상태는 자동적으로 정상으로 바뀌기 때문,
	// 기본적으로 error상테로 설정을 해놓음.
	DBBinary.SetCurUsedSize( -1 );

	if( 0 == strQuery.GetLength() )
	{
		return DBBinary;
	}

	HSTMT			hStmt;
	unsigned char	Data[ BINARY_FIELD_MAX_SIZE ]	= {0};
	SQLINTEGER		ind								= SQL_DATA_AT_EXEC;

	if( SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, m_pDatabase->m_hdbc, &hStmt) )
	{
		return DBBinary;
	}
	
	SQLPrepare( hStmt, (SQLCHAR*)strQuery.GetBuffer(), SQL_NTS );

	if( SQL_ERROR == SQLBindParameter( hStmt, 1, SQL_PARAM_OUTPUT, SQL_C_BINARY, SQL_BINARY, BINARY_FIELD_MAX_SIZE, 
									   0, (SQLPOINTER)Data, BINARY_FIELD_MAX_SIZE, &ind) )
	{
		return DBBinary;
	}

	if( SQL_ERROR == SQLExecute(hStmt) )
	{
		return DBBinary;
	}

	SQLLEN sqlLen;

	// 초기화. insert와 select를 사용하기전에 반드시 해줘야 함.
	// 여기서 부터는 error가 있을시 DBBinary.SetCurUsedSize( -1 )를 반드시 해줘야 함.
	DBBinary.Begin();
	
	while( SQL_SUCCESS == SQLFetch(hStmt) )
	{
		if( !m_pDatabase->IsOpen() )
		{
			DBBinary.SetCurUsedSize( -1 );
			return DBBinary;
		}

		if( SQL_ERROR == SQLGetData(hStmt, 1, SQL_BINARY, (SQLPOINTER)Data, BINARY_FIELD_MAX_SIZE, &sqlLen) )
		{
			DBBinary.SetCurUsedSize( -1 );
			return DBBinary;
		}

		// NULL을 읽었을시를 위해서. NULL을 읽었을시는 데이터 크기가 -1이됨.
		if( -1 == sqlLen )
		{
			if( 0 != DBBinary.InsertData(Data, 0) )
			{
				DBBinary.SetCurUsedSize( -1 );
				return DBBinary;
			}
			continue;
		}

		if( SQL_ERROR == SQLGetData(hStmt, 1, SQL_C_DEFAULT, (SQLPOINTER)Data, sqlLen, &ind) )
		{
			DBBinary.SetCurUsedSize( -1 );
			return DBBinary;
		}

		if( sqlLen != DBBinary.InsertData(Data, sqlLen) )
		{
			DBBinary.SetCurUsedSize( -1 );
			return DBBinary;
		}
	}

	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

	return DBBinary;
}


////////////////////////////////////////////////////////////////

void	CODBCRecordset::AllocDataBuffer( CDBVariant& varValue, CODBCFieldInfo& fi )
{
	int	nFieldType = GetCFieldType( fi.m_nSQLType );
	switch( nFieldType )
	{
	case SQL_C_BIT:
		varValue.m_dwType = DBVT_BOOL;
		break;

	case SQL_C_UTINYINT:
		varValue.m_dwType = DBVT_UCHAR;
		break;

	case SQL_C_SSHORT:
		varValue.m_dwType = DBVT_SHORT;
		break;

	case SQL_C_SLONG:
		varValue.m_dwType = DBVT_LONG;
		break;

	case SQL_C_FLOAT:
		varValue.m_dwType = DBVT_SINGLE;
		break;

	case SQL_C_DOUBLE:
		varValue.m_dwType = DBVT_DOUBLE;
		break;

	case SQL_C_TIMESTAMP:
		varValue.m_pdate = new TIMESTAMP_STRUCT;
		varValue.m_dwType = DBVT_DATE;
		break;

	case SQL_C_CHAR:
		varValue.m_pstring = new CString;
		varValue.m_dwType = DBVT_STRING;
		break;

	case SQL_C_BINARY:
		varValue.m_pbinary = new CLongBinary;
		varValue.m_dwType = DBVT_BINARY;
		break;

	default:
		//	Unknown data type
		ASSERT( FALSE );
	}
}
////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////
//
//	CDBField implementation
//
////////////////////////////////////////////////////////////////

void	BinaryToString( CString& cStr, const CLongBinary& bin, bool bExpand )
{
	int	nSize = min( (int)bin.m_dwDataLength, cStr.GetLength() );
	if( bExpand )
		nSize = bin.m_dwDataLength;

	if( nSize < 1 ) {
		cStr.Empty();
		return;
	}

	void*	p = ::GlobalLock( bin.m_hData );
	if( p == NULL )
		::AfxThrowMemoryException();
	
	char* pStr = cStr.GetBuffer( nSize );
	memcpy( pStr, p, nSize );
	cStr.ReleaseBuffer( nSize );
	::GlobalUnlock( bin.m_hData );
}
////////////////////////////////////////////////////////////////

CDBField::CDBField() {
	m_bIsNull = true;
}
////////////////////////////////////////////////////////////////

CDBField::CDBField( const CDBField& dbv ) {
	m_bIsNull = true;
}
////////////////////////////////////////////////////////////////

CDBField::~CDBField() {
}
////////////////////////////////////////////////////////////////

bool	CDBField::AsBool()	const
{
	if( IsNull() )
		return	false;

	CString	cValue;
	switch( m_dwType ) {
	case	DBVT_NULL:
				return	false;

	case	DBVT_BOOL:
				return	(m_boolVal == TRUE);

	case	DBVT_UCHAR:
				return	(m_chVal == 'T' || m_chVal == '1');

	case	DBVT_SHORT:
				return	(m_iVal != 0);

	case	DBVT_LONG:
				return	(m_lVal != 0);

	case	DBVT_SINGLE:
				return	(m_fltVal != 0.0);

	case	DBVT_DOUBLE:
				return	(m_dblVal != 0.0);

	case	DBVT_DATE:
				//	Cannot convert date to bool
				ASSERT( FALSE );
				break;

	case	DBVT_STRING:
				ASSERT( m_pstring != NULL );
				if( m_pstring->GetLength() < 1 )
					return	false;
				return	((*m_pstring)[0] == 'T' || (*m_pstring)[0] == '1');

	case	DBVT_BINARY:
				//	Cannot convert long binary to bool
				ASSERT( FALSE );
				break;
	}
	//	Undefined data type
	ASSERT( FALSE );
	return	false;
}

unsigned char	CDBField::AsChar()	const
{
	if( IsNull() )
		return	' ';

	switch( m_dwType ) {
	case	DBVT_NULL:
				return	' ';

	case	DBVT_BOOL:
				return	(m_boolVal) ? 'T' : 'F';

	case	DBVT_UCHAR:
				return	m_chVal;

	case	DBVT_SHORT:
				return	(unsigned char)m_iVal;

	case	DBVT_LONG:
				return	(unsigned char)m_lVal;

	case	DBVT_SINGLE:
				return	(unsigned char)m_fltVal;

	case	DBVT_DOUBLE:
				return	(unsigned char)m_dblVal;

	case	DBVT_DATE:
				//	Cannot convert date to unsigned char
				ASSERT( FALSE );
				break;

	case	DBVT_STRING:
				ASSERT( m_pstring != NULL );
				if( m_pstring->GetLength() < 1 )
					return	' ';
				return	(unsigned char)((*m_pstring)[0]);

	case	DBVT_BINARY:
				//	Cannot convert long binary to unsigned char
				ASSERT( FALSE );
				break;
	}
	//	Undefined data type
	ASSERT( FALSE );
	return	' ';
}

short	CDBField::AsShort()	const
{
	if( IsNull() )
		return	0;

	switch( m_dwType ) {
	case	DBVT_NULL:
				return	0;

	case	DBVT_BOOL:
				return	m_boolVal ? 1 : 0;

	case	DBVT_UCHAR:
				return	(short)m_chVal;

	case	DBVT_SHORT:
				return	m_iVal;

	case	DBVT_LONG:
				return	(short)m_lVal;

	case	DBVT_SINGLE:
				return	(short)m_fltVal;

	case	DBVT_DOUBLE:
				return	(short)m_dblVal;

	case	DBVT_DATE:
				//	Cannot convert date to short
				ASSERT( FALSE );
				break;

	case	DBVT_STRING:
				ASSERT( m_pstring != NULL );
				return	(short)atoi( *m_pstring );

	case	DBVT_BINARY:
				//	Cannot conver long binary to short
				ASSERT( FALSE );
				break;
	}
	//	Undefined data type
	ASSERT( FALSE );
	return	0;
}

int		CDBField::AsInt()	const
{
	return	AsLong();
}

long	CDBField::AsLong()	const
{
	if( IsNull() )
		return	0;

	switch( m_dwType ) {
	case	DBVT_NULL:
				return	0;

	case	DBVT_BOOL:
				return	m_boolVal ? 1 : 0;

	case	DBVT_UCHAR:
				return	(long)m_chVal;

	case	DBVT_SHORT:
				return	(long)m_iVal;

	case	DBVT_LONG:
				return	m_lVal;

	case	DBVT_SINGLE:
				return	(long)m_fltVal;

	case	DBVT_DOUBLE:
				return	(long)m_dblVal;

	case	DBVT_DATE:
				//	Cannot convert date to long
				ASSERT( FALSE );
				break;

	case	DBVT_STRING:
				ASSERT( m_pstring != NULL );
				return	atol( *m_pstring );

	case	DBVT_BINARY:
				//	Cannot conver long binary to long
				ASSERT( FALSE );
				break;
	}
	//	Undefined data type
	ASSERT( FALSE );
	return	0;
}

float	CDBField::AsFloat()	const
{
	if( IsNull() )
		return	0.0;

	switch( m_dwType ) {
	case	DBVT_NULL:
				return	0.0;

	case	DBVT_BOOL:
				return	(float)(m_boolVal ? 1.0 : 0.0);

	case	DBVT_UCHAR:
				return	(float)m_chVal;

	case	DBVT_SHORT:
				return	(float)m_iVal;

	case	DBVT_LONG:
				return	(float)m_lVal;

	case	DBVT_SINGLE:
				return	m_fltVal;

	case	DBVT_DOUBLE:
				return	(float)m_dblVal;

	case	DBVT_DATE:
				//	Cannot convert date to float
				ASSERT( FALSE );
				break;

	case	DBVT_STRING:
				ASSERT( m_pstring != NULL );
				return	(float)atof( *m_pstring );

	case	DBVT_BINARY:
				//	Cannot conver long binary to float
				ASSERT( FALSE );
				break;
	}
	//	Undefined data type
	ASSERT( FALSE );
	return	0.0;
}

double	CDBField::AsDouble()	const
{
	if( IsNull() )
		return	0.0;

	switch( m_dwType ) {
	case	DBVT_NULL:
				return	0.0;

	case	DBVT_BOOL:
				return	m_boolVal ? 1.0 : 0.0;

	case	DBVT_UCHAR:
				return	(double)m_chVal;

	case	DBVT_SHORT:
				return	(double)m_iVal;

	case	DBVT_LONG:
				return	(double)m_lVal;

	case	DBVT_SINGLE:
				return	(double)m_fltVal;

	case	DBVT_DOUBLE:
				return	m_dblVal;

	case	DBVT_DATE:
				//	Cannot convert date to double
				ASSERT( FALSE );
				break;

	case	DBVT_STRING:
				ASSERT( m_pstring != NULL );
				return	atof( *m_pstring );

	case	DBVT_BINARY:
				//	Cannot conver long binary to double
				ASSERT( FALSE );
				break;
	}
	//	Undefined data type
	ASSERT( FALSE );
	return	0.0;
}

COleDateTime	CDBField::AsDate()	const
{
	COleDateTime	date;
	if( IsNull() ) {
		date.SetStatus( COleDateTime::null );
		return	date;
	}

	switch( m_dwType ) {
	case	DBVT_NULL:
			date.SetStatus( COleDateTime::null );
			return	date;

	case	DBVT_BOOL:
			date.SetStatus( COleDateTime::invalid );
			return	date;

	case	DBVT_UCHAR:
			date.SetStatus( COleDateTime::invalid );
			return	date;

	case	DBVT_SHORT:
			return	COleDateTime( (time_t)m_iVal );

	case	DBVT_LONG:
			return	COleDateTime( (time_t)m_lVal );

	case	DBVT_SINGLE:
			return	COleDateTime( (time_t)m_fltVal );

	case	DBVT_DOUBLE:
			return	COleDateTime( (time_t)m_dblVal );

	case	DBVT_DATE:
			ASSERT( m_pdate != NULL );
			return	COleDateTime(	m_pdate->year, m_pdate->month, m_pdate->day,
									m_pdate->hour, m_pdate->minute, m_pdate->second );

	case	DBVT_STRING:
			ASSERT( m_pstring != NULL );
			date.ParseDateTime( *m_pstring );
			return	date;

	case	DBVT_BINARY:
			//	Cannot conver long binary to date
			ASSERT( FALSE );
			break;
	}
	//	Undefined data type
	ASSERT( FALSE );
	date.SetStatus( COleDateTime::invalid );
	return	date;
}

CString		CDBField::AsString()	const
{
	CString	cValue;

	switch( m_dwType ) {
	case	DBVT_NULL:
			return	cValue;

	case	DBVT_BOOL:
			return	CString( m_boolVal ? "T" : "F" );

	case	DBVT_UCHAR:
			return	CString( (TCHAR)m_chVal );

	case	DBVT_SHORT:
			cValue.Format( "%hd", m_iVal );
			return	cValue;

	case	DBVT_LONG:
			cValue.Format( "%ld", m_lVal );
			return	cValue;

	case	DBVT_SINGLE:
			cValue.Format( "%f", m_fltVal );
			return	cValue;

	case	DBVT_DOUBLE:
			cValue.Format( "%f", m_dblVal );
			return	cValue;

	case	DBVT_DATE:
		{
			ASSERT( m_pdate != NULL );
			COleDateTime	date( m_pdate->year, m_pdate->month, m_pdate->day,
								  m_pdate->hour, m_pdate->minute, m_pdate->second );
			return	date.Format();
		}
	case	DBVT_STRING:
			ASSERT( m_pstring != NULL );
			return	*m_pstring;

	case	DBVT_BINARY:
			ASSERT( m_pbinary != NULL );
			::BinaryToString( cValue, *m_pbinary, true );
			return	cValue;
	}
	//	Undefined data type
	ASSERT( FALSE );
	return	cValue;
}

CLongBinary*	CDBField::AsBinary()	const
{
	switch( m_dwType ) {
	case	DBVT_NULL:
			return	NULL;

	case	DBVT_BOOL:
	case	DBVT_UCHAR:
	case	DBVT_SHORT:
	case	DBVT_LONG:
	case	DBVT_SINGLE:
	case	DBVT_DOUBLE:
	case	DBVT_DATE:
	case	DBVT_STRING:
			//	Cannot convert to long binary
			ASSERT( FALSE );
			break;

	case	DBVT_BINARY:
			return	m_pbinary;
	}
	//	Undefined data type
	ASSERT( FALSE );
	return	m_pbinary;
}
////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////
//	Assignment operators
////////////////////////////////////////////////////////////////

CDBField& CDBField::operator =( const bool bVal )
{
	switch( m_dwType ) {
	case	DBVT_NULL:
			//	Undefined data type
			ASSERT( FALSE );

	case	DBVT_BOOL:
			m_boolVal = bVal;
			return	*this;

	case	DBVT_UCHAR:
			m_chVal = (bVal) ? 'T' : 'F';
			return	*this;

	case	DBVT_SHORT:
			m_iVal = (bVal) ? 1 : 0;
			return	*this;

	case	DBVT_LONG:
			m_lVal = (bVal) ? 1 : 0;
			return	*this;

	case	DBVT_SINGLE:
			m_fltVal = (float)((bVal) ? 1.0 : 0.0);
			return	*this;

	case	DBVT_DOUBLE:
			m_dblVal = (double)((bVal) ? 1.0 : 0.0);
			return	*this;

	case	DBVT_DATE:
			//	Cannot convert to datetime
			ASSERT( FALSE );
			return	*this;

	case	DBVT_STRING:
			ASSERT( m_pstring != NULL );
			m_pstring->Format( "%c", (bVal) ? 'T' : 'F' );
			return	*this;

	case	DBVT_BINARY:
			//	CRecordset does not support writing to CLongBinary fields
			ASSERT( FALSE );
			return	*this;
	}
	//	Undefined data type
	ASSERT( FALSE );
	return	*this;
}
CDBField& CDBField::operator =( const char chVal )
{
	return	operator =( (unsigned char) chVal );
}
CDBField& CDBField::operator =( const unsigned char chVal )
{
	switch( m_dwType ) {
	case	DBVT_NULL:
			//	Undefined data type
			ASSERT( FALSE );

	case	DBVT_BOOL:
			m_boolVal = (chVal == 'T' || chVal == '1');
			return	*this;

	case	DBVT_UCHAR:
			m_chVal = (unsigned char)chVal;
			return	*this;

	case	DBVT_SHORT:
			m_iVal = (short)chVal;
			return	*this;

	case	DBVT_LONG:
			m_lVal = (long)chVal;
			return	*this;

	case	DBVT_SINGLE:
			m_fltVal = (float)chVal;
			return	*this;

	case	DBVT_DOUBLE:
			m_dblVal = (double)chVal;
			return	*this;

	case	DBVT_DATE:
			//	Cannot convert to datetime
			ASSERT( FALSE );
			return	*this;

	case	DBVT_STRING:
			ASSERT( m_pstring != NULL );
			m_pstring->Format( "%c", chVal );
			return	*this;

	case	DBVT_BINARY:
			//	CRecordset does not support writing to CLongBinary fields
			ASSERT( FALSE );
			return	*this;
	}
	//	Undefined data type
	ASSERT( FALSE );
	return	*this;
}
CDBField& CDBField::operator =( const short sVal )
{
	switch( m_dwType ) {
	case	DBVT_NULL:
			//	Undefined data type
			ASSERT( FALSE );

	case	DBVT_BOOL:
			m_boolVal = (sVal != 0);
			return	*this;

	case	DBVT_UCHAR:
			m_chVal = (unsigned char)sVal;
			return	*this;

	case	DBVT_SHORT:
			m_iVal = (short)sVal;
			return	*this;

	case	DBVT_LONG:
			m_lVal = (long)sVal;
			return	*this;

	case	DBVT_SINGLE:
			m_fltVal = (float)sVal;
			return	*this;

	case	DBVT_DOUBLE:
			m_dblVal = (double)sVal;
			return	*this;

	case	DBVT_DATE:
			//	Cannot convert to datetime
			ASSERT( FALSE );
			return	*this;

	case	DBVT_STRING:
			ASSERT( m_pstring != NULL );
			m_pstring->Format( "%hd", sVal );
			return	*this;

	case	DBVT_BINARY:
			//	CRecordset does not support writing to CLongBinary fields
			ASSERT( FALSE );
			return	*this;
	}
	//	Undefined data type
	ASSERT( FALSE );
	return	*this;
}
CDBField& CDBField::operator =( const int iVal )
{
	switch( m_dwType ) {
	case	DBVT_NULL:
			//	Undefined data type
			ASSERT( FALSE );

	case	DBVT_BOOL:
			m_boolVal = (iVal != 0);
			return	*this;

	case	DBVT_UCHAR:
			m_chVal = (unsigned char)iVal;
			return	*this;

	case	DBVT_SHORT:
			m_iVal = (short)iVal;
			return	*this;

	case	DBVT_LONG:
			m_lVal = (long)iVal;
			return	*this;

	case	DBVT_SINGLE:
			m_fltVal = (float)iVal;
			return	*this;

	case	DBVT_DOUBLE:
			m_dblVal = (double)iVal;
			return	*this;

	case	DBVT_DATE:
			//	Cannot convert to datetime
			ASSERT( FALSE );
			return	*this;

	case	DBVT_STRING:
			ASSERT( m_pstring != NULL );
			m_pstring->Format( "%d", iVal );
			return	*this;

	case	DBVT_BINARY:
			//	CRecordset does not support writing to CLongBinary fields
			ASSERT( FALSE );
			return	*this;
	}
	//	Undefined data type
	ASSERT( FALSE );
	return	*this;
}
CDBField& CDBField::operator =( const long lVal )
{
	switch( m_dwType ) {
	case	DBVT_NULL:
			//	Undefined data type
			ASSERT( FALSE );

	case	DBVT_BOOL:
			m_boolVal = (lVal != 0);
			return	*this;

	case	DBVT_UCHAR:
			m_chVal = (unsigned char)lVal;
			return	*this;

	case	DBVT_SHORT:
			m_iVal = (short)lVal;
			return	*this;

	case	DBVT_LONG:
			m_lVal = (long)lVal;
			return	*this;

	case	DBVT_SINGLE:
			m_fltVal = (float)lVal;
			return	*this;

	case	DBVT_DOUBLE:
			m_dblVal = (double)lVal;
			return	*this;

	case	DBVT_DATE:
			//	Cannot convert to datetime
			ASSERT( FALSE );
			return	*this;

	case	DBVT_STRING:
			ASSERT( m_pstring != NULL );
			m_pstring->Format( "%ld", lVal );
			return	*this;

	case	DBVT_BINARY:
			//	CRecordset does not support writing to CLongBinary fields
			ASSERT( FALSE );
			return	*this;
	}
	//	Undefined data type
	ASSERT( FALSE );
	return	*this;
}
CDBField& CDBField::operator =( const float fltVal )
{
	switch( m_dwType ) {
	case	DBVT_NULL:
			//	Undefined data type
			ASSERT( FALSE );

	case	DBVT_BOOL:
			m_boolVal = (fltVal != 0.0);
			return	*this;

	case	DBVT_UCHAR:
			m_chVal = (unsigned char)fltVal;
			return	*this;

	case	DBVT_SHORT:
			m_iVal = (short)fltVal;
			return	*this;

	case	DBVT_LONG:
			m_lVal = (long)fltVal;
			return	*this;

	case	DBVT_SINGLE:
			m_fltVal = (float)fltVal;
			return	*this;

	case	DBVT_DOUBLE:
			m_dblVal = (double)fltVal;
			return	*this;

	case	DBVT_DATE:
			//	Cannot convert to datetime
			ASSERT( FALSE );
			return	*this;

	case	DBVT_STRING:
			ASSERT( m_pstring != NULL );
			m_pstring->Format( "%f", fltVal );
			return	*this;

	case	DBVT_BINARY:
			//	CRecordset does not support writing to CLongBinary fields
			ASSERT( FALSE );
			return	*this;
	}
	//	Undefined data type
	ASSERT( FALSE );
	return	*this;
}
CDBField& CDBField::operator =( const double dblVal )
{
	switch( m_dwType ) {
	case	DBVT_NULL:
			//	Undefined data type
			ASSERT( FALSE );

	case	DBVT_BOOL:
			m_boolVal = (dblVal != 0.0);
			return	*this;

	case	DBVT_UCHAR:
			m_chVal = (unsigned char)dblVal;
			return	*this;

	case	DBVT_SHORT:
			m_iVal = (short)dblVal;
			return	*this;

	case	DBVT_LONG:
			m_lVal = (long)dblVal;
			return	*this;

	case	DBVT_SINGLE:
			m_fltVal = (float)dblVal;
			return	*this;

	case	DBVT_DOUBLE:
			m_dblVal = (double)dblVal;
			return	*this;

	case	DBVT_DATE:
			//	Cannot convert to datetime
			ASSERT( FALSE );
			return	*this;

	case	DBVT_STRING:
			ASSERT( m_pstring != NULL );
			m_pstring->Format( "%f", dblVal );
			return	*this;

	case	DBVT_BINARY:
			//	CRecordset does not support writing to CLongBinary fields
			ASSERT( FALSE );
			return	*this;
	}
	//	Undefined data type
	ASSERT( FALSE );
	return	*this;
}
CDBField& CDBField::operator =( const COleDateTime& dtVal )
{
	switch( m_dwType ) {
	case	DBVT_NULL:
			//	Undefined data type
			ASSERT( FALSE );

	case	DBVT_BOOL:
	case	DBVT_UCHAR:
	case	DBVT_SHORT:
	case	DBVT_LONG:
	case	DBVT_SINGLE:
	case	DBVT_DOUBLE:
			//	Cannot convert to the current data type
			ASSERT( FALSE );
			return	*this;

	case	DBVT_DATE:
			ASSERT( m_pdate != NULL );
			m_pdate->year	= dtVal.GetYear();
			m_pdate->month	= dtVal.GetMonth();
			m_pdate->day	= dtVal.GetDay();
			m_pdate->hour	= dtVal.GetHour();
			m_pdate->minute	= dtVal.GetMinute();
			m_pdate->second = dtVal.GetSecond();
			m_pdate->fraction = 0;
			return	*this;

	case	DBVT_STRING:
			ASSERT( m_pstring != NULL );
			*m_pstring = dtVal.Format();
			return	*this;

	case	DBVT_BINARY:
			//	CRecordset does not support writing to CLongBinary fields
			ASSERT( FALSE );
			return	*this;
	}
	//	Undefined data type
	ASSERT( FALSE );
	return	*this;
}
CDBField& CDBField::operator =( const CString& cVal )
{
	return	operator =( (LPCTSTR)cVal );
}
CDBField& CDBField::operator =( const LPCTSTR szVal )
{
	switch( m_dwType ) {
	case	DBVT_NULL:
			//	Undefined data type
			ASSERT( FALSE );

	case	DBVT_BOOL:
			m_boolVal = (szVal != NULL || atoi( szVal ) != 0 );
			return	*this;

	case	DBVT_UCHAR:
			m_chVal = (unsigned char)szVal[0];
			return	*this;

	case	DBVT_SHORT:
			m_iVal = (short)atoi( szVal );
			return	*this;

	case	DBVT_LONG:
			m_lVal = (long)atol( szVal );
			return	*this;

	case	DBVT_SINGLE:
			m_fltVal = (float)atof( szVal );
			return	*this;

	case	DBVT_DOUBLE:
			m_dblVal = (double)atof( szVal );
			return	*this;

	case	DBVT_DATE:
		{
			ASSERT( m_pdate != NULL );
			COleDateTime	dt;
			dt.ParseDateTime( szVal );
			m_pdate->year	= dt.GetYear();
			m_pdate->month	= dt.GetMonth();
			m_pdate->day	= dt.GetDay();
			m_pdate->hour	= dt.GetHour();
			m_pdate->minute	= dt.GetMinute();
			m_pdate->second = dt.GetSecond();
			m_pdate->fraction = 0;
			return	*this;
		}

	case	DBVT_STRING:
			ASSERT( m_pstring != NULL );
			*m_pstring = szVal;
			return	*this;

	case	DBVT_BINARY:
			//	CRecordset does not support writing to CLongBinary fields
			ASSERT( FALSE );
			return	*this;
	}
	//	Undefined data type
	ASSERT( FALSE );
	return	*this;
}
////////////////////////////////////////////////////////////////
