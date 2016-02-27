//-----------------------------------------------------------------------------
// File: DMUtil.cpp
//
// Desc: DirectMusic framework classes for playing DirectMusic segments and
//       DirectMusic scripts. Feel free to use this class as a starting point 
//       for adding extra functionality.
//
// Copyright (c) Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <dmusicc.h>
#include <dmusici.h>
#include <dsound.h>
#include <dxerr9.h>
#include "DMUtil.h"
#include "DXUtil.h"




//-----------------------------------------------------------------------------
// Name: CMusicManager::CMusicManager()
// Desc: Constructs the class
//-----------------------------------------------------------------------------
CMusicManager::CMusicManager()
{
    m_pLoader       = NULL;
    m_pPerformance  = NULL;
    m_pDSListener   = NULL;
    
    // Initialize COM
    HRESULT hr = CoInitialize(NULL);
    m_bCleanupCOM = SUCCEEDED(hr);
}




//-----------------------------------------------------------------------------
// Name: CMusicManager::~CMusicManager()
// Desc: Destroys the class
//-----------------------------------------------------------------------------
CMusicManager::~CMusicManager()
{
    SAFE_RELEASE( m_pLoader ); 
    SAFE_RELEASE( m_pDSListener );

    if( m_pPerformance )
    {
        // If there is any music playing, stop it.
        m_pPerformance->Stop( NULL, NULL, 0, 0 );
        m_pPerformance->CloseDown();

        SAFE_RELEASE( m_pPerformance );
    }

    if( m_bCleanupCOM )
        CoUninitialize();
}




//-----------------------------------------------------------------------------
// Name: CMusicManager::Initialize()
// Desc: Inits DirectMusic using a standard audio path
//-----------------------------------------------------------------------------
HRESULT CMusicManager::Initialize( HWND hWnd, DWORD dwPChannels, DWORD dwDefaultPathType, LPDIRECTSOUND pDS )
{
    HRESULT hr;
    IDirectSound** ppDirectSound;

    if( pDS )
        ppDirectSound = &pDS;
    else
        ppDirectSound = NULL;
 
    // Create loader object
    if( FAILED( hr = CoCreateInstance( CLSID_DirectMusicLoader, NULL, CLSCTX_INPROC, 
                                       IID_IDirectMusicLoader8, (void**)&m_pLoader ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("CoCreateInstance"), hr );

    // Create performance object
    if( FAILED( hr = CoCreateInstance( CLSID_DirectMusicPerformance, NULL, CLSCTX_INPROC, 
                                       IID_IDirectMusicPerformance8, (void**)&m_pPerformance ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("CoCreateInstance"), hr );

    // Initialize the performance with the standard audio path.
    // This initializes both DirectMusic and DirectSound and 
    // sets up the synthesizer. Typcially its easist to use an 
    // audio path for playing music and sound effects.
    if( FAILED( hr = m_pPerformance->InitAudio( NULL, ppDirectSound, hWnd, dwDefaultPathType, 
                                                dwPChannels, DMUS_AUDIOF_ALL, NULL ) ) )
    {
        if( hr == DSERR_NODRIVER )
        {
            DXTRACE( TEXT("Warning: No sound card found\n") );
            return hr;
        }

        return DXTRACE_ERR_MSGBOX( TEXT("InitAudio"), hr );
    }

    // Get the listener from the in the default audio path.
    IDirectMusicAudioPath8* pAudioPath = GetDefaultAudioPath();
    if( pAudioPath ) // might be NULL if dwDefaultPathType == 0
    {
        if( SUCCEEDED( hr = pAudioPath->GetObjectInPath( 0, DMUS_PATH_PRIMARY_BUFFER, 0,
                                                        GUID_NULL, 0, IID_IDirectSound3DListener, 
                                                        (LPVOID*) &m_pDSListener ) ) )
        {
            // Get listener parameters
            m_dsListenerParams.dwSize = sizeof(DS3DLISTENER);
            m_pDSListener->GetAllParameters( &m_dsListenerParams );
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CMusicManager::SetSearchDirectory()
// Desc: Sets the search directory.  If not called, the current working
//       directory is used to load content.
//-----------------------------------------------------------------------------
HRESULT CMusicManager::SetSearchDirectory( const TCHAR* strMediaPath )
{
    if( NULL == m_pLoader )
        return E_UNEXPECTED;
    if( NULL == strMediaPath )
        return E_INVALIDARG;

    // DMusic only takes wide strings
    WCHAR wstrMediaPath[MAX_PATH];
    DXUtil_ConvertGenericStringToWideCb( wstrMediaPath, strMediaPath, sizeof(wstrMediaPath) );

    return m_pLoader->SetSearchDirectory( GUID_DirectMusicAllTypes, 
                                          wstrMediaPath, FALSE );

}




//-----------------------------------------------------------------------------
// Name: CMusicManager::GetDefaultAudioPath()
// Desc: 
//-----------------------------------------------------------------------------
IDirectMusicAudioPath8* CMusicManager::GetDefaultAudioPath()
{
    IDirectMusicAudioPath8* pAudioPath = NULL;
    if( NULL == m_pPerformance )
        return NULL;

    m_pPerformance->GetDefaultAudioPath( &pAudioPath );
    return pAudioPath;
}




//-----------------------------------------------------------------------------
// Name: CMusicManager::CollectGarbage()
// Desc: Tells the loader to cleanup any garbage from previously 
//       released objects.
//-----------------------------------------------------------------------------
VOID CMusicManager::CollectGarbage()
{
    if( m_pLoader )
        m_pLoader->CollectGarbage();
}




//-----------------------------------------------------------------------------
// Name: CMusicManager::StopAll()
// Desc: Stops all segments.  Also simply calling Stop() on the segment won't 
//       stop any MIDI sustain pedals, but calling StopAll() will.
//-----------------------------------------------------------------------------
VOID CMusicManager::StopAll()
{
    if( m_pPerformance )
        m_pPerformance->Stop( NULL, NULL, 0, 0 );
}



//-----------------------------------------------------------------------------
// Name: CMusicManager::CreateSegmentFromFile()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMusicManager::CreateSegmentFromFile( CMusicSegment** ppSegment, 
                                              TCHAR* strFileName, 
                                              BOOL bDownloadNow,
                                              BOOL bIsMidiFile )
{
    HRESULT               hr;
    IDirectMusicSegment8* pSegment = NULL;

    // DMusic only takes wide strings
    WCHAR wstrFileName[MAX_PATH];
    DXUtil_ConvertGenericStringToWideCb( wstrFileName, strFileName, sizeof(wstrFileName) );

    if ( FAILED( hr = m_pLoader->LoadObjectFromFile( CLSID_DirectMusicSegment,
                                                     IID_IDirectMusicSegment8,
                                                     wstrFileName,
                                                     (LPVOID*) &pSegment ) ) )
    {
        if( hr == DMUS_E_LOADER_FAILEDOPEN )
            return hr;
        return DXTRACE_ERR_MSGBOX( TEXT("LoadObjectFromFile"), hr );
    }

    *ppSegment = new CMusicSegment( m_pPerformance, m_pLoader, pSegment );
    if (!*ppSegment)
        return E_OUTOFMEMORY;

    if( bIsMidiFile )
    {
        if( FAILED( hr = pSegment->SetParam( GUID_StandardMIDIFile, 
                                             0xFFFFFFFF, 0, 0, NULL ) ) )
            return DXTRACE_ERR_MSGBOX( TEXT("SetParam"), hr );
    }

    if( bDownloadNow )
    {
        if( FAILED( hr = (*ppSegment)->Download() ) )
            return DXTRACE_ERR_MSGBOX( TEXT("Download"), hr );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CMusicManager::CreateSegmentFromResource()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMusicManager::CreateSegmentFromResource( CMusicSegment** ppSegment, 
                                                  TCHAR* strResource,
                                                  TCHAR* strResourceType,
                                                  BOOL bDownloadNow,
                                                  BOOL bIsMidiFile )
{
    HRESULT               hr;
    IDirectMusicSegment8* pSegment      = NULL;
    HRSRC                 hres          = NULL;
    void*                 pMem          = NULL;
    DWORD                 dwSize        = 0;
    DMUS_OBJECTDESC       objdesc;

    // Find the resource
    hres = FindResource( NULL,strResource,strResourceType );
    if( NULL == hres ) 
        return E_FAIL;

    // Load the resource
    pMem = (void*)LoadResource( NULL, hres );
    if( NULL == pMem ) 
        return E_FAIL;

    // Store the size of the resource
    dwSize = SizeofResource( NULL, hres ); 
    
    // Set up our object description 
    ZeroMemory(&objdesc,sizeof(DMUS_OBJECTDESC));
    objdesc.dwSize = sizeof(DMUS_OBJECTDESC);
    objdesc.dwValidData = DMUS_OBJ_MEMORY | DMUS_OBJ_CLASS;
    objdesc.guidClass = CLSID_DirectMusicSegment;
    objdesc.llMemLength =(LONGLONG)dwSize;
    objdesc.pbMemData = (BYTE*)pMem;
    
    if (FAILED ( hr = m_pLoader->GetObject( &objdesc,
                                            IID_IDirectMusicSegment8,
                                            (void**)&pSegment ) ) )
    {
        if( hr == DMUS_E_LOADER_FAILEDOPEN )
            return hr;
        return DXTRACE_ERR_MSGBOX( TEXT("LoadObjectFromFile"), hr );
    }

    *ppSegment = new CMusicSegment( m_pPerformance, m_pLoader, pSegment );
    if( NULL == *ppSegment )
        return E_OUTOFMEMORY;

    if( bIsMidiFile )
    {
        // Do this to make sure that the default General MIDI set 
        // is connected appropriately to the MIDI file and 
        // all instruments sound correct.                  
        if( FAILED( hr = pSegment->SetParam( GUID_StandardMIDIFile, 
                                             0xFFFFFFFF, 0, 0, NULL ) ) )
            return DXTRACE_ERR_MSGBOX( TEXT("SetParam"), hr );
    }

    if( bDownloadNow )
    {
        // The segment needs to be download first before playing.  
        // However, some apps may want to wait before calling this 
        // to because the download allocates memory for the 
        // instruments. The more instruments currently downloaded, 
        // the more memory is in use by the synthesizer.
        if( FAILED( hr = (*ppSegment)->Download() ) )
            return DXTRACE_ERR_MSGBOX( TEXT("Download"), hr );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CMusicManager::Create3DSegmentFromFile()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMusicManager::Create3DSegmentFromFile( C3DMusicSegment** pp3DMusicSegment, 
                                              TCHAR* strFileName, 
                                              BOOL bDownloadNow,
                                              BOOL bIsMidiFile,
                                              IDirectMusicAudioPath8* p3DAudioPath )
{
    HRESULT               hr;
    IDirectMusicSegment8* pSegment = NULL;

    // DMusic only takes wide strings
    WCHAR wstrFileName[MAX_PATH];
    DXUtil_ConvertGenericStringToWideCb( wstrFileName, strFileName, sizeof(wstrFileName) );

    if ( FAILED( hr = m_pLoader->LoadObjectFromFile( CLSID_DirectMusicSegment,
                                                     IID_IDirectMusicSegment8,
                                                     wstrFileName,
                                                     (LPVOID*) &pSegment ) ) )
    {
        if( hr == DMUS_E_LOADER_FAILEDOPEN )
            return hr;
        return DXTRACE_ERR( TEXT("LoadObjectFromFile"), hr );
    }

    *pp3DMusicSegment = new C3DMusicSegment( m_pPerformance, m_pLoader, pSegment, p3DAudioPath );
    if (!*pp3DMusicSegment)
        return E_OUTOFMEMORY;

    if( FAILED( hr = (*pp3DMusicSegment)->Init() ) ) 
        return DXTRACE_ERR( TEXT("SetParam"), hr );

    if( bIsMidiFile )
    {
        if( FAILED( hr = pSegment->SetParam( GUID_StandardMIDIFile, 
                                             0xFFFFFFFF, 0, 0, NULL ) ) )
            return DXTRACE_ERR( TEXT("SetParam"), hr );
    }

    if( bDownloadNow )
    {
        if( FAILED( hr = (*pp3DMusicSegment)->Download() ) )
            return DXTRACE_ERR( TEXT("Download"), hr );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CMusicManager::CreateScriptFromFile()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMusicManager::CreateScriptFromFile( CMusicScript** ppScript, 
                                             TCHAR* strFileName )
{
    HRESULT               hr;
    IDirectMusicScript* pScript = NULL;

    // DMusic only takes wide strings
    WCHAR wstrFileName[MAX_PATH];
    DXUtil_ConvertGenericStringToWideCb( wstrFileName, strFileName, sizeof(wstrFileName) );
    
    if ( FAILED( hr = m_pLoader->LoadObjectFromFile( CLSID_DirectMusicScript,
                                                     IID_IDirectMusicScript8,
                                                     wstrFileName,
                                                     (LPVOID*) &pScript ) ) )
        return DXTRACE_ERR( TEXT("LoadObjectFromFile"), hr );

    if ( FAILED( hr = pScript->Init( m_pPerformance, NULL ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("Init"), hr );

    *ppScript = new CMusicScript( m_pPerformance, m_pLoader, pScript );
    if (!*ppScript)
        return E_OUTOFMEMORY;

    return hr;
}




//-----------------------------------------------------------------------------
// Name: CMusicManager::CreateChordMapFromFile()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMusicManager::CreateChordMapFromFile( IDirectMusicChordMap8** ppChordMap, 
                                               TCHAR* strFileName )
{
    // DMusic only takes wide strings
    WCHAR wstrFileName[MAX_PATH];
    DXUtil_ConvertGenericStringToWideCb( wstrFileName, strFileName, sizeof(wstrFileName) );

    return m_pLoader->LoadObjectFromFile( CLSID_DirectMusicChordMap,
                                          IID_IDirectMusicChordMap8,
                                          wstrFileName, (LPVOID*) ppChordMap );
}




//-----------------------------------------------------------------------------
// Name: CMusicManager::CreateChordMapFromFile()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMusicManager::CreateStyleFromFile( IDirectMusicStyle8** ppStyle, 
                                            TCHAR* strFileName )
{
    // DMusic only takes wide strings
    WCHAR wstrFileName[MAX_PATH];
    DXUtil_ConvertGenericStringToWideCb( wstrFileName, strFileName, sizeof(wstrFileName) );

    return m_pLoader->LoadObjectFromFile( CLSID_DirectMusicStyle,
                                          IID_IDirectMusicStyle8,
                                          wstrFileName, (LPVOID*) ppStyle );
}




//-----------------------------------------------------------------------------
// Name: CMusicManager::GetMotifFromStyle()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMusicManager::GetMotifFromStyle( IDirectMusicSegment8** ppMotif8, 
                                          TCHAR* strStyle, TCHAR* strMotif )
{       
    UNREFERENCED_PARAMETER( strMotif );
	
    HRESULT              hr;
    IDirectMusicStyle8*  pStyle = NULL;
    IDirectMusicSegment* pMotif = NULL;

    if( FAILED( hr = CreateStyleFromFile( &pStyle, strStyle ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("CreateStyleFromFile"), hr );

    if( pStyle )
    {
        // DMusic only takes wide strings
        WCHAR wstrMotif[MAX_PATH];
        DXUtil_ConvertGenericStringToWideCb( wstrMotif, strStyle, sizeof(wstrMotif) );

        hr = pStyle->GetMotif( wstrMotif, &pMotif );
        SAFE_RELEASE( pStyle );

        if( FAILED( hr ) )
            return DXTRACE_ERR_MSGBOX( TEXT("GetMotif"), hr );

        pMotif->QueryInterface( IID_IDirectMusicSegment8, (LPVOID*) ppMotif8 );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Set3DParameters
// Desc: 
//-----------------------------------------------------------------------------
VOID CMusicManager::Set3DParameters( FLOAT fDistanceFactor, FLOAT fDopplerFactor, FLOAT fRolloffFactor )
{
    m_dsListenerParams.flDistanceFactor = fDistanceFactor;
    m_dsListenerParams.flDopplerFactor = fDopplerFactor;
    m_dsListenerParams.flRolloffFactor = fRolloffFactor;

    if( m_pDSListener )
        m_pDSListener->SetAllParameters( &m_dsListenerParams, DS3D_IMMEDIATE );
}




//-----------------------------------------------------------------------------
// Name: CMusicSegment::CMusicSegment()
// Desc: Constructs the class
//-----------------------------------------------------------------------------
CMusicSegment::CMusicSegment( IDirectMusicPerformance8* pPerformance, 
                              IDirectMusicLoader8*      pLoader,
                              IDirectMusicSegment8*     pSegment )
{
    m_pPerformance          = pPerformance;
    m_pLoader               = pLoader;
    m_pSegment              = pSegment;
    m_pEmbeddedAudioPath    = NULL;
    m_bDownloaded           = FALSE;
    
    // Try to pull out an audio path from the segment itself if there is one.
    // This embedded audio path will be used instead of the default
    // audio path if the app doesn't wish to use an overriding audio path.
    IUnknown* pConfig = NULL;
    if( SUCCEEDED( m_pSegment->GetAudioPathConfig( &pConfig ) ) )
    {
        m_pPerformance->CreateAudioPath( pConfig, TRUE, &m_pEmbeddedAudioPath );
        SAFE_RELEASE( pConfig );
    } 

}




//-----------------------------------------------------------------------------
// Name: CMusicSegment::~CMusicSegment()
// Desc: Destroys the class
//-----------------------------------------------------------------------------
CMusicSegment::~CMusicSegment()
{
    if( m_pSegment )
    {
        // Tell the loader that this object should now be released
        if( m_pLoader )
            m_pLoader->ReleaseObjectByUnknown( m_pSegment );

        if( m_bDownloaded )
        {
            if( m_pEmbeddedAudioPath )
                m_pSegment->Unload( m_pEmbeddedAudioPath );
            else
                m_pSegment->Unload( m_pPerformance );
        }

        SAFE_RELEASE( m_pEmbeddedAudioPath ); 
        SAFE_RELEASE( m_pSegment ); 
    }

    m_pPerformance = NULL;
}




//-----------------------------------------------------------------------------
// Name: CMusicSegment::Play()
// Desc: Plays the sound using voice management flags.  Pass in DSBPLAY_LOOPING
//       in the dwFlags to loop the sound
//-----------------------------------------------------------------------------
HRESULT CMusicSegment::Play( DWORD dwFlags, IDirectMusicAudioPath8* pAudioPath )
{
    if( m_pSegment == NULL || m_pPerformance == NULL )
        return CO_E_NOTINITIALIZED;

    if( !m_bDownloaded )
        return E_FAIL;

    // If an audio path was passed in then use it, otherwise
    // use the embedded audio path if there was one.
    if( pAudioPath == NULL && m_pEmbeddedAudioPath != NULL )
        pAudioPath = m_pEmbeddedAudioPath;
        
    // If pAudioPath is NULL then this plays on the default audio path.
    return m_pPerformance->PlaySegmentEx( m_pSegment, 0, NULL, dwFlags, 
                                          0, 0, NULL, pAudioPath );
}





//-----------------------------------------------------------------------------
// Name: CMusicSegment::Download()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMusicSegment::Download( IDirectMusicAudioPath8* pAudioPath )
{
    HRESULT hr;
    
    if( m_pSegment == NULL )
        return CO_E_NOTINITIALIZED;

    // If no audio path was passed in, then download
    // to the embedded audio path if it exists 
    // else download to the performance
    if( pAudioPath == NULL )
    {
        if( m_pEmbeddedAudioPath )
            hr = m_pSegment->Download( m_pEmbeddedAudioPath );
        else    
            hr = m_pSegment->Download( m_pPerformance );
    }
    else
    {
        hr = m_pSegment->Download( pAudioPath );
    }
    
    if ( SUCCEEDED( hr ) )
        m_bDownloaded = TRUE;
        
    return hr;
}




//-----------------------------------------------------------------------------
// Name: CMusicSegment::Unload()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMusicSegment::Unload( IDirectMusicAudioPath8* pAudioPath )
{
    HRESULT hr;
    
    if( m_pSegment == NULL )
        return CO_E_NOTINITIALIZED;

    // If no audio path was passed in, then unload 
    // from the embedded audio path if it exists 
    // else unload from the performance
    if( pAudioPath == NULL )
    {
        if( m_pEmbeddedAudioPath )
            hr = m_pSegment->Unload( m_pEmbeddedAudioPath );
        else    
            hr = m_pSegment->Unload( m_pPerformance );
    }
    else
    {
        hr = m_pSegment->Unload( pAudioPath );
    }
        
    if ( SUCCEEDED( hr ) )
        m_bDownloaded = FALSE;

    return hr;
}




//-----------------------------------------------------------------------------
// Name: CMusicSegment::IsPlaying()
// Desc: 
//-----------------------------------------------------------------------------
BOOL CMusicSegment::IsPlaying()
{
    if( m_pSegment == NULL || m_pPerformance == NULL )
        return FALSE;

    return ( m_pPerformance->IsPlaying( m_pSegment, NULL ) == S_OK );
}




//-----------------------------------------------------------------------------
// Name: CMusicSegment::Stop()
// Desc: Stops the sound from playing
//-----------------------------------------------------------------------------
HRESULT CMusicSegment::Stop( DWORD dwFlags )
{
    if( m_pSegment == NULL || m_pPerformance == NULL )
        return CO_E_NOTINITIALIZED;

    return m_pPerformance->Stop( m_pSegment, NULL, 0, dwFlags );;
}




//-----------------------------------------------------------------------------
// Name: CMusicSegment::SetRepeats()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMusicSegment::SetRepeats( DWORD dwRepeats )
{
    if( m_pSegment == NULL )
        return CO_E_NOTINITIALIZED;

    return m_pSegment->SetRepeats( dwRepeats );
}




//-----------------------------------------------------------------------------
// Name: CMusicSegment::GetStyle()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMusicSegment::GetStyle( IDirectMusicStyle8** ppStyle, DWORD dwStyleIndex )
{
    // Get the Style from the Segment by calling the Segment's GetData() with
    // the data type GUID_StyleTrackStyle. 0xffffffff indicates to look at
    // tracks in all TrackGroups in the segment. The first 0 indicates to
    // retrieve the Style from the first Track  in the indicated TrackGroup.
    // The second 0 indicates to retrieve the Style from the beginning of the
    // segment, i.e. time 0 in Segment time. If this Segment was loaded from a
    // section file, there is only one Style and it is at time 0.
    return m_pSegment->GetParam( GUID_IDirectMusicStyle, 0xffffffff, dwStyleIndex, 
                                 0, NULL, (VOID*)ppStyle );
}





//-----------------------------------------------------------------------------
// Name: C3DMusicSegment::C3DMusicSegment()
// Desc: Constructs the class
//-----------------------------------------------------------------------------
C3DMusicSegment::C3DMusicSegment( IDirectMusicPerformance8* pPerformance, 
                              IDirectMusicLoader8*      pLoader,
                              IDirectMusicSegment8*     pSegment,
                              IDirectMusicAudioPath8* pAudioPath ) :
    CMusicSegment( pPerformance, pLoader, pSegment )
{
    m_p3DAudioPath      = pAudioPath;
    m_pDS3DBuffer       = NULL;
    m_bDeferSettings    = FALSE;
    m_bCleanupAudioPath = FALSE;
}




//-----------------------------------------------------------------------------
// Name: C3DMusicSegment::~C3DMusicSegment()
// Desc: Destroys the class
//-----------------------------------------------------------------------------
C3DMusicSegment::~C3DMusicSegment()
{
    SAFE_RELEASE( m_pDS3DBuffer );
    if( m_bCleanupAudioPath )
        SAFE_RELEASE( m_p3DAudioPath );
}




//-----------------------------------------------------------------------------
// Name: Init()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT C3DMusicSegment::Init()
{
    HRESULT hr;

    if( NULL == m_p3DAudioPath )
    {
        // Create a 3D audiopath with a 3d buffer.
        // We can then play all segments into this buffer and directly control its
        // 3D parameters.
        if( FAILED( hr = m_pPerformance->CreateStandardAudioPath( DMUS_APATH_DYNAMIC_3D, 
                                                                64, TRUE, &m_p3DAudioPath ) ) )
            return DXTRACE_ERR( TEXT("CreateStandardAudioPath"), hr );
        m_bCleanupAudioPath = TRUE;
    }

    // Get the 3D buffer in the audio path.
    if( FAILED( hr = m_p3DAudioPath->GetObjectInPath( 0, DMUS_PATH_BUFFER, 0,
                                                      GUID_NULL, 0, IID_IDirectSound3DBuffer, 
                                                      (LPVOID*) &m_pDS3DBuffer ) ) )
        return DXTRACE_ERR( TEXT("GetObjectInPath"), hr );

    // Get the 3D buffer parameters
    m_dsBufferParams.dwSize = sizeof(DS3DBUFFER);
    m_pDS3DBuffer->GetAllParameters( &m_dsBufferParams );

    // Set new 3D buffer parameters
    m_dsBufferParams.dwMode = DS3DMODE_HEADRELATIVE;
    m_pDS3DBuffer->SetAllParameters( &m_dsBufferParams, DS3D_IMMEDIATE );
   
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Play
// Desc: 
//-----------------------------------------------------------------------------
HRESULT C3DMusicSegment::Play( DWORD dwFlags, IDirectMusicAudioPath8* pAudioPath )
{
    if( pAudioPath ) 
        return CMusicSegment::Play( dwFlags, pAudioPath );
    else
        return CMusicSegment::Play( dwFlags, m_p3DAudioPath );
}




//-----------------------------------------------------------------------------
// Name: Set3DParameters
// Desc: 
//-----------------------------------------------------------------------------
VOID C3DMusicSegment::Set3DParameters( FLOAT fMinDistance, FLOAT fMaxDistance )
{
    // Every change to 3-D sound buffer and listener settings causes 
    // DirectSound to remix, at the expense of CPU cycles. 
    // To minimize the performance impact of changing 3-D settings, 
    // use the DS3D_DEFERRED flag in the dwApply parameter of any of 
    // the IDirectSound3DListener or IDirectSound3DBuffer methods that 
    // change 3-D settings. Then call the IDirectSound3DListener::CommitDeferredSettings 
    // method to execute all of the deferred commands at once.
    DWORD dwApplyFlag = ( m_bDeferSettings ) ? DS3D_DEFERRED : DS3D_IMMEDIATE;

    m_dsBufferParams.flMinDistance = fMinDistance;
    m_dsBufferParams.flMaxDistance = fMaxDistance;

    if( m_pDS3DBuffer )
        m_pDS3DBuffer->SetAllParameters( &m_dsBufferParams, dwApplyFlag );
}




//-----------------------------------------------------------------------------
// Name: SetObjectProperties
// Desc: 
//-----------------------------------------------------------------------------
VOID C3DMusicSegment::SetObjectProperties( D3DVECTOR* pvPosition, D3DVECTOR* pvVelocity )
{
    DWORD dwApplyFlag = ( m_bDeferSettings ) ? DS3D_DEFERRED : DS3D_IMMEDIATE;

    // Every change to 3-D sound buffer and listener settings causes 
    // DirectSound to remix, at the expense of CPU cycles. 
    // To minimize the performance impact of changing 3-D settings, 
    // use the DS3D_DEFERRED flag in the dwApply parameter of any of 
    // the IDirectSound3DListener or IDirectSound3DBuffer methods that 
    // change 3-D settings. Then call the IDirectSound3DListener::CommitDeferredSettings 
    // method to execute all of the deferred commands at once.
    memcpy( &m_dsBufferParams.vPosition, pvPosition, sizeof(D3DVECTOR) );
    memcpy( &m_dsBufferParams.vVelocity, pvVelocity, sizeof(D3DVECTOR) );

    if( m_pDS3DBuffer )
    {
        m_pDS3DBuffer->SetPosition( m_dsBufferParams.vPosition.x,
                                    m_dsBufferParams.vPosition.y,
                                    m_dsBufferParams.vPosition.z, dwApplyFlag );

        m_pDS3DBuffer->SetVelocity( m_dsBufferParams.vVelocity.x,
                                    m_dsBufferParams.vVelocity.y,
                                    m_dsBufferParams.vVelocity.z, dwApplyFlag );
    }
}



//-----------------------------------------------------------------------------
// Name: CMusicScript::CMusicScript()
// Desc: Constructs the class
//-----------------------------------------------------------------------------
CMusicScript::CMusicScript( IDirectMusicPerformance8* pPerformance, 
                            IDirectMusicLoader8* pLoader,                   
                            IDirectMusicScript8* pScript )
{
    m_pPerformance = pPerformance;
    m_pLoader      = pLoader;
    m_pScript      = pScript;
}




//-----------------------------------------------------------------------------
// Name: CMusicScript::~CMusicScript()
// Desc: Destroys the class
//-----------------------------------------------------------------------------
CMusicScript::~CMusicScript()
{
    if( m_pLoader )
    {
        // Tell the loader that this object should now be released
        m_pLoader->ReleaseObjectByUnknown( m_pScript );
        m_pLoader = NULL;
    }

    SAFE_RELEASE( m_pScript ); 
    m_pPerformance = NULL;
}




//-----------------------------------------------------------------------------
// Name: CMusicScript::Play()
// Desc: Calls a routine in the script
//-----------------------------------------------------------------------------
HRESULT CMusicScript::CallRoutine( TCHAR* strRoutine )
{
    // DMusic only takes wide strings
    WCHAR wstrRoutine[MAX_PATH];
    DXUtil_ConvertGenericStringToWideCb( wstrRoutine, strRoutine, sizeof(wstrRoutine) );

    return m_pScript->CallRoutine( wstrRoutine, NULL );
}




//-----------------------------------------------------------------------------
// Name: CMusicScript::SetVariableNumber()
// Desc: Sets the value of a variable in the script
//-----------------------------------------------------------------------------
HRESULT CMusicScript::SetVariableNumber( TCHAR* strVariable, LONG lValue )
{
    // DMusic only takes wide strings
    WCHAR wstrVariable[MAX_PATH];
    DXUtil_ConvertGenericStringToWideCb( wstrVariable, strVariable, sizeof(wstrVariable) );

    return m_pScript->SetVariableNumber( wstrVariable, lValue, NULL );
}




//-----------------------------------------------------------------------------
// Name: CMusicScript::GetVariableNumber()
// Desc: Gets the value of a variable in the script
//-----------------------------------------------------------------------------
HRESULT CMusicScript::GetVariableNumber( TCHAR* strVariable, LONG* plValue )
{
    // DMusic only takes wide strings
    WCHAR wstrVariable[MAX_PATH];
    DXUtil_ConvertGenericStringToWideCb( wstrVariable, strVariable, sizeof(wstrVariable) );

    return m_pScript->GetVariableNumber( wstrVariable, plValue, NULL );
}



//-----------------------------------------------------------------------------
// Name: CMusicScript::SetVariableObject()
// Desc: Sets an object in the script
//-----------------------------------------------------------------------------
HRESULT CMusicScript::SetVariableObject( TCHAR* strVariable, IUnknown *punkValue )
{
    // DMusic only takes wide strings
    WCHAR wstrVariable[MAX_PATH];
    DXUtil_ConvertGenericStringToWideCb( wstrVariable, strVariable, sizeof(wstrVariable) );

    return m_pScript->SetVariableObject( wstrVariable, punkValue, NULL );
}




//-----------------------------------------------------------------------------
// Name: CMusicScript::GetVariableObject()
// Desc: Gets an object from the script
//-----------------------------------------------------------------------------
HRESULT CMusicScript::GetVariableObject( TCHAR* strVariable, REFIID riid, LPVOID FAR *ppv )
{
    // DMusic only takes wide strings
    WCHAR wstrVariable[MAX_PATH];
    DXUtil_ConvertGenericStringToWideCb( wstrVariable, strVariable, sizeof(wstrVariable) );

    return m_pScript->GetVariableObject( wstrVariable, riid, ppv, NULL );
}

