//-----------------------------------------------------------------------------
// File: SessionInfo.cpp
//
// Desc: Implemenation for the CSessionInfo utility class. This utility stores
//       player, group, and message information gathered from the application's
//       DirectPlay message handler, and provides a dialog UI to display the
//       data.
//
//       In order to use this class, simply create an instance using a
//       pointer to the DirectPlay Peer, Client, or Server interface your 
//       application uses. Add a call to the MessageHandler member function at 
//       the beginning of your application's message handler, and call 
//       ShowDialog to launch the UI. 
//
//       This class supports multiple concurrent modeless dialogs to help with
//       debugging an application during runtime.  
//
// Copyright (C) 2000-2001 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------
#include "sessioninfo.h"


// Global variables
CSessionInfo* g_pSI = NULL;    // Global instance pointer
 
// Custom fonts
#ifdef UNDER_CE
LOGFONT g_lfName =       { 20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS, TEXT("MS Sans Serif") };
LOGFONT g_lfConnection = { 14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, TEXT("Courier New") };
#else
LOGFONT g_lfName =       { 24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS, TEXT("MS Sans Serif") };
LOGFONT g_lfConnection = { 14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, TEXT("Courier New") };
#endif



//-----------------------------------------------------------------------------
// Name: CSIGroup()
// Desc: Constructor
//-----------------------------------------------------------------------------
CSIGroup::CSIGroup( DPNID dpnid )
{
    id = dpnid;
    lstrcpy( strName, TEXT("") );
    pMembers = new CArrayList( AL_VALUE, sizeof(DPNID) );
}




//-----------------------------------------------------------------------------
// Name: ~CSIGroup()
// Desc: Destructor
//-----------------------------------------------------------------------------
CSIGroup::~CSIGroup()
{
    SAFE_DELETE( pMembers );
}




//-----------------------------------------------------------------------------
// Name: CSIPlayer()
// Desc: Constructor
//-----------------------------------------------------------------------------
CSIPlayer::CSIPlayer( DPNID dpnid ) 
{
    id = dpnid;
    bIsHost = FALSE;
    lstrcpy( strName, TEXT("") );
    lstrcpy( strURL, TEXT("") );
}



//-----------------------------------------------------------------------------
// Name: AddMember()
// Desc: Adds the given player ID to the list of member players in this group
//-----------------------------------------------------------------------------
HRESULT CSIGroup::AddMember( DPNID id )
{
    // Check to see if this Member is already present
    if( IsMember( id ) )
        return S_OK;

    // Add the new player id
    return pMembers->Add( &id );
}




//-----------------------------------------------------------------------------
// Name: RemoveMember
// Desc: Removes the given player ID from the list of member players in this
//       group
//-----------------------------------------------------------------------------
HRESULT CSIGroup::RemoveMember( DPNID id )
{
    // Find the Member
    for( UINT i=0; i < pMembers->Count(); i++ )
    {
        DPNID* pID = (DPNID*) pMembers->GetPtr( i );
    
        // Member found
        if( id == *pID )
        {
            // Remove the id and return
            pMembers->Remove( i );
            return S_OK;
        }
    }

    // Not found
    return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: CSessionInfo()
// Desc: Constructor
//-----------------------------------------------------------------------------
CSessionInfo::CSessionInfo( IDirectPlay8Peer* pPeer ) 
{
    Initialize();
  
    m_eType         = PEER;
    m_pPeer         = pPeer;
    
    m_pPeer->AddRef();
}




//-----------------------------------------------------------------------------
// Name: CSessionInfo()
// Desc: Constructor
//-----------------------------------------------------------------------------
CSessionInfo::CSessionInfo( IDirectPlay8Client* pClient ) 
{
    Initialize();

    m_eType        = CLIENT;
    m_pClient      = pClient;
    
    m_pClient->AddRef();
}




//-----------------------------------------------------------------------------
// Name: CSessionInfo()
// Desc: Constructor
//-----------------------------------------------------------------------------
CSessionInfo::CSessionInfo( IDirectPlay8Server* pServer ) 
{
    Initialize();

    m_eType         = SERVER;
    m_pServer       = pServer;
    
    m_pServer->AddRef();
}




//-----------------------------------------------------------------------------
// Name: Initialize()
// Desc: Performs common initialization for all connection types
//-----------------------------------------------------------------------------
VOID CSessionInfo::Initialize()
{
    g_pSI   = this;
    m_pPlayers       = new CArrayList( AL_REFERENCE );
    m_pGroups        = new CArrayList( AL_REFERENCE );

    m_eType         = INVALID;
    m_pPeer         = NULL;
    m_pClient       = NULL;
    m_pServer       = NULL;

    m_dpnidLocal    = 0;
    
    m_hDlg          = NULL;
    m_hDlgParent    = NULL;
    m_hDlgPlayers   = NULL;
    m_hDlgMessages  = NULL;
    m_hDlgThread    = NULL;

    // Load custom fonts and resources
#ifndef UNDER_CE 
    HMODULE hShellLib = LoadLibraryEx( TEXT("Shell32.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE );
    if( hShellLib )
    {
        m_hPlayerIcon = (HICON) LoadImage( hShellLib, MAKEINTRESOURCE(18),  
                                           IMAGE_ICON, 24, 24, LR_LOADTRANSPARENT);
        m_hGroupIcon =  (HICON) LoadImage( hShellLib, MAKEINTRESOURCE(273), 
                                           IMAGE_ICON, 24, 24, LR_LOADTRANSPARENT);
        FreeLibrary( hShellLib );
    }
#endif // !UNDER_CE

    // Create display fonts
    m_hNameFont = CreateFontIndirect( &g_lfName );
    m_hConnectionFont = CreateFontIndirect( &g_lfConnection );

    InitializeCriticalSection( &m_csLock );
}




//-----------------------------------------------------------------------------
// Name: ~CSessionInfo()
// Desc: Destructor
//-----------------------------------------------------------------------------
CSessionInfo::~CSessionInfo()
{
    // Cleanup dialog
    if( m_hDlg )
        SendMessage( m_hDlg, WM_CLOSE, 0, 0 );
    
    SafeDestroyThread( &m_hDlgThread );

    // Cleanup interfaces
    SAFE_RELEASE( m_pPeer );
    SAFE_RELEASE( m_pClient );
    SAFE_RELEASE( m_pServer );

    for( UINT i=0; i < m_pPlayers->Count(); i++ )
    {
        CSIPlayer* pPlayer = (CSIPlayer*) m_pPlayers->GetPtr( i );
        SAFE_DELETE( pPlayer );
    }

    for( i=0; i < m_pGroups->Count(); i++ )
    {
        CSIGroup* pGroup = (CSIGroup*) m_pGroups->GetPtr( i );
        SAFE_DELETE( pGroup );
    }

    SAFE_DELETE( m_pPlayers );
    SAFE_DELETE( m_pGroups );

    DeleteObject( m_hNameFont );
    DeleteObject( m_hConnectionFont );

    DeleteCriticalSection( &m_csLock );
}




//-----------------------------------------------------------------------------
// Name: InitializeLocalPlayer()
// Desc: Initialize the local player given the guessed local dpnid. 
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::InitializeLocalPlayer( DPNID idLocal )
{
    HRESULT hr = S_OK;
    DPN_PLAYER_INFO* pInfo = NULL;

#ifdef _DEBUG
    if( NULL == idLocal )
        return E_INVALIDARG;
#endif // _DEBUG

    hr = GetDpPlayerInfo( idLocal, &pInfo );

    switch( m_eType )
    {
        // Peer types can query information for any peer, including
        // themselves, so simply check the returned playerinfo structure
        // for the local flag
        case PEER:
            if( SUCCEEDED(hr) )
            {
                if( pInfo->dwPlayerFlags & DPNPLAYER_LOCAL )
                    m_dpnidLocal = idLocal;
                if( pInfo->dwPlayerFlags & DPNPLAYER_HOST )
                    m_dpnidHost = idLocal;
            }
            break;

        // Server types can't query for local information. In a typical game,
        // the player context would be checked to determine whether the created
        // player is the server; to avoid changing the player contexts for
        // every application this class is used with, we'll simply take a best
        // guess and assume that if the new player isn't a client it must be
        // the server.
        case SERVER:
            if( hr == DPNERR_INVALIDPLAYER )
            {
                m_dpnidLocal = m_dpnidHost = idLocal;
                hr = S_OK;
            }
            break;

        // Client types can simply wait for the server to send their local
        // information.
        case CLIENT:
            break;
    }

    return hr;
}




//-----------------------------------------------------------------------------
// Name: FindPlayer
// Desc: Search for and return the player based on dpnid
//-----------------------------------------------------------------------------
CSIPlayer* CSessionInfo::FindPlayer( DPNID id )
{
    // Find the player
    for( UINT i=0; i < m_pPlayers->Count(); i++ )
    {
       CSIPlayer* pPlayer = (CSIPlayer*)m_pPlayers->GetPtr( i );
       
        // Player found
        if( id == pPlayer->id )
            return pPlayer;
    }

    // Not found
    return NULL;
}




//-----------------------------------------------------------------------------
// Name: FindGroup
// Desc: Search for and return the group based on dpnid
//-----------------------------------------------------------------------------
CSIGroup* CSessionInfo::FindGroup( DPNID id )
{
    // Find the group
    for( UINT i=0; i < m_pGroups->Count(); i++ )
    {
        CSIGroup* pGroup = (CSIGroup*) m_pGroups->GetPtr( i );

        // Group found
        if( id == pGroup->id )
            return pGroup;
    }

    // Not found
    return NULL;
}




//-----------------------------------------------------------------------------
// Name: CreatePlayer()
// Desc: Creates a new player with the given ID and name, and adds the new node
//       to the player list
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::CreatePlayer( DPNID id )
{
    HRESULT hr;

    // Create a new player object
    CSIPlayer* pNewPlayer = new CSIPlayer( id );
    if( NULL == pNewPlayer )
        return E_OUTOFMEMORY;

    // Add the new object to the list
    hr = m_pPlayers->Add( pNewPlayer );
    if( FAILED(hr) )
    {
        // Release the allocated memory and return the error code
        SAFE_DELETE( pNewPlayer );
        return hr;
    }

   return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DestroyPlayer()
// Desc: Removes the player with the given ID from the player list
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::DestroyPlayer( DPNID id )
{
    // Find the player
    for( UINT i=0; i < m_pPlayers->Count(); i++ )
    {
        CSIPlayer* pPlayer = (CSIPlayer*)m_pPlayers->GetPtr( i );

        // Player found
        if( id == pPlayer->id )
        {
            // Zero out the spot and return
            m_pPlayers->Remove( i );
            SAFE_DELETE( pPlayer );
            return S_OK;
        }
    }

    // Not found
    return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: CreateGroup()
// Desc: Creates a group with the given ID and name, and adds the new node to
//       the group list
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::CreateGroup( DPNID id )
{
    HRESULT hr;

    // Create a new group object
    CSIGroup* pNewGroup = new CSIGroup( id );
    if( NULL == pNewGroup )
        return E_OUTOFMEMORY;

    // Add the new object to the list
    hr = m_pGroups->Add( pNewGroup );
    if( FAILED(hr) )
    {
        // Release the allocated memory and return the error code
        SAFE_DELETE( pNewGroup);
        return hr;
    }

   return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DestroyGroup()
// Desc: Removes the group with the given ID from the group list
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::DestroyGroup( DPNID id )
{
    // Find the group
    for( UINT i=0; i < m_pGroups->Count(); i++ )
    {
        CSIGroup* pGroup = (CSIGroup*) m_pGroups->GetPtr( i );

        // Group found
        if( id == pGroup->id )
        {
            // Zero out the spot and return
            m_pGroups->Remove( i );
            SAFE_DELETE( pGroup );
            return S_OK;
        }
    }

    // Not found
    return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: AddPlayerToGroup()
// Desc: Adds the given player ID to the list of member players in the group
//       with the given group ID 
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::AddPlayerToGroup( DPNID idPlayer, DPNID idGroup )
{
    // Find the group
    for( UINT i=0; i < m_pGroups->Count(); i++ )
    {
        CSIGroup* pGroup = (CSIGroup*) m_pGroups->GetPtr( i );

        // Group found
        if( idGroup == pGroup->id )
        {
            // Add the dpnid and return
            return pGroup->AddMember( idPlayer );
        }
    }

    // Not found
    return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: RemovePlayerFromGroup()
// Desc: Removes the given player ID from the list of member players in the 
//       group with the given group ID
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::RemovePlayerFromGroup( DPNID idPlayer, DPNID idGroup )
{
    // Find the group
    for( UINT i=0; i < m_pGroups->Count(); i++ )
    {
        CSIGroup* pGroup = (CSIGroup*) m_pGroups->GetPtr( i );

        // Group found
        if( idGroup == pGroup->id )
        {
            // Remove the dpnid and return
            return pGroup->RemoveMember( idPlayer );
        }
    }

    // Not found
    return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: RefreshPlayerInfo()
// Desc: Use DirectPlay to refresh all info for the player with the given ID
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::RefreshPlayerInfo( DPNID id )
{
    HRESULT                 hr              = S_OK;
    LPWSTR                  strURL          = NULL;
    DWORD                   dwNumChars      = 0;
    DPN_PLAYER_INFO*        pDpPlayerInfo   = NULL;
    IDirectPlay8Address*    rpAddress[16]   = {0};
    DWORD                   dwNumAddresses  = 16;

#ifdef _DEBUG
    // Parameter validation
    if( NULL == id )
        return E_INVALIDARG;
#endif // _DEBUG

    // Attempt to get the name and flags
    hr = GetDpPlayerInfo( id, &pDpPlayerInfo );
    if( FAILED(hr) )
        goto LCleanReturn;


    // If receiving information about the local player, determine
    // whether this app is the session host
    if( id == m_dpnidLocal )
    {
        // Attempt to get the local address
        switch( m_eType )
        {
            case PEER:
                hr = m_pPeer->GetLocalHostAddresses( rpAddress, &dwNumAddresses, 0 );
                break;

            case SERVER:
                hr = m_pServer->GetLocalHostAddresses( rpAddress, &dwNumAddresses, 0 );
                break;

            default:
                hr = E_FAIL;
                break;
        }
    }
    else
    {
        // Attempt to get the remote address
        dwNumAddresses = 1;

        switch( m_eType )
        {
            case PEER:
                hr = m_pPeer->GetPeerAddress( id, rpAddress, 0 );
                break;

            case SERVER:
                hr = m_pServer->GetClientAddress( id, rpAddress, 0 );
                break;

            case CLIENT:
                if( id == m_dpnidHost )
                    hr = m_pClient->GetServerAddress( rpAddress, 0 );
                else
                    hr = DPNERR_INVALIDPLAYER;
                break;

            default:
                hr = E_FAIL;
                break;
        }
    }
    
    // If the address was retrieved, extract the URL
    if( SUCCEEDED(hr) )
    {
        // Get needed allocation size for the URL string
        hr = rpAddress[0]->GetURLW( NULL, &dwNumChars );
        if( FAILED(hr) && hr != DPNERR_BUFFERTOOSMALL )
            goto LCleanReturn;

        // Allocate the URL string
        strURL = new WCHAR[ dwNumChars ];
        if( NULL == strURL )
        {
            hr = E_OUTOFMEMORY;
            goto LCleanReturn;
        }

        // Get the URL
        hr = rpAddress[0]->GetURLW( strURL, &dwNumChars );
        if( FAILED(hr) )
            goto LCleanReturn;
    }

    // Locate the stored player data
    Lock();

    CSIPlayer* pPlayer;
    pPlayer = FindPlayer( id );  
    if( pPlayer )
    {
        // Set the player data
        pPlayer->bIsHost = ( pDpPlayerInfo->dwPlayerFlags & DPNPLAYER_HOST );

        if( strURL )
        {
            DXUtil_ConvertWideStringToGenericCch( pPlayer->strURL, strURL, 256 );
        }

        if( pDpPlayerInfo->pwszName)
        {
            DXUtil_ConvertWideStringToGenericCch( pPlayer->strName, 
                                                  pDpPlayerInfo->pwszName, 256 ); 
        }
    }

    Unlock();


LCleanReturn:
    // Release resources
    SAFE_DELETE_ARRAY( strURL );
 
    for( UINT i=0; i < dwNumAddresses; i++ )
        SAFE_RELEASE( rpAddress[i] );

    SAFE_DELETE_ARRAY( pDpPlayerInfo );

   
    return hr;
}




//-----------------------------------------------------------------------------
// Name: RefreshGroupInfo()
// Desc: Use DirectPlay to refresh all info for the group with the given ID
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::RefreshGroupInfo( DPNID id )
{
    HRESULT hr       = S_OK;
    DPN_GROUP_INFO* pDpGroupInfo    = NULL;
    
    // Attempt to get the name
    hr = GetDpGroupInfo( id, &pDpGroupInfo );
    if( FAILED(hr) )
        goto LCleanReturn;

    // Locate the stored player data
    Lock();

    CSIGroup* pGroup;
    pGroup = FindGroup( id );

    if( pGroup )
    {
        // Set the group data
        DXUtil_ConvertWideStringToGenericCch( pGroup->strName, pDpGroupInfo->pwszName, 256 ); 
    }

    Unlock();


LCleanReturn:
    // Release resources
    SAFE_DELETE_ARRAY( pDpGroupInfo );

    return hr;
}





//-----------------------------------------------------------------------------
// Name: MessageHandler
// Desc: Sift the information headed for the application's DirectPlay message
//       handler, remove any messages used exclusively by this utility class,
//       and store any useful information before the message is passed off to
//       the application.
//-----------------------------------------------------------------------------
BOOL CSessionInfo::MessageHandler( DWORD dwMessageId, PVOID pMsgBuffer )
{
    HRESULT hr = S_OK;
    TCHAR strMessage[ 256 ]; 

    switch( dwMessageId )
    {
        case DPN_MSGID_ADD_PLAYER_TO_GROUP:
        {
            DPNMSG_ADD_PLAYER_TO_GROUP* pMsg = (DPNMSG_ADD_PLAYER_TO_GROUP*) pMsgBuffer;
        
            Lock();
            hr = AddPlayerToGroup( pMsg->dpnidPlayer, pMsg->dpnidGroup );
            Unlock();
            
            // Invalidate the dialog
            m_bDlgValid = FALSE;
    
            // Log the message
            _sntprintf( strMessage, 
                        200, 
                        TEXT("Add Player To Group: player 0x%x, group 0x%x"), 
                        pMsg->dpnidPlayer, 
                        pMsg->dpnidGroup ); 
            
            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_APPLICATION_DESC:
        {
            // Set the window title
            DPN_APPLICATION_DESC* pAppDesc;
            if( SUCCEEDED( GetDpAppDesc( &pAppDesc ) ) )
            {
                TCHAR strTitle[ 256 ];
                
                DXUtil_ConvertWideStringToGenericCch( strTitle, 
                                                      pAppDesc->pwszSessionName, 
                                                      256-50 );

                lstrcat( strTitle, TEXT(" - Session Info") );
                SendMessage( m_hDlg, WM_SETTEXT, 0, (LPARAM) strTitle ); 
            }

            SAFE_DELETE_ARRAY( pAppDesc );

            // Log the message
            _sntprintf( strMessage, 200, TEXT("Application Desc") );
            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_ASYNC_OP_COMPLETE:
        {
            DPNMSG_ASYNC_OP_COMPLETE* pMsg = (DPNMSG_ASYNC_OP_COMPLETE*) pMsgBuffer;

            // The messages sent by this helper class always use the same context.
            // If the completed operation was initiated by this class, we can
            // safely hide the message from the application.
            if( SI_ASYNC_CONTEXT == pMsg->pvUserContext )
                return TRUE;

            // Log the message
            _sntprintf( strMessage, 
                        200, 
                        TEXT("Async Op Complete: handle 0x%x, result 0x%x"), 
                        pMsg->hAsyncOp, 
                        pMsg->hResultCode );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_CLIENT_INFO:
        {
            DPNMSG_CLIENT_INFO* pMsg = (DPNMSG_CLIENT_INFO*) pMsgBuffer;
            OnDpInfoChange( pMsg->dpnidClient );

            // Log the message
            _sntprintf( strMessage, 
                        200, 
                        TEXT("Client Info: client 0x%x"), 
                        pMsg->dpnidClient );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_CONNECT_COMPLETE:
        {
            DPNMSG_CONNECT_COMPLETE* pMsg = (DPNMSG_CONNECT_COMPLETE*) pMsgBuffer;
        
            // Log the message
            _sntprintf( strMessage, 
                        200, 
                        TEXT("Connect Complete: handle 0x%x, result 0x%x"), 
                        pMsg->hAsyncOp, 
                        pMsg->hResultCode );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_CREATE_GROUP:
        {
            DPNMSG_CREATE_GROUP* pMsg = (DPNMSG_CREATE_GROUP*) pMsgBuffer;
        
            Lock();
            hr = CreateGroup( pMsg->dpnidGroup );
            Unlock();
            
            RefreshGroupInfo( pMsg->dpnidGroup );
            
            if( m_eType == SERVER )
                SendGroupInfoToAll( pMsg->dpnidGroup );

            // Invalidate the dialog
            m_bDlgValid = FALSE;

            // Log the message
            _sntprintf( strMessage, 
                        200, 
                        TEXT("Create Group: group 0x%x, owner 0x%x"), 
                        pMsg->dpnidGroup, 
                        pMsg->dpnidOwner );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_CREATE_PLAYER:
        {
            DPNMSG_CREATE_PLAYER* pMsg = (DPNMSG_CREATE_PLAYER*) pMsgBuffer;
        
            Lock();
            hr = CreatePlayer( pMsg->dpnidPlayer );
            Unlock();      
           
            // If we don't know our local dpnid yet, try to initialize
            if( m_dpnidLocal == NULL )
                InitializeLocalPlayer( pMsg->dpnidPlayer );

            // Update information about the new player
            RefreshPlayerInfo( pMsg->dpnidPlayer );
            
            if( m_eType == SERVER )
            {
                // Clients receive all their information from the server,
                // so bundle up the new player's name and flags, and broadcast
                // to the session.
                SendPlayerInfoToAll( pMsg->dpnidPlayer );
                SynchronizeWithPlayer( pMsg->dpnidPlayer );
            }   

            // Invalidate the dialog
            m_bDlgValid = FALSE;

            // Log the message
            _sntprintf( strMessage, 
                        200, 
                        TEXT("Create Player: player 0x%x"), 
                        pMsg->dpnidPlayer );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_DESTROY_GROUP:
        {
            DPNMSG_DESTROY_GROUP* pMsg = (DPNMSG_DESTROY_GROUP*) pMsgBuffer;
        
            Lock();
            hr = DestroyGroup( pMsg->dpnidGroup );
            Unlock();
             
            // Invalidate the dialog
            m_bDlgValid = FALSE;

            // Log the message
            _sntprintf( strMessage, 
                        200, 
                        TEXT("Destroy Group: group 0x%x"), 
                        pMsg->dpnidGroup );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_DESTROY_PLAYER:
        {
            DPNMSG_DESTROY_PLAYER* pMsg = (DPNMSG_DESTROY_PLAYER*) pMsgBuffer;
        
            Lock();
            hr = DestroyPlayer( pMsg->dpnidPlayer );
            Unlock();   
            
            // Invalidate the dialog
            m_bDlgValid = FALSE;

            // Log the message
            _sntprintf( strMessage, 
                        200, 
                        TEXT("Destroy Player: player 0x%x"), 
                        pMsg->dpnidPlayer );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_ENUM_HOSTS_QUERY:
        {
            // Log the message
            _sntprintf( strMessage, 
                        200, 
                        TEXT("Enum Hosts Query") );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_ENUM_HOSTS_RESPONSE:
        {
            DPNMSG_ENUM_HOSTS_RESPONSE* pMsg = (DPNMSG_ENUM_HOSTS_RESPONSE*) pMsgBuffer;
        
            // Log the message
            _sntprintf( strMessage, 
                        200, 
                        TEXT("Enum Hosts Response: latency %d ms, session \"%s\""), 
                        pMsg->dwRoundTripLatencyMS, 
                        pMsg->pApplicationDescription->pwszSessionName );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_GROUP_INFO:
        {
            DPNMSG_GROUP_INFO* pMsg = (DPNMSG_GROUP_INFO*) pMsgBuffer;
            
            RefreshGroupInfo( pMsg->dpnidGroup );
             
            if( m_eType == SERVER )
                SendGroupInfoToAll( pMsg->dpnidGroup );

            // Invalidate the dialog
            m_bDlgValid = FALSE;
            
            // Log the message
            _sntprintf( strMessage, 
                        200, 
                        TEXT("Group Info: group 0x%x"), 
                        pMsg->dpnidGroup );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_HOST_MIGRATE:
        {
            DPNMSG_HOST_MIGRATE* pMsg = (DPNMSG_HOST_MIGRATE*) pMsgBuffer;

            m_dpnidHost = pMsg->dpnidNewHost;
            RefreshPlayerInfo( pMsg->dpnidNewHost );

            // Invalidate the dialog
            m_bDlgValid = FALSE;

            // Log the message
            _sntprintf( strMessage, 
                        200, 
                        TEXT("Host Migrate: new host 0x%x"), 
                        pMsg->dpnidNewHost );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_INDICATE_CONNECT:
        {
            DPNMSG_INDICATE_CONNECT* pMsg = (DPNMSG_INDICATE_CONNECT*) pMsgBuffer;
        
            // Log the message
            _sntprintf( strMessage, 
                        200, 
                    #ifdef _WIN64
                        TEXT("Indicate Connect: player context 0x%I64x"), 
                    #else
                        TEXT("Indicate Connect: player context 0x%x"), 
                    #endif // _WIN64
                        pMsg->pvPlayerContext );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_INDICATED_CONNECT_ABORTED:
        {
            DPNMSG_INDICATED_CONNECT_ABORTED* pMsg = (DPNMSG_INDICATED_CONNECT_ABORTED*) pMsgBuffer;
        
            // Log the message
            _sntprintf( strMessage, 
                        200, 
                    #ifdef _WIN64
                        TEXT("Indicated Connect Aborted: player context 0x%I64x"), 
                    #else
                        TEXT("Indicated Connect Aborted: player context 0x%x"), 
                    #endif // _WIN64
                        pMsg->pvPlayerContext );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_PEER_INFO:
        {
            DPNMSG_PEER_INFO* pMsg = (DPNMSG_PEER_INFO*) pMsgBuffer;
            OnDpInfoChange( pMsg->dpnidPeer );

            // Log the message
            _sntprintf( strMessage, 
                        200, 
                        TEXT("Peer Info: peer 0x%x"), 
                        pMsg->dpnidPeer );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_RECEIVE:
        {
            DPNMSG_RECEIVE* pMsg = (DPNMSG_RECEIVE*) pMsgBuffer;
            
            if( pMsg->dwSize < sizeof(SI_MSG) )
                break;
            
            SI_MSG* pSIMsg = (SI_MSG*) pMsg->pReceiveData;

            switch( pSIMsg->dwMsgID )
            {
                case SI_MSGID_PLAYERINFO:
                {
                    if( pMsg->dwReceiveDataSize < sizeof(SI_MSG_PLAYERINFO) )
                        break;

                    SI_MSG_PLAYERINFO* pPlayerInfo = (SI_MSG_PLAYERINFO*) pMsg->pReceiveData;

                    // Verify the message is properly sized
                    if( pMsg->dwReceiveDataSize != sizeof(SI_MSG_PLAYERINFO) +
                        ( sizeof(WCHAR) * ( pPlayerInfo->dwNameLength + 1 ) ) )
                        break;
                    
                    // Pass the data off to the message handler function
                    OnPlayerInfoReceive( pPlayerInfo );

                    // Attempt to get additional information about the player
                    RefreshPlayerInfo( pPlayerInfo->dpnID );
                    
                    // Invalidate the dialog
                    m_bDlgValid = FALSE;

                    return TRUE;
                }

                case SI_MSGID_GROUPINFO:
                {
                    if( pMsg->dwReceiveDataSize < sizeof(SI_MSG_GROUPINFO) )
                        break;

                    SI_MSG_GROUPINFO* pGroupInfo = (SI_MSG_GROUPINFO*) pMsg->pReceiveData;

                    if( pMsg->dwReceiveDataSize != sizeof(SI_MSG_GROUPINFO) +
                        ( sizeof(WCHAR) * ( pGroupInfo->dwNameLength + 1 ) ) )
                    {
                        break;
                    }

                    // Pass the data off to the message handler function
                    OnGroupInfoReceive( pGroupInfo );

                    // Invalidate the dialog
                    m_bDlgValid = FALSE;

                    return TRUE;
                }
            }

            // Log the message
            _sntprintf( strMessage, 
                        200, 
                        TEXT("Receive: sender 0x%x, data size %d bytes"), 
                        pMsg->dpnidSender, 
                        pMsg->dwReceiveDataSize );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_REMOVE_PLAYER_FROM_GROUP:
        {
            DPNMSG_REMOVE_PLAYER_FROM_GROUP* pMsg = (DPNMSG_REMOVE_PLAYER_FROM_GROUP*) pMsgBuffer;
        
            Lock();
            hr = RemovePlayerFromGroup( pMsg->dpnidPlayer, pMsg->dpnidGroup );
            Unlock();
            
            // Invalidate the dialog
            m_bDlgValid = FALSE;

            // Log the message
            _sntprintf( strMessage, 
                        200, 
                        TEXT("Remove Player From Group: player 0x%x, group 0x%x"), 
                        pMsg->dpnidPlayer, 
                        pMsg->dpnidGroup );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_RETURN_BUFFER:
        {
            DPNMSG_RETURN_BUFFER* pMsg = (DPNMSG_RETURN_BUFFER*) pMsgBuffer;
            
            // Log the message
            _sntprintf( strMessage, 
                        200, 
                    #ifdef _WIN64
                        TEXT("Return Buffer: user context 0x%I64x, result 0x%x"), 
                    #else
                        TEXT("Return Buffer: user context 0x%x, result 0x%x"), 
                    #endif // _WIN64
                        pMsg->pvUserContext, 
                        pMsg->hResultCode );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_SEND_COMPLETE:
        {
            DPNMSG_SEND_COMPLETE* pMsg = (DPNMSG_SEND_COMPLETE*) pMsgBuffer;
            
            // Log the message
            _sntprintf( strMessage, 
                        200, 
                        TEXT("Send Complete: handle 0x%x, result 0x%x, send time %d ms"), 
                        pMsg->hAsyncOp, 
                        pMsg->hResultCode, 
                        pMsg->dwSendTime );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }
        
        case DPN_MSGID_SERVER_INFO:
        {
            DPNMSG_SERVER_INFO* pMsg = (DPNMSG_SERVER_INFO*) pMsgBuffer;
            OnDpInfoChange( pMsg->dpnidServer );

            // Log the message
            _sntprintf( strMessage, 
                        200, 
                        TEXT("Server Info: server 0x%x"), 
                        pMsg->dpnidServer );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        case DPN_MSGID_TERMINATE_SESSION:
        {
            DPNMSG_TERMINATE_SESSION* pMsg = (DPNMSG_TERMINATE_SESSION*) pMsgBuffer;
            
            // Log the message
            _sntprintf( strMessage, 
                        200, 
                        TEXT("Terminate Session: result 0x%x"), 
                        pMsg->hResultCode );

            strMessage[ 200 ] = TEXT('\0');
            break;
        }

        
    }

    CMessageList* pMsgList = NULL;
    int nDlgID = 0;

    // Add the message string to the stored list
    switch( dwMessageId )
    {
        case DPN_MSGID_RECEIVE:
        case DPN_MSGID_SEND_COMPLETE:
        {
            pMsgList = &m_AppMessages;
            nDlgID = IDC_SI_APPMSG;
            break; 
        }

        default:
        {
            pMsgList = &m_DPlayMessages;
            nDlgID = IDC_SI_DPLAYMSG;
            break;   
        }
    }
    
    // Lock the message list
    pMsgList->Lock();

    // If the message queue is already full, remove the bottom item from the dialog box 
    if( pMsgList->IsFull() )
        SendMessage( GetDlgItem( m_hDlgMessages, nDlgID ), LB_DELETESTRING, SI_MAX_MESSAGES-1, 0 );

    // Add the message to the stored list
    TCHAR* strTimeStamped = pMsgList->AddMessage( strMessage );

    // Unlock the message list
    pMsgList->Unlock();

    // Post the new string to the top of the list box
    if( m_hDlgMessages )
        SendMessage( GetDlgItem( m_hDlgMessages, nDlgID ), LB_INSERTSTRING, 0, (LPARAM) strTimeStamped );

    // Return false to indicate that either the message was not handled, or the
    // handled message should also be sent to the application message handler.
    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: OnPlayerInfoReceive()
// Desc: Handles the extraction and storage of incoming player data
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::OnPlayerInfoReceive( SI_MSG_PLAYERINFO* pPlayerInfo )
{
    HRESULT hr = S_OK;

    // Extract the player name
    LPWSTR pStrName = (LPWSTR) (pPlayerInfo + 1);
    pStrName[ pPlayerInfo->dwNameLength ] = 0;

    Lock();
      
    // Search for the player with the given ID
    CSIPlayer* pPlayer = FindPlayer( pPlayerInfo->dpnID ); 

    // If not found, create a new player
    if( NULL == pPlayer )
    {
        hr = CreatePlayer( pPlayerInfo->dpnID );
        if( FAILED(hr) )
            goto LCleanReturn;
            
        pPlayer = FindPlayer( pPlayerInfo->dpnID );
    }

    // Set updated information
    pPlayer->bIsHost = pPlayerInfo->dwFlags & DPNPLAYER_HOST;
    
    DXUtil_ConvertWideStringToGenericCch( pPlayer->strName, pStrName, 256 ); 
    
    if( pPlayerInfo->dwFlags & DPNPLAYER_LOCAL )
        m_dpnidLocal = pPlayerInfo->dpnID;
    if( pPlayerInfo->dwFlags & DPNPLAYER_HOST )
        m_dpnidHost = pPlayerInfo->dpnID;


    hr = S_OK;

LCleanReturn:
    Unlock();
    return hr;
}




//-----------------------------------------------------------------------------
// Name: OnGroupInfoReceive()
// Desc: Handles the extraction and storage of incoming group data
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::OnGroupInfoReceive( SI_MSG_GROUPINFO* pGroupInfo )
{
    HRESULT hr = S_OK;

    // Extract the name
    LPWSTR pStr = (LPWSTR) (pGroupInfo+1);
    pStr[ pGroupInfo->dwNameLength ] = 0;

    // Set the data
    Lock();
    
    // Search for the group with the given ID
    CSIGroup* pGroup = FindGroup( pGroupInfo->dpnID );
    
    // If not found, create a new group
    if( NULL == pGroup )
    {
        hr = CreateGroup( pGroupInfo->dpnID );
        if( FAILED(hr) )
            goto LCleanReturn;

        pGroup = FindGroup( pGroupInfo->dpnID );
    }

    // Set updated information
    DXUtil_ConvertWideStringToGenericCch( pGroup->strName, pStr, 256 ); 
    
    hr = S_OK;

LCleanReturn:
    Unlock();
    return hr;
}




//-----------------------------------------------------------------------------
// Name: OnDpInfoChange()
// Desc: Handles PEER_INFO, CLIENT_INFO, and SERVER_INFO messages
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::OnDpInfoChange( DPNID dpnid )
{
    RefreshPlayerInfo( dpnid );
    
    if( m_eType == SERVER )
        SendPlayerInfoToAll( dpnid );

    // Invalidate the dialog
    m_bDlgValid = FALSE;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: GetDpPlayerInfo()
// Desc: Get the DirectPlay player info using the stored connection interface
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::GetDpPlayerInfo( DPNID dpnid, DPN_PLAYER_INFO** ppPlayerInfo )
{
    HRESULT hr;
    DWORD dwSize = 0;

#ifdef _DEBUG
    if( NULL == ppPlayerInfo || NULL == dpnid )
        return E_INVALIDARG;
#endif // _DEBUG
    
    switch( m_eType )
    {
        case PEER:
        {
            // GetPeerInfo might return DPNERR_CONNECTING when connecting, 
            // so just keep calling it if it does
            do 
            {
                hr = m_pPeer->GetPeerInfo( dpnid, *ppPlayerInfo, &dwSize, 0 ); 
            }
            while( hr == DPNERR_CONNECTING );

            break;
        }

        case SERVER:
        {
            // Special case: Server can't query for local information, so
            // this should be filled in manually
            if( m_dpnidLocal == dpnid )
            {
                dwSize = sizeof( DPN_PLAYER_INFO );
                hr = DPNERR_BUFFERTOOSMALL;
            }
            else
            {
                hr = m_pServer->GetClientInfo( dpnid, *ppPlayerInfo, &dwSize, 0 );
            }

            break;
        }

        case CLIENT:
        {
            if( m_dpnidHost == dpnid )
                hr = m_pClient->GetServerInfo( *ppPlayerInfo, &dwSize, 0 );
            else
                hr = DPNERR_INVALIDPLAYER;
            break;
        }

        default:
            // Unknown type
            return E_FAIL;
    }
      
    // DirectPlay should return BufferTooSmall to give the correct allocation size
    if( hr != DPNERR_BUFFERTOOSMALL )
        return hr;

    // Allocate the memory
    *ppPlayerInfo = (DPN_PLAYER_INFO*) new BYTE[ dwSize ];
    if( NULL == *ppPlayerInfo )
        return E_OUTOFMEMORY;

    // Initialize the struct
    ZeroMemory( *ppPlayerInfo, dwSize );
    (*ppPlayerInfo)->dwSize = sizeof(DPN_PLAYER_INFO);
    
    // Get peer info
    switch( m_eType )
    {
        case PEER: 
            hr = m_pPeer->GetPeerInfo( dpnid, *ppPlayerInfo, &dwSize, 0 ); 
            break;

        case SERVER: 
            // Special case: Server can't query for local information, so
            // this should be filled in manually
            if( m_dpnidLocal == dpnid )
            {  
                (*ppPlayerInfo)->dwInfoFlags = DPNINFO_NAME | DPNINFO_DATA;
                (*ppPlayerInfo)->pwszName = L"(Server)";
                (*ppPlayerInfo)->dwPlayerFlags = DPNPLAYER_LOCAL | DPNPLAYER_HOST;
                hr = S_OK;
            }
            else
            {
                hr = m_pServer->GetClientInfo( dpnid, *ppPlayerInfo, &dwSize, 0 ); 
            }
            break;

        case CLIENT:
            hr = m_pClient->GetServerInfo( *ppPlayerInfo, &dwSize, 0 );
            break;

        default: 
            SAFE_DELETE_ARRAY( *ppPlayerInfo );
            return E_FAIL;
    }

    return hr;
}




//-----------------------------------------------------------------------------
// Name: GetDpGroupInfo
// Desc: Get the DirectPlay group info using the stored connection interface
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::GetDpGroupInfo( DPNID dpnid, DPN_GROUP_INFO** ppGroupInfo )
{
    HRESULT hr;
    DWORD dwSize = 0;

#ifdef _DEBUG
    if( NULL == ppGroupInfo )
        return E_INVALIDARG;
#endif // _DEBUG
    
    switch( m_eType )
    {
        case PEER:
            hr = m_pPeer->GetGroupInfo( dpnid, NULL, &dwSize, NULL );
            break;

        case SERVER:
            hr = m_pServer->GetGroupInfo( dpnid, NULL, &dwSize, NULL );
            break;

        default:
            return E_FAIL;
    }

    // DirectPlay should return BufferTooSmall to give the correct allocation size
    if( hr != DPNERR_BUFFERTOOSMALL )
        return hr;

    // Allocate the memory
    *ppGroupInfo = (DPN_GROUP_INFO*) new BYTE[ dwSize ];
    if( NULL == *ppGroupInfo )
        return E_OUTOFMEMORY;

    // Initialize the struct
    ZeroMemory( *ppGroupInfo, dwSize );
    (*ppGroupInfo)->dwSize = sizeof(DPN_GROUP_INFO);
    
    // Get group info
    switch( m_eType )
    {
        case PEER:
            hr = m_pPeer->GetGroupInfo( dpnid, *ppGroupInfo, &dwSize, 0 );
            break;

        case SERVER:
            hr = m_pServer->GetGroupInfo( dpnid, *ppGroupInfo, &dwSize, 0 );
            break;

        default:
            SAFE_DELETE_ARRAY( *ppGroupInfo );
            return E_FAIL;
    }

    return hr;
}




//-----------------------------------------------------------------------------
// Name: GetDpAppDesc
// Desc: Get the DirectPlay application description using the stored connection 
//       interface
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::GetDpAppDesc( DPN_APPLICATION_DESC** ppAppDesc )
{
    HRESULT hr;
    DWORD dwSize = 0;

#ifdef _DEBUG
    if( NULL == ppAppDesc )
        return E_INVALIDARG;
#endif // _DEBUG
    
    switch( m_eType )
    {
        case PEER:
            hr = m_pPeer->GetApplicationDesc( NULL, &dwSize, NULL );
            break;

        case SERVER:
            hr = m_pServer->GetApplicationDesc( NULL, &dwSize, NULL );
            break;

        case CLIENT:
            hr = m_pClient->GetApplicationDesc( NULL, &dwSize, NULL );
            break;

        default:
            return E_FAIL;
    }

    // DirectPlay should return BufferTooSmall to give the correct allocation size
    if( hr != DPNERR_BUFFERTOOSMALL )
        return hr;

    // Allocate the memory
    *ppAppDesc = (DPN_APPLICATION_DESC*) new BYTE[ dwSize ];
    if( NULL == *ppAppDesc )
        return E_OUTOFMEMORY;

    // Initialize the struct
    ZeroMemory( *ppAppDesc, dwSize );
    (*ppAppDesc)->dwSize = sizeof(DPN_APPLICATION_DESC);
    
    // Get group info
    switch( m_eType )
    {
           case PEER:
            hr = m_pPeer->GetApplicationDesc( *ppAppDesc, &dwSize, NULL );
            break;

        case SERVER:
            hr = m_pServer->GetApplicationDesc( *ppAppDesc, &dwSize, NULL );
            break;

        case CLIENT:
            hr = m_pClient->GetApplicationDesc( *ppAppDesc, &dwSize, NULL );
            break;

        default:
            return E_FAIL;
    }

    return hr;
}




//-----------------------------------------------------------------------------
// Name: SendPlayerInfoToPlayer()
// Desc: Send all stored player information with the given player ID to the
//       player(s) with the given target ID
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::SendPlayerInfoToPlayer( DPNID idPlayer, DPNID idTarget )
{
    HRESULT hr = S_OK;

    Lock();
    
    CSIPlayer* pPlayer = FindPlayer( idPlayer );
    if( NULL == pPlayer )
    {
        Unlock();
        return E_INVALIDARG;
    }

    // Package and send the given player info
    DWORD dwNameLen = lstrlen( pPlayer->strName );
    
    DPN_BUFFER_DESC dpBufDesc = {0};
    dpBufDesc.dwBufferSize = sizeof(SI_MSG_PLAYERINFO) + 
                           ( sizeof( WCHAR ) * (dwNameLen + 1) );
                           
    // Allocate space
    SI_MSG_PLAYERINFO* pMsg = (SI_MSG_PLAYERINFO*) new BYTE[ dpBufDesc.dwBufferSize ];
    if( NULL == pMsg )
    {
        Unlock();
        return E_OUTOFMEMORY;
    }

    // Set the data pointer
    dpBufDesc.pBufferData = (BYTE*) pMsg;

    // Store values
    pMsg->dwMsgID       = SI_MSGID_PLAYERINFO;
    pMsg->dpnID         = pPlayer->id;
    pMsg->dwFlags       = 0;
    pMsg->dwNameLength  = dwNameLen;
    
    // Set flags
    if( pPlayer->bIsHost )
        pMsg->dwFlags |= DPNPLAYER_HOST;
    if( idPlayer == idTarget )
        pMsg->dwFlags |= DPNPLAYER_LOCAL;

    // Pack the string
    LPWSTR pStr = (LPWSTR) (pMsg+1);
    DXUtil_ConvertGenericStringToWideCch( pStr, pPlayer->strName, dwNameLen+1 );
    
    // Release the lock
    Unlock();

    // Send the information
    DPNHANDLE dpAsync;

    switch( m_eType )
    {
        case PEER:
            hr = m_pPeer->SendTo( idTarget, &dpBufDesc, 1, 0, SI_ASYNC_CONTEXT, &dpAsync, DPNSEND_NOCOMPLETE | DPNSEND_NOLOOPBACK );
            break;

        case SERVER:
            hr = m_pServer->SendTo( idTarget, &dpBufDesc, 1, 0, SI_ASYNC_CONTEXT, &dpAsync, DPNSEND_NOCOMPLETE | DPNSEND_NOLOOPBACK );
            break;

        case CLIENT:
            hr = m_pClient->Send( &dpBufDesc, 1, 0, SI_ASYNC_CONTEXT, &dpAsync, DPNSEND_NOCOMPLETE | DPNSEND_NOLOOPBACK );
            break;

        default:
            hr = E_FAIL;
            break;
    }

    // Release allocated memory and return
    SAFE_DELETE_ARRAY( pMsg );
    return hr;
}




//-----------------------------------------------------------------------------
// Name: SendGroupInfoToPlayer
// Desc: Send all stored information about the given group ID to the player(s)
//       with the given target ID
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::SendGroupInfoToPlayer( DPNID idGroup, DPNID idTarget )
{
    HRESULT hr = S_OK;

    Lock();

    CSIGroup* pGroup = FindGroup( idGroup ); 
    if( NULL == pGroup )
    {
        Unlock();
        return E_INVALIDARG;
    }

    // Package and send the given player info
    DWORD dwNameLen = lstrlen( pGroup->strName );
   
    DPN_BUFFER_DESC dpBufDesc = {0};
    dpBufDesc.dwBufferSize = sizeof(SI_MSG_GROUPINFO) + 
                           ( sizeof( WCHAR ) * (dwNameLen + 1) ); 

    // Allocate space
    SI_MSG_GROUPINFO* pMsg = (SI_MSG_GROUPINFO*) new BYTE[ dpBufDesc.dwBufferSize ];
    if( NULL == pMsg )
    {
        Unlock();
        return E_OUTOFMEMORY;
    }

    // Set the data pointer
    dpBufDesc.pBufferData = (BYTE*) pMsg;

    // Store values
    pMsg->dwMsgID       = SI_MSGID_GROUPINFO;
    pMsg->dpnID         = idGroup;
    pMsg->dwNameLength  = dwNameLen;

    // Pack the strings
    LPWSTR pStr = (LPWSTR) (pMsg+1);
    DXUtil_ConvertGenericStringToWideCch( pStr, pGroup->strName, dwNameLen+1 );
    
    // Release the lock
    Unlock();

    // Send the information
    DPNHANDLE dpAsync;
            
    switch( m_eType )
    {
        case PEER:
            hr = m_pPeer->SendTo( idTarget, &dpBufDesc, 1, 0, SI_ASYNC_CONTEXT, &dpAsync, DPNSEND_NOCOMPLETE | DPNSEND_NOLOOPBACK );
            break;

        case SERVER:
            hr = m_pServer->SendTo( idTarget, &dpBufDesc, 1, 0, SI_ASYNC_CONTEXT, &dpAsync, DPNSEND_NOCOMPLETE | DPNSEND_NOLOOPBACK );
            break;

        default:
            hr = E_FAIL;
            break;
    }

    // Release allocated memory and return
    SAFE_DELETE_ARRAY( pMsg );
    return hr;
}




//-----------------------------------------------------------------------------
// Name: SynchronizeWithPlayer()
// Desc: Send all stored group and player information to the player with the 
//       given ID
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::SynchronizeWithPlayer( DPNID id )
{
    HRESULT hr = S_OK;

    Lock();

    // Send all player and group information
    for( UINT i=0; i < m_pPlayers->Count(); i++ )
    {
        CSIPlayer* pPlayer = (CSIPlayer*) m_pPlayers->GetPtr( i );

        // Attempt to send 
        hr = SendPlayerInfoToPlayer( pPlayer->id, id );
        if( FAILED(hr) )
            break;
    }

    for( i=0; i < m_pGroups->Count(); i++ )
    {
        CSIGroup* pGroup = (CSIGroup*) m_pGroups->GetPtr( i );

        // Attempt to send 
        hr = SendGroupInfoToPlayer( pGroup->id, id );
        if( FAILED(hr) )
            break;
    }

    Unlock();

    return hr;
}




//-----------------------------------------------------------------------------
// Name: ShowDialog()
// Desc: Show the dialog UI for all stored player, group, and message data.
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::ShowDialog( HWND hParent )
{
    // If a dialog already exists, bring it to the front
    if( m_hDlg )
    {
        if( FALSE == BringWindowToTop( m_hDlg ) )
            return E_FAIL;
    }
    else
    {
        // If there is an old thread handle, release it now
        SafeDestroyThread( &m_hDlgThread );

        // Launch a new dialog thread
        DWORD dwThreadID;

        m_hDlgParent = hParent;
        m_hDlgThread = chBEGINTHREADEX( NULL, 0, StaticDialogThread, (void*) this, 0, &dwThreadID );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: StaticDialogThread()
// Desc: Message pump thread for modeless dialogs
//-----------------------------------------------------------------------------
DWORD WINAPI CSessionInfo::StaticDialogThread( void* pvRef )
{
    CSessionInfo* pThis = (CSessionInfo*) pvRef;

    pThis->m_hDlgPlayers = CreateDialog( GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SI_PLAYERS), pThis->m_hDlgParent, StaticDlgProcPlayers );
    pThis->m_hDlgMessages = CreateDialog( GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SI_MESSAGES), pThis->m_hDlgParent, StaticDlgProcMessages );
    pThis->m_hDlg = CreateDialog( GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SI_MAIN), pThis->m_hDlgParent, StaticDlgProcMain );   
    
    ShowWindow( pThis->m_hDlg, SW_SHOW );

    MSG msg;
    while( GetMessage( &msg, NULL, 0, 0) )
    {  
        // Forward the message to the dialog
        if( IsDialogMessage( pThis->m_hDlg, &msg ) )
            continue;

        if( IsDialogMessage( pThis->m_hDlgPlayers, &msg ) )
            continue;

        if( IsDialogMessage( pThis->m_hDlgMessages, &msg ) )
            continue;
    }

    // Release the window resources
    DestroyWindow( pThis->m_hDlgMessages ); 
    pThis->m_hDlgMessages = NULL;

    DestroyWindow( pThis->m_hDlgPlayers );  
    pThis->m_hDlgPlayers = NULL;

    DestroyWindow( pThis->m_hDlg );         
    pThis->m_hDlg = NULL;

    return 0;
}




//-----------------------------------------------------------------------------
// Name: StaticDlgProcMain()
// Desc: Static dialog procedure sorts incoming messages according to the
//       window handle and assigns them to the member dialog procedure for the
//       correct instance
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CSessionInfo::StaticDlgProcMain( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    if( g_pSI )
        return g_pSI->DlgProcMain( hDlg, uMsg, wParam, lParam );

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: StaticDlgProcPlayers()
// Desc: Static dialog procedure sorts incoming messages according to the
//       window handle and assigns them to the member dialog procedure for the
//       correct instance
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CSessionInfo::StaticDlgProcPlayers( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    if( g_pSI )
        return g_pSI->DlgProcPlayers( hDlg, uMsg, wParam, lParam );

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: StaticDlgProcMessages()
// Desc: Static dialog procedure sorts incoming messages according to the
//       window handle and assigns them to the member dialog procedure for the
//       correct instance
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CSessionInfo::StaticDlgProcMessages( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    if( g_pSI )
        return g_pSI->DlgProcMessages( hDlg, uMsg, wParam, lParam );

    return FALSE;
}
  



//-----------------------------------------------------------------------------
// Name: DlgProcMain()
// Desc: Dialog procedure for the UI
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CSessionInfo::DlgProcMain( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg )
    {
        case WM_INITDIALOG:
        {
            SetParent( m_hDlgPlayers, GetDlgItem( hDlg, IDC_SI_TAB ) );
            SetParent( m_hDlgMessages, GetDlgItem( hDlg, IDC_SI_TAB ) );

            // Set the window title
            DPN_APPLICATION_DESC* pAppDesc;
            if( SUCCEEDED( GetDpAppDesc( &pAppDesc ) ) )
            {
                TCHAR strTitle[ 256 ];
                
                DXUtil_ConvertWideStringToGenericCch( strTitle, pAppDesc->pwszSessionName, 256-50 );
                lstrcat( strTitle, TEXT(" - Session Info") );
                SendMessage( hDlg, WM_SETTEXT, 0, (LPARAM) strTitle ); 
            }

            // Add tabs
            TCITEM tcItem = {0};
            tcItem.mask = TCIF_TEXT;

            tcItem.pszText = TEXT("Players");
            SendMessage( GetDlgItem( hDlg, IDC_SI_TAB ), TCM_INSERTITEM, 0, (LPARAM) &tcItem );
            tcItem.pszText = TEXT("Messages");
            SendMessage( GetDlgItem( hDlg, IDC_SI_TAB ), TCM_INSERTITEM, 1, (LPARAM) &tcItem );


            RECT rcWindow = {0};
            GetWindowRect( GetDlgItem( hDlg, IDC_SI_TAB ), &rcWindow );
            SendMessage( GetDlgItem( hDlg, IDC_SI_TAB ), TCM_ADJUSTRECT, FALSE, (LPARAM) &rcWindow );

            POINT ptWindow = { rcWindow.left, rcWindow.top };
            ScreenToClient( GetDlgItem( hDlg, IDC_SI_TAB ), &ptWindow );
            MoveWindow( m_hDlgPlayers, ptWindow.x, ptWindow.y, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, FALSE );
            MoveWindow( m_hDlgMessages, ptWindow.x, ptWindow.y, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, FALSE );

            ShowWindow( m_hDlgPlayers, SW_SHOW );

            SAFE_DELETE_ARRAY( pAppDesc );
            return TRUE;
        }

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDCANCEL:
                    SendMessage( hDlg, WM_CLOSE, 0, 0 );
                    return TRUE;
            }

            break;

        case WM_NOTIFY:
        {
            NMHDR* pHdr = (NMHDR*) lParam; 
            switch( pHdr->code )
            {
                case TCN_SELCHANGE:
                    DWORD dwCurSel = SendMessage( GetDlgItem( hDlg, IDC_SI_TAB ), TCM_GETCURSEL, 0, 0 );

                    ShowWindow( m_hDlgPlayers,  ( 0 == dwCurSel ) ? SW_SHOW : SW_HIDE );
                    ShowWindow( m_hDlgMessages, ( 1 == dwCurSel ) ? SW_SHOW : SW_HIDE ); 
                    return TRUE;
            }
            break;
        }

        case WM_CLOSE:
            if( m_hDlgThread )
                PostQuitMessage( 0 );
            else
                EndDialog( hDlg, 0 );
            
            return TRUE;
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: DlgProcPlayers()
// Desc: Dialog procedure for the UI
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CSessionInfo::DlgProcPlayers( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg )
    {
        case WM_INITDIALOG:
        {
            // Set custom dialog settings
            if( m_hNameFont )
                SendMessage( GetDlgItem( hDlg, IDC_SI_NAME ), WM_SETFONT, (WPARAM) m_hNameFont, TRUE );

            if( m_hConnectionFont )
                SendMessage( GetDlgItem( hDlg, IDC_SI_DESCRIPTION ), WM_SETFONT, (WPARAM) m_hConnectionFont, TRUE );
            
            // Refresh the dialog contents
            PaintDialog( hDlg );
            m_bDlgValid = TRUE;
            
            // Set the refresh timer
            SetTimer( hDlg, SI_REFRESH_TIMER, SI_REFRESH_INTERVAL, NULL );
            return TRUE;
        }

        case WM_TIMER:
        {
            if( FALSE == m_bDlgValid )
                PaintDialog( hDlg );

            return TRUE;
        }

        case WM_COMMAND:
        {
            switch( LOWORD(wParam) )
            {
                case IDC_SI_PLAYERS:
                {
                    if( LBN_SELCHANGE == HIWORD(wParam) )
                    {
                        DWORD dwCurSel;
                        DWORD dwItemData;

                        if( LB_ERR != ( dwCurSel   = SendMessage( (HWND) lParam, LB_GETCURSEL, 0, 0 ) ) &&
                            LB_ERR != ( dwItemData = SendMessage( (HWND) lParam, LB_GETITEMDATA, dwCurSel, 0 ) ) )
                            DisplayPlayer( dwItemData, hDlg );
                            
                        return TRUE;
                    }

                    break;
                }

                case IDC_SI_GROUPS:
                {
                    if( LBN_SELCHANGE == HIWORD(wParam) )
                    {
                        DWORD dwCurSel;
                        DWORD dwItemData;

                        if( LB_ERR != ( dwCurSel   = SendMessage( (HWND) lParam, LB_GETCURSEL, 0, 0 ) ) &&
                            LB_ERR != ( dwItemData = SendMessage( (HWND) lParam, LB_GETITEMDATA, dwCurSel, 0 ) ) )
                            DisplayGroup( dwItemData, hDlg );
                            
                        return TRUE;
                    }

                    break;
                }

                case IDC_SI_MEMBERSHIP:
                {
                    if( LBN_SELCHANGE == HIWORD(wParam) )
                    {
                        DWORD dwCurSel;
                        DWORD dwItemData;

                        if( LB_ERR != ( dwCurSel   = SendMessage( (HWND) lParam, LB_GETCURSEL, 0, 0 ) ) &&
                            LB_ERR != ( dwItemData = SendMessage( (HWND) lParam, LB_GETITEMDATA, dwCurSel, 0 ) ) )
                        {
                            if( LB_ERR != SendMessage( GetDlgItem( hDlg, IDC_SI_PLAYERS ), LB_GETCURSEL, 0, 0 ) )
                                DisplayGroup( dwItemData, hDlg );   
                            else if( LB_ERR != SendMessage( GetDlgItem( hDlg, IDC_SI_GROUPS ), LB_GETCURSEL, 0, 0 ) )
                                DisplayPlayer( dwItemData, hDlg );
                        }

                        return TRUE;
                    }
                    break;
                }
            }
            break;
        }
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: DlgProcMessages()
// Desc: Dialog procedure for the UI
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CSessionInfo::DlgProcMessages( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg )
    {
        case WM_INITDIALOG:
        {
            // List box settings
            PostMessage( GetDlgItem( hDlg, IDC_SI_DPLAYMSG ), LB_INITSTORAGE, SI_MAX_MESSAGES, sizeof(TCHAR) * 256 );
            PostMessage( GetDlgItem( hDlg, IDC_SI_APPMSG ),   LB_INITSTORAGE, SI_MAX_MESSAGES, sizeof(TCHAR) * 256 );
            PostMessage( GetDlgItem( hDlg, IDC_SI_DPLAYMSG ), LB_SETHORIZONTALEXTENT, 400, 0 );
            PostMessage( GetDlgItem( hDlg, IDC_SI_APPMSG ),   LB_SETHORIZONTALEXTENT, 400, 0 );

            // Fill the message windows
            m_DPlayMessages.Lock();
            for( UINT i=0; i < m_DPlayMessages.GetNumOfMessages(); i++ )
            {
                SendMessage( GetDlgItem( hDlg, IDC_SI_DPLAYMSG ), LB_INSERTSTRING, 0, (LPARAM) m_DPlayMessages.GetMessage( i ) );
            }
            m_DPlayMessages.Unlock();
            
            m_AppMessages.Lock();
            for( i=0; i < m_AppMessages.GetNumOfMessages(); i++ )
            {
                SendMessage( GetDlgItem( hDlg, IDC_SI_APPMSG ), LB_INSERTSTRING, 0, (LPARAM) m_AppMessages.GetMessage( i ) );
            }
            m_AppMessages.Unlock();

            return TRUE;
        }
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: PaintDialog()
// Desc: Fill the dialog with the stored player/group data
//-----------------------------------------------------------------------------
VOID CSessionInfo::PaintDialog( HWND hDlg )
{
    m_bDlgValid = TRUE;

    // Store the current selection
    enum { NONE, PLAYER, GROUP } eSelected = NONE;

    DWORD dwItemData = 0;
    DWORD dwCurSel = 0;

    if( LB_ERR != ( dwCurSel   = SendMessage( GetDlgItem( hDlg, IDC_SI_PLAYERS ), LB_GETCURSEL, 0, 0 ) ) &&
        LB_ERR != ( dwItemData = SendMessage( GetDlgItem( hDlg, IDC_SI_PLAYERS ), LB_GETITEMDATA, dwCurSel, 0 ) ) )
    {
        eSelected = PLAYER;
    }
    else if( LB_ERR != ( dwCurSel   = SendMessage( GetDlgItem( hDlg, IDC_SI_GROUPS ), LB_GETCURSEL, 0, 0 ) ) &&
             LB_ERR != ( dwItemData = SendMessage( GetDlgItem( hDlg, IDC_SI_GROUPS ), LB_GETITEMDATA, dwCurSel, 0 ) ) )
    {
        eSelected = GROUP;
    }

    // Clear the current contents
    SendMessage( GetDlgItem( hDlg, IDC_SI_PLAYERS ), LB_RESETCONTENT, 0, 0 );
    SendMessage( GetDlgItem( hDlg, IDC_SI_GROUPS ), LB_RESETCONTENT, 0, 0 );

    Lock();
    
    // Add Players
    for( UINT i=0; i < m_pPlayers->Count(); i++ )
    {
        CSIPlayer* pPlayer = (CSIPlayer*) m_pPlayers->GetPtr( i );

        DWORD dwIndex = SendMessage( GetDlgItem( hDlg, IDC_SI_PLAYERS ), LB_ADDSTRING, 0, (LPARAM) pPlayer->strName );
        SendMessage( GetDlgItem( hDlg, IDC_SI_PLAYERS ), LB_SETITEMDATA, dwIndex, pPlayer->id );
    }

    // Add Groups
    for( i=0; i < m_pGroups->Count(); i++ )
    {
        CSIGroup* pGroup = (CSIGroup*) m_pGroups->GetPtr( i );

        DWORD dwIndex = SendMessage( GetDlgItem( hDlg, IDC_SI_GROUPS ), LB_ADDSTRING, 0, (LPARAM) pGroup->strName );
        SendMessage( GetDlgItem( hDlg, IDC_SI_GROUPS ), LB_SETITEMDATA, dwIndex, pGroup->id );
    }

    Unlock();

    // Restore the selection
    switch( eSelected )
    {
        case PLAYER: 
            if( FAILED( DisplayPlayer( dwItemData, hDlg ) ) )
                DisplayPlayer( m_dpnidLocal, hDlg );
            break;

        case GROUP:  
            if( FAILED( DisplayGroup( dwItemData, hDlg ) ) )
                DisplayPlayer( m_dpnidLocal, hDlg );
            break;

        default: 
            DisplayPlayer( m_dpnidLocal, hDlg );
            break;
    }
}




//-----------------------------------------------------------------------------
// Name: DisplayPlayer()
// Desc: Display all stored information about the given player in the dialog UI
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::DisplayPlayer( DPNID id, HWND hDlg )
{
    HRESULT hr;

    // Set the icon
#ifndef UNDER_CE
    if( m_hPlayerIcon )
        SendMessage( GetDlgItem( hDlg, IDC_SI_NAME_ICON ), STM_SETICON, (LPARAM) m_hPlayerIcon, 0 );
#endif // !UNDER_CE

    // Clear the dialog data
    SendMessage( GetDlgItem( hDlg, IDC_SI_NAME ), WM_SETTEXT, 0, (LPARAM) TEXT("") );
    SendMessage( GetDlgItem( hDlg, IDC_SI_GROUPS ), LB_SETCURSEL, (WPARAM) -1, 0 );
    SendMessage( GetDlgItem( hDlg, IDC_SI_MEMBERSHIP ), LB_RESETCONTENT, 0, 0 );
    SendMessage( GetDlgItem( hDlg, IDC_SI_DESCRIPTION ), WM_SETTEXT, (WPARAM) 0, (LPARAM) TEXT("") );

    SendMessage( GetDlgItem( hDlg, IDC_SI_MEMBERSHIP_TEXT ), WM_SETTEXT, 0, (LPARAM) TEXT("Member Of") );
    
    Lock();

    // Search for the given player
    CSIPlayer* pPlayer = FindPlayer( id );
    if( NULL == pPlayer )
    {
        hr = E_INVALIDARG;
        goto LCleanReturn;
    }

    // Set the selected item
    SelectListboxItem( GetDlgItem( hDlg, IDC_SI_PLAYERS ), pPlayer->id, pPlayer->strName );

    // Show the player name
    SendMessage( GetDlgItem( hDlg, IDC_SI_NAME ), WM_SETTEXT, 0, (LPARAM) pPlayer->strName );

    // Fill the description box
    PrintPlayerInfo( GetDlgItem( hDlg, IDC_SI_DESCRIPTION ), pPlayer );

    // Search for the player id in the group list
    UINT i;
    for( i=0; i < m_pGroups->Count(); i++ )
    {
        CSIGroup* pGroup = (CSIGroup*) m_pGroups->GetPtr( i );

        if( pGroup->IsMember( id ) )
        {
            DWORD dwIndex = SendMessage( GetDlgItem( hDlg, IDC_SI_MEMBERSHIP ), LB_ADDSTRING, 0, (LPARAM) pGroup->strName );
            if( LB_ERR != dwIndex )
                SendMessage( GetDlgItem( hDlg, IDC_SI_MEMBERSHIP ), LB_SETITEMDATA, dwIndex, pGroup->id );
        }
    }

    
    hr = S_OK;

LCleanReturn:
    Unlock();
    return hr;
}




//-----------------------------------------------------------------------------
// Name: DisplayGroup()
// Desc: Display all stored information about the given group in the dialog UI
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::DisplayGroup( DPNID id, HWND hDlg )
{
    HRESULT hr;

    // Set the icon
#ifndef UNDER_CE
    if( m_hPlayerIcon )
        SendMessage( GetDlgItem( hDlg, IDC_SI_NAME_ICON ), STM_SETICON, (LPARAM) m_hGroupIcon, 0 );
#endif // !UNDER_CE

    // Clear the dialog data
    SendMessage( GetDlgItem( hDlg, IDC_SI_NAME ), WM_SETTEXT, 0, (LPARAM) TEXT("") );
    SendMessage( GetDlgItem( hDlg, IDC_SI_PLAYERS ), LB_SETCURSEL, (WPARAM) -1, 0 );
    SendMessage( GetDlgItem( hDlg, IDC_SI_MEMBERSHIP ), LB_RESETCONTENT, 0, 0 );
    SendMessage( GetDlgItem( hDlg, IDC_SI_DESCRIPTION ), WM_SETTEXT, 0, (LPARAM) TEXT("") );

    SendMessage( GetDlgItem( hDlg, IDC_SI_MEMBERSHIP_TEXT ), WM_SETTEXT, 0, (LPARAM) TEXT("Members") );
    
    Lock();

    // Search for the given group
    CSIGroup* pGroup = FindGroup( id );
    if( NULL == pGroup )
    {
        hr = E_INVALIDARG;
        goto LCleanReturn;
    }

    // Set the selected item
    SelectListboxItem( GetDlgItem( hDlg, IDC_SI_GROUPS ), pGroup->id, pGroup->strName );

    // Fill the description box
    PrintGroupInfo( GetDlgItem( hDlg, IDC_SI_DESCRIPTION ), pGroup );

    // Show the group name
    SendMessage( GetDlgItem( hDlg, IDC_SI_NAME ), WM_SETTEXT, 0, (LPARAM) pGroup->strName );

    // List member players
    UINT i;
    for( i=0; i < pGroup->pMembers->Count(); i++ )
    {
        DPNID* pID = (DPNID*) pGroup->pMembers->GetPtr( i );
        CSIPlayer* pPlayer = FindPlayer( *pID );

        if( pPlayer )
        {
            DWORD dwIndex;

            dwIndex = SendMessage( GetDlgItem( hDlg, IDC_SI_MEMBERSHIP ), LB_ADDSTRING, 0, (LPARAM) pPlayer->strName );
            if( LB_ERR != dwIndex )
                SendMessage( GetDlgItem( hDlg, IDC_SI_MEMBERSHIP ), LB_SETITEMDATA, dwIndex, pPlayer->id );
        }
    }

    hr = S_OK;

LCleanReturn:
    Unlock();
    return hr;
}




//-----------------------------------------------------------------------------
// Name: PrintPlayerInfo()
// Desc: Print the player description to the given edit box
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::PrintPlayerInfo( HWND hWndEdit, CSIPlayer* pPlayer)
{
    TCHAR strBuf[51] = {0};

    // Validate parameters
#ifdef _DEBUG
    if( NULL == pPlayer )
        return E_INVALIDARG;
#endif // _DEBUG


    // Output DPNID info
    _sntprintf( strBuf, 50, TEXT("dpnid     0x%x (%u)\015\012"), pPlayer->id, pPlayer->id );
    strBuf[ 50 ] = TEXT('\0');
    SendMessage( hWndEdit, EM_REPLACESEL, FALSE, (LPARAM) strBuf );

    // Output flags
    ZeroMemory( strBuf, sizeof( strBuf ) );
    lstrcpy( strBuf, TEXT("flags     ") );

    if( pPlayer->bIsHost )
    {
        lstrcat( strBuf, TEXT("Host") );
    }

    if( m_dpnidLocal == pPlayer->id )
    {
        if( pPlayer->bIsHost )
            lstrcat( strBuf, TEXT(", ") );

        lstrcat( strBuf, TEXT("Local") );
    }

    lstrcat( strBuf, TEXT("\015\012") );
    SendMessage( hWndEdit, EM_REPLACESEL, FALSE, (LPARAM) strBuf );

    // If the player has an associated URL address, print the
    // encoded information
    if( pPlayer->strURL && lstrlen( pPlayer->strURL ) > lstrlen( TEXT("x-directplay:/") ) )
    {
        // Duplicate input string since it will be modified
        TCHAR* strCopy = _tcsdup( pPlayer->strURL + lstrlen( TEXT("x-directplay:/") ) );
        if( NULL == strCopy )
            return E_OUTOFMEMORY;

        TCHAR* strToken = NULL;
        strToken = _tcstok( strCopy, TEXT("=") );

        // While more URL information is encoded in input string
        while( strToken )
        {
            TCHAR strSpacing[] = TEXT("           ");

            // output key
            SendMessage( hWndEdit, EM_REPLACESEL, FALSE, (LPARAM) strToken );
            strSpacing[ 10 - lstrlen( strToken ) ] = L'\0';

            // insert column spacing
            SendMessage( hWndEdit, EM_REPLACESEL, FALSE, (LPARAM) strSpacing );

            // output value
            strToken = _tcstok( NULL, TEXT(";") );
            SendMessage( hWndEdit, EM_REPLACESEL, FALSE, (LPARAM) strToken );
            SendMessage( hWndEdit, EM_REPLACESEL, FALSE, (LPARAM) TEXT("\015\012") );

            // find next key
            strToken = _tcstok( NULL, TEXT("=") );
        }

        // release resources
        if( strCopy )
            free( strCopy );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: PrintGroupInfo()
// Desc: Print the group description to the given edit box
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::PrintGroupInfo( HWND hWndEdit, CSIGroup* pGroup )
{
    TCHAR strBuf[51] = {0};

    // Validate parameters
#ifdef _DEBUG
    if( NULL == pGroup )
        return E_INVALIDARG;
#endif // _DEBUG

    // Output DPNID info
    _sntprintf( strBuf, 50, TEXT("dpnid     0x%x (%u)\015\012"), pGroup->id, pGroup->id );
    strBuf[ 50 ] = TEXT('\0');
    SendMessage( hWndEdit, EM_REPLACESEL, FALSE, (LPARAM) strBuf );

    return S_OK;
}





//-----------------------------------------------------------------------------
// Name: CMessageList()
// Desc: Constructor
//-----------------------------------------------------------------------------
CMessageList::CMessageList() : m_dwStartIndex( 0 ), m_dwNumMessages( 0 )
{
    InitializeCriticalSection( &m_csLock );
}




//-----------------------------------------------------------------------------
// Name: ~CMessageList()
// Desc: Destructor
//-----------------------------------------------------------------------------
CMessageList::~CMessageList()
{
    DeleteCriticalSection( &m_csLock );
}




//-----------------------------------------------------------------------------
// Name: GetMessage()
// Desc: Returns the string pointer for the given message number
//-----------------------------------------------------------------------------
TCHAR* CMessageList::GetMessage( DWORD dwMessageNum )
{
    if( dwMessageNum >= m_dwNumMessages )
        return NULL;

    return m_rStrMessage[ ( m_dwStartIndex + dwMessageNum ) % SI_MAX_MESSAGES ];
}




//-----------------------------------------------------------------------------
// Name: AddMessage()
// Desc: Adds and time-stamp and copies the string to the end of the message
//       list. If the list is full, the oldest item is removed to make room
//-----------------------------------------------------------------------------
TCHAR* CMessageList::AddMessage( TCHAR* strMessage )
{
    TCHAR* strDest = NULL;
 
    if( m_dwNumMessages < SI_MAX_MESSAGES )
    {
        // If there is still room in the array, increment the number of messages
        // and get the newly added string pointer
        strDest = GetMessage( m_dwNumMessages++ );
    }
    else
    {
        // Else, increment the start index and get the last element
        m_dwStartIndex++;
        strDest = GetMessage( SI_MAX_MESSAGES - 1 );
    }

    // Copy the string, adding a time stamp
    SYSTEMTIME sysTime;
    GetLocalTime( &sysTime );
    _sntprintf( strDest, 56, TEXT("[%02d:%02d:%02d]  "),
               sysTime.wHour, sysTime.wMinute, sysTime.wSecond );

    _tcsncat( strDest, strMessage, 200 );
    strDest[ 255 ] = TEXT('\0');

    return strDest;
}




//-----------------------------------------------------------------------------
// Name: SelectListboxItem
// Desc: Search for and select the matching item in the list box
//-----------------------------------------------------------------------------
HRESULT CSessionInfo::SelectListboxItem( HWND hListBox, DWORD dwData, const TCHAR* strItem )
{
    int nIndex = -1;
    int nItemData = LB_ERR; 

    do 
    {
        // Find the string in the list
        nIndex = SendMessage( hListBox, LB_FINDSTRING, nIndex, (LPARAM) strItem );
        if( LB_ERR == nIndex )
            break;

        // Confirm the id number matches
        nItemData = SendMessage( hListBox, LB_GETITEMDATA, nIndex, 0 );
        if( LB_ERR == nItemData )
            break;

        // Match found, select the item and return
        if( (DWORD) nItemData == dwData )
        {
            SendMessage( hListBox, LB_SETCURSEL, nIndex, 0 );
            return S_OK;
        }
    }
    while( nIndex != LB_ERR );

    return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: SafeDestroyThread
// Desc: Signal the window close event and wait for the thread to shut down  
//-----------------------------------------------------------------------------
VOID CSessionInfo::SafeDestroyThread( LPHANDLE phThread )
{
    if( *phThread )
    {
        WaitForSingleObject( *phThread, INFINITE );
        CloseHandle( *phThread );
        *phThread = NULL;
    }
}