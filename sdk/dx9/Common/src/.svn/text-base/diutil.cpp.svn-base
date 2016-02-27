//-----------------------------------------------------------------------------
// File: DIUtil.cpp
//
// Desc: DirectInput framework class using semantic mapping.  Feel free to use 
//       this class as a starting point for adding extra functionality.
//
// Copyright (C) 1995-2001 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------
#define STRICT

#include <basetsd.h>
#include <tchar.h>
#include <stdio.h>
#include "DIUtil.h"
#include "DXUtil.h"



 

//-----------------------------------------------------------------------------
// Name: CInputDeviceManager()
// Desc: Constructor 
//-----------------------------------------------------------------------------
CInputDeviceManager::CInputDeviceManager()
{
    HRESULT hr = CoInitialize(NULL);
    m_bCleanupCOM = SUCCEEDED(hr);

    m_dwNumDevices = 0;
    m_dwMaxDevices = 10;
    m_pDI          = NULL;

    // Allocate DeviceInfo structs
    m_pDevices = (DeviceInfo*) malloc( m_dwMaxDevices*sizeof(DeviceInfo) );
    if( m_pDevices )
        ZeroMemory( m_pDevices, m_dwMaxDevices*sizeof(DeviceInfo) );
}




//-----------------------------------------------------------------------------
// Name: ~CInputDeviceManager()
// Desc: Destructor
//-----------------------------------------------------------------------------
CInputDeviceManager::~CInputDeviceManager()
{
    if( m_pDevices )
    {
        // Release() all devices
        for( DWORD i=0; i<m_dwNumDevices; i++ )
        {
            if( m_pDevices[i].pdidDevice )
                m_pDevices[i].pdidDevice->Unacquire();

            SAFE_RELEASE( m_pDevices[i].pdidDevice );
        }

        free( m_pDevices );
    }

    // Release() base object
    SAFE_RELEASE( m_pDI );

    if( m_bCleanupCOM )
        CoUninitialize();
}




//-----------------------------------------------------------------------------
// Name: GetDevices()
// Desc: Get the DeviceInfo array and number of devices
//-----------------------------------------------------------------------------
HRESULT CInputDeviceManager::GetDevices( DeviceInfo** ppDeviceInfo, 
                                         DWORD* pdwCount )
{
    if( NULL==ppDeviceInfo || NULL==pdwCount )
        return E_INVALIDARG;

    (*ppDeviceInfo) = m_pDevices;
    (*pdwCount) = m_dwNumDevices;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: AddDevice()
// Desc: Add the provided device to the list and perform initialization
//-----------------------------------------------------------------------------
HRESULT CInputDeviceManager::AddDevice( const DIDEVICEINSTANCE* pdidi, 
                                        const LPDIRECTINPUTDEVICE8 pdidDevice )
{
    HRESULT hr;

    // Sanity check
    if( NULL == pdidDevice )
        return E_INVALIDARG;

    pdidDevice->Unacquire();

    // Set the device's coop level
    hr = pdidDevice->SetCooperativeLevel( m_hWnd, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND );
    if( FAILED(hr) )
        return hr;

    // Enlarge the array if needed
    if( m_dwNumDevices >= m_dwMaxDevices )
    {
        // Attempt to reallocate memory
        DWORD dwNewMax = m_dwMaxDevices + 10;
        DeviceInfo* pNewDevices = (DeviceInfo*) realloc( m_pDevices, sizeof(DeviceInfo) * dwNewMax );
        
        // Check for out of memory condition
        if( NULL == pNewDevices )
            return E_OUTOFMEMORY;

        // Initialize the new memory block
        ZeroMemory( pNewDevices + m_dwMaxDevices, sizeof(DeviceInfo) * (dwNewMax - m_dwMaxDevices) );

        // Copy the new pointer and update the max device count
        m_pDevices = pNewDevices;
        m_dwMaxDevices = dwNewMax;
    }

    // Add new device to the end of the list, but don't finalize addition until
    // the end of the function. If the remaining initialization calls fail, this
    // spot will be used by the next valid device.
    m_pDevices[m_dwNumDevices].pdidDevice = pdidDevice;
    
    // Callback into the app so it can adjust the device and set
    // the m_pDevices[dwCurrentDevice].pParam field with a device state struct
    if( m_AddDeviceCallback )
    {
        hr = m_AddDeviceCallback( &m_pDevices[m_dwNumDevices], pdidi, m_AddDeviceCallbackParam ); 
        if( FAILED(hr) )    
            return hr;
    }

    // Build the action map
    hr = m_pDevices[m_dwNumDevices].pdidDevice->BuildActionMap( &m_diaf, m_strUserName, 0 );
    if( FAILED(hr) )
        return hr;

    // Set the action map for the current device
    hr = m_pDevices[m_dwNumDevices].pdidDevice->SetActionMap( &m_diaf, m_strUserName, 0 );
    if( FAILED(hr) )
        return hr;

    // Device addition succeeded. Increment reference and index.
    m_pDevices[m_dwNumDevices].pdidDevice->AddRef();
    m_dwNumDevices++;

    // Continue enumerating suitable devices
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: EnumSuitableDevicesCB()
// Desc: Callback function for device enumeration. Adds all devices which
//       met the search criteria
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumSuitableDevicesCB( LPCDIDEVICEINSTANCE pdidi, 
                                     LPDIRECTINPUTDEVICE8 pdidDevice, 
                                     DWORD dwFlags, DWORD dwDeviceRemaining,
                                     VOID* pContext )
{
    UNREFERENCED_PARAMETER( dwFlags );
    UNREFERENCED_PARAMETER( dwDeviceRemaining );
    
    // Add the device to the device manager's internal list
    ((CInputDeviceManager*)pContext)->AddDevice( pdidi, pdidDevice );

    // Continue enumerating suitable devices
    return DIENUM_CONTINUE;
}




//-----------------------------------------------------------------------------
// Name: SetActionFormat()
// Desc: Set the action format to the provided DIACTIONFORMAT structure, and
//       destroy and recreate device list if flagged
//-----------------------------------------------------------------------------
HRESULT CInputDeviceManager::SetActionFormat( DIACTIONFORMAT& diaf, BOOL bReenumerate )
{
    HRESULT hr = S_OK;

    // Store the new action format
    m_diaf = diaf;

    // Only destroy and re-enumerate devices if the caller explicitly wants to. The 
    // device list may be used within a loop, and kicking off an enumeration while 
    // the device array is in use would cause problems.
    if( bReenumerate )
    {
        // Cleanup any previously enumerated devices
        for( DWORD i=0; i<m_dwNumDevices; i++ )
        {
            if( m_pDevices[i].pdidDevice )
                m_pDevices[i].pdidDevice->Unacquire();

            SAFE_RELEASE( m_pDevices[i].pdidDevice );
        }
        m_dwNumDevices = 0;

        // Enumerate suitable DirectInput devices
        hr = m_pDI->EnumDevicesBySemantics( m_strUserName, &m_diaf, 
                                            EnumSuitableDevicesCB, this, 0L );
        if( FAILED(hr) )
            return hr;
    }
    else // Just apply the new maps.
    {
        // Devices must be unacquired to have a new action map set.
        UnacquireDevices();

        // Apply the new action map to the current devices.
        for( DWORD i=0; i<m_dwNumDevices; i++ )
        {
            hr = m_pDevices[i].pdidDevice->BuildActionMap( &m_diaf, m_strUserName, 0 );
            if( FAILED(hr) )
                return hr;

            hr = m_pDevices[i].pdidDevice->SetActionMap( &m_diaf, m_strUserName, 0 );
            if( FAILED(hr) )
                return hr;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Create()
// Desc: Create DirectInput object and perform initialization
//-----------------------------------------------------------------------------
HRESULT CInputDeviceManager::Create( HWND hWnd, TCHAR* strUserName, 
                                     DIACTIONFORMAT& diaf,
                                     LPDIMANAGERCALLBACK AddDeviceCallback, 
                                     LPVOID pCallbackParam )
{
    HRESULT hr;

    // Store data
    m_hWnd        = hWnd;
    m_strUserName = strUserName;
    m_AddDeviceCallback = AddDeviceCallback;
    m_AddDeviceCallbackParam = pCallbackParam;
    
    // Create the base DirectInput object
    hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
                              IID_IDirectInput8, (VOID**)&m_pDI, NULL );
    if( FAILED(hr) )
        return hr;

    hr = SetActionFormat( diaf, TRUE );
    if( FAILED(hr) )
        return hr;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ConfigureDevices()
// Desc: Pause input and display the device configuration UI
//-----------------------------------------------------------------------------
HRESULT CInputDeviceManager::ConfigureDevices( HWND hWnd, IUnknown* pSurface,
                                               VOID* ConfigureDevicesCB,
                                               DWORD dwFlags, LPVOID pvCBParam )
{
    HRESULT hr;

    // Initialize all the colors here
    DICOLORSET dics;
    ZeroMemory(&dics, sizeof(DICOLORSET));
    dics.dwSize = sizeof(DICOLORSET);

    // Fill in all the params
    DICONFIGUREDEVICESPARAMS dicdp;
    ZeroMemory(&dicdp, sizeof(dicdp));
    dicdp.dwSize = sizeof(dicdp);
    dicdp.dwcFormats     = 1;
    dicdp.lprgFormats    = &m_diaf;
    dicdp.hwnd           = hWnd;
    dicdp.lpUnkDDSTarget = pSurface;

    if( m_strUserName )
    {
        dicdp.dwcUsers       = 1;
        dicdp.lptszUserNames = m_strUserName;
    }

    // Unacquire the devices so that mouse doesn't control the game while in control panel
    UnacquireDevices();

    hr = m_pDI->ConfigureDevices( (LPDICONFIGUREDEVICESCALLBACK)ConfigureDevicesCB, 
                                  &dicdp, dwFlags, pvCBParam );
    if( FAILED(hr) )
        return hr;

    if( dwFlags & DICD_EDIT )
    {
        // Re-set up the devices
        hr = SetActionFormat( m_diaf, TRUE );
        if( FAILED(hr) )
            return hr;
    }

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: UnacquireDevices()
// Desc: Unacquire all devices in the member list
//-----------------------------------------------------------------------------
VOID CInputDeviceManager::UnacquireDevices()
{
    for( DWORD i=0; i<m_dwNumDevices; i++ )
        m_pDevices[i].pdidDevice->Unacquire();
}




//-----------------------------------------------------------------------------
// Name: SetFocus()
// Desc: Sets the DirectInput focus to a new HWND
//-----------------------------------------------------------------------------
VOID CInputDeviceManager::SetFocus( HWND hWnd ) 
{
    m_hWnd = hWnd;

    UnacquireDevices();

    for( DWORD i=0; i<m_dwNumDevices; i++ )
    {
        // Set the device's coop level
        m_pDevices[i].pdidDevice->SetCooperativeLevel( m_hWnd, 
                                        DISCL_NONEXCLUSIVE|DISCL_FOREGROUND );
    }
}


