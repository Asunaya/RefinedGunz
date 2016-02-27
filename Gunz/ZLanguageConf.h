#ifndef _ZLANGUAGECONF_H
#define _ZLANGUAGECONF_H

//#pragma once

#include <string>
#include <map>

/*
#include "MLanguageConf.h"


using std::string;
using std::map;
using std::pair;

class ZLanguageConf;
class MMessenger;

class ZMessageNameGroup : public MMessageNameGroup
{
public :
	friend ZLanguageConf;
	
	ZMessageNameGroup();
	~ZMessageNameGroup();

	const string GetMessageDir();
};

class ZLanguageConf : public MLanguageConf
{
public :
	ZLanguageConf();
	~ZLanguageConf();

private :
	void MakeCompleteDir();
	bool CheckSubTaskFile( MZFileSystem* pfs, MMessageNameGroup* pMsgNameGroup );
};



class ZMessengerManager : public MMessengerManager
{
public :
	ZMessengerManager();
	~ZMessengerManager();

	MMessenger* GetMessage()	{ return m_pMessenger; }
	
private :
	bool AllocateSubTask( const string& strKey, const string& strFile, MZFileSystem* pfs );
	void ReleaseSubTask();

private :
	MMessenger*	m_pMessenger;
};

*/

#endif