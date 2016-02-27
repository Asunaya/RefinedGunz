//-----------------------------------------------------------------------------
// File: SessionInfo.h
//
// Desc: Header file for the CSessionInfo utility class. This utility stores
//       player, group, and message information gathered from the application's
//       DirectPlay message handler, and provides a dialog UI to display the
//       data.
//
// Copyright (C) 2000-2001 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _SESSIONINFO_H_
#define _SESSIONINFO_H_

#include <dplay8.h>
#include <commctrl.h>
#include <string.h>
#include <stdio.h>
#include <tchar.h>
#include "dxutil.h"

// Global option defines
#define SI_MAX_MESSAGES     100
#define SI_REFRESH_INTERVAL 3000

// Thread creation is platform-specific
#ifdef UNDER_CE
#define chBEGINTHREADEX CreateThread
#else
#include <process.h>
typedef unsigned (__stdcall *PTHREAD_START) (void *);
#define chBEGINTHREADEX(psa, cbStack, pfnStartAddr, pvParam, fdwCreate, pdwThreadID) ((HANDLE) _beginthreadex( (void *) (psa), (unsigned) (cbStack), (PTHREAD_START) (pfnStartAddr), (void *) (pvParam), (unsigned) (fdwCreate), (unsigned *) (pdwThreadID))) 
#endif // UNDER_CE
 
#ifdef _WIN64
#define SI_HEXVAL TEXT("0x%x")
#else
#define SI_HEXVAL TEXT("0x%I64x")
#endif // _WIN64

#define IDD_SI_MAIN                     15000
#define IDD_SI_PLAYERS                  15001
#define IDD_SI_MESSAGES                 15002
#define IDC_SI_PLAYERS                  16000
#define IDC_SI_GROUPS                   16001
#define IDC_SI_DESCRIPTION              16002
#define IDC_SI_NAME_BORDER              16003
#define IDC_SI_NAME                     16004
#define IDC_SI_MEMBERSHIP               16005
#define IDC_SI_MEMBERSHIP_TEXT          16006
#define IDC_SI_NAME_ICON                16007
#define IDC_SI_DPLAYMSG                 16008
#define IDC_SI_APPMSG                   16009
#define IDC_SI_TAB                      16010

#define SI_REFRESH_TIMER                1

// Message type IDs
#define SI_MSGID_PLAYERINFO     0xDD01             
#define SI_MSGID_GROUPINFO      0xDD02

#define SI_ASYNC_CONTEXT        ((VOID*)0xDD00)

// Change compiler pack alignment to be BYTE aligned
#pragma pack( push, 1 )

// Application message formats
struct SI_MSG
{
    DWORD dwMsgID;
};

struct SI_MSG_PLAYERINFO : public SI_MSG
{
    DPNID dpnID;
    DWORD dwFlags;
    DWORD dwNameLength;
};

struct SI_MSG_GROUPINFO : public SI_MSG
{
    DPNID dpnID;
    DWORD dwNameLength;
};

#pragma pack( pop )



//-----------------------------------------------------------------------------
// Name: class CSIPlayer
// Desc: Describes a DirectPlay player.
//-----------------------------------------------------------------------------
class CSIPlayer
{
public:
    // Constructors/Destructors
    CSIPlayer( DPNID dpnid );
    
    // Member variables
    DPNID id;              // Unique DPNID value
    BOOL  bIsHost;         // Host flag
    TCHAR strName[ 256 ];  // Name string
    TCHAR strURL[ 256 ];   // URL string
};





//-----------------------------------------------------------------------------
// Name: class CSIGroup
// Desc: Describes a DirectPlay group. Contains a DPNID list of member players.
//-----------------------------------------------------------------------------
class CSIGroup
{
public:
    // Constructors/Destructors
    CSIGroup( DPNID dpnid );
    ~CSIGroup();
   
    HRESULT AddMember( DPNID id );
    HRESULT RemoveMember( DPNID id );
    BOOL    IsMember( DPNID id ) { return pMembers->Contains( &id ); }

    // Member variables
    DPNID       id;                  // Unique DPNID value
    TCHAR       strName[ 256 ];      // Name string
    CArrayList* pMembers;            // List of player IDs
};




//-----------------------------------------------------------------------------
// Name: class CMessageList
// Desc: Circular array implementation of a string list
//-----------------------------------------------------------------------------
class CMessageList
{
public:
    // Constructor/Destructor
    CMessageList();
    ~CMessageList();

    // Accessor methods
    DWORD   GetNumOfMessages() { return m_dwNumMessages; }
    BOOL    IsFull() { return m_dwNumMessages == SI_MAX_MESSAGES; }
    TCHAR*  GetMessage( DWORD dwMessageNum );
    TCHAR*  AddMessage( TCHAR* strMessage );

    // Critical section access
    VOID Lock() { EnterCriticalSection( &m_csLock ); }
    VOID Unlock() { LeaveCriticalSection( &m_csLock ); }

private:
    DWORD m_dwStartIndex;   // Starting index in the circular array
    DWORD m_dwNumMessages;  // Total number of stored messages

    TCHAR m_rStrMessage[ SI_MAX_MESSAGES ][ 256 ];  // Array of message strings

    CRITICAL_SECTION m_csLock;  // Access lock for multithreading
};




//-----------------------------------------------------------------------------
// Name: class CSessionInfo
// Desc: Utility class to track players and groups within a session
//-----------------------------------------------------------------------------
class CSessionInfo
{
public:
    // Constructors/Destructor
    CSessionInfo( IDirectPlay8Peer*   pPeer );
    CSessionInfo( IDirectPlay8Client* pClient );
    CSessionInfo( IDirectPlay8Server* pServer );
    ~CSessionInfo();

    // DirectPlay message handler
    BOOL    MessageHandler( DWORD dwMessageId, PVOID pMsgBuffer );
    
    // Dialog display
    HRESULT ShowDialog( HWND hParent );

private:
    // Private initialization
    VOID    Initialize();
    HRESULT InitializeLocalPlayer( DPNID idLocal );
    
    // Accessor methods
    CSIPlayer* FindPlayer( DPNID id );
    CSIGroup*  FindGroup( DPNID id );

    // Message handler helper functions
    HRESULT CreatePlayer( DPNID id );
    HRESULT CreateGroup( DPNID id );

    HRESULT DestroyPlayer( DPNID id );
    HRESULT DestroyGroup( DPNID id );

    HRESULT AddPlayerToGroup( DPNID idPlayer, DPNID idGroup );
    HRESULT RemovePlayerFromGroup( DPNID idPlayer, DPNID idGroup );

    HRESULT OnPlayerInfoReceive( SI_MSG_PLAYERINFO* pPlayerInfo );
    HRESULT OnGroupInfoReceive( SI_MSG_GROUPINFO* pGroupInfo );
    HRESULT OnDpInfoChange( DPNID dpnid );

    // Network communication methods
    HRESULT SendPlayerInfoToAll( DPNID idPlayer ) { return SendPlayerInfoToPlayer( idPlayer, DPNID_ALL_PLAYERS_GROUP ); }
    HRESULT SendPlayerInfoToPlayer( DPNID idPlayer, DPNID idTarget );
    
    HRESULT SendGroupInfoToAll( DPNID idGroup ) { return SendGroupInfoToPlayer( idGroup, DPNID_ALL_PLAYERS_GROUP ); }
    HRESULT SendGroupInfoToPlayer( DPNID idGroup, DPNID idTarget );

    HRESULT SynchronizeWithPlayer( DPNID idPlayer );

    HRESULT RefreshPlayerInfo( DPNID idPlayer );
    HRESULT RefreshGroupInfo( DPNID idGroup );
    
    // DirectPlay helper functions
    HRESULT GetDpPlayerInfo( DPNID dpnid, DPN_PLAYER_INFO** ppPlayerInfo );
    HRESULT GetDpGroupInfo( DPNID dpnid, DPN_GROUP_INFO** ppGroupInfo );
    HRESULT GetDpAppDesc( DPN_APPLICATION_DESC** ppAppDesc );
    
    // Message pump and dialog procedures
    static  INT_PTR CALLBACK StaticDlgProcMain( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static  INT_PTR CALLBACK StaticDlgProcPlayers( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static  INT_PTR CALLBACK StaticDlgProcMessages( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    
    static  DWORD WINAPI StaticDialogThread( void* pvRef );
     
    INT_PTR CALLBACK DlgProcMain( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    INT_PTR CALLBACK DlgProcPlayers( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    INT_PTR CALLBACK DlgProcMessages( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

    // Dialog UI methods
    VOID    PaintDialog( HWND hDlg );
    
    HRESULT DisplayPlayer( DPNID idPlayer, HWND hDlg );
    HRESULT DisplayGroup( DPNID idGroup, HWND hDlg );
    
    HRESULT PrintPlayerInfo( HWND hWndEdit, CSIPlayer* pPlayer );
    HRESULT PrintGroupInfo( HWND hWndEdit, CSIGroup* pGroup );

    // Thread synchronization
    VOID    Lock() { EnterCriticalSection( &m_csLock ); }
    VOID    Unlock() { LeaveCriticalSection( &m_csLock ); }

    // Helper functions
    static  HRESULT SelectListboxItem( HWND hListBox, DWORD dwData, const TCHAR* strItem );
    static  VOID    SafeDestroyThread( LPHANDLE phThread );  

    // Private member variables
    CArrayList*        m_pPlayers;    // List of players
    CArrayList*        m_pGroups;     // List of groups

    CMessageList m_DPlayMessages;     // List of received DirectPlay message strings
    CMessageList m_AppMessages;       // List of received Application messages
 
    IDirectPlay8Peer*   m_pPeer;      // Interface for peers
    IDirectPlay8Client* m_pClient;    // Interface for clients
    IDirectPlay8Server* m_pServer;    // Interface for servers

    enum TYPE { INVALID, PEER, CLIENT, SERVER } m_eType;       // Specifies the connection type

    DPNID  m_dpnidLocal;              // DPNID for local player
    DPNID  m_dpnidHost;               // DPNID for host player
    
    HWND   m_hDlg;                    // Dialog window handle
    BOOL   m_bDlgValid;               // Invalid flag
    HANDLE m_hDlgThread;              // Thread handle
    
    HWND   m_hDlgParent;              // Parent window handle
    HWND   m_hDlgPlayers;             // Players window handle
    HWND   m_hDlgMessages;            // Messages window handle

    HFONT  m_hNameFont;               // Player/group name font
    HFONT  m_hConnectionFont;         // Description box font
    HICON  m_hPlayerIcon;             // Player icon
    HICON  m_hGroupIcon;              // Group icon

    CRITICAL_SECTION m_csLock;        // Access lock for multithreading
};


#endif // _SESSIONINFO_H_