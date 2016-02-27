#include "MALConverter.h"
#include "MDebug.h"


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
		Log("MConverter::CheckOpen - Database 재접\n");
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
			Log("MConverter::Connect - %s\n", e->m_strError);
		}
	} else {
		try {
			bRet = m_DB.Open(NULL,			//	DSN
							 FALSE,			//	Exclusive
							 FALSE,			//	ReadOnly
							 strDSNConnect,	//	ODBC Connect string
							 TRUE);			//	Use cursor lib
		} catch(CDBException* e) {
			Log("MConverter::Connect - %s\n", e->m_strError);
			try {
				bRet = m_DB.Open(NULL);
			} catch(CDBException* e) {
				Log("MConverter::Connect - %s\n", e->m_strError);

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


void MConverter::Log(const char *pFormat,...)
{
	va_list args;
	char msg[1024];

	va_start(args, pFormat);
	vsprintf(msg, pFormat, args);

	mlog(msg);

	va_end(args);
}


bool MConverter::InitDB()
{
	// 우선 그냥 하드코딩
	CString str = BuildDSNString("gunzdb", "sa", "3dalok");

	if (Connect(str))
	{
		Log("DBMS connected");
	}
	else
	{
		Log("Can't Connect To DBMS");
		return false;
	}

	return true;
}


bool MConverter::ConvertActionLeague()
{
	if (!CheckOpen()) return false;

	CString strSQL;
	strSQL.Format("SELECT TOP 1 * FROM Account(nolock) where AID=1960");
	CODBCRecordset rs(&m_DB);
	
	bool bException = false;
	try 
	{
		rs.Open(strSQL, CRecordset::forwardOnly, CRecordset::readOnly);
	} 
	catch(CDBException* e)
	{
		bException = true;
		return false;
	}

	if (rs.IsOpen() == FALSE)
	{
		return false;
	}

	int t = 0;
	for( ; ! rs.IsEOF(); rs.MoveNext() )
	{
//		memset(&poutCharList[t], 0, sizeof(MTD_CharInfo));
		char text[256];
		int nAID =  rs.Field("AID").AsInt();
		CString UserID = rs.Field("UserID").AsString();

		sprintf(text, "AID = %d, UserID=%s\n", nAID, (LPCTSTR)UserID);

		t++;
	}

	return true;













}























