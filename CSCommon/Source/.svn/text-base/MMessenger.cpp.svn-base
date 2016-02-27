#include "stdafx.h"
#include <crtdbg.h>

#include "MXml.h"
#include "MZFileSystem.h"
#include "MMessenger.h"

#define MESSENGER_TOK_MSG		"MSG"

MMessenger::MMessenger( const char* pszTOK_MESSAGE, const char* pszTOK_CID ) : 
	m_strTOK_MESSAGE( pszTOK_MESSAGE ), m_strTOK_CID( pszTOK_CID )
{
}

MMessenger::~MMessenger()
{
}

bool MMessenger::InitializeMessage( const char* pszFileName,  MZFileSystem *pfs )
{
	if( 0 == pszFileName ) return false;

	MZFile mzf;
	if( !mzf.Open(pszFileName, pfs) )
		return false;

	char *buffer = new char[mzf.GetLength()+1];
	if( 0 == buffer ) return false;

	if( !mzf.Read(buffer,mzf.GetLength()) ) return false;

	buffer[mzf.GetLength()]=0;

	MXmlDocument aXml;
	aXml.Create();
	if(!aXml.LoadFromMemory(buffer))
	{
		delete buffer;
		return false;
	}
	delete buffer;

	MXmlElement aParent = aXml.GetDocumentElement();
	const int	iCount  = aParent.GetChildNodeCount();

	MXmlElement	aChild;
	int			nCID=0;
	char		szTagName[256];
	char		szContents[512];

	for (int i = 0; i < iCount; i++)
	{
		aChild = aParent.GetChildNode(i);
		aChild.GetTagName(szTagName);

		if ((stricmp( szTagName,m_strTOK_MESSAGE.c_str() )==0) || (stricmp( szTagName, MESSENGER_TOK_MSG )==0))
		{
			if(aChild.GetAttribute(&nCID,m_strTOK_CID.c_str()))
			{
				// 이미 등록되어있는게 없어야 한다. ( 메시지 중복 )
				_ASSERT( m_MessageMap.find(nCID)==m_MessageMap.end() );

				aChild.GetContents(szContents);

				m_MessageMap.insert( MsgPair(nCID,string(szContents)) );
			}
		}
	}

	return true;
}


void MMessenger::Release()
{
	m_strTOK_MESSAGE.clear();
	m_strTOK_CID.clear();

	m_MessageMap.clear();
}


bool MMessenger::TranslateMessage( char* poutStr, const int nStringCode, const int argnum, const char* arg1, ... )
{
	MsgIter itor = m_MessageMap.find(nStringCode);
	if(itor==m_MessageMap.end()) 
	{
		_ASSERT(0);
		return false;
	}

	// 인자가 없으면 단지 복사만 하고 끝낸다.
	if ((argnum <= 0) || (argnum > 9))
	{
		strcpy(poutStr, (*itor).second.c_str());
		return true;
	}

	const char* argv[9] = {NULL, };

	va_list args;
	va_start(args, arg1);
	argv[0] = arg1;

	for (int i = 1; i < argnum; i++)
	{
		argv[i] = va_arg(args, const char*);
	}
	va_end(args);

	bool bPercent=false;

	int taridx = 0;
	poutStr[taridx] = 0;
	string formatstring = (*itor).second;
	
#define CHAR_MSG_VALUE			'$'


	// 인자 파싱
	for(size_t j=0;j<formatstring.size();j++)
	{
		char cur=formatstring[j];
		if(bPercent)
		{
			if(cur==CHAR_MSG_VALUE)
			{
				poutStr[taridx++] = CHAR_MSG_VALUE;
				poutStr[taridx] = 0;
			}
			else
			if ( ('1' <= cur) && (cur <= '9') )
			{
				int nParam = cur - '0' - 1;
				if ( (nParam < argnum) && (argv[nParam] != NULL) )
				{
					strcat(poutStr, argv[nParam]);
					taridx += (int)strlen(argv[nParam]);
				}
				else
				{
					_ASSERT(0);		// xml의 인자 설정이 잘못되었거나, 함수호출시 가변인자가 부족하다.
				}
			}

			bPercent = false;
			continue;
		}

		if(!bPercent)
		{
			if(cur==CHAR_MSG_VALUE)
				bPercent=true;
			else
			{
				poutStr[taridx++] = cur;
				poutStr[taridx] = 0;
			}
		}
	}

	return true;
}


const char* MMessenger::GetStr( const int nStringCode )
{
	MsgIter it = m_MessageMap.find( nStringCode );
	if( m_MessageMap.end() == it ){
		_ASSERT( 0 );
		static char *szBlahBlah = "no message";
		return szBlahBlah;
	}
	
	return it->second.c_str();
}