#ifndef _MALCONVERTER_H
#define _MALCONVERTER_H

#include "MMatchGlobal.h"
#include "ODBCRecordset.h"
#include <map>
using namespace std;


struct MTeamInfo
{
	int nCID[4];
	int nCount;
	MTeamInfo();
};


class MTeamInfoMap : public map<int, MTeamInfo*>
{
private:
public:
	void Insert(int nTeamNO, int nAID, int nCID);
	void Clear();
	void SortAll();
	void DebugPrint();
};


class MConverter
{
private:
	CDatabase	m_DB;
	CString		m_strDSNConnect;

	bool CheckOpen();
	

	bool UpdateTeam4();
public:
	MConverter();
	virtual ~MConverter();
	CDatabase* GetDatabase()	{ return &m_DB; }
	CString BuildDSNString(const CString strDSN, const CString strUserName, const CString strPassword);
	bool Connect(CString strDSNConnect);
	void Disconnect();


	bool InitDB();
	bool ConvertActionLeague();
	bool ExportRiddickEventUserid();
};


void blog(const char *pFormat,...);

#endif