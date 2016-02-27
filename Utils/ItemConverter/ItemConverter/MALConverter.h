#ifndef _MALCONVERTER_H
#define _MALCONVERTER_H

#include "MMatchGlobal.h"
#include "ODBCRecordset.h"

class MConverter
{
private:
	CDatabase	m_DB;
	CString		m_strDSNConnect;

	bool CheckOpen();
	void Log(const char *pFormat,...);

public:
	MConverter();
	virtual ~MConverter();
	CDatabase* GetDatabase()	{ return &m_DB; }
	CString BuildDSNString(const CString strDSN, const CString strUserName, const CString strPassword);
	bool Connect(CString strDSNConnect);
	void Disconnect();


	bool InitDB();
	bool ConvertActionLeague();
};




#endif