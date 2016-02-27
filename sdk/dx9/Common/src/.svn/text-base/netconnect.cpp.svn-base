//-----------------------------------------------------------------------------
// File: NetConnect.cpp
//
// Desc: This is a class that given a IDirectPlay8Peer, then DoConnectWizard()
//       will enumerate service providers, enumerate hosts, and allow the
//       user to either join or host a session.  The class uses
//       dialog boxes and GDI for the interactive UI.  Most games will
//       want to change the graphics to use Direct3D or another graphics
//       layer, but this simplistic sample uses dialog boxes.  Feel 
//       free to use this class as a starting point for adding extra 
//       functionality.
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef STRICT
#define STRICT
#endif // !STRICT
#include <windows.h>
#include <basetsd.h>
#include <stdio.h>
#include <mmsystem.h>
#include <dxerr9.h>
#include <dplay8.h>
#include <dpaddr.h>
#include <dplobby8.h>
#include "NetConnect.h"
#include "NetConnectRes.h"
#include "DXUtil.h"

#if defined(WIN32_PLATFORM_PSPC) && (_WIN32_WCE >= 300)
#include <aygshell.h>
#endif // PocketPC



//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
CNetConnectWizard* g_pNCW = NULL;           // Pointer to the net connect wizard




//-----------------------------------------------------------------------------
// Name: CNetConnectWizard
// Desc: Init the class
//-----------------------------------------------------------------------------
CNetConnectWizard::CNetConnectWizard( HINSTANCE hInst, HWND hWndParent, 
                                      TCHAR* strAppName, GUID* pGuidApp )
{
    g_pNCW              = this;
    m_hInst             = hInst;
    m_hWndParent        = hWndParent;
    m_pDP               = NULL;
    m_pLobbiedApp       = NULL;
    m_bHaveConnectionSettingsFromLobby = FALSE;
    m_hLobbyClient      = NULL;
    m_guidApp           = *pGuidApp;
    m_hDlg              = NULL;
    m_bConnecting       = FALSE;
    m_hConnectAsyncOp   = NULL;
    m_hEnumAsyncOp      = NULL;
    m_bMigrateHost      = FALSE;
    m_bUseDPNSVR        = FALSE;
    m_dwPort            = 0;
    m_eSigningType      = SIGN_NONE;
    m_dwEnumHostExpireInterval = 0;

    ZeroMemory(&m_guidSP, sizeof(GUID));

    // Set the max players unlimited by default.  This can be changed by the app
    // by calling SetMaxPlayers()
    m_dwMaxPlayers   = 0;

    _tcsncpy( m_strAppName, strAppName, MAX_PATH-1 );
    m_strAppName[ MAX_PATH-1 ] = 0; 
    
    _tcsncpy( m_strPreferredProvider, TEXT("DirectPlay8 TCP/IP Service Provider"), MAX_PATH-1 );
    m_strPreferredProvider[ MAX_PATH-1 ] = 0; 

    _tcsncpy( m_strHostname, TEXT(""), MAX_PATH-1 );

    InitializeCriticalSection( &m_csHostEnum );
    m_hConnectCompleteEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hLobbyConnectionEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    // Setup the m_DPHostEnumHead circular linked list
    ZeroMemory( &m_DPHostEnumHead, sizeof( DPHostEnumInfo ) );
    m_DPHostEnumHead.pNext = &m_DPHostEnumHead;
}




//-----------------------------------------------------------------------------
// Name: ~CNetConnectWizard
// Desc: Cleanup the class
//-----------------------------------------------------------------------------
CNetConnectWizard::~CNetConnectWizard()
{
    DeleteCriticalSection( &m_csHostEnum );
    CloseHandle( m_hConnectCompleteEvent );
    CloseHandle( m_hLobbyConnectionEvent );
}



//-----------------------------------------------------------------------------
// Name: Init
// Desc:
//-----------------------------------------------------------------------------
HRESULT CNetConnectWizard::Init( IDirectPlay8Peer* pDP,
                                 IDirectPlay8LobbiedApplication* pLobbiedApp )
{
    if( NULL == pDP || NULL == pLobbiedApp )
        return E_INVALIDARG;

    m_pDP               = pDP;
    m_pLobbiedApp       = pLobbiedApp;
    m_bHaveConnectionSettingsFromLobby = FALSE;
    m_hLobbyClient      = NULL;

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: Shutdown
// Desc: Releases the DirectPlay interfaces
//-----------------------------------------------------------------------------
VOID CNetConnectWizard::Shutdown()
{
}




//-----------------------------------------------------------------------------
// Name: DoConnectWizard
// Desc: This is the main external function.  This will launch a series of
//       dialog boxes that enumerate service providers, enumerate hosts,
//       and allow the user to either join or host a session
//-----------------------------------------------------------------------------
HRESULT CNetConnectWizard::DoConnectWizard( BOOL bBackTrack )
{
    if( m_pDP == NULL )
        return E_INVALIDARG;

    int nStep;

    // If the back track flag is true, then the user has already been through
    // the connect process once, and has back tracked out of the main game
    // so start at the last dialog box
    if( bBackTrack )
        nStep = 1;
    else
        nStep = 0;

    // Show the dialog boxes to connect
    for( ;; )
    {
        m_hrDialog = S_OK;

        switch( nStep )
        {
            case 0:
                // Display the multiplayer connect dialog box.
                DialogBox( m_hInst, MAKEINTRESOURCE(IDD_MULTIPLAYER_CONNECT),
                           m_hWndParent, (DLGPROC) StaticConnectionsDlgProc );
                break;

            case 1:
                // Display the multiplayer games dialog box.
                DialogBox( m_hInst, MAKEINTRESOURCE(IDD_MULTIPLAYER_GAMES),
                           m_hWndParent, (DLGPROC) StaticSessionsDlgProc );
                break;
        }

        if( FAILED( m_hrDialog ) ||
            m_hrDialog == NCW_S_QUIT ||
            m_hrDialog == NCW_S_LOBBYCONNECT )
            break;

        if( m_hrDialog == NCW_S_BACKUP )
            nStep--;
        else
            nStep++;

        // If we go beyond the last step in the wizard, then stop
        // and return.
        if( nStep == 2 )
            break;
    }

    // Depending upon a successful m_hrDialog the user has
    // either successfully join or created a game, depending on m_bHostPlayer
    m_pDP = NULL;
    return m_hrDialog;
}




//-----------------------------------------------------------------------------
// Name: StaticConnectionsDlgProc()
// Desc: Static msg handler which passes messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CNetConnectWizard::StaticConnectionsDlgProc( HWND hDlg, UINT uMsg,
                                                              WPARAM wParam, LPARAM lParam )
{
    if( g_pNCW )
        return g_pNCW->ConnectionsDlgProc( hDlg, uMsg, wParam, lParam );

    return FALSE; // Message not handled
}




//-----------------------------------------------------------------------------
// Name: ConnectionsDlgProc()
// Desc: Handles messages for the multiplayer connect dialog
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CNetConnectWizard::ConnectionsDlgProc( HWND hDlg, UINT msg,
                                                        WPARAM wParam, LPARAM lParam )
{
    UNREFERENCED_PARAMETER( lParam );
    
    switch( msg )
    {
        case WM_INITDIALOG:
            {
#if defined(WIN32_PLATFORM_PSPC) && (_WIN32_WCE >= 300)
                SHINITDLGINFO   shidi;
                memset(&shidi, 0, sizeof(SHINITDLGINFO));
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
                shidi.hDlg = hDlg;

                SetForegroundWindow(hDlg);
                SHInitDialog(&shidi);
#endif // WIN32_PLATFORM_PSPC

                SetDlgItemText( hDlg, IDC_PLAYER_NAME_EDIT, m_strLocalPlayerName );

                // Load and set the icon
                HICON hIcon = LoadIcon( m_hInst, MAKEINTRESOURCE( IDI_MAIN ) );
                SendMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
                SendMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

                // Set the window title
                TCHAR strWindowTitle[256];
                wsprintf( strWindowTitle, TEXT("%s - Multiplayer Connect"), m_strAppName );
                SetWindowText( hDlg, strWindowTitle );

                // Fill the list box with the service providers
                if( FAILED( m_hrDialog = ConnectionsDlgFillListBox( hDlg ) ) )
                {
                    DXTRACE_ERR_MSGBOX( TEXT("ConnectionsDlgFillListBox"), m_hrDialog );
                    EndDialog( hDlg, 0 );
                }
            }
            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDC_CONNECTION_LIST:
                    if( HIWORD(wParam) != LBN_DBLCLK )
                        break;
                    // Fall through

                case IDOK:
                    if( FAILED( m_hrDialog = ConnectionsDlgOnOK( hDlg ) ) )
                    {
                        DXTRACE_ERR_MSGBOX( TEXT("ConnectionsDlgOnOK"), m_hrDialog );
                        EndDialog( hDlg, 0 );
                    }

                    if( m_hrDialog == NCW_S_LOBBYCONNECT )
                    {
                        EndDialog( hDlg, 0 );
                    }
                    break;

                case IDCANCEL:
                    m_hrDialog = NCW_S_QUIT;
                    EndDialog( hDlg, 0 );
                    break;

                default:
                    return FALSE; // Message not handled
            }
            break;

        case WM_DESTROY:
            ConnectionsDlgCleanup( hDlg );
            break;

        default:
            return FALSE; // Message not handled
    }

    // Message was handled
    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: ConnectionsDlgFillListBox()
// Desc: Fills the DirectPlay connection listbox with service providers,
//       and also adds a "Wait for Lobby" connection option.
//-----------------------------------------------------------------------------
HRESULT CNetConnectWizard::ConnectionsDlgFillListBox( HWND hDlg )
{
    HRESULT                     hr;
    int                         iLBIndex;
    DWORD                       dwItems     = 0;
    DPN_SERVICE_PROVIDER_INFO*  pdnSPInfo   = NULL;
    DWORD                       dwSize      = 0;
    HWND                        hWndListBox = GetDlgItem( hDlg, IDC_CONNECTION_LIST );
    TCHAR                       strName[MAX_PATH];

    // Enumerate all DirectPlay service providers, and store them in the listbox

    // Get required space for all providers
    hr = m_pDP->EnumServiceProviders( NULL, NULL, pdnSPInfo, &dwSize,
                                      &dwItems, 0 );
    if( FAILED(hr) && hr != DPNERR_BUFFERTOOSMALL )
    {
        DXTRACE_ERR_MSGBOX( TEXT("EnumServiceProviders"), hr );
        goto LCleanReturn;
    }

    // Allocate required space
    pdnSPInfo = (DPN_SERVICE_PROVIDER_INFO*) new BYTE[dwSize];
    if( NULL == pdnSPInfo )
    {
        hr = E_OUTOFMEMORY;
        DXTRACE_ERR_MSGBOX( TEXT("ConnectionsDlgFillListBox"), hr );
        goto LCleanReturn;
    }

    // Perform the enumeration
    hr = m_pDP->EnumServiceProviders( NULL, NULL, pdnSPInfo,
                                      &dwSize, &dwItems, 0 );
    if( FAILED(hr) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("EnumServiceProviders"), hr );
        goto LCleanReturn;
    }

    // For each detected provider, add an item to the listbox
    DPN_SERVICE_PROVIDER_INFO* pdnSPInfoEnum;
    pdnSPInfoEnum = pdnSPInfo;

    DWORD i;
    for ( i = 0; i < dwItems; i++ )
    {
        DXUtil_ConvertWideStringToGenericCch( strName, pdnSPInfoEnum->pwszName, MAX_PATH );

        // Found a service provider, so put it in the listbox
        iLBIndex = (int)SendMessage( hWndListBox, LB_ADDSTRING, 0,
                                     (LPARAM)strName );
        if( iLBIndex == CB_ERR )
        {
            // Error, stop enumerating
            hr = E_FAIL;
            DXTRACE_ERR_MSGBOX( TEXT("ConnectionsDlgFillListBox"), hr );
            goto LCleanReturn;
        }

        // Store pointer to GUID in listbox
        GUID* pGuid = new GUID;
        if( NULL == pGuid )
        {
            hr = E_OUTOFMEMORY;
            DXTRACE_ERR_MSGBOX( TEXT("ConnectionsDlgFillListBox"), hr );
            goto LCleanReturn;
        }

        memcpy( pGuid, &pdnSPInfoEnum->guid, sizeof(GUID) );
        SendMessage( hWndListBox, LB_SETITEMDATA, iLBIndex,
                     (LPARAM)pGuid );

        // Advance to next provider
        pdnSPInfoEnum++;
    }

    // Add "Wait for Lobby Connection" selection in list box
    SendMessage( hWndListBox, LB_ADDSTRING, 0,
                 (LPARAM) TEXT("Wait for Lobby Connection") );

    SetFocus( hWndListBox );

    // Try to select the default preferred provider
    iLBIndex = (int)SendMessage( hWndListBox, LB_FINDSTRINGEXACT, (WPARAM)-1,
                                (LPARAM)m_strPreferredProvider );
    if( iLBIndex != LB_ERR )
        SendMessage( hWndListBox, LB_SETCURSEL, iLBIndex, 0 );
    else
        SendMessage( hWndListBox, LB_SETCURSEL, 0, 0 );


    hr = S_OK;

LCleanReturn:
    SAFE_DELETE_ARRAY( pdnSPInfo );

    return hr;
}




//-----------------------------------------------------------------------------
// Name: ConnectionsDlgOnOK()
// Desc: Stores the player name m_strPlayerName, and in creates a IDirectPlay
//       object based on the connection type the user selected.
//-----------------------------------------------------------------------------
HRESULT CNetConnectWizard::ConnectionsDlgOnOK( HWND hDlg )
{
    LRESULT iIndex;
    HRESULT hr;

    GetDlgItemText( hDlg, IDC_PLAYER_NAME_EDIT, m_strLocalPlayerName, MAX_PATH );

    if( _tcslen( m_strLocalPlayerName ) == 0 )
    {
        MessageBox( hDlg, TEXT("You must enter a valid player name."),
                    TEXT("DirectPlay Sample"), MB_OK );
        return S_OK;
    }

    HWND hWndListBox = GetDlgItem( hDlg, IDC_CONNECTION_LIST );

    iIndex = SendMessage( hWndListBox, LB_GETCURSEL, 0, 0 );
    SendMessage( hWndListBox, LB_GETTEXT, iIndex, (LPARAM)m_strPreferredProvider );

    GUID* pGuid = (GUID*) SendMessage( hWndListBox, LB_GETITEMDATA, iIndex, 0 );
    if( NULL == pGuid )
    {
        // 'Wait for lobby launch' SP has been selected, so wait for a connection
        if( FAILED( hr = m_pLobbiedApp->SetAppAvailable( TRUE, 0 ) ) )
            return DXTRACE_ERR_MSGBOX( TEXT("SetAppAvailable"), hr );

        // Display the multiplayer connect dialog box.
        DialogBox( m_hInst, MAKEINTRESOURCE(IDD_LOBBY_WAIT_STATUS),
                   hDlg, (DLGPROC) StaticLobbyWaitDlgProc );

        if( m_bHaveConnectionSettingsFromLobby )
        {
            if( FAILED( hr = ConnectUsingLobbySettings() ) )
                return DXTRACE_ERR_MSGBOX( TEXT("ConnectUsingLobbySettings"), hr );

            return NCW_S_LOBBYCONNECT;
        }

        // 'Wait for lobby launch' was canceled, so don't wait for a connection anymore
        if( FAILED( hr = m_pLobbiedApp->SetAppAvailable( FALSE, 0 ) ) )
            return DXTRACE_ERR_MSGBOX( TEXT("SetAppAvailable"), hr );

        return S_OK;
    }

    // Query for the enum host timeout for this SP
    DPN_SP_CAPS dpspCaps;
    ZeroMemory( &dpspCaps, sizeof(DPN_SP_CAPS) );
    dpspCaps.dwSize = sizeof(DPN_SP_CAPS);
    if( FAILED( hr = m_pDP->GetSPCaps( pGuid, &dpspCaps, 0 ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("GetSPCaps"), hr );

    // Set the host expire time to around 3 times
    // length of the dwDefaultEnumRetryInterval
    m_dwEnumHostExpireInterval = dpspCaps.dwDefaultEnumRetryInterval * 3;

    m_guidSP = *pGuid;

    // The SP has been chosen, so move forward in the wizard
    m_hrDialog = NCW_S_FORWARD;
    EndDialog( hDlg, 0 );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ConnectionsDlgCleanup()
// Desc: Deletes the connection buffers from the listbox
//-----------------------------------------------------------------------------
VOID CNetConnectWizard::ConnectionsDlgCleanup( HWND hDlg )
{
    GUID*   pGuid = NULL;
    DWORD   iIndex;
    DWORD   dwCount;

    HWND hWndListBox = GetDlgItem( hDlg, IDC_CONNECTION_LIST );

    dwCount = (DWORD)SendMessage( hWndListBox, LB_GETCOUNT, 0, 0 );
    for( iIndex = 0; iIndex < dwCount; iIndex++ )
    {
        pGuid = (GUID*) SendMessage( hWndListBox, LB_GETITEMDATA,
                                     iIndex, 0 );
        SAFE_DELETE( pGuid );
    }
}




//-----------------------------------------------------------------------------
// Name: StaticSessionsDlgProc()
// Desc: Static msg handler which passes messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CNetConnectWizard::StaticSessionsDlgProc( HWND hDlg, UINT uMsg,
                                                           WPARAM wParam, LPARAM lParam )
{
    if( g_pNCW )
        return g_pNCW->SessionsDlgProc( hDlg, uMsg, wParam, lParam );

    return FALSE; // Message not handled
}




//-----------------------------------------------------------------------------
// Name: SessionsDlgProc()
// Desc: Handles messages fro the multiplayer games dialog
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CNetConnectWizard::SessionsDlgProc( HWND hDlg, UINT msg,
                                                     WPARAM wParam, LPARAM lParam )
{
    UNREFERENCED_PARAMETER( lParam );
    HRESULT hr;

    switch( msg )
    {
        case WM_INITDIALOG:
            {
#if defined(WIN32_PLATFORM_PSPC) && (_WIN32_WCE >= 300)
                SHINITDLGINFO   shidi;
                memset(&shidi, 0, sizeof(SHINITDLGINFO));
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
                shidi.hDlg = hDlg;

                SetForegroundWindow(hDlg);
                SHInitDialog(&shidi);
#endif // WIN32_PLATFORM_PSPC

                m_hDlg = hDlg;

                // Load and set the icon
                HICON hIcon = LoadIcon( m_hInst, MAKEINTRESOURCE( IDI_MAIN ) );
                SendMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
                SendMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

                // Set the window title
                TCHAR strWindowTitle[256];
                wsprintf( strWindowTitle, TEXT("%s - Multiplayer Games"), m_strAppName );
                SetWindowText( hDlg, strWindowTitle );

                // Init the search portion of the dialog
                m_bSearchingForSessions = FALSE;
                
                // Check to see if a former search is still waiting to end
                if(m_hEnumAsyncOp != NULL)
                {
                    EnableWindow( GetDlgItem(hDlg, IDC_SEARCH_CHECK), FALSE );
                    SetDlgItemText( hDlg, IDC_SEARCH_CHECK, TEXT("Stopping...") );
                }

                SessionsDlgInitListbox( hDlg );
            }
            break;

        case WM_TIMER:
            // Upon this timer message, then refresh the list of hosts
            // by expiring old hosts, and displaying the list in the
            // dialog box
            if( wParam == TIMERID_DISPLAY_HOSTS )
            {
                // Don't refresh if we are not enumerating hosts
                if( !m_bSearchingForSessions )
                    break;

                // Expire all of the hosts that haven't
                // refreshed in a certain period of time
                SessionsDlgExpireOldHostEnums();

                // Display the list of hosts in the dialog
                if( FAILED( hr = SessionsDlgDisplayEnumList( hDlg ) ) )
                {
                    DXTRACE_ERR_MSGBOX( TEXT("SessionsDlgDisplayEnumList"), hr );
                    KillTimer( hDlg, TIMERID_DISPLAY_HOSTS );
                    MessageBox( hDlg, TEXT("Error enumerating DirectPlay games."),
                                TEXT("DirectPlay Sample"),
                                MB_OK | MB_ICONERROR );
                }
            }
            else if( wParam == TIMERID_CONNECT_COMPLETE )
            {
                // Check to see if the MessageHandler has set an event to tell us the
                // DPN_MSGID_CONNECT_COMPLETE has been processed.  Now m_hrConnectComplete
                // is valid.
                if( WAIT_OBJECT_0 == WaitForSingleObject( m_hConnectCompleteEvent, 0 ) )
                {
                    m_bConnecting = FALSE;

                    // Re-enable create button
                    EnableWindow( GetDlgItem( hDlg, IDC_CREATE ), TRUE );

                    if( FAILED( m_hrConnectComplete ) )
                    {
                        DXTRACE_ERR_MSGBOX( TEXT("DPN_MSGID_CONNECT_COMPLETE"), m_hrConnectComplete );
                        MessageBox( m_hDlg, TEXT("Unable to join game."),
                                    TEXT("DirectPlay Sample"),
                                    MB_OK | MB_ICONERROR );
                    }
                    else
                    {
                        // DirectPlay connect successful, so end dialog
                        m_hrDialog = NCW_S_FORWARD;
                        EndDialog( m_hDlg, 0 );
                    }

                    KillTimer( hDlg, TIMERID_CONNECT_COMPLETE );
                }
            }

            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDC_SEARCH_CHECK:
                    m_bSearchingForSessions = !m_bSearchingForSessions;

                    if( m_bSearchingForSessions )
                    {
                        // Ask the user for the remote address
                        if( SPRequiresPort( &m_guidSP ) )
                        {
                            int nResult = (int)DialogBox( m_hInst, MAKEINTRESOURCE(IDD_MULTIPLAYER_ADDRESS),
                                                          hDlg, (DLGPROC) StaticAddressDlgProc );
                    
                            // If the user cancelled the remote address dialog box, 
                            // don't start the search
                            if( nResult == IDCANCEL )
                            {
                                m_bSearchingForSessions = FALSE;
                                break;
                            }
                        }

                        SetDlgItemText( hDlg, IDC_SEARCH_CHECK, TEXT("Searching...") );

                        // Start the timer to display the host list every so often
                        SetTimer( hDlg, TIMERID_DISPLAY_HOSTS, DISPLAY_REFRESH_RATE, NULL );

                        // Start the async enumeration
                        if( FAILED( hr = SessionsDlgEnumHosts( hDlg ) ) )
                        {
                            DXTRACE_ERR_MSGBOX( TEXT("SessionsDlgEnumHosts"), hr );
                            KillTimer( hDlg, TIMERID_DISPLAY_HOSTS );
                            MessageBox( hDlg, TEXT("Error enumerating DirectPlay games."),
                                        TEXT("DirectPlay Sample"),
                                        MB_OK | MB_ICONERROR );
                        }
                    }
                    else
                    {
                        SessionsDlgStopEnumHosts( hDlg );
                    }

                    break;

                case IDC_GAMES_LIST:
                    if( HIWORD(wParam) != LBN_DBLCLK )
                        break;
                    // Fall through

                case IDC_JOIN:
                    if( FAILED( hr = SessionsDlgJoinGame( hDlg ) ) )
                    {
                        DXTRACE_ERR_MSGBOX( TEXT("SessionsDlgJoinGame"), hr );
                        MessageBox( hDlg, TEXT("Unable to join game."),
                                    TEXT("DirectPlay Sample"),
                                    MB_OK | MB_ICONERROR );
                    }
                    break;

                case IDC_CREATE:
                    if( FAILED( hr = SessionsDlgCreateGame( hDlg ) ) )
                    {
                        DXTRACE_ERR_MSGBOX( TEXT("SessionsDlgCreateGame"), hr );
                        MessageBox( hDlg, TEXT("Unable to create game."),
                                    TEXT("DirectPlay Sample"),
                                    MB_OK | MB_ICONERROR );
                    }
                    break;

                case IDCANCEL: // The close button was press
                    m_hrDialog = NCW_S_QUIT;
                    EndDialog(hDlg, 0);
                    break;

                case IDC_BACK: // Cancel button was pressed
                    m_hrDialog = NCW_S_BACKUP;
                    EndDialog(hDlg, 0);
                    break;

                default:
                    return FALSE; // Message not handled
            }
            break;
        
            case WM_DESTROY:
                SessionsDlgStopEnumHosts(hDlg);
                break;

        default:
            return FALSE; // Message not handled
    }

    // Message was handled
    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: SessionsDlgInitListbox()
// Desc: Initializes the listbox
//-----------------------------------------------------------------------------
VOID CNetConnectWizard::SessionsDlgInitListbox( HWND hDlg )
{
    HWND hWndListBox = GetDlgItem( hDlg, IDC_GAMES_LIST );

    // Clear the contents from the list box, and
    // display "Looking for games" text in listbox
    SendMessage( hWndListBox, LB_RESETCONTENT, 0, 0 );
    if( m_bSearchingForSessions )
    {
        SendMessage( hWndListBox, LB_ADDSTRING, 0,
                     (LPARAM) TEXT("Looking for games...") );
    }
    else
    {
        SendMessage( hWndListBox, LB_ADDSTRING, 0,
                     (LPARAM) TEXT("Click Start Search to see a list of games."));
        SendMessage( hWndListBox, LB_ADDSTRING, 0,
                     (LPARAM) TEXT("Click Create to start a new game.") );
    }

    SendMessage( hWndListBox, LB_SETITEMDATA,  0, NULL );
    SendMessage( hWndListBox, LB_SETCURSEL,    0, 0 );

    // Disable the join button until sessions are found
    EnableWindow( GetDlgItem( hDlg, IDC_JOIN ), FALSE );
}




//-----------------------------------------------------------------------------
// Name: SessionsDlgEnumHosts()
// Desc: Enumerates the DirectPlay sessions, and displays them in the listbox
//-----------------------------------------------------------------------------
HRESULT CNetConnectWizard::SessionsDlgEnumHosts( HWND hDlg )
{
    UNREFERENCED_PARAMETER( hDlg );
    HRESULT hr = S_OK;

    m_bEnumListChanged = TRUE;

    IDirectPlay8Address*   pDP8AddressHost  = NULL;
    IDirectPlay8Address*   pDP8AddressLocal = NULL;
    WCHAR*                 wszHostName      = NULL;

    // Create the local device address object
    if( FAILED( hr = CoCreateInstance( CLSID_DirectPlay8Address, NULL, 
                                       CLSCTX_ALL, IID_IDirectPlay8Address,
                                       (LPVOID*) &pDP8AddressLocal ) ) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("CoCreateInstance"), hr );
        goto LCleanup;
    }

    // Set local service provider
    if( FAILED( hr = pDP8AddressLocal->SetSP( &m_guidSP ) ) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("SetSP"), hr );
        goto LCleanup;
    }

    // Create the remote host address object
    if( FAILED( hr = CoCreateInstance( CLSID_DirectPlay8Address, NULL, 
                                       CLSCTX_ALL, IID_IDirectPlay8Address,
                                       (LPVOID*) &pDP8AddressHost ) ) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("CoCreateInstance"), hr );
        goto LCleanup;
    }

    // Set remote service provider
    if( FAILED( hr = pDP8AddressHost->SetSP( &m_guidSP ) ) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("SetSP"), hr );
        goto LCleanup;
    }

    // If we're using a TCP/IP (including network simulator) or IPX
    // service provider, the user was given an option for hostname and
    // port before the search started. 
    if( SPRequiresPort( &m_guidSP ) ) 
    {
        // Add the hostname. If this is blank, DirectPlay will attempt
        // to search the local network.
        if( _tcscmp(m_strHostname, TEXT("")) != 0 )
        {
            WCHAR wszHostName[MAX_PATH];
            DXUtil_ConvertGenericStringToWideCch( wszHostName, m_strHostname, MAX_PATH );
            hr = pDP8AddressHost->AddComponent( DPNA_KEY_HOSTNAME, wszHostName, 
                                                (DWORD) (wcslen(wszHostName)+1)*sizeof(WCHAR), 
                                                DPNA_DATATYPE_STRING );
            if( FAILED(hr) )
            {
                DXTRACE_ERR_MSGBOX( TEXT("AddComponent"), hr );
                goto LCleanup;
            }
        }

        // Add the requested port value. The port value is required in order to
        // receive any search hits if DPNSVR isn't running on the remote machine.
        // Games will typically hard code the port so the user need not know it
        if( m_dwPort != 0 )
        {
            hr = pDP8AddressHost->AddComponent( DPNA_KEY_PORT, 
                                                &m_dwPort, sizeof(m_dwPort),
                                                DPNA_DATATYPE_DWORD );
            if( FAILED(hr) )
            {
                DXTRACE_ERR_MSGBOX( TEXT("AddComponent"), hr );
                goto LCleanup;
            }
        }
    }

    // Enumerate hosts
    DPN_APPLICATION_DESC    dnAppDesc;
    ZeroMemory( &dnAppDesc, sizeof(DPN_APPLICATION_DESC) );
    dnAppDesc.dwSize          = sizeof(DPN_APPLICATION_DESC);
    dnAppDesc.guidApplication = m_guidApp;

    DWORD dwFlags;
    dwFlags = 0;

    // For certain service providers the user has not been
    // asked to fill in the components of the remote address,
    // so DirectPlay should ask for required fields.
    if( !SPRequiresPort( &m_guidSP ) )
    {
        dwFlags = DPNENUMHOSTS_OKTOQUERYFORADDRESSING;
    }

    // Enumerate all the active DirectPlay games on the selected connection
    hr = m_pDP->EnumHosts( &dnAppDesc,                            // application description
                           pDP8AddressHost,                       // host address
                           pDP8AddressLocal,                      // device address
                           NULL,                                  // pointer to user data
                           0,                                     // user data size
                           INFINITE,                              // retry count (forever)
                           0,                                     // retry interval (0=default)
                           INFINITE,                              // time out (forever)
                           NULL,                                  // user context
                           &m_hEnumAsyncOp,                       // async handle
                           dwFlags                                // flags
                           );
    if( FAILED(hr) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("EnumHosts"), hr );
        goto LCleanup;
    }

LCleanup:
    SAFE_RELEASE( pDP8AddressHost);
    SAFE_RELEASE( pDP8AddressLocal );
    SAFE_DELETE( wszHostName );

    return hr;
}




//-----------------------------------------------------------------------------
// Name: SessionsDlgStopEnumHosts()
// Desc: Stops the running session enumeration
//-----------------------------------------------------------------------------
VOID CNetConnectWizard::SessionsDlgStopEnumHosts( HWND hDlg )
{
    HRESULT hr;

    // Update the UI 
    EnableWindow( GetDlgItem( hDlg, IDC_SEARCH_CHECK ), FALSE );
    SetDlgItemText( hDlg, IDC_SEARCH_CHECK, TEXT("Stopping...") );

    // Stop the timer, and stop the async enumeration
    KillTimer( hDlg, TIMERID_DISPLAY_HOSTS );

    // Until the CancelAsyncOperation returns, it is possible
    // to still receive host enumerations. Instruct DirectPlay to
    // stop the current enumeration and handle the rest of the cleanup
    // when receiving the ASYNC_OP_COMPLETE message. If this method
    // fails we should assume there's no active enumeration and call
    // the finalization method directly.
    if( m_hEnumAsyncOp )
    {
        hr = m_pDP->CancelAsyncOperation( m_hEnumAsyncOp, 0 );
        if( FAILED(hr) )
            SessionsDlgFinalizeEnumHosts( hDlg );
    }
    else
    {
        SessionsDlgFinalizeEnumHosts( hDlg );
    }
}




//-----------------------------------------------------------------------------
// Name: SessionsDlgFinalizeEnumHosts()
// Desc: This method should be called when we receive confirmation from 
//       DirectPlay that the enumeration has completed. It reset the search
//       UI and state variables, and enables the search button.
//-----------------------------------------------------------------------------
VOID CNetConnectWizard::SessionsDlgFinalizeEnumHosts( HWND hDlg )
{
    // Clear the data list
    SessionsDlgEnumListCleanup();

    // Reset the search state variables
    m_hEnumAsyncOp = NULL;
    m_bSearchingForSessions = FALSE;
    
    // Reset the search portion of the dialog
    SessionsDlgInitListbox( hDlg ); 
    CheckDlgButton( hDlg, IDC_SEARCH_CHECK, BST_UNCHECKED );
    SetDlgItemText( hDlg, IDC_SEARCH_CHECK, TEXT("Start &Search") );
    EnableWindow( GetDlgItem( hDlg, IDC_SEARCH_CHECK ), TRUE );
}




//-----------------------------------------------------------------------------
// Name: SessionsDlgNoteEnumResponse()
// Desc: Stores them in the linked list, m_DPHostEnumHead.  This is
//       called from the DirectPlay message handler so it could be
//       called simultaneously from multiple threads.
//-----------------------------------------------------------------------------
HRESULT CNetConnectWizard::SessionsDlgNoteEnumResponse( PDPNMSG_ENUM_HOSTS_RESPONSE pEnumHostsResponseMsg )
{
    HRESULT hr = S_OK;
    BOOL    bFound;

    // This function is called from the DirectPlay message handler so it could be
    // called simultaneously from multiple threads, so enter a critical section
    // to assure that it we don't get race conditions.  Locking the entire
    // function is crude, and could be more optimal but is effective for this
    // simple sample
    EnterCriticalSection( &m_csHostEnum );

    DPHostEnumInfo* pDPHostEnum          = m_DPHostEnumHead.pNext;
    DPHostEnumInfo* pDPHostEnumNext      = NULL;
    const DPN_APPLICATION_DESC* pResponseMsgAppDesc =
                            pEnumHostsResponseMsg->pApplicationDescription;

    // Look for a matching session instance GUID.
    bFound = FALSE;
    while ( pDPHostEnum != &m_DPHostEnumHead )
    {
        if( pResponseMsgAppDesc->guidInstance == pDPHostEnum->pAppDesc->guidInstance )
        {
            bFound = TRUE;
            break;
        }

        pDPHostEnumNext = pDPHostEnum;
        pDPHostEnum = pDPHostEnum->pNext;
    }

    if( !bFound )
    {
        m_bEnumListChanged = TRUE;

        // If there's no match, then look for invalid session and use it
        pDPHostEnum = m_DPHostEnumHead.pNext;
        while ( pDPHostEnum != &m_DPHostEnumHead )
        {
            if( !pDPHostEnum->bValid )
                break;

            pDPHostEnum = pDPHostEnum->pNext;
        }

        // If no invalid sessions are found then make a new one
        if( pDPHostEnum == &m_DPHostEnumHead )
        {
            // Found a new session, so create a new node
            pDPHostEnum = new DPHostEnumInfo;
            if( NULL == pDPHostEnum )
            {
                hr = E_OUTOFMEMORY;
                goto LCleanup;
            }

            ZeroMemory( pDPHostEnum, sizeof(DPHostEnumInfo) );

            // Add pDPHostEnum to the circular linked list, m_DPHostEnumHead
            pDPHostEnum->pNext = m_DPHostEnumHead.pNext;
            m_DPHostEnumHead.pNext = pDPHostEnum;
        }
    }

    // Update the pDPHostEnum with new information
    TCHAR strName[MAX_PATH];
    if( pResponseMsgAppDesc->pwszSessionName )
    {
        DXUtil_ConvertWideStringToGenericCch( strName, pResponseMsgAppDesc->pwszSessionName, MAX_PATH );
    }

    // Cleanup any old enum
    if( pDPHostEnum->pAppDesc )
    {
        SAFE_DELETE_ARRAY( pDPHostEnum->pAppDesc->pwszSessionName );
        SAFE_DELETE( pDPHostEnum->pAppDesc );
    }
    SAFE_RELEASE( pDPHostEnum->pHostAddr );
    SAFE_RELEASE( pDPHostEnum->pDeviceAddr );

    //
    // Duplicate pEnumHostsResponseMsg->pAddressSender in pDPHostEnum->pHostAddr.
    // Duplicate pEnumHostsResponseMsg->pAddressDevice in pDPHostEnum->pDeviceAddr.
    //
    if( FAILED( hr = pEnumHostsResponseMsg->pAddressSender->Duplicate( &pDPHostEnum->pHostAddr ) ) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("Duplicate"), hr );
        goto LCleanup;
    }

    if( FAILED( hr = pEnumHostsResponseMsg->pAddressDevice->Duplicate( &pDPHostEnum->pDeviceAddr ) ) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("Duplicate"), hr );
        goto LCleanup;
    }

    // Deep copy the DPN_APPLICATION_DESC from
    pDPHostEnum->pAppDesc = new DPN_APPLICATION_DESC;
    if( NULL == pDPHostEnum->pAppDesc )
    {
        hr = E_OUTOFMEMORY;
        DXTRACE_ERR_MSGBOX( TEXT("SessionsDlgNoteEnumResponse"), hr );
        goto LCleanup;
    }

    ZeroMemory( pDPHostEnum->pAppDesc, sizeof(DPN_APPLICATION_DESC) );
    memcpy( pDPHostEnum->pAppDesc, pResponseMsgAppDesc, sizeof(DPN_APPLICATION_DESC) );
    if( pResponseMsgAppDesc->pwszSessionName )
    {
        pDPHostEnum->pAppDesc->pwszSessionName = new WCHAR[ wcslen(pResponseMsgAppDesc->pwszSessionName)+1 ];
        wcscpy( pDPHostEnum->pAppDesc->pwszSessionName,
                pResponseMsgAppDesc->pwszSessionName );
    }

    // Update the time this was done, so that we can expire this host
    // if it doesn't refresh w/in a certain amount of time
    pDPHostEnum->dwLastPollTime = GETTIMESTAMP();

    // Check to see if the current number of players changed
    TCHAR szSessionTemp[MAX_PATH];
    if( pResponseMsgAppDesc->dwMaxPlayers > 0 )
    {
        _sntprintf( szSessionTemp, MAX_PATH-1, TEXT("%s (%d/%d) (%dms)"), strName,
                    pResponseMsgAppDesc->dwCurrentPlayers,
                    pResponseMsgAppDesc->dwMaxPlayers,
                    pEnumHostsResponseMsg->dwRoundTripLatencyMS );

        // Null terminate
        szSessionTemp[ MAX_PATH-1 ] = 0;
    }
    else
    {
        _sntprintf( szSessionTemp, MAX_PATH-1, TEXT("%s (%d) (%dms)"), strName,
                    pResponseMsgAppDesc->dwCurrentPlayers,
                    pEnumHostsResponseMsg->dwRoundTripLatencyMS );

        // Null terminate
        szSessionTemp[ MAX_PATH-1 ] = 0;
    }

    // if this node was previously invalidated, or the session name is now
    // different the session list in the dialog needs to be updated
    if( ( pDPHostEnum->bValid == FALSE ) ||
        ( _tcscmp( pDPHostEnum->szSession, szSessionTemp ) != 0 ) )
    {
        m_bEnumListChanged = TRUE;
    }
    _tcscpy( pDPHostEnum->szSession, szSessionTemp );

    // This host is now valid
    pDPHostEnum->bValid = TRUE;

LCleanup:
    LeaveCriticalSection( &m_csHostEnum );

    return hr;
}




//-----------------------------------------------------------------------------
// Name: SessionsDlgExpireOldHostEnums
// Desc: Check all nodes to see if any have expired yet.
//-----------------------------------------------------------------------------
VOID CNetConnectWizard::SessionsDlgExpireOldHostEnums()
{
    DWORD dwCurrentTime = GETTIMESTAMP();

    // This is called from the dialog UI thread, SessionsDlgNoteEnumResponse
    // is called from the DirectPlay message handler threads so
    // they may also be inside it at this time, so we need to go into the
    // critical section first
    EnterCriticalSection( &m_csHostEnum );

    DPHostEnumInfo* pDPHostEnum = m_DPHostEnumHead.pNext;
    while ( pDPHostEnum != &m_DPHostEnumHead )
    {
        // Check the poll time to expire stale entries.  Also check to see if
        // the entry is already invalid.  If so, don't note that the enum list
        // changed because that causes the list in the dialog to constantly redraw.
        if( ( pDPHostEnum->bValid != FALSE ) &&
            ( pDPHostEnum->dwLastPollTime < dwCurrentTime - m_dwEnumHostExpireInterval ) )
        {
            // This node has expired, so invalidate it.
            pDPHostEnum->bValid = FALSE;
            m_bEnumListChanged  = TRUE;
        }

        pDPHostEnum = pDPHostEnum->pNext;
    }

    LeaveCriticalSection( &m_csHostEnum );
}




//-----------------------------------------------------------------------------
// Name: SessionsDlgDisplayEnumList
// Desc: Display the list of hosts in the dialog box
//-----------------------------------------------------------------------------
HRESULT CNetConnectWizard::SessionsDlgDisplayEnumList( HWND hDlg )
{
    HWND           hWndListBox   = GetDlgItem( hDlg, IDC_GAMES_LIST );
    DPHostEnumInfo* pDPHostEnumSelected = NULL;
    GUID           guidSelectedInstance;
    BOOL           bFindSelectedGUID;
    BOOL           bFoundSelectedGUID;
    int            nItemSelected;

    // This is called from the dialog UI thread, SessionsDlgNoteEnumResponse
    // is called from the DirectPlay message handler threads so
    // they may also be inside it at this time, so we need to go into the
    // critical section first
    EnterCriticalSection( &m_csHostEnum );

    // Only update the display list if it has changed since last time
    if( !m_bEnumListChanged )
    {
        LeaveCriticalSection( &m_csHostEnum );
        return S_OK;
    }

    m_bEnumListChanged = FALSE;

    bFindSelectedGUID  = FALSE;
    bFoundSelectedGUID = FALSE;

    // Try to keep the same session selected unless it goes away or
    // there is no real session currently selected
    nItemSelected = (int)SendMessage( hWndListBox, LB_GETCURSEL, 0, 0 );
    if( nItemSelected != LB_ERR )
    {
        pDPHostEnumSelected = (DPHostEnumInfo*) SendMessage( hWndListBox, LB_GETITEMDATA,
                                                             nItemSelected, 0 );
        if( pDPHostEnumSelected != NULL && pDPHostEnumSelected->bValid )
        {
            guidSelectedInstance = pDPHostEnumSelected->pAppDesc->guidInstance;
            bFindSelectedGUID = TRUE;
        }
    }

    // Tell listbox not to redraw itself since the contents are going to change
    SendMessage( hWndListBox, WM_SETREDRAW, FALSE, 0 );

    // Test to see if any sessions exist in the linked list
    DPHostEnumInfo* pDPHostEnum = m_DPHostEnumHead.pNext;
    while ( pDPHostEnum != &m_DPHostEnumHead )
    {
        if( pDPHostEnum->bValid )
            break;
        pDPHostEnum = pDPHostEnum->pNext;
    }

    // If there are any sessions in list,
    // then add them to the listbox
    if( pDPHostEnum != &m_DPHostEnumHead )
    {
        // Clear the contents from the list box and enable the join button
        SendMessage( hWndListBox, LB_RESETCONTENT, 0, 0 );

        // Enable the join button only if not already connecting to a game
        if( !m_bConnecting )
            EnableWindow( GetDlgItem( hDlg, IDC_JOIN ), TRUE );

        pDPHostEnum = m_DPHostEnumHead.pNext;
        while ( pDPHostEnum != &m_DPHostEnumHead )
        {
            // Add host to list box if it is valid
            if( pDPHostEnum->bValid )
            {
                int nIndex = (int)SendMessage( hWndListBox, LB_ADDSTRING, 0,
                                               (LPARAM)pDPHostEnum->szSession );
                SendMessage( hWndListBox, LB_SETITEMDATA, nIndex, (LPARAM)pDPHostEnum );

                if( bFindSelectedGUID )
                {
                    // Look for the session the was selected before
                    if( pDPHostEnum->pAppDesc->guidInstance == guidSelectedInstance )
                    {
                        SendMessage( hWndListBox, LB_SETCURSEL, nIndex, 0 );
                        bFoundSelectedGUID = TRUE;
                    }
                }
            }

            pDPHostEnum = pDPHostEnum->pNext;
        }

        if( !bFindSelectedGUID || !bFoundSelectedGUID )
            SendMessage( hWndListBox, LB_SETCURSEL, 0, 0 );
    }
    else
    {
        // There are no active session, so just reset the listbox
        SessionsDlgInitListbox( hDlg );
    }

    // Tell listbox to redraw itself now since the contents have changed
    SendMessage( hWndListBox, WM_SETREDRAW, TRUE, 0 );
    InvalidateRect( hWndListBox, NULL, FALSE );

    LeaveCriticalSection( &m_csHostEnum );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SessionsDlgJoinGame()
// Desc: Joins the selected DirectPlay session
//-----------------------------------------------------------------------------
HRESULT CNetConnectWizard::SessionsDlgJoinGame( HWND hDlg )
{
    HRESULT         hr;
    HWND            hWndListBox = GetDlgItem( hDlg, IDC_GAMES_LIST );
    DPHostEnumInfo* pDPHostEnumSelected = NULL;
    int             nItemSelected;

    m_bHostPlayer = FALSE;

    // Add status text in list box
    nItemSelected = (int)SendMessage( hWndListBox, LB_GETCURSEL, 0, 0 );

    EnterCriticalSection( &m_csHostEnum );

    pDPHostEnumSelected = (DPHostEnumInfo*) SendMessage( hWndListBox, LB_GETITEMDATA,
                                                         nItemSelected, 0 );

    if( NULL == pDPHostEnumSelected )
    {
        MessageBox( hDlg, TEXT("There are no games to join."),
                    TEXT("DirectPlay Sample"), MB_OK );
        hr = S_OK;
        goto LCleanReturn;
    }

    m_bConnecting = TRUE;

    // Set the peer info
    WCHAR wszPeerName[MAX_PATH];
    DXUtil_ConvertGenericStringToWideCch( wszPeerName, m_strLocalPlayerName, MAX_PATH );

    DPN_PLAYER_INFO dpPlayerInfo;
    ZeroMemory( &dpPlayerInfo, sizeof(DPN_PLAYER_INFO) );
    dpPlayerInfo.dwSize = sizeof(DPN_PLAYER_INFO);
    dpPlayerInfo.dwInfoFlags = DPNINFO_NAME;
    dpPlayerInfo.pwszName = wszPeerName;

    // Set the peer info, and use the DPNOP_SYNC since by default this
    // is an async call.  If it is not DPNOP_SYNC, then the peer info may not
    // be set by the time we call Connect() below.
    if( FAILED( hr = m_pDP->SetPeerInfo( &dpPlayerInfo, NULL, NULL, DPNOP_SYNC ) ) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("SetPeerInfo"), hr );
        goto LCleanReturn;
    }

    ResetEvent( m_hConnectCompleteEvent );

    // Connect to an existing session. DPNCONNECT_OKTOQUERYFORADDRESSING allows
    // DirectPlay to prompt the user using a dialog box for any device address
    // or host address information that is missing
    // We also pass in copies of the app desc and host addr, since pDPHostEnumSelected
    // might be deleted from another thread that calls SessionsDlgExpireOldHostEnums().
    // This process could also be done using reference counting instead.
    hr = m_pDP->Connect( pDPHostEnumSelected->pAppDesc,       // the application desc
                         pDPHostEnumSelected->pHostAddr,      // address of the host of the session
                         pDPHostEnumSelected->pDeviceAddr,    // address of the local device the enum responses were received on
                         NULL, NULL,                          // DPN_SECURITY_DESC, DPN_SECURITY_CREDENTIALS
                         NULL, 0,                             // user data, user data size
                         NULL,                                // player context,
                         NULL, &m_hConnectAsyncOp,            // async context, async handle,
                         DPNCONNECT_OKTOQUERYFORADDRESSING ); // flags
    if( FAILED(hr) && hr != E_PENDING )
    {
        DXTRACE_ERR_MSGBOX( TEXT("Connect"), hr );
        goto LCleanReturn;
    }

    // Set a timer to wait for m_hConnectCompleteEvent to be signaled.
    // This will tell us when DPN_MSGID_CONNECT_COMPLETE has been processed
    // which lets us know if the connect was successful or not.
    SetTimer( hDlg, TIMERID_CONNECT_COMPLETE, 100, NULL );

    // Disable the create/join buttons until connect succeeds or fails
    EnableWindow( GetDlgItem( hDlg, IDC_JOIN ), FALSE );
    EnableWindow( GetDlgItem( hDlg, IDC_CREATE ), FALSE );

    hr = S_OK;

LCleanReturn:
    LeaveCriticalSection( &m_csHostEnum );

    return hr;
}




//-----------------------------------------------------------------------------
// Name: SessionsDlgCreateGame()
// Desc: Asks the user the session name, and creates a new DirectPlay session
//-----------------------------------------------------------------------------
HRESULT CNetConnectWizard::SessionsDlgCreateGame( HWND hDlg )
{
    HRESULT hr = S_OK;
    int     nResult;

    // Display a modal multiplayer connect dialog box.
    EnableWindow( hDlg, FALSE );
    nResult = (int)DialogBox( m_hInst, MAKEINTRESOURCE(IDD_MULTIPLAYER_CREATE),
                              hDlg, (DLGPROC) StaticCreateSessionDlgProc );
    EnableWindow( hDlg, TRUE );

    if( nResult == IDCANCEL )
        return S_OK;

    // Stop the search if we are about to connect
    if( m_bSearchingForSessions )
    {
        CheckDlgButton( m_hDlg, IDC_SEARCH_CHECK, BST_UNCHECKED );
        SendMessage( m_hDlg, WM_COMMAND, IDC_SEARCH_CHECK, 0 );
    }

    m_bHostPlayer = TRUE;

    IDirectPlay8Address*   pDP8AddressHost  = NULL;
    WCHAR*                 wszHostName      = NULL;

    // Create the local host address object
    if( FAILED( hr = CoCreateInstance( CLSID_DirectPlay8Address, NULL, 
                                       CLSCTX_ALL, IID_IDirectPlay8Address,
                                       (LPVOID*) &pDP8AddressHost ) ) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("CoCreateInstance"), hr );
        goto LCleanup;
    }

    // Set service provider
    if( FAILED( hr = pDP8AddressHost->SetSP( &m_guidSP ) ) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("SetSP"), hr );
        goto LCleanup;
    }

    // If were are using a service provider that requires a port value, 
    // and the user has requested a particular port number, set that
    // port as a component of the host address. If no port is specified, 
    // DirectPlay will automatically select an open port
    if( m_dwPort != 0 && SPRequiresPort( &m_guidSP ) )
    {
        hr = pDP8AddressHost->AddComponent( DPNA_KEY_PORT, 
                                            &m_dwPort, 
                                            sizeof(m_dwPort),
                                            DPNA_DATATYPE_DWORD );
        if( FAILED(hr) )
        {
            DXTRACE_ERR_MSGBOX( TEXT("AddComponent"), hr );
            goto LCleanup;
        }
    }

    // Set peer info name
    WCHAR wszPeerName[MAX_PATH];
    DXUtil_ConvertGenericStringToWideCch( wszPeerName, m_strLocalPlayerName, MAX_PATH );

    DPN_PLAYER_INFO dpPlayerInfo;
    ZeroMemory( &dpPlayerInfo, sizeof(DPN_PLAYER_INFO) );
    dpPlayerInfo.dwSize = sizeof(DPN_PLAYER_INFO);
    dpPlayerInfo.dwInfoFlags = DPNINFO_NAME;
    dpPlayerInfo.pwszName = wszPeerName;

    // Set the peer info, and use the DPNOP_SYNC since by default this
    // is an async call.  If it is not DPNOP_SYNC, then the peer info may not
    // be set by the time we call Host() below.
    if( FAILED( hr = m_pDP->SetPeerInfo( &dpPlayerInfo, NULL, NULL, DPNOP_SYNC ) ) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("SetPeerInfo"), hr );
        goto LCleanup;
    }

    WCHAR wszSessionName[MAX_PATH];
    DXUtil_ConvertGenericStringToWideCch( wszSessionName, m_strSessionName, MAX_PATH );

    // Setup the application desc
    DPN_APPLICATION_DESC dnAppDesc;
    ZeroMemory( &dnAppDesc, sizeof(DPN_APPLICATION_DESC) );
    dnAppDesc.dwSize          = sizeof(DPN_APPLICATION_DESC);
    dnAppDesc.guidApplication = m_guidApp;
    dnAppDesc.pwszSessionName = wszSessionName;
    dnAppDesc.dwMaxPlayers    = m_dwMaxPlayers;
    dnAppDesc.dwFlags         = 0;
    if( m_bMigrateHost )
        dnAppDesc.dwFlags |= DPNSESSION_MIGRATE_HOST;

    if( !m_bUseDPNSVR )
        dnAppDesc.dwFlags |= DPNSESSION_NODPNSVR;

    if( SIGN_FAST == m_eSigningType )
        dnAppDesc.dwFlags |= DPNSESSION_FAST_SIGNED;
    else if( SIGN_FULL == m_eSigningType )
        dnAppDesc.dwFlags |= DPNSESSION_FULL_SIGNED;

    
    // Host a game on m_pDeviceAddress as described by dnAppDesc
    // DPNHOST_OKTOQUERYFORADDRESSING allows DirectPlay to prompt the user
    // using a dialog box for any device address information that is missing
    if( FAILED( hr = m_pDP->Host( &dnAppDesc,               // the application desc
                                  &pDP8AddressHost,         // array of addresses of the local devices used to connect to the host
                                  1,                        // number in array
                                  NULL, NULL,               // DPN_SECURITY_DESC, DPN_SECURITY_CREDENTIALS
                                  NULL,                     // player context
                                  DPNHOST_OKTOQUERYFORADDRESSING ) ) ) // flags
    { 
        // This error is often caused by a port conflict
        if( hr == DPNERR_INVALIDDEVICEADDRESS && 
            m_dwPort != 0 && SPRequiresPort( &m_guidSP ) )
        {
            MessageBox( hDlg, TEXT("This error is often caused by a port conflict.\n\n")
                              TEXT("If another application is already using the port you specified,\n")
                              TEXT("try creating the game using a different port number."),
                        TEXT("Invalid Device Address"), MB_OK | MB_ICONINFORMATION );
        }

        DXTRACE_ERR_MSGBOX( TEXT("Host"), hr );
        goto LCleanup;
    }

    // DirectPlay connect successful, so end dialog
    m_hrDialog = NCW_S_FORWARD;
    EndDialog( hDlg, 0 );

LCleanup:
    SAFE_RELEASE( pDP8AddressHost );
    SAFE_DELETE( wszHostName );

    return hr;
}




//-----------------------------------------------------------------------------
// Name: StaticConnectionsDlgProc()
// Desc: Static msg handler which passes messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CNetConnectWizard::StaticCreateSessionDlgProc( HWND hDlg, UINT uMsg,
                                                                WPARAM wParam, LPARAM lParam )
{
    if( g_pNCW )
        return g_pNCW->CreateSessionDlgProc( hDlg, uMsg, wParam, lParam );

    return FALSE; // Message not handled
}




//-----------------------------------------------------------------------------
// Name: CreateSessionDlgProc()
// Desc: Handles messages fro the multiplayer create game dialog
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CNetConnectWizard::CreateSessionDlgProc( HWND hDlg, UINT msg,
                                                          WPARAM wParam, LPARAM lParam )
{
    UNREFERENCED_PARAMETER( lParam );
    DWORD dwNameLength;

    switch( msg )
    {
        case WM_INITDIALOG:
#if defined(WIN32_PLATFORM_PSPC) && (_WIN32_WCE >= 300)
            SHINITDLGINFO   shidi;
            memset(&shidi, 0, sizeof(SHINITDLGINFO));
            shidi.dwMask = SHIDIM_FLAGS;
            shidi.dwFlags = SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
            shidi.hDlg = hDlg;

            SetForegroundWindow(hDlg);
            SHInitDialog(&shidi);
#endif // WIN32_PLATFORM_PSPC

            SetDlgItemText( hDlg, IDC_EDIT_SESSION_NAME, m_strSessionName );
            CheckDlgButton( hDlg, IDC_MIGRATE_HOST, BST_CHECKED );
            CheckDlgButton( hDlg, IDC_SIGNING_FAST, BST_CHECKED );

            // Fill in the port value if the calling application gave us a default
            if( m_dwPort != 0 )
            {
                TCHAR strPort[40];
                _itot( m_dwPort, strPort, 10 );
                SetDlgItemText( hDlg, IDC_LOCAL_PORT, strPort );
            }

            // Hide the port field for service providers which don't use it
            if( !SPRequiresPort( &m_guidSP ) )
            {
                ShowWindow( GetDlgItem( hDlg, IDC_LOCAL_PORT ), SW_HIDE );
                ShowWindow( GetDlgItem( hDlg, IDC_LOCAL_PORT_TEXT ), SW_HIDE );
            }
            
            return TRUE;

// Context-sensitive help is not supported on PocketPC
#ifndef UNDER_CE
        case WM_HELP:
            LPHELPINFO lphi;
            lphi = (LPHELPINFO) lParam;

            switch( lphi->iCtrlId )
            {
                case IDC_EDIT_SESSION_NAME:
                    MessageBox( hDlg, TEXT("The name used to help other players identify\n")
                                      TEXT("your session across the network."), 
                                TEXT("Game Name"), MB_OK | MB_ICONQUESTION );
                    break;
                
                case IDC_SIGNING_FULL:
                case IDC_SIGNING_FAST:
                    MessageBox( hDlg, TEXT("The level of cryptographic security to use for\n")
                                      TEXT("network data; full-signing provides the most\n")
                                      TEXT("protection against spoofing at the cost of\n")
                                      TEXT("additional packet size."), 
                                TEXT("Session Signing"), MB_OK | MB_ICONQUESTION );
                    break;

                case IDC_MIGRATE_HOST:
                    MessageBox( hDlg, TEXT("If enabled, DirectPlay will automatically select\n")
                                      TEXT("a new session host if the current host exits."), 
                                TEXT("Host Migration"), MB_OK | MB_ICONQUESTION );
                    break;

                case IDC_USE_DPNSVR:
                    MessageBox( hDlg, TEXT("This DirectPlay-managed application accepts\n")
                                      TEXT("enumeration requests on a known port and forwards\n")
                                      TEXT("incoming requests to all sessions on the machine,\n")
                                      TEXT("which allows the querying player to find sessions\n")
                                      TEXT("without knowing the port address.\n\n")
                                      TEXT("This service may not work properly with certain\n")
                                      TEXT("NAT configurations."), 
                                TEXT("DPNSVR"), MB_OK | MB_ICONQUESTION );
                    break;

                case IDC_LOCAL_PORT:
                    MessageBox( hDlg, TEXT("Specifies the local port on which to host the new\n")
                                      TEXT("session. If set blank, DirectPlay will automatically\n")
                                      TEXT("select an open port."), 
                                TEXT("Local Port"), MB_OK | MB_ICONQUESTION );
                    break;

                default:
                    return FALSE; // Not handled
            }
            return TRUE;
#endif // !UNDER_CE

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDOK:
                    dwNameLength = GetDlgItemText( hDlg, IDC_EDIT_SESSION_NAME,
                                                   m_strSessionName,
                                                   MAX_PATH );
                    if( dwNameLength == 0 )
                        return TRUE; // Don't accept blank session names

                    m_bMigrateHost = ( IsDlgButtonChecked( hDlg,
                                       IDC_MIGRATE_HOST ) == BST_CHECKED );

                    m_bUseDPNSVR = ( IsDlgButtonChecked( hDlg,
                                     IDC_USE_DPNSVR ) == BST_CHECKED );

                    // Set the desired port value
                    TCHAR strPort[40];
                    GetDlgItemText( hDlg, IDC_LOCAL_PORT, strPort, 40 );
                    strPort[39] = 0;

                    m_dwPort = _ttoi( strPort );

                    // Set session signing options
                    if( BST_CHECKED == IsDlgButtonChecked( hDlg, IDC_SIGNING_FAST ) )
                        m_eSigningType = SIGN_FAST;
                    else if( BST_CHECKED == IsDlgButtonChecked( hDlg, IDC_SIGNING_FULL ) )
                        m_eSigningType = SIGN_FULL;
                    else
                        m_eSigningType = SIGN_NONE;

                    EndDialog( hDlg, IDOK );
                    return TRUE;

                case IDCANCEL:
                    EndDialog( hDlg, IDCANCEL );
                    return TRUE;
            }
            break;
    }

    return FALSE; // Didn't handle message
}




//-----------------------------------------------------------------------------
// Name: SessionsDlgEnumListCleanup()
// Desc: Deletes the linked list, g_DPHostEnumInfoHead
//-----------------------------------------------------------------------------
VOID CNetConnectWizard::SessionsDlgEnumListCleanup()
{
    DPHostEnumInfo* pDPHostEnum = m_DPHostEnumHead.pNext;
    DPHostEnumInfo* pDPHostEnumDelete;

    while ( pDPHostEnum != &m_DPHostEnumHead )
    {
        pDPHostEnumDelete = pDPHostEnum;
        pDPHostEnum = pDPHostEnum->pNext;

        if( pDPHostEnumDelete->pAppDesc )
        {
            SAFE_DELETE_ARRAY( pDPHostEnumDelete->pAppDesc->pwszSessionName );
            SAFE_DELETE( pDPHostEnumDelete->pAppDesc );
        }

        // Changed from array delete to Release
        SAFE_RELEASE( pDPHostEnumDelete->pHostAddr );
        SAFE_RELEASE( pDPHostEnumDelete->pDeviceAddr );
        SAFE_DELETE( pDPHostEnumDelete );
    }

    // Re-link the g_DPHostEnumInfoHead circular linked list
    m_DPHostEnumHead.pNext = &m_DPHostEnumHead;
}




//-----------------------------------------------------------------------------
// Name: StaticAddressDlgProc()
// Desc: Static msg handler which passes messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CNetConnectWizard::StaticAddressDlgProc( HWND hDlg, UINT uMsg,
                                                          WPARAM wParam, LPARAM lParam )
{
    if( g_pNCW )
        return g_pNCW->AddressDlgProc( hDlg, uMsg, wParam, lParam );

    return FALSE; // Message not handled
}




//-----------------------------------------------------------------------------
// Name: AddressDlgProc()
// Desc: Handles messages for the multiplayer connect dialog
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CNetConnectWizard::AddressDlgProc( HWND hDlg, UINT msg,
                                                    WPARAM wParam, LPARAM lParam )
{
    UNREFERENCED_PARAMETER( lParam );
    
    switch( msg )
    {
        case WM_INITDIALOG:
        {
#if defined(WIN32_PLATFORM_PSPC) && (_WIN32_WCE >= 300)
            SHINITDLGINFO   shidi;
            memset(&shidi, 0, sizeof(SHINITDLGINFO));
            shidi.dwMask = SHIDIM_FLAGS;
            shidi.dwFlags = SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
            shidi.hDlg = hDlg;

            SetForegroundWindow(hDlg);
            SHInitDialog(&shidi);
#endif // WIN32_PLATFORM_PSPC

            // Set the default port
            if( m_dwPort != 0 )
            {
                TCHAR strPort[40];
                _itot( m_dwPort, strPort, 10 );
                SetDlgItemText( hDlg, IDC_REMOTE_PORT, strPort );
            }

            return TRUE;
        }

        case WM_COMMAND:
        {
            switch( LOWORD(wParam) )
            {
                case IDOK:
                    // Store the user's choices
                    TCHAR strPort[40];
                    GetDlgItemText( hDlg, IDC_REMOTE_PORT, strPort, 40 );
                    strPort[39] = 0;
                    m_dwPort = _ttoi( strPort );

                    GetDlgItemText( hDlg, IDC_REMOTE_HOSTNAME, m_strHostname, MAX_PATH );
                    m_strHostname[MAX_PATH-1] = 0;

                    EndDialog( hDlg, IDOK );
                    return TRUE;

                case IDCANCEL:
                    EndDialog( hDlg, IDCANCEL );
                    return TRUE;
            }
            break;
        }
    }

    return FALSE; // Not handled
}




//-----------------------------------------------------------------------------
// Name: MessageHandler
// Desc: Handler for DirectPlay messages.  This function is called by
//       the DirectPlay message handler pool of threads, so be careful of thread
//       synchronization problems with shared memory
//-----------------------------------------------------------------------------
HRESULT WINAPI CNetConnectWizard::MessageHandler( PVOID pvUserContext,
                                                  DWORD dwMessageId,
                                                  PVOID pMsgBuffer )
{
    UNREFERENCED_PARAMETER( pvUserContext );
    
    // Try not to stay in this message handler for too long, otherwise
    // there will be a backlog of data.  The best solution is to
    // queue data as it comes in, and then handle it on other threads.

    // This function is called by the DirectPlay message handler pool of
    // threads, so be careful of thread synchronization problems with shared memory

    switch(dwMessageId)
    {
        case DPN_MSGID_ENUM_HOSTS_RESPONSE:
        {
            PDPNMSG_ENUM_HOSTS_RESPONSE pEnumHostsResponseMsg;
            pEnumHostsResponseMsg = (PDPNMSG_ENUM_HOSTS_RESPONSE)pMsgBuffer;

            // Take note of the host response
            SessionsDlgNoteEnumResponse( pEnumHostsResponseMsg );
            break;
        }

        case DPN_MSGID_ASYNC_OP_COMPLETE:
        {
            PDPNMSG_ASYNC_OP_COMPLETE pAsyncOpCompleteMsg;
            pAsyncOpCompleteMsg = (PDPNMSG_ASYNC_OP_COMPLETE)pMsgBuffer;

            if( pAsyncOpCompleteMsg->hAsyncOp == m_hEnumAsyncOp )
                SessionsDlgFinalizeEnumHosts( m_hDlg );

            break;
        }

        case DPN_MSGID_CONNECT_COMPLETE:
        {
            PDPNMSG_CONNECT_COMPLETE pConnectCompleteMsg;
            pConnectCompleteMsg = (PDPNMSG_CONNECT_COMPLETE)pMsgBuffer;

            // Set m_hrConnectComplete, then set an event letting
            // everyone know that the DPN_MSGID_CONNECT_COMPLETE msg
            // has been handled
            m_hrConnectComplete = pConnectCompleteMsg->hResultCode;
            SetEvent( m_hConnectCompleteEvent );
            break;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ConnectUsingLobbySettings
// Desc: Call this after the DPL_MSGID_CONNECT has been processed to carry out
//       the connection settings received by the lobby client.  DPL_MSGID_CONNECT
//       will have already been processed if we were lobby launched, or after
//       WaitForConnection returns without timing out.
//-----------------------------------------------------------------------------
HRESULT CNetConnectWizard::ConnectUsingLobbySettings()
{
    HRESULT hr;
    DPNHANDLE hAsync;

    if( m_hLobbyClient == NULL )
        return E_INVALIDARG;

    DPL_CONNECTION_SETTINGS* pSettings = NULL;
    DWORD dwSettingsSize = 0;

    // Get the connection settings from the lobby.
    hr = m_pLobbiedApp->GetConnectionSettings( m_hLobbyClient, pSettings, &dwSettingsSize, 0 );
    if( hr != DPNERR_BUFFERTOOSMALL )
    {
        DXTRACE_ERR_MSGBOX( TEXT("GetConnectionSettings"), hr );
        goto LCleanReturn;
    }

    pSettings = (DPL_CONNECTION_SETTINGS*) new BYTE[dwSettingsSize];
    if( NULL == pSettings )
    {
        hr = E_OUTOFMEMORY;
        DXTRACE_ERR_MSGBOX( TEXT("ConnectUsingLobbySettings"), hr );
        goto LCleanReturn;
    }

    if( FAILED( hr = m_pLobbiedApp->GetConnectionSettings( m_hLobbyClient, pSettings, &dwSettingsSize, 0 ) ) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("GetConnectionSettings"), hr );
        goto LCleanReturn;
    }

    // Check if the lobby told us to host the game
    m_bHostPlayer = (pSettings->dwFlags & DPLCONNECTSETTINGS_HOST);

    // Set the peer info
    WCHAR wszPeerName[MAX_PATH];
    DXUtil_ConvertGenericStringToWideCch( wszPeerName, m_strLocalPlayerName, MAX_PATH );
    DPN_PLAYER_INFO dpPlayerInfo;
    ZeroMemory( &dpPlayerInfo, sizeof(DPN_PLAYER_INFO) );
    dpPlayerInfo.dwSize = sizeof(DPN_PLAYER_INFO);
    dpPlayerInfo.dwInfoFlags = DPNINFO_NAME;
    dpPlayerInfo.pwszName = wszPeerName;

    // Set the peer info, and use the DPNOP_SYNC since by default this
    // is an async call.  If it is not DPNOP_SYNC, then the peer info may not
    // be set by the time we call Connect() below.
    if( FAILED( hr = m_pDP->SetPeerInfo( &dpPlayerInfo, NULL, NULL, DPNOP_SYNC ) ) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("SetPeerInfo"), hr );
        goto LCleanReturn;
    }

    if( m_bHostPlayer )
    {
        // Enable host migrate by default.
        pSettings->dpnAppDesc.dwFlags |= DPNSESSION_MIGRATE_HOST;

        // Disable DPNSVR by default
        pSettings->dpnAppDesc.dwFlags |= DPNSESSION_NODPNSVR;

        // Host a game as described by pSettings
        if( FAILED( hr = m_pDP->Host( &pSettings->dpnAppDesc,               // the application desc
                                      pSettings->ppdp8DeviceAddresses,      // array of addresses of the local devices used to connect to the host
                                      pSettings->cNumDeviceAddresses,       // number in array
                                      NULL, NULL,                           // DPN_SECURITY_DESC, DPN_SECURITY_CREDENTIALS
                                      NULL,                                 // player context
                                      0 ) ) )                               // flags
        {
            DXTRACE_ERR_MSGBOX( TEXT("Host"), hr );
            goto LCleanReturn;
        }
    }
    else
    {
        // Connect to an existing session. There should only be on device address in
        // the connection settings structure when connecting to a session, so just
        // pass in the first one.
        // The enumeration is automatically cancelled after Connect is called 
        hr = m_pDP->Connect( &pSettings->dpnAppDesc,              // the application desc
                             pSettings->pdp8HostAddress,          // address of the host of the session
                             pSettings->ppdp8DeviceAddresses[0],  // address of the local device used to connect to the host
                             NULL, NULL,                          // DPN_SECURITY_DESC, DPN_SECURITY_CREDENTIALS
                             NULL, 0,                             // user data, user data size
                             NULL,                                // player context,
                             NULL, &hAsync,                       // async context, async handle,
                             0 );                                 // flags
        if( hr != E_PENDING && FAILED(hr) )
        {
            DXTRACE_ERR_MSGBOX( TEXT("Connect"), hr );
            goto LCleanReturn;
        }

        hr = S_OK; // Accept E_PENDING.

        // Wait until the MessageHandler sets an event to tell us the
        // DPN_MSGID_CONNECT_COMPLETE has been processed.  Then m_hrConnectComplete
        // will be valid.
        WaitForSingleObject( m_hConnectCompleteEvent, INFINITE );

        if( FAILED( m_hrConnectComplete ) )
        {
            DXTRACE_ERR_MSGBOX( TEXT("DPN_MSGID_CONNECT_COMPLETE"), m_hrConnectComplete );
            MessageBox( m_hDlg, TEXT("Unable to join game."),
                        TEXT("DirectPlay Sample"),
                        MB_OK | MB_ICONERROR );
            hr = m_hrConnectComplete;
        }
    }

LCleanReturn:
    // Cleanup the addresses and memory obtained from GetConnectionSettings
    
    if( pSettings )
    {
        SAFE_RELEASE( pSettings->pdp8HostAddress );
        
        for( DWORD dwIndex=0; dwIndex < pSettings->cNumDeviceAddresses; dwIndex++ )
            SAFE_RELEASE( pSettings->ppdp8DeviceAddresses[dwIndex] );

        SAFE_DELETE_ARRAY( pSettings );
    }

    return hr;
}




//-----------------------------------------------------------------------------
// Name: LobbyMessageHandler
// Desc: Handler for DirectPlay messages.  This function is called by
//       the DirectPlay lobby message handler pool of threads, so be careful of thread
//       synchronization problems with shared memory
//-----------------------------------------------------------------------------
HRESULT WINAPI CNetConnectWizard::LobbyMessageHandler( PVOID pvUserContext,
                                                       DWORD dwMessageId,
                                                       PVOID pMsgBuffer )
{
    UNREFERENCED_PARAMETER( pvUserContext );
    HRESULT hr = S_OK;

    switch(dwMessageId)
    {
        case DPL_MSGID_CONNECT:
        {
            // This message will be processed when a lobby connection has been
            // established. If you were lobby launched then
            // IDirectPlay8LobbiedApplication::Initialize()
            // waits until this message has been processed before returning, so
            // take care not to deadlock by making calls that need to be handled by
            // the thread who called Initialize().  The same is true for WaitForConnection()

            PDPL_MESSAGE_CONNECT pConnectMsg;
            pConnectMsg = (PDPL_MESSAGE_CONNECT)pMsgBuffer;
            PDPL_CONNECTION_SETTINGS pSettings = pConnectMsg->pdplConnectionSettings;

            m_hLobbyClient = pConnectMsg->hConnectId;

            if( FAILED( hr = m_pDP->RegisterLobby( m_hLobbyClient, m_pLobbiedApp,
                                                   DPNLOBBY_REGISTER ) ) )
                return DXTRACE_ERR_MSGBOX( TEXT("RegisterLobby"), hr );

            if( pSettings == NULL )
            {
                // There aren't connection settings from the lobby
                m_bHaveConnectionSettingsFromLobby = FALSE;
            }
            else
            {
                // Record the player name if found
                if( pSettings->pwszPlayerName != NULL )
                {
                    TCHAR strPlayerName[MAX_PATH];
                    DXUtil_ConvertWideStringToGenericCch( strPlayerName, pSettings->pwszPlayerName, MAX_PATH );
                    _tcscpy( m_strLocalPlayerName, strPlayerName );
                }
                else
                {
                    _tcscpy( m_strLocalPlayerName, TEXT("Unknown player name") );
                }

                m_bHaveConnectionSettingsFromLobby = TRUE;
            }

            // Tell everyone we have a lobby connection now
            SetEvent( m_hLobbyConnectionEvent );
            break;
        }
    }

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: StaticLobbyWaitDlgProc()
// Desc: Static msg handler which passes messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CNetConnectWizard::StaticLobbyWaitDlgProc( HWND hDlg, UINT uMsg,
                                                                WPARAM wParam, LPARAM lParam )
{
    if( g_pNCW )
        return g_pNCW->LobbyWaitDlgProc( hDlg, uMsg, wParam, lParam );

    return FALSE; // Message not handled
}




//-----------------------------------------------------------------------------
// Name: LobbyWaitDlgProc()
// Desc: Handles messages for the lobby wait status dialog
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CNetConnectWizard::LobbyWaitDlgProc( HWND hDlg, UINT msg,
                                                      WPARAM wParam, LPARAM lParam )
{
    UNREFERENCED_PARAMETER( lParam );
    switch( msg )
    {
        case WM_INITDIALOG:
#if defined(WIN32_PLATFORM_PSPC) && (_WIN32_WCE >= 300)
            SHINITDLGINFO   shidi;
            memset(&shidi, 0, sizeof(SHINITDLGINFO));
            shidi.dwMask = SHIDIM_FLAGS;
            shidi.dwFlags = SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
            shidi.hDlg = hDlg;

            SetForegroundWindow(hDlg);
            SHInitDialog(&shidi);
#endif // WIN32_PLATFORM_PSPC

            // Set a timer to wait for m_hConnectCompleteEvent to be signaled.
            // This will tell us when DPN_MSGID_CONNECT_COMPLETE has been processed
            // which lets us know if the connect was successful or not.
            SetTimer( hDlg, TIMERID_CONNECT_COMPLETE, 100, NULL );

            SetDlgItemText( hDlg, IDC_WAIT_TEXT, TEXT("Waiting for lobby connection...") );
            return TRUE;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDCANCEL:
                    EndDialog( hDlg, IDCANCEL );
                    return TRUE;
            }
            break;

        case WM_TIMER:
        {
            if( wParam == TIMERID_CONNECT_COMPLETE )
            {
                // Wait for a lobby connection.  If this call
                // returns WAIT_OBJECT_0 then the DPL_MSGID_CONNECT will
                // have already been processed.
                DWORD dwResult = WaitForSingleObject( m_hLobbyConnectionEvent, 100 );
                if( dwResult != WAIT_TIMEOUT )
                    EndDialog( hDlg, IDOK );
            }
            break;
        }
    }

    return FALSE; // Didn't handle message
}




