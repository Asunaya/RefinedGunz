//-----------------------------------------------------------------------------
// File: DIDevImg.h
//
// Desc: Header for CDIDevImage class, which encapsulates methods for drawing
//       device images, callout strings, and object highlights.
//
// This code is provided to aid developers in creating custom interfaces for
// device configuration. The provided interface is flexible enough for most
// situations, but is freely modifyable.
//
// The CDIDevImage class acts as the interface to the UI client, and the 
// exposed public methods should be called by the client to set the drawing
// options and perform renderings. The CDIDIObject class is used as a helper
// class, and the exposed public methods should not be called by client code.
//
// Copyright( c ) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef __DIDEVIMG_H__
#define __DIDEVIMG_H__

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif

#include <windows.h>
#include <dinput.h>
#include <d3d9.h>
#include <tchar.h>

// macros
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#endif //SAFE_DELETE

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)  { if(p) { delete[] (p);     (p)=NULL; } }
#endif //SAFE_DELETE_ARRAY

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#endif //SAFE_RELEASE

// global constants
#define DIDICONST_MAX_IMAGE_WIDTH                 1024
#define DIDICONST_MAX_IMAGE_HEIGHT                1024
#define DIDICONST_CUSTOM_VIEW_WIDTH                400
#define DIDICONST_CUSTOM_VIEW_HEIGHT               300

// SetCalloutState flags
#define DIDICOS_HIGHLIGHTED                 0x00000001
#define DIDICOS_INVISIBLE                   0x00000002
#define DIDICOS_TOOLTIP                     0x00000004

// SetOutputImageSize flags
#define DIDISOIS_DEFAULT                    0x00000000
#define DIDISOIS_RESIZE                     0x00000001
#define DIDISOIS_MAINTAINASPECTUSINGWIDTH   0x00000002
#define DIDISOIS_MAINTAINASPECTUSINGHEIGHT  0x00000004

// Possible render targets
enum DIDIRENDERTARGET
{
    DIDIRT_SURFACE,
    DIDIRT_DC
};

// Custom HRESULTs
#define DI_IMAGENOTFOUND     \
    MAKE_HRESULT(0, FACILITY_WIN32, ERROR_FILE_NOT_FOUND)

// Forward references
class CDIDIObject;
typedef TCHAR MAXSTRING[MAX_PATH];




//-----------------------------------------------------------------------------
// Name: CDIDevImage
// Desc: Handles device image drawing 
//-----------------------------------------------------------------------------
class CDIDevImage
{
public:
    // __________________________
    // Constructors / Destructors
    CDIDevImage();
    ~CDIDevImage();

    // ______________
    // Public Methods
    HRESULT Init( LPDIRECTINPUTDEVICE8 pDID );
    HRESULT SetCalloutState( DWORD dwObjId, DWORD dwCalloutState );
    HRESULT GetCalloutState( DWORD dwObjId, LPDWORD pdwCalloutState );
    HRESULT SetCalloutColors( DWORD dwObjId, COLORREF crColorNormal, COLORREF crColorHigh );
    HRESULT GetCalloutColors( DWORD dwObjId, LPCOLORREF pcrColorNormal, LPCOLORREF pcrColorHigh );
    HRESULT SetCalloutText( DWORD dwObjId, LPCTSTR strText );
    HRESULT GetCalloutText( DWORD dwObjId, LPTSTR strText, DWORD dwSize );
    HRESULT GetObjFromPoint( POINT Pt, LPDWORD pdwObjId );
    HRESULT SetActiveView( DWORD dwView );
    HRESULT GetActiveView( LPDWORD pdwView, LPDWORD pdwNumViews = NULL );
    HRESULT GetViewForObj( DWORD dwObjId, LPDWORD pdwView );
    HRESULT SetOutputImageSize( DWORD dwWidth, DWORD dwHeight, DWORD dwFlags );
    HRESULT SetFont( HFONT hFont );
    HRESULT SetColors( D3DCOLOR Background, COLORREF crCalloutNormal, COLORREF crCalloutHigh );
    HRESULT Render( LPDIRECT3DTEXTURE9 pTexture );
    HRESULT RenderToDC( HDC hDC );

    friend BOOL CALLBACK EnumDeviceObjectsCB( LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef );

protected:
    // _________________
    // Protected Methods
    VOID    CleanUp();
    HRESULT LoadImageInfo( LPDIRECTINPUTDEVICE8 pDIDevice );
    HRESULT RenderToTarget( LPVOID pvTarget, DIDIRENDERTARGET eTarget );
    HRESULT RenderCustomToTarget( LPVOID pvTarget, DIDIRENDERTARGET eTarget );
    HRESULT CreateCustomImageInfo( LPDIRECTINPUTDEVICE8 pDIDevice );
    HRESULT LoadImages();
    HRESULT BuildCustomUI();
    HRESULT CreateScaledSurfaceCopy( LPDIRECT3DSURFACE9 pSurfaceSrc, DWORD dwWidthSrc, DWORD dwHeightSrc, 
                                     FLOAT fxScale, FLOAT fyScale, LPDIRECT3DSURFACE9 *ppSurfaceDest );
    VOID    CreateFont();
    VOID    DestroyImages();
    HRESULT GetCustomUISize( SIZE* pSize );

    HRESULT AddObject( DWORD dwID );
    CDIDIObject* GetObject( DWORD dwID );


    // _________________
    // Protected Members
    BOOL                     m_bInitialized;    // Init method has been successfully run
    BOOL                     m_bCustomUI;       // ImageInfo not found. UI is built by program
    BOOL                     m_bInvalidUI;      // The custom UI needs to be recreated
    MAXSTRING*               m_atszImagePath;   // Array of MAXSTRING strings for background images
    HBITMAP*                 m_ahImages;        // Array of pointers to background bitmaps

    DWORD                    m_dwActiveView;    // Index of the currently active view
    DWORD                    m_dwNumViews;      // Total number of views for this devic3    
    
    D3DCOLOR                 m_BkColor;         // Background color for main device image
    HFONT                    m_hFont;           // Font to be used when drawing all text
    
    CDIDIObject**            m_apObject;        // Array of pointers to CDIDIObject data objects
    DWORD                    m_dwNumObjects;    // Total number of objects
    
    DWORD                    m_dwWidthPref;     // User-set preferred width
    DWORD                    m_dwHeightPref;    // User-set preffered height
    DWORD                    m_dwScaleMethod;   // Method to use when scaling to preferred size
                    

};




//-----------------------------------------------------------------------------
// Name: DIDICallout
// Desc: Encapsulates information a specific view callout
//-----------------------------------------------------------------------------
struct DIDICallout
{
    RECT  rcInit;
    RECT  rcScaled;
    POINT aptLineInit[5];
    POINT aptLineScaled[5];
    DWORD dwNumPoints;
    DWORD dwTextAlign;
};



//-----------------------------------------------------------------------------
// Name: DIDIOverlay
// Desc: Encapsulates information a specific view overlay
//-----------------------------------------------------------------------------
struct DIDIOverlay
{
    MAXSTRING strImagePath;
    RECT     rcInit;
    RECT     rcScaled;
    HBITMAP  hImage;
};




//-----------------------------------------------------------------------------
// Name: CDIDIObject
// Desc: Encapsulates information about the device object (axis, button, POV)
//-----------------------------------------------------------------------------
class CDIDIObject
{
public:
    // __________________________
    // Constructors / Destructors
    CDIDIObject( DWORD dwID, DWORD dwNumViews ); 
    ~CDIDIObject();


    // ______________
    // Public Methods
    VOID    SetOverlay( DWORD dwViewID, LPCTSTR tszImagePath, RECT rect );
    VOID    SetCallout( DWORD dwViewID, DWORD dwNumPoints, POINT *aptLine, RECT rect, DWORD dwTextAlign );
    VOID    SetCalloutState( DWORD dwCalloutState ) {m_dwState = dwCalloutState;}
    DWORD   GetCalloutState() {return m_dwState;}
    VOID    SetCalloutColors( COLORREF crColorNormal, COLORREF crColorHigh ) { m_crNormColor = crColorNormal; m_crHighColor = crColorHigh; }
    VOID    GetCalloutColors( LPCOLORREF pcrColorNormal, LPCOLORREF pcrColorHigh ) { if( pcrColorNormal ) *pcrColorNormal = m_crNormColor; if( pcrColorHigh ) *pcrColorHigh = m_crHighColor; }
    VOID    SetCalloutText( LPCTSTR strText );
    VOID    GetCalloutText( LPTSTR strText, DWORD dwSize );
    DWORD   GetID() { return m_dwID; }
    VOID    GetName( LPTSTR strName, DWORD dwSize ) { _tcsncpy( strName, m_strName, dwSize ); }
    VOID    SetName( LPCTSTR strName ) { _tcsncpy( m_strName, strName, MAX_PATH-4 ); }
    VOID    DestroyImages();
    HRESULT AllocateViews( DWORD dwNumViews );
    VOID    ScaleView( DWORD dwViewID, FLOAT fxScale, FLOAT fyScale );

    DIDICallout* GetCallout( DWORD dwViewID ) { return &m_aCallout[dwViewID]; }
    DIDIOverlay* GetOverlay( DWORD dwViewID ) { return &m_aOverlay[dwViewID]; }

private:
    // _______________
    // Private Members
    DWORD         m_dwID;
    MAXSTRING     m_strCallout;
    MAXSTRING     m_strName;

    COLORREF      m_crNormColor;
    COLORREF      m_crHighColor;

    DWORD         m_dwState;
    DWORD         m_dwNumViews;

    DIDICallout  *m_aCallout;
    DIDIOverlay  *m_aOverlay;

};

//-----------------------------------------------------------------------------
// Rendering constants and inline functions
//-----------------------------------------------------------------------------
const DWORD ALPHA_MASK = D3DCOLOR_ARGB(255, 0, 0, 0);
const DWORD RED_MASK   = D3DCOLOR_ARGB(0, 255, 0, 0);
const DWORD GREEN_MASK = D3DCOLOR_ARGB(0, 0, 255, 0);
const DWORD BLUE_MASK  = D3DCOLOR_ARGB(0, 0, 0, 255);


//-----------------------------------------------------------------------------
// Name: GetAlpha, GetRed, GetGreen, GetBlue
// Desc: Extracts the specified color component
//-----------------------------------------------------------------------------
inline BYTE GetAlpha( D3DCOLOR Color ) { return (BYTE)( Color >> 24 ); }
inline BYTE   GetRed( D3DCOLOR Color ) { return (BYTE)( Color >> 16 ); }
inline BYTE GetGreen( D3DCOLOR Color ) { return (BYTE)( Color >> 8  ); }
inline BYTE  GetBlue( D3DCOLOR Color ) { return (BYTE)( Color );       }




//-----------------------------------------------------------------------------
// Name: ColorFromCR
// Desc: Returns a D3DCOLOR from the given COLORREF
//-----------------------------------------------------------------------------
inline D3DCOLOR ColorFromCR( COLORREF cr ) 
{ 
    return GetRValue(cr) << 16 | GetGValue(cr) << 8 | GetBValue(cr); 
}




//-----------------------------------------------------------------------------
// Name: CRFromColor
// Desc: Returns a COLORREF from the given D3DCOLOR
//-----------------------------------------------------------------------------
inline COLORREF CRFromColor( D3DCOLOR color ) 
{ 
    return GetBlue(color) << 16 | GetGreen(color) << 8 | GetRed(color); 
}




//-----------------------------------------------------------------------------
// Name: ScaleRect
// Desc: Convenience inline function for scaling a RECT structure
//-----------------------------------------------------------------------------
inline VOID ScaleRect( const RECT* pRectSrc, RECT* pRectDest, FLOAT fxScale, FLOAT fyScale )
{
    pRectDest->top    = (LONG) ( fyScale * pRectSrc->top + 0.5 );
    pRectDest->bottom = (LONG) ( fyScale * pRectSrc->bottom + 0.5 );
    pRectDest->left   = (LONG) ( fxScale * pRectSrc->left + 0.5);
    pRectDest->right  = (LONG) ( fxScale * pRectSrc->right + 0.5);
}




//-----------------------------------------------------------------------------
// Name: ScalePoint
// Desc: Convenience inline function for scaling a POINT structure
//-----------------------------------------------------------------------------
inline VOID ScalePoint( const POINT* pPointSrc, POINT* pPointDest, FLOAT fxScale, FLOAT fyScale )
{
    pPointDest->x    = (LONG) ( fxScale * pPointSrc->x + 0.5 );
    pPointDest->y    = (LONG) ( fyScale * pPointSrc->y + 0.5 );
}




//-----------------------------------------------------------------------------
// Helper Functions
// These external functions don't require direct access to the private member
// variables, but are used at one or more points within the class methods.
//-----------------------------------------------------------------------------
HRESULT ApplyOverlay( HBITMAP hbmpDest, CONST RECT* prcDest, HBITMAP hbmpSrc );
HRESULT ApplyAlphaChannel( HBITMAP hbmpDest, HBITMAP hbmpAlpha, BOOL bOpaque );
HRESULT FillBackground( HBITMAP hbmpDest, D3DCOLOR Fill );
HRESULT CreateDIBSectionFromSurface( LPDIRECT3DSURFACE9 pSurface, HBITMAP* phBitmap, SIZE* pSize = NULL ); 
HRESULT RestoreRect( HBITMAP hbmpDest, CONST RECT* prcDest, LPBYTE pSrcPixels );
HRESULT DrawTooltip( HDC hdcRender, HDC hdcAlpha, TCHAR* strTooltip, RECT* prcBitmap, 
                     RECT* prcTruncated, COLORREF crFore, COLORREF crBack, COLORREF crBorder );
IDirect3DSurface9* GetCloneSurface( int iWidth, int iHeight );

BOOL CALLBACK EnumDeviceObjectsCB( LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef );


#endif  //__DIDEVIMG_H__