//-----------------------------------------------------------------------------
// File: DMUtil.h
//
// Desc: DirectMusic framework classes for playing DirectMusic segments and
//       DirectMusic scripts. Feel free to use this class as a starting point 
//       for adding extra functionality.
//
// Copyright (c) Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef DMUTIL_H
#define DMUTIL_H

#include <dmusicc.h>
#include <dmusici.h>
#include <dsound.h>


//-----------------------------------------------------------------------------
// Classes used by this header
//-----------------------------------------------------------------------------
class CMusicManager;
class CMusicSegment;
class C3DMusicSegment;
class CMusicScript;




//-----------------------------------------------------------------------------
// Name: class CMusicManager
// Desc: 
//-----------------------------------------------------------------------------
class CMusicManager
{
protected:
    BOOL                      m_bCleanupCOM;
    IDirectMusicLoader8*      m_pLoader;
    IDirectMusicPerformance8* m_pPerformance;
    IDirectSound3DListener*   m_pDSListener;
    DS3DLISTENER              m_dsListenerParams;                // Listener properties

public:
    CMusicManager();
    ~CMusicManager();

    inline IDirectMusicLoader8*      GetLoader()      { return m_pLoader; }
    inline IDirectMusicPerformance8* GetPerformance() { return m_pPerformance; }
    inline IDirectSound3DListener*   GetListener()    { return m_pDSListener; }

    IDirectMusicAudioPath8* GetDefaultAudioPath();

    HRESULT Initialize( HWND hWnd, DWORD dwPChannels = 128, DWORD dwDefaultPathType = DMUS_APATH_DYNAMIC_STEREO, LPDIRECTSOUND pDS = NULL  );

    HRESULT SetSearchDirectory( const TCHAR* strMediaPath );
    VOID    CollectGarbage();
    VOID    StopAll();

    HRESULT CreateSegmentFromFile( CMusicSegment** ppSegment, TCHAR* strFileName, 
		                           BOOL bDownloadNow = TRUE, BOOL bIsMidiFile = FALSE );
    HRESULT Create3DSegmentFromFile( C3DMusicSegment** ppSegment, TCHAR* strFileName, 
		                           BOOL bDownloadNow = TRUE, BOOL bIsMidiFile = FALSE, 
                                   IDirectMusicAudioPath8* p3DAudioPath = NULL );
    HRESULT CreateScriptFromFile( CMusicScript** ppScript, TCHAR* strFileName );

    HRESULT CreateChordMapFromFile( IDirectMusicChordMap8** ppChordMap, TCHAR* strFileName );
    HRESULT CreateStyleFromFile( IDirectMusicStyle8** ppStyle, TCHAR* strFileName );
    HRESULT GetMotifFromStyle( IDirectMusicSegment8** ppMotif, TCHAR* strStyle, TCHAR* wstrMotif );

    HRESULT CreateSegmentFromResource( CMusicSegment** ppSegment, TCHAR* strResource, TCHAR* strResourceType, 
		                           BOOL bDownloadNow = TRUE, BOOL bIsMidiFile = FALSE );

    VOID Set3DParameters( FLOAT fDistanceFactor, FLOAT fDopplerFactor, FLOAT fRolloffFactor );
};




//-----------------------------------------------------------------------------
// Name: class CMusicSegment
// Desc: Encapsulates functionality of an IDirectMusicSegment
//-----------------------------------------------------------------------------
class CMusicSegment
{
protected:
    IDirectMusicSegment8*     m_pSegment;
    IDirectMusicLoader8*      m_pLoader;
    IDirectMusicPerformance8* m_pPerformance;
    IDirectMusicAudioPath8*   m_pEmbeddedAudioPath;
    BOOL                      m_bDownloaded;

public:
    CMusicSegment( IDirectMusicPerformance8* pPerformance, 
                   IDirectMusicLoader8* pLoader,
                   IDirectMusicSegment8* pSegment );
    virtual ~CMusicSegment();

    inline  IDirectMusicSegment8* GetSegment() { return m_pSegment; }
    HRESULT GetStyle( IDirectMusicStyle8** ppStyle, DWORD dwStyleIndex = 0 );

    HRESULT SetRepeats( DWORD dwRepeats );
    virtual HRESULT Play( DWORD dwFlags = DMUS_SEGF_SECONDARY, IDirectMusicAudioPath8* pAudioPath = NULL );
    HRESULT Stop( DWORD dwFlags = 0 );
    HRESULT Download( IDirectMusicAudioPath8* pAudioPath = NULL );
    HRESULT Unload( IDirectMusicAudioPath8* pAudioPath = NULL );

    BOOL    IsPlaying();
};




//-----------------------------------------------------------------------------
// Name: class CMusicSegment
// Desc: Encapsulates functionality of an IDirectMusicSegment
//-----------------------------------------------------------------------------
class C3DMusicSegment : public CMusicSegment
{
protected:
    IDirectMusicAudioPath8* m_p3DAudioPath;
    IDirectSound3DBuffer*   m_pDS3DBuffer;

    DS3DBUFFER              m_dsBufferParams;                  // 3D buffer properties
    BOOL                    m_bDeferSettings;
    BOOL                    m_bCleanupAudioPath;

public:
    C3DMusicSegment( IDirectMusicPerformance8* pPerformance, 
                   IDirectMusicLoader8* pLoader,
                   IDirectMusicSegment8* pSegment,
                   IDirectMusicAudioPath8* pAudioPath );
    virtual ~C3DMusicSegment();

    HRESULT Init();
    IDirectMusicAudioPath8* GetAudioPath() { return m_p3DAudioPath; }
    HRESULT Play( DWORD dwFlags = DMUS_SEGF_SECONDARY, IDirectMusicAudioPath8* pAudioPath = NULL );

    VOID Set3DParameters( FLOAT fMinDistance,   FLOAT fMaxDistance );
    VOID SetObjectProperties( D3DVECTOR* pvPosition, D3DVECTOR* pvVelocity );
};




//-----------------------------------------------------------------------------
// Name: class CMusicScript
// Desc: Encapsulates functionality of an IDirectMusicScript
//-----------------------------------------------------------------------------
class CMusicScript
{
protected:
    IDirectMusicScript8*      m_pScript;
    IDirectMusicLoader8*      m_pLoader;
    IDirectMusicPerformance8* m_pPerformance;

public:
    CMusicScript( IDirectMusicPerformance8* pPerformance, 
                  IDirectMusicLoader8* pLoader,
                  IDirectMusicScript8* pScript );
    virtual ~CMusicScript();

    inline  IDirectMusicScript8* GetScript() { return m_pScript; }

    HRESULT CallRoutine( TCHAR* strRoutine );
    HRESULT SetVariableNumber( TCHAR* strVariable, LONG lValue );
    HRESULT GetVariableNumber( TCHAR* strVariable, LONG* plValue );
    HRESULT SetVariableObject( TCHAR* strVariable, IUnknown *punkValue);
    HRESULT GetVariableObject( TCHAR* strVariable, REFIID riid, LPVOID FAR *ppv);
};




#endif // DMUTIL_H
