#ifndef _MLOG_MANAGER
#define _MLOG_MANAGER

#include <vector>
#include <string>

using namespace std;

class MLogManager
{
public :
	MLogManager();
	~MLogManager();

	void InsertLog( const string& strLog );
	void SafeInsertLog( const string& strLog );

	void Lock()		{ EnterCriticalSection( &m_csLock ); }
	void Unlock()	{ LeaveCriticalSection( &m_csLock ); }

	void WriteMLog();
	void SafeWriteMLog();

	void Reset() { m_MLog.clear(); }
	void SafeReset();

	static MLogManager& GetInstance()
	{
		static MLogManager LogManager;
		return LogManager;
	}
	
private :
	CRITICAL_SECTION m_csLock;

	vector< string > m_MLog;
};

MLogManager& GetLogManager();

#endif