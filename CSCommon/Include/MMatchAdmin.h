#ifndef _MMATCHADMIN_H
#define _MMATCHADMIN_H

#include <string>
#include <list>
#include <vector>
#include <map>
#include <deque>
#include <algorithm>
using namespace std;

#include "MUID.h"

#define	CONSOLE_TEXTSIZE	4096	///< Console Command Usage, Help 문자열 길이
#define	CONSOLE_NAMESIZE	256		///< Console Name 문자열 길이
#define	ARGVNoMin (-1)
#define ARGVNoMax (-1)

#define ADMIN_COMMAND_PREFIX		'@'

struct MAdminArgvInfo
{
	char *cargv[255];
	int cargc;
	char argbuf[2048];		// 실제 argv 값이 들어가는 buf
};

class MMatchServer;

class MMatchAdmin
{
private:
protected:
	MMatchServer*		m_pMatchServer;
	bool MakeArgv(char* szStr, MAdminArgvInfo* pAi);
public:
	MMatchAdmin();
	virtual ~MMatchAdmin();
	bool Create(MMatchServer* pServer);
	void Destroy();
	bool Execute(const MUID& uidAdmin, const char* szStr);
};


#endif