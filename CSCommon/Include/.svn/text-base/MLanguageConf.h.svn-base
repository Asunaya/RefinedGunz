#ifndef _MLANGUAGECONF_H
#define _MLANGUAGECONF_H

//#pragma once

#include <map>
#include <string>

using std::map;
using std::string;
using std::pair;

typedef map< string, string >	MsgGroupMap;
typedef MsgGroupMap::iterator	MsgGroupIter;
typedef pair< string, string >	MsgGroupPair;

const char CSC_ERR_MESSAGE_TYPE[]		= "cserror";
const char CSC_ERR_MESSAGE_FILE_NAME[] = "cserror.xml";

class MLanguageConf;
class MMessenger;
class MZFileSystem;

/*
 * 언어 타입을 string이나 int형으로 관리해 주는 클래스.
 */
class MLanguageCode
{
public :
	enum LANGUAGE_CODE
	{
		KOR = 1,
		ENG,
		JPN,
		INTERNATIONAL,
	};

	MLanguageCode();
	~MLanguageCode();

	string	ConvCodeToStrLangType( const int nLangCode );
	int		ConvStrLangTypeToCode( const char* pszLangType );

	inline int GetLangCode() { return m_nLangCode; }

	inline void SetLangCode( const int nLangCode )		
	{ 
		m_nLangCode = nLangCode; 
		m_isCreated = true;
	}

	inline void SetLangCode( const char* pszLangType )	
	{ 
		m_nLangCode = ConvStrLangTypeToCode( pszLangType ); 
		m_isCreated = true;
	}

	inline bool IsCreated() { return m_isCreated; }

private :
	int		m_nLangCode;
	bool	m_isCreated;
};

/*
 * MLanguageConf클래스에 의해 설정된 언어에따라 등록할 메시지 파일의 정보를 저장함.
 * MMessengerManager를 설정할때 기본 데이터로 사용됨.
 */
class MMessageNameGroup
{
public :
	friend MLanguageConf;
	
	MMessageNameGroup();
	virtual ~MMessageNameGroup();

	string GetErrMessageDir();

	inline MsgGroupMap& GetGroup() { return m_mMsgGroup; }

public :
	virtual void Release() { m_mMsgGroup.clear(); }

protected :
	MsgGroupMap m_mMsgGroup;
};

/*
 * 언어 타입에대한 환경 설정( 언어 타입, 언어 파일들이 있는 폴더 등 ).
 */
class MLanguageConf
{
public :
	MLanguageConf();
	virtual ~MLanguageConf();

	inline const string& GetRootDir() const		{ return m_strRootDir; }
	inline const string& GetLangtype() const	{ return m_strLangType; }
	inline const string& GetLangDir() const		{ return m_strLangDir; }

	bool Init( MZFileSystem* pfs, MMessageNameGroup* pMsgNameGroup, const char* pszRootDir = "" );

	inline bool IsHaveLangType() { return m_LangCode.IsCreated(); }

	inline MMessageNameGroup* GetMessageNameGroup() { return m_pMsgNameGroup; }

	inline int		GetCurLangCode() { return m_LangCode.GetLangCode(); }
	inline string	GetCurLangType() { return m_strLangType; }

	// 상위 인터페이스.
	bool CheckFile( MZFileSystem* pfs, MMessageNameGroup* pMsgNameGroup );

	// 언어 타입 저장.
	bool SetLangTypeByStr( const char* pszLangType );

	virtual void Release();

private :
	// 각 상속받은 클래스에 맞는 폴더의 완전한 상대경로를 만들기 위해서 제정의 해야하는 함수.
	virtual void MakeCompleteDir() = 0;	
	// 추가적인 메시지 파일을 등록 시키려면 이 함수를 제정의 하여 등록시키면 됨.
	virtual bool CheckSubTaskFile( MZFileSystem* pfs, MMessageNameGroup* pMsgNameGroup ) = 0;

protected :
	string			m_strRootDir;
	string			m_strLangType;
	string			m_strLangDir;
	MLanguageCode	m_LangCode;

	MMessageNameGroup* m_pMsgNameGroup;
};

/*
 * 메신저를 관리함. 
 * MMessageNameGroup과 MLanguageConf의 데이터를 바탕으로 등록할 메신저가 결정됨.
 */
class MMessengerManager
{
public :
	MMessengerManager();
	virtual ~MMessengerManager();

	bool Init( MMessageNameGroup* pMMsgNameGroup, MZFileSystem* pfs );

	inline MMessenger* GetErrMessage() { return m_pErrMessenger; }

	void Release();

private :
	// 상위 인터페이스.
	bool Allocate( const string& strKey, const string& strFile, MZFileSystem* pfs );

private :
	virtual bool LoadFile( MMessageNameGroup* pMMsgNameGroup, MZFileSystem* pfs );

	// 상속된 클래스에서 추가로 등록된 메신저를 등록( 할당 )하려면 이 함수를 제정의 해야함.
	virtual bool AllocateSubTask( const string& strKey, const string& strFile, MZFileSystem* pfs ) = 0;
	// 상속된 클래스에서 등록시킨 객체를 제거하기 위해서는 이 함수를 제정의 해야함.
	virtual void ReleaseSubTask() = 0;

protected :
	// 서버와 클라이언트가 공용으로 사용되는 부분은 여기에 등록함.

	MMessenger* m_pErrMessenger;
};

#endif