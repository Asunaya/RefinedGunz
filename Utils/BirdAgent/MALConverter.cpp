#include "MALConverter.h"
#include "MDebug.h"
#include <Windows.h>
#include <Winuser.h>
#include <list>
#include <string>
#include <vector>
using namespace std;

TCHAR g_DB_LEAGUE_SELECT_CID[] = _T("{CALL spLeague_GetCID}");

// tid4, member1cid, member2cid, member3cid, member4cid
TCHAR g_DB_LEAGUE_UPDATE_TEAM4[] = _T("{CALL spUpdateTeam4 (%d, %d, %d, %d, %d)}");

TCHAR g_DB_EXPORT_RIDDICK_EVENT_USERID[] = _T("SELECT DISTINCT userid FROM ConnLog(nolock) WHERE Time BETWEEN '2004.7.21' AND '2004.8.15'");
//TCHAR g_DB_EXPORT_RIDDICK_EVENT_USERID[] = _T("SELECT TOP 1000 userid FROM ConnLog(nolock)");





/////////////////////////////////////////////////////////////////////////////////////////////

MConverter::MConverter()
{
	m_strDSNConnect = "";


}

MConverter::~MConverter()
{

}

bool MConverter::CheckOpen()
{
	bool ret = true;
	if (!m_DB.IsOpen())
	{
		ret = Connect(m_strDSNConnect);
		blog("MConverter::CheckOpen - Database 재접\n");
	}

	return ret;
}

CString MConverter::BuildDSNString(const CString strDSN, const CString strUserName, const CString strPassword)
{
	CString strDSNConnect =  _T("ODBC;DSN=") + strDSN
					+ _T(";UID=") + strUserName
					+ _T(";PWD=") + strPassword;
	return strDSNConnect;
}

bool MConverter::Connect(CString strDSNConnect)
{
	if (m_DB.m_hdbc && m_DB.IsOpen()) m_DB.Close();

	m_strDSNConnect = strDSNConnect;

	BOOL bRet = FALSE;
	if (strDSNConnect.IsEmpty()) {
		try {
			bRet = m_DB.Open(NULL);
		} catch(CDBException* e) {
			blog("MConverter::Connect - %s\n", e->m_strError);
		}
	} else {
		try {
			bRet = m_DB.Open(NULL,			//	DSN
							 FALSE,			//	Exclusive
							 FALSE,			//	ReadOnly
							 strDSNConnect,	//	ODBC Connect string
							 TRUE);			//	Use cursor lib
		} catch(CDBException* e) {
			blog("MConverter::Connect - %s\n", e->m_strError);
			try {
				bRet = m_DB.Open(NULL);
			} catch(CDBException* e) {
				blog("MConverter::Connect - %s\n", e->m_strError);

				AfxMessageBox(e->m_strError);
			}
		}
	}
	if (bRet == TRUE) {
		return true;
	} else {
		OutputDebugString("DATABASE Error \n");
		return false;
	}
}

void MConverter::Disconnect()
{
	if (m_DB.IsOpen())
		m_DB.Close();
}

void blog(const char *pFormat,...)
{
	va_list args;
	char msg1[1024];
	va_start(args, pFormat);
	vsprintf(msg1, pFormat, args);


	CTime theTime = CTime::GetCurrentTime();
	CString szTime = theTime.Format( "[%c]" );

	char msg[1024];
	sprintf(msg, "%s %s", (LPCSTR)szTime, msg1);

	mlog(msg);

	va_end(args);
}


bool MConverter::InitDB()
{
	// 우선 그냥 하드코딩
	CString str = BuildDSNString("gunzdb", "sa", "3dalok");

	if (Connect(str))
	{
		blog("DBMS connected\n");
	}
	else
	{
		blog("Can't Connect To DBMS\n");
		return false;
	}

	return true;
}

MTeamInfo::MTeamInfo() 
{ 
	memset(nCID, 0, sizeof(int)*4); 
//	memset(nAID, 0, sizeof(int)*4); 
	nCount=0; 
};

void MTeamInfoMap::Insert(int nTeamNO, int nAID, int nCID)
{
	static nCIDZeroCount = 0;
	if ((nAID == 0) || (nCID == 0)) 
	{
		nCIDZeroCount++;
		return;
	}

	iterator itor = find(nTeamNO);
	if (itor == end())
	{
		MTeamInfo* pNewTeamInfo = new MTeamInfo;
		pNewTeamInfo->nCID[0] = nCID;
		pNewTeamInfo->nCount = 1;

		insert(value_type(nTeamNO, pNewTeamInfo));
	}
	else
	{
		MTeamInfo* pTeamInfo = (*itor).second;

		if (pTeamInfo->nCount >= 4)
		{
/*
			char text[256];
			sprintf(text, "멤버수가 4명이상 - teamid:%d\n", nTeamNO);
			blog(text);
*/
			return;
		}

		int index = pTeamInfo->nCount;
		pTeamInfo->nCID[index] = nCID;
		pTeamInfo->nCount++;

	}
}

void MTeamInfoMap::Clear()
{
	while (!empty())
	{
		MTeamInfo* pTeamInfo = (*begin()).second;
		delete pTeamInfo;
		erase(begin());
	}
}

static int __cdecl _int_sortfunc(const void* a, const void* b)
{
	return *((int*)a) - *((int*)b);
}

void MTeamInfoMap::SortAll()
{
	int nTotalTeam=0;
	int nTeam1 = 0;
	for (iterator itor = begin(); itor != end(); ++itor)
	{
		nTotalTeam++;

		MTeamInfo* pTeamInfo = (*itor).second;
		if (pTeamInfo->nCount != 4) 
		{
			nTeam1++;
			continue;
		}

		qsort(pTeamInfo->nCID, 4, sizeof(int), _int_sortfunc);
	}

/*
	char text[256];
	sprintf(text, "총팀=%d, 3명이하팀수: %d\n", nTotalTeam, nTeam1);
	blog(text);
*/
}

void MTeamInfoMap::DebugPrint()
{
	int n=0;
	int nNot4Team = 0, nNot4Member=0;
	int nRealTeam = 0, nRealMember = 0;
	char text[256];

	blog("-----------------------\n");
	for (iterator itor = begin(); itor != end(); ++itor)
	{
		MTeamInfo* pTeamInfo = (*itor).second;
		if (pTeamInfo->nCount < 4) 
		{
			nNot4Team++;
			nNot4Member += pTeamInfo->nCount;

			continue;
		}
		else if (pTeamInfo->nCount > 4)
		{
			continue;
		}

		nRealTeam++;
		nRealMember += pTeamInfo->nCount;

		sprintf(text, "%5d(%4d) : %7d %7d %7d %7d\n", (*itor).first, ++n, pTeamInfo->nCID[0], 
			pTeamInfo->nCID[1], pTeamInfo->nCID[2], pTeamInfo->nCID[3]);
		blog(text);
	}
	sprintf(text, "Not4Team = %d, Not4Member = %d , RealTeam = %d, RealMember=%d\n", nNot4Team, nNot4Member,
		nRealTeam, nRealMember);
	blog(text);

	blog("-----------------------\n");
}

MTeamInfoMap		g_TeamInfoMap;

bool MConverter::UpdateTeam4()
{
	if (!CheckOpen()) return false;

	for (MTeamInfoMap::iterator itor = g_TeamInfoMap.begin(); itor != g_TeamInfoMap.end(); ++itor)
	{
		MTeamInfo* pTeamInfo = (*itor).second;
		if (pTeamInfo->nCount != 4) continue;

		try 
		{
			CString strSQL;
			strSQL.Format(g_DB_LEAGUE_UPDATE_TEAM4, (*itor).first, pTeamInfo->nCID[0],
				pTeamInfo->nCID[1], pTeamInfo->nCID[2], pTeamInfo->nCID[3]);

			m_DB.ExecuteSQL( strSQL );
		} 
		catch(CDBException* e)
		{
			blog("MConverter::UpdateTeam4 - %s\n", e->m_strError);
			continue;
		}
	}

	return true;


}

bool MConverter::ConvertActionLeague()
{
	InitDB();

	if (!CheckOpen())
	{
		blog("DB Connect 실패\n");
		return false;
	}

	CODBCRecordset rs(&m_DB);
	
	bool bException = false;
	try 
	{
		CString strSQL;
		strSQL.Format(g_DB_LEAGUE_SELECT_CID);
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		return false;
	}

	g_TeamInfoMap.Clear();

	char text[256];

	int t = 0;
	for( ; ! rs.IsEOF(); rs.MoveNext() )
	{
		int nTeamNO = rs.Field("NO").AsInt();
		int nAID =  rs.Field("AID").AsInt();
		int nCID = rs.Field("CID").AsInt();
		
		g_TeamInfoMap.Insert(nTeamNO, nAID, nCID);

		t++;
	}
	blog("total = %d\n", t);


	g_TeamInfoMap.SortAll();
//	g_TeamInfoMap.DebugPrint();

	// 실제로 업데이트
	UpdateTeam4();

	blog("DB 익스포트 완료\n");


	MessageBox(0, "DB 익스포트 완료", "알림", MB_OK);


	Disconnect();

	return true;


}



bool MConverter::ExportRiddickEventUserid()
{
	return false;
	InitDB();

	if (!CheckOpen())
	{
		blog("DB Connect 실패\n");
		return false;
	}

	CODBCRecordset rs(&m_DB);
	
	bool bException = false;
	try 
	{
		CString strSQL;
		strSQL.Format(g_DB_EXPORT_RIDDICK_EVENT_USERID);
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		return false;
	}

	if ((rs.IsOpen() == FALSE) || (rs.GetRecordCount() <= 0) || (rs.IsBOF()==TRUE)) 
	{
		return false;
	}

	vector<string>	userid_list;
	userid_list.reserve(1000000);
	
#define RIDDICK_USERID_COUNT	50

	int t = 0;
	for( ; ! rs.IsEOF(); rs.MoveNext() )
	{
		string userid = rs.Field("userid").AsString();
		userid_list.push_back(userid);
		t++;
	}

	blog("레코드수: %d\n", t);

	srand( (unsigned)time(NULL) );
	int nRandomIndex[RIDDICK_USERID_COUNT] = {0, };
	for (int i = 0; i < RIDDICK_USERID_COUNT; i++)
	{
		int index = 0;
		bool bExist = false;
		do
		{
			bExist = false;
			index = rand() % t;
			for (int j = 0; j < i; j++)
			{
				if (nRandomIndex[j] == index) 
				{
					bExist = true;
					break;
				}
			}
		}
		while (bExist != false);

		nRandomIndex[i] = index;
	}
	
	FILE* fp = fopen("riddick_userid.csv", "wt");
	for (int i = 0; i < RIDDICK_USERID_COUNT; i++)
	{
		int idx = nRandomIndex[i];
		fprintf(fp, "%s\n", userid_list[idx].c_str());
	}
	fclose(fp);

	blog("DB 익스포트 완료\n");
	MessageBox(0, "DB 익스포트 완료", "알림", MB_OK);


	Disconnect();

	return true;

}



















