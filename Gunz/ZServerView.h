#ifndef ZSERVERVIEW
#define ZSERVERVIEW

#include "ZPrerequisites.h"
#include "MWidget.h"
#include <list>

using namespace RealSpace2;



// ServerInfo
struct ServerInfo
{
	char	szName[ 32];
	char	szAddress[ 32];
	int		nPort;
	int		nType;
	int		nNumOfUser;
	int		nCapacity;
	bool	bIsLive;
};
typedef list<ServerInfo*>	SERVERLIST;



// Class ZServerView
class ZServerView : public MWidget
{
protected:
	SERVERLIST		m_cServerList;
	int				m_nSelectNum;
	int				m_nTextOffset;


protected:
	virtual void OnDraw( MDrawContext* pDC);
	virtual bool OnEvent( MEvent* pEvent, MListener* pListener);


public:
	ZServerView( const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZServerView(void);


	void ClearServerList( void);
	bool AddServer( char* szName, char* szAddress, int nPort, int nType, int nNumOfUser, int nCapacity, bool IsLive );

	ServerInfo* GetSelectedServer();
	int GetCurrSel();
	int GetCurrSel2()						{ return m_nSelectNum; }
	void SetCurrSel( int nNumber);
	bool IsSelected()						{ return ( (GetCurrSel() > -1) ? true : false); }
	void SetTextOffset( int nOffset)		{ m_nTextOffset = nOffset; }


#define MINT_SERVERVIEW	"ServerView"
	virtual const char* GetClassName(void){ return MINT_SERVERVIEW; }

};

#endif