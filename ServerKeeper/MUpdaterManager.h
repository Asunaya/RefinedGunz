#ifndef _MUpdaterManager
#define _MUpdaterManager

#include <map>
using namespace std;

class MUpdater;

enum UPDATER_TYPE
{
	UT_SERVER = 1,
	UT_AGENT,
	UT_LOCATOR,

	UT_END,
};

class MUpdaterManager : private map< UPDATER_TYPE, MUpdater* >
{
public :
	MUpdaterManager();
	~MUpdaterManager();

	void Clear();
	bool Insert( const UPDATER_TYPE nType, MUpdater* pUpdater );
	MUpdater* Find( const UPDATER_TYPE nType );

private :
};

#endif