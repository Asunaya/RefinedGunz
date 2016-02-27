#include "stdafx.h"
#include "Tester.h"
#include "MSafeUDP.h"
#include "MSharedCommandTable.h"
#include "Msg.h"
#include "MCommandCommunicator.h"
#include "MXml.h"

#include <map>
using namespace std;
#include <process.h>


Tester* g_Test = 0;


Tester::Tester() : m_pCmd( 0 ), m_nElapse( 1000 )
{
}


Tester::~Tester()
{
}

bool Tester::Create()
{
	MAddSharedCommandTable( &m_CommandManager, 0 );


	m_pSafeUDP = new MSafeUDP;
	if( 0 == m_pSafeUDP )
		return false;

	if( m_pSafeUDP->Create(true, 9901) )
	{
		m_pSafeUDP->SetCustomRecvCallback( UDPSocketRecvEvent );
	}
	else
		return false;

	if( !CreateTestThread() )
	{
		mlog( "Create fail.\n" );
		return false;
	}

	mlog( "Created.\n" );

	g_Test = this;

	return true;
}


bool Tester::CreateTestThread()
{
	HANDLE  hThread;
	DWORD   dwThreadId;

	hThread = CreateThread(NULL, 0, LocatorTesterThread, this, 0, &dwThreadId);

	return (NULL != hThread);
}


bool Tester::SendUDPtoLocator()
{
	// 222.111.150.87
	// SendCommandByUDP( inet_addr("127.0.0.1"), 8900, GetCommand() );
	// SendCommandByUDP( inet_addr("222.111.150.84"), 8900, GetCommand() );

// India
//	SendCommandByUDP( inet_addr("202.92.10.152"), 8900, GetCommand() );
//	SendCommandByUDP( inet_addr("202.92.10.153"), 8900, GetCommand() );
	
	SendCommandByUDP( inet_addr("202.92.10.176"), 8900, GetCommand() );	// India Test
	
	
// Brazil
//	SendCommandByUDP( inet_addr("200.229.52.13"), 8900, GetCommand() );

	return true;
}


DWORD WINAPI Tester::LocatorTesterThread( void* pWorkContext )
{
	// 미친듯이 보내BoA요~

	Tester* pTester = reinterpret_cast<Tester*>(pWorkContext);

	int nCount = 0;
	DWORD dwStart = timeGetTime();
	DWORD dwEnd;

	while( true )
	{
		if( pTester->SendUDPtoLocator() )
		{
			++nCount;
		}
		else
			ASSERT( 0 );

		dwEnd = timeGetTime();
		if( (0 < nCount) && (10000 < (dwEnd - dwStart)) )
		{
			mlog( "Time:%u Send Count:%d\n", dwEnd - dwStart, nCount / ((dwEnd - dwStart)/1000));

			dwStart = timeGetTime();
			nCount = 0;
		}
		
		Sleep( 1000 );
	}
	
	return 0;
}


const int Tester::MakeCmdPacket( char* pOutPacket, const int nMaxSize, MCommand* pCmd )
{
	if( (0 == pOutPacket) || (0 > nMaxSize) || (0 == pCmd) ) 
		return -1;

	MCommandMsg* pMsg = reinterpret_cast< MCommandMsg* >( pOutPacket );

	const nCmdSize = nMaxSize - sizeof(MPacketHeader);

	pMsg->Buffer[ 0 ] = 0;
	pMsg->nCheckSum = 0;

	if( pCmd->m_pCommandDesc->IsFlag(MCCT_NON_ENCRYPTED) )
	{
		pMsg->nMsg = MSGID_RAWCOMMAND;

		const int nGetCmdSize = pCmd->GetData( pMsg->Buffer, nCmdSize );
		if( nGetCmdSize != nCmdSize )
			return -1;

		pMsg->nSize		= static_cast< unsigned int >( sizeof(MPacketHeader) ) + nGetCmdSize;
		pMsg->nCheckSum = MBuildCheckSum(pMsg, pMsg->nSize);
	}
	else
	{
		ASSERT( 0 && "암호화된 커맨드 처리는 없음.\n" );
		
		return -1;
	}

	return pMsg->nSize;
}


void Tester::SendCommandByUDP( DWORD dwIP, int nPort, MCommand* pCmd )
{
	const int nPacketSize = CalcPacketSize( pCmd );
	char* pszPacketBuf = new char[ nPacketSize ];
	if( 0 != pszPacketBuf ) 
	{
		const int nMakePacketSize = MakeCmdPacket( pszPacketBuf, nPacketSize, pCmd );
		if( nPacketSize == nMakePacketSize )
		{
			if( !m_pSafeUDP->Send(dwIP, nPort, pszPacketBuf, nMakePacketSize) )

			{
				mlog( "MLocator::SendCommandByUDP - UDP send 실패.\n" );
			}
		}
		else
		{
			delete [] pszPacketBuf;
			ASSERT( 0 && "Packet을 만드는데 문제가 있음." );
		}
	}
}


MCommand* Tester::GetCommand()
{
	if( 0 != m_pCmd ) return m_pCmd;

	MCommandDesc* pCmdDesc = m_CommandManager.GetCommandDescByID( MC_REQUEST_SERVER_LIST_INFO );
	if( 0 == pCmdDesc )
	{
		ASSERT( 0 );
		mlog( "1" );
		return false;
	}

	m_pCmd = new MCommand( MC_REQUEST_SERVER_LIST_INFO, 
								  MUID(0, 0), 
								  MUID(0, 0), 
								  &m_CommandManager);
	if( 0 == m_pCmd )
	{
		ASSERT( 0 );
		mlog( "2" );
		return false;
	}

	return m_pCmd;
}


void Tester::ReleaseCommand()
{
	if( 0 != m_pCmd )
		delete m_pCmd;
}


bool Tester::UDPSocketRecvEvent( DWORD dwIP, WORD wRawPort, char* pPacket, DWORD dwSize )
{
	static DWORD dwStart = timeGetTime();
	static unsigned int nCount = 0;
	static DWORD dwEnd;

	dwEnd = timeGetTime();
	++nCount;

	// mlog( "(%u:%d) ", dwEnd - dwStart, nCount );

	if( 1000 < (dwEnd - dwStart) )
	{
		char szTime[256]="";

		SYSTEMTIME tm;
		GetLocalTime(&tm);

		string strTime;
		strTime = "[";
		wsprintf(szTime, "%02i", tm.wYear%100);
		strTime += szTime;
		strTime +="/";
		wsprintf(szTime, "%02i", tm.wMonth);
		strTime += szTime;
		strTime +="/";
		wsprintf(szTime, "%02i", tm.wDay);
		strTime += szTime;
		strTime += " ";
		wsprintf(szTime, "%02i", tm.wHour);
		strTime += szTime;
		strTime += ":";
		wsprintf(szTime, "%02i", tm.wMinute);
		strTime += szTime;
		strTime += ":";
		wsprintf(szTime, "%02i", tm.wSecond);
		strTime += szTime;
		strTime += "]";

		char szBuf[ 1024 ] = {0,};
		sprintf( szBuf, "%s Time:%.1f, Recv Count : %d\n", strTime.c_str(), (dwEnd - dwStart) / 1000.0f, nCount );
		// mlog( "\nTime:%u, Recv Count : %d\n", dwEnd - dwStart, nCount );
		nCount = 0;
		dwStart = timeGetTime();

		if( 0 != g_Test )
		{
			g_Test->GetOut()->SetWindowText( szBuf );
		}
	
	}
	return true;
}


struct ix
{
	string id;
	string name;
	string desc;
};

