#pragma once

#include "MDatabase.h"

#include <vector>
using std::vector;

#include <afxdb.h>
#include <afxtempl.h>
#include <afxdtctl.h>

#define BINARY_FIELD_MAX_SIZE	7000
#define BINARY_CHUNK_SIZE		7000
#define RESERVE_SIZE			100

class CSimpleDBBinary
{
public :
	CSimpleDBBinary() : m_UsedSize( 0 ) {}
	~CSimpleDBBinary() {}

	int GetUsedSize() { return m_UsedSize; }

	void SetUsedSize( const int iSize ) 
	{ 
		m_UsedSize = iSize; 
	}

	int GetData( unsigned char* pOutBuf, const int nOutBufSize )
	{
		if( (0 == pOutBuf) || 
			(m_UsedSize > nOutBufSize) ||
			(0 >= nOutBufSize) )
			return -1;

		memcpy( pOutBuf, m_Data, m_UsedSize );

		return m_UsedSize;
	}

	int SetData( const unsigned char* pData, const int nInDataSize )
	{
		if( (0 == pData) || 
			(BINARY_FIELD_MAX_SIZE < nInDataSize) || 
			(0 > nInDataSize) )
			return -1;

		m_UsedSize = nInDataSize;

		memcpy( m_Data, pData, nInDataSize );

		return m_UsedSize;
	}

private :
	char	m_Data[ BINARY_FIELD_MAX_SIZE ];
	int		m_UsedSize;

public :
	int test;
};



class CDBBinary
{
public :
	CDBBinary( const int iFieldNum = RESERVE_SIZE, const int iReserveSize = RESERVE_SIZE ) : 
	   m_iIndex( 0 ), m_iCurrentUsedSize( 0 ), m_ReserveSize( iReserveSize )
	{
		Reserve( iFieldNum );
	}

    ~CDBBinary()
	{
		m_vBinary.clear();
	}

	typedef int							BinaryLength;
	typedef vector< CSimpleDBBinary >	BinaryDataVec;
	typedef BinaryDataVec::iterator		BinaryDataIter;

	void Begin()
	{
		m_iIndex = 0;
	}

	void Clear() { m_vBinary.clear(); }
	
	int	GetCurUsedSize()	{ return m_iCurrentUsedSize; }
	int	GetCurIndex()		{ return m_iIndex; }
	int	GetReserverSize()	{ return m_ReserveSize; }

	void SetCurUsedSize( const int iCurUsedSize ) { m_iCurrentUsedSize = iCurUsedSize; }

	void SetReserveSize( const int nReserveSize ) 
	{
		if( 0 > nReserveSize )
		{
			ASSERT( 0 );
			return;
		}

		m_ReserveSize = nReserveSize;
	}

	int GetNextData( unsigned char* pOutputBuf, const int nDestBufSize )
	{
		if( m_iIndex > m_iCurrentUsedSize )
			return -1;

		if( -1 == m_vBinary[m_iIndex].GetData(pOutputBuf, nDestBufSize) )
			return -1;

		return m_vBinary[ m_iIndex++ ].GetUsedSize();
	}

	int InsertData( const unsigned char* pData, const int nSrcDataSize )
	{
		if( m_iIndex >= static_cast<int>(m_vBinary.capacity()) )
		{
			if( !Reserve(m_ReserveSize) )
				return -1;
		}

		++m_iIndex;
		m_iCurrentUsedSize = m_iIndex;

		CSimpleDBBinary sbn;

		// test
		sbn.test = m_iIndex;

		if( nSrcDataSize != sbn.SetData(pData, nSrcDataSize) )
		{
			--m_iIndex;
			--m_iCurrentUsedSize;

			return -1;
		}

		m_vBinary.push_back( sbn );
		
		return sbn.GetUsedSize();
	}

private :
	bool Reserve( const int iExtSize )
	{
		if( 0 > iExtSize )
		{
			ASSERT( 0 );
			return false;
		}

		try
		{
			m_vBinary.reserve( iExtSize + m_vBinary.size() );
		}
		catch( ... )
		{
			m_iCurrentUsedSize	= -1;
			m_iIndex			= -1;

			return false;
		}

		return true;
	}
	
private :
	int				m_iIndex;
	int				m_iCurrentUsedSize;
	int				m_ReserveSize;
	BinaryDataVec	m_vBinary;
};


class	CDBField : public CDBVariant {
//	Constructors are private to avoid instantiation except by CODBCRecordset
private:
	CDBField();
	CDBField( const CDBField& dbv );
	
	CDBField& operator =( const CDBField& dbv );

	virtual ~CDBField();

public:
	//	Assignment operators
	CDBField& operator =( const bool bVal );
	CDBField& operator =( const char chVal );
	CDBField& operator =( const unsigned char chVal );
	CDBField& operator =( const short sVal );
	CDBField& operator =( const int iVal );
	CDBField& operator =( const long lVal );
	CDBField& operator =( const float fVal );
	CDBField& operator =( const double fVal );
	CDBField& operator =( const COleDateTime& dtVal );
	CDBField& operator =( const CString& cVal );
	CDBField& operator =( const LPCTSTR szVal );



	//	Get the bool value. Do appropriate conversion.
	bool			AsBool()	const;
	//	Get the unsigned char value. Do appropriate conversion.
	unsigned char	AsChar()	const;
	//	Get the short value. Do appropriate conversion.
	short			AsShort()	const;
	//***	Get the int value which is equal to long value. Do appropriate conversion.
	int				AsInt()		const;
	//	Get the long value. Do appropriate conversion.
	long			AsLong()	const;
	//	Get the float value. Do appropriate conversion.
	float			AsFloat()	const;
	//	Get the double value. Do appropriate conversion.
	double			AsDouble()	const;
	//	Get the date and time value. Do appropriate conversion.
	COleDateTime	AsDate()	const;
	//	Get the CString value. Do appropriate conversion.
	CString			AsString()	const;
	//	Get the long binary value. Do appropriate conversion.
	CLongBinary*	AsBinary()	const;


	//	Data type conversion operators

	operator	bool() const;
	operator	unsigned char() const;
	operator	short() const;
	operator	int() const;
	operator	long() const;
	operator	float() const;
	operator	double() const;
	operator	COleDateTime() const;
	operator	CString() const;
	operator	CLongBinary*() const;

	//	Field type query methods

	bool	IsNull()	const	{ return m_bIsNull || m_dwType == DBVT_NULL; };
	bool	IsBool()	const	{ return m_dwType == DBVT_BOOL; };
	bool	IsChar()	const	{ return m_dwType == DBVT_UCHAR; };
	bool	IsShort()	const	{ return m_dwType == DBVT_SHORT; };
	bool	IsInt()		const	{ return m_dwType == DBVT_LONG; };
	bool	IsLong()	const	{ return m_dwType == DBVT_LONG; };
	bool	IsFloat()	const	{ return m_dwType == DBVT_SINGLE; };
	bool	IsDouble()	const	{ return m_dwType == DBVT_DOUBLE; };
	bool	IsNumber()	const	{ return IsShort() || IsLong() || IsFloat() || IsDouble(); };
	bool	IsDate()	const	{ return m_dwType == DBVT_DATE; };
	bool	IsString()	const	{ return m_dwType == DBVT_STRING; };
	bool	IsBinary()	const	{ return m_dwType == DBVT_BINARY; };

	//	Get the field name
	const	CString&	GetName()	const;

private:
	bool		m_bIsNull;	//	If this field contain a NULL value
	CString		m_cName;	//	Contain the field (column) name
	
	void	SetNull( bool bIsNull )	{ m_bIsNull = bIsNull; };

	friend	class	CODBCRecordset;
};
/////////////////////////////////////////////////////////////////////////////
inline
const	CString&	CDBField::GetName()	const {
	return	m_cName;
}

inline
CDBField::operator	bool() const {
	return	AsBool();
}
inline
CDBField::operator	unsigned char() const {
	return	AsChar();
}
inline
CDBField::operator	short() const {
	return	AsShort();
}
inline
CDBField::operator	int() const {
	return	AsInt();
}
inline
CDBField::operator	long() const {
	return	AsLong();
}
inline
CDBField::operator	float() const {
	return	AsFloat();
}
inline
CDBField::operator	double() const {
	return	AsDouble();
}
inline
CDBField::operator	COleDateTime() const {
	return	AsDate();
}
inline
CDBField::operator	CString() const {
	return	AsString();
}
inline
CDBField::operator	CLongBinary*() const {
	return	NULL;
}
/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
//	CODBCRecordset
class	CODBCRecordset : public CRecordset
{
public:
	// Our constructor uses CRecordset constructor
	// CODBCRecordset( CDatabase* pDatabase = NULL );
	CODBCRecordset( MDatabase* pDatabase = NULL );
	virtual	~CODBCRecordset();

	virtual	BOOL	Open( LPCTSTR lpszSQL, 
						  UINT nOpenType = AFX_DB_USE_DEFAULT_TYPE,
						  DWORD dwOptions = 0 );

	virtual	void	Move( long nRows, WORD wFetchType = SQL_FETCH_RELATIVE );// throw( CDBException, CMemoryException );


	//	Number of fields could be get by 
	//	CRecordset::GetODBCFieldCount() method


	//	New functions:

	//	Get the field ID by name - case insensitive
	//	CRecordset::GetFieldIndexByName() works, 
	//	but is case sensitive
	int			GetFieldID( LPCTSTR szName );
	//	Get the field name by ID
	CString		GetFieldName( int nID );


	//	Gets a field by name
	CDBField&	Field( LPCTSTR szName );
	//	Gets a field by number
	CDBField&	Field( int nField );

	//	Gets a field by name
	CDBField&	operator()( LPCTSTR szField );
	//	Gets a field by number
	CDBField&	operator()( int nField );


	//	Get values by field names. Do conversion as appropriate.

	bool			GetBool( LPCTSTR szName );
	unsigned char	GetChar( LPCTSTR szName );
	short			GetShort( LPCTSTR szName );
	int				GetInt( LPCTSTR szName );
	long			GetLong( LPCTSTR szName );
	float			GetFloat( LPCTSTR szName );
	double			GetDouble( LPCTSTR szName );
	COleDateTime	GetDate( LPCTSTR szName );
	CString			GetString( LPCTSTR szName );
	CLongBinary*	GetBinary( LPCTSTR szName );


	//	Get values by field number. Do conversion as appropriate.

	bool			GetBool( int nCol );
	unsigned char	GetChar( int nCol );
	short			GetShort( int nCol );
	int				GetInt( int nCol );
	long			GetLong( int nCol );
	float			GetFloat( int nCol );
	double			GetDouble( int nCol );
	COleDateTime	GetDate( int nCol );
	CString			GetString( int nCol );
	CLongBinary*	GetBinary( int nCol );

	bool		InsertBinary( CString strQuery, unsigned char* pData, const int nSize );
	CDBBinary	SelectBinary( CString strQuery );


private:
	CDBField*			m_fields;			//	Storage for fields
	int					m_AllocatedFields;	//	Used to make smart storage reallocation
	CMapStringToPtr		m_mapNameIdx;		//	Map field names to their ID
	bool				m_bNotLoadedFieldsMap;	//	Is field names map still not loaded

	void	LoadFieldNamesMap();

	//	Clear the internal data structures
	void	Clear();
	
	//	RFX support
	virtual	void	DoFieldExchange( CFieldExchange* pFX );

	//	Allocate data buffer and set the field type
	void		AllocDataBuffer( CDBVariant& varValue, CODBCFieldInfo& fi );
	short		GetCFieldType( short nSQLType );
};
/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
//	CODBCRecordset class inline methods
inline
//	Gets a field by name
CDBField&	CODBCRecordset::Field( LPCTSTR szName ) {
	return	Field( GetFieldID( szName ) );
}

inline
//	Gets a field by number
CDBField&	CODBCRecordset::Field( int nField ) {
	//	There is no allocated storage
	ASSERT( m_fields != NULL );
	//	The field ID is invalid
	ASSERT( nField >= 0 );
	ASSERT( nField < GetODBCFieldCount() );
	//	There is no data in the current recordset position
	ASSERT( IsOpen() );

	return	m_fields[ nField ];
}

////////////////////////////////////////////////////////////////
//	Get field methods
inline
CDBField&	CODBCRecordset::operator()( LPCTSTR szField ) {
	return	Field( szField );
}

inline
CDBField&	CODBCRecordset::operator()( int nField ) {
	return	Field( nField );
}

////////////////////////////////////////////////////////////////
//	Get by Column Name
inline
bool	CODBCRecordset::GetBool( LPCTSTR szName ) {
	return	Field( szName ).AsBool();
}

inline
unsigned char	CODBCRecordset::GetChar( LPCTSTR szName ) {
	return	Field( szName ).AsChar();
}

inline
short	CODBCRecordset::GetShort( LPCTSTR szName ) {
	return	Field( szName ).AsShort();
}

inline
int		CODBCRecordset::GetInt( LPCTSTR szName ) {
	return	Field( szName ).AsInt();
}

inline
long	CODBCRecordset::GetLong( LPCTSTR szName ) {
	return	Field( szName ).AsLong();
}

inline
float	CODBCRecordset::GetFloat( LPCTSTR szName ) {
	return	Field( szName ).AsFloat();
}

inline
double	CODBCRecordset:: GetDouble( LPCTSTR szName ) {
	return	Field( szName ).AsDouble();
}

inline
COleDateTime	CODBCRecordset::GetDate( LPCTSTR szName ) {
	return	Field( szName ).AsDate();
}

inline
CString	CODBCRecordset::GetString( LPCTSTR szName ) {
	return	Field( szName ).AsString();
}

inline
CLongBinary*	CODBCRecordset::GetBinary( LPCTSTR szName ) {
	return	Field( szName ).AsBinary();
}

////////////////////////////////////////////////////////////////
// Get by Column Number
inline
bool	CODBCRecordset::GetBool( int nCol ) {
	return	Field( nCol ).AsBool();
}

inline
unsigned char	CODBCRecordset::GetChar( int nCol ) {
	return	Field( nCol ).AsChar();
}

inline
short	CODBCRecordset::GetShort( int nCol ) {
	return	Field( nCol ).AsShort();
}

inline
int		CODBCRecordset::GetInt( int nCol ) {
	return	Field( nCol ).AsInt();
}

inline
long	CODBCRecordset::GetLong( int nCol ) {
	return	Field( nCol ).AsLong();
}

inline
float	CODBCRecordset::GetFloat( int nCol ) {
	return	Field( nCol ).AsFloat();
}

inline
double	CODBCRecordset:: GetDouble( int nCol ) {
	return	Field( nCol ).AsDouble();
}

inline
COleDateTime	CODBCRecordset::GetDate( int nCol ) {
	return	Field( nCol ).AsDate();
}

inline
CString	CODBCRecordset::GetString( int nCol ) {
	return	Field( nCol ).AsString();
}

inline
CLongBinary*	CODBCRecordset::GetBinary( int nCol ) {
	return	Field( nCol ).AsBinary();
}