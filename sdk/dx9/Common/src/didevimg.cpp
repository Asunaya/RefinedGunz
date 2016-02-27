//-----------------------------------------------------------------------------
// File: DIDevImg.cpp
//
// Desc: Implementation for CDIDevImage class, which encapsulates methods for 
//       drawing device images, callout strings, and object highlights.
//
// Copyright( c ) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "DIDevImg.h"
#include <d3dx9tex.h>



//-----------------------------------------------------------------------------
// Name: CDIDevImage
// Desc: Constructor
//-----------------------------------------------------------------------------
CDIDevImage::CDIDevImage() 
{
    m_bInitialized  = FALSE;
    m_bCustomUI     = FALSE;
    m_bInvalidUI    = TRUE;
    m_ahImages      = NULL;
    m_dwNumViews    = 0;
    m_dwNumObjects  = 0;
    m_apObject      = NULL;
    m_dwWidthPref   = 0;
    m_dwHeightPref  = 0;
    m_dwScaleMethod = 0;
    m_BkColor       = D3DCOLOR_ARGB(255, 0, 0, 0);
    m_hFont         = NULL;
}




//-----------------------------------------------------------------------------
// Name: ~CDIDevImage
// Desc: Destructor
//-----------------------------------------------------------------------------
CDIDevImage::~CDIDevImage()
{
    CleanUp();
}




//-----------------------------------------------------------------------------
// Name: Init
// Desc: Responsible for initializing and preparing the CDIDevImage object for
//       rendering the device image. Init must be called before the other
//       public functions.
// Args: pDIDevice - Pointer to a DirectInputDevice object for which the
//         configuration UI should be created.
//  Ret: DI_OK - Success; image found.
//       DI_IMAGENOTFOUND - Success; no image. Default UI created instead.
//       DIERR_INVALIDPARAM - Fail; invalid argument passed.
//-----------------------------------------------------------------------------    
HRESULT CDIDevImage::Init( LPDIRECTINPUTDEVICE8 pDIDevice )
{
    HRESULT hr = DI_OK;
    
    // Sanity check
    if( NULL == pDIDevice )
        return DIERR_INVALIDPARAM;

    // Always start clean
    CleanUp();
    
    // retrieve the image info from DirectInput
    hr = LoadImageInfo( pDIDevice );
    if( FAILED(hr) )
    {
        // Image information may have partially loaded. Clean out whatever
        // is stored since we'll need to create a custom UI from scratch.
        CleanUp();


        // For one reason or another, the image info for this device
        // could not be loaded. At this point, create a default UI.
        m_bCustomUI = true;
        hr = CreateCustomImageInfo( pDIDevice );
        if( FAILED(hr) )
        {
            SAFE_RELEASE(pDIDevice);
            CleanUp();
            return hr;
        }
    }
    
    // Create the default callout font
    CreateFont();

    // Made it through initialization. Set the initialized flag to allow the
    // other member functions to be called.
    m_bInitialized = true;

    // Both of these values indicate success, but the client may wish to know
    // whether an image was actually found for this device, or if we are just
    // producing a default UI.
    return m_bCustomUI ? DI_IMAGENOTFOUND : DI_OK;
}




//-----------------------------------------------------------------------------
// Name: SetCalloutState
// Desc: Sets the state for a specific callout
// Args: dwObjId - Object ID of the requested callout. This corresponds to the
//         dwType value returned by EnumDeviceObjects.
//       dwCalloutState - New state of the callout, which may be zero or more
//         combinations of:
//         DIDICOS_HIGHLIGHTED - Overlay drawn. Callout drawn in high color.
//         DIDICOS_INVISIBLE - Overlay and callout string not drawn.
//         DIDICOS_TOOLTIP - Tooltip drawn if callout text is truncated.
//  Ret: DI_OK - Success.
//       DIERR_NOTINITIALIZED - Fail; Init() must be called first.
//       DIERR_OBJECTNOTFOUND - Fail; Given ID not found in list of objects
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::SetCalloutState( DWORD dwObjId, DWORD dwCalloutState )
{
    // Verify initialization
    if( false == m_bInitialized )
        return DIERR_NOTINITIALIZED;

    CDIDIObject* pObject = GetObject( dwObjId );
    if( NULL == pObject )
        return DIERR_OBJECTNOTFOUND;
                 
    DWORD dwOldState = pObject->GetCalloutState();
    pObject->SetCalloutState( dwCalloutState );

    // This action might change the layout for the custom UI
    if( m_bCustomUI &&
        ( DIDICOS_INVISIBLE & dwOldState ||
          DIDICOS_INVISIBLE & dwCalloutState ) )
          m_bInvalidUI = TRUE;

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: GetCalloutState
// Desc: Returns the state of a specific callout
// Args: dwObjId - Object ID of the requested callout. This corresponds to the
//         dwType value returned by EnumDeviceObjects.
//       pdwCalloutState - Pointer to a variable which will contain the current
//         callout value after a successful return.
//  Ret: DI_OK - Success.
//       DIERR_NOTINITIALIZED - Fail; Init() must be called first.
//       DIERR_OBJECTNOTFOUND - Fail; Given ID not found in list of objects
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::GetCalloutState( DWORD dwObjId, LPDWORD pdwCalloutState )
{
    // Verify initialization
    if( false == m_bInitialized )
        return DIERR_NOTINITIALIZED;

    CDIDIObject* pObject = GetObject( dwObjId );
    if( NULL == pObject )
        return DIERR_OBJECTNOTFOUND;

    *pdwCalloutState = pObject->GetCalloutState();
    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: SetCalloutColors
// Desc: Sets the callout-unique colors to be used when painting a specific
//       callout
// Args: dwObjId - Object ID of the requested callout. This corresponds to the
//         dwType value returned by EnumDeviceObjects.
//       crColorNormal - Foreground text color for callout strings in a normal
//         state.
//       crColorHigh - Foreground text color for callout strings in a
//         highlighted state.
//  Ret: DI_OK - Success.
//       DIERR_NOTINITIALIZED - Fail; Init() must be called first.
//       DIERR_OBJECTNOTFOUND - Fail; Given ID not found in list of objects
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::SetCalloutColors( DWORD dwObjId, COLORREF crColorNormal, COLORREF crColorHigh )
{
    // Verify initialization
    if( false == m_bInitialized )
        return DIERR_NOTINITIALIZED;

    CDIDIObject* pObject = GetObject( dwObjId );
    if( NULL == pObject )
        return DIERR_OBJECTNOTFOUND;

    pObject->SetCalloutColors( crColorNormal, crColorHigh );
    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: GetCalloutColors
// Desc: Obtains the callout-unique colors used when painting a specific
//       callout
// Args: dwObjId - Object ID of the requested callout. This corresponds to the
//         dwType value returned by EnumDeviceObjects.
//       pcrColorNormal - Pointer to a COLORREF variable which will contain
//         the normal callout color after a successful return. May be NULL.
//       pcrColorHigh - Pointer to a COLORREF variable which will contain
//         the highlight callout color after a successful return. May be NULL.
//  Ret: DI_OK - Success.
//       DIERR_NOTINITIALIZED - Fail; Init() must be called first.
//       DIERR_OBJECTNOTFOUND - Fail; Given ID not found in list of objects
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::GetCalloutColors( DWORD dwObjId, LPCOLORREF pcrColorNormal, LPCOLORREF pcrColorHigh )
{
    // Verify initialization
    if( false == m_bInitialized )
        return DIERR_NOTINITIALIZED;

    CDIDIObject* pObject = GetObject( dwObjId );
    if( NULL == pObject )
        return DIERR_OBJECTNOTFOUND;

    pObject->GetCalloutColors( pcrColorNormal, pcrColorHigh );
    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: SetCalloutText
// Desc: Sets the text associated with the callout specified by an object ID
// Args: dwObjId - Object ID of the requested callout. This corresponds to the
//         dwType value returned by EnumDeviceObjects.
//       strText - New callout text.
//  Ret: DI_OK - Success.
//       DIERR_INVALIDPARAM - Fail; Null pointer passed.
//       DIERR_NOTINITIALIZED - Fail; Init() must be called first.
//       DIERR_OBJECTNOTFOUND - Fail; Given ID not found in list of objects
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::SetCalloutText( DWORD dwObjId, LPCTSTR strText )
{
    // Verify initialization
    if( false == m_bInitialized )
        return DIERR_NOTINITIALIZED;

    if( NULL == strText )
        return DIERR_INVALIDPARAM;

    CDIDIObject* pObject = GetObject( dwObjId );
    if( NULL == pObject )
        return DIERR_OBJECTNOTFOUND;

    pObject->SetCalloutText( strText );

    // This action might change the layout for the custom UI
    if( m_bCustomUI )
        m_bInvalidUI = TRUE;

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: GetCalloutText
// Desc: Returns the text associated with a specific callout
// Args: dwObjId - Object ID of the requested callout. This corresponds to the
//         dwType value returned by EnumDeviceObjects.
//       pstrText - Pointer to a string buffer which will collect the current
//         callout string.
//       dwSize - Maximum number of characters to copy into the buffer.
//  Ret: DI_OK - Success.
//       DIERR_INVALIDPARAM - Fail; Null pointer passed.
//       DIERR_NOTINITIALIZED - Fail; Init() must be called first.
//       DIERR_OBJECTNOTFOUND - Fail; Given ID not found in list of objects
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::GetCalloutText( DWORD dwObjId, LPTSTR pstrText, DWORD dwSize )
{
    // Verify initialization
    if( false == m_bInitialized )
        return DIERR_NOTINITIALIZED;

    if( NULL == pstrText )
        return DIERR_INVALIDPARAM;

    CDIDIObject* pObject = GetObject( dwObjId );
    if( NULL == pObject )
        return DIERR_OBJECTNOTFOUND;

    pObject->GetCalloutText( pstrText, dwSize );

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: GetObjFromPoint
// Desc: Returns the ID of the object on the device corresponding to the
//       callout which contains the given point.
// Args: Pt - Point to check against callout rect coordinates
//       pdwObjId - Pointer to a variable which will contain the object ID upon
//         successful return.
//  Ret: DI_OK - Success.
//       DIERR_INVALIDPARAM - Fail; Null pointer passed.
//       DIERR_NOTINITIALIZED - Fail; Init() must be called first.
//       DIERR_OBJECTNOTFOUND - Fail; Given ID not found in list of objects
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::GetObjFromPoint( POINT Pt, LPDWORD pdwObjId )
{
    // Verify initialization
    if( false == m_bInitialized )
        return DIERR_NOTINITIALIZED;

    if( NULL == pdwObjId )
        return DIERR_INVALIDPARAM;

    if( m_dwActiveView > m_dwNumViews )
        return E_FAIL;

    // For a custom UI, this method depends on the UI being calculated
    if( m_bCustomUI && m_bInvalidUI )
        BuildCustomUI();

    DIDICallout *pCallout = NULL;

    // for each object
    for( UINT i=0; i < m_dwNumObjects; i++ )
    {
        pCallout = m_apObject[i]->GetCallout( m_dwActiveView );
        if( PtInRect( &(pCallout->rcScaled), Pt ) )
        {
            // if the point is inside the scaled bounding rect, the
            // correct callout has been found.
            *pdwObjId = m_apObject[i]->GetID();
            return DI_OK;
        }
    }

    return S_FALSE;
}




//-----------------------------------------------------------------------------
// Name: SetActiveView
// Desc: Activates the provided view. This view will be painted when the 
//       device image is rendered.
// Args: dwView - The index of the new view.
//  Ret: DI_OK - Success.
//       DIERR_INVALIDPARAM - Fail; View is out of range.
//       DIERR_NOTINITIALIZED - Fail; Init() must be called first.
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::SetActiveView( DWORD dwView )
{
    // Verify initialization
    if( false == m_bInitialized )
        return DIERR_NOTINITIALIZED;

    // For a custom UI, this method depends on the UI being calculated
    if( m_bCustomUI && m_bInvalidUI )
        BuildCustomUI();

    // Make sure view exists
    if( dwView >= m_dwNumViews )
        return DIERR_INVALIDPARAM;

    m_dwActiveView = dwView;
    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: GetActiveView
// Desc: Returns the index of the currently active view. The active view is
//       what CDIDevImage paints when the device image is rendered.
// Args: pdwView - Pointer to a variable which will contain the current view
//         index upon successful return. May be NULL.
//       pdwNumViews - Pointer to a variable which will contain the total
//         number of views upon successful return. May be NULL.
//  Ret: DI_OK - Success.
//       DIERR_NOTINITIALIZED - Fail; Init() must be called first.
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::GetActiveView( LPDWORD pdwView, LPDWORD pdwNumViews )
{
    // Verify initialization
    if( false == m_bInitialized )
        return DIERR_NOTINITIALIZED;

    // For a custom UI, this method depends on the UI being calculated
    if( m_bCustomUI && m_bInvalidUI )
        BuildCustomUI();

    if( pdwView )
        *pdwView = m_dwActiveView;

    if( pdwNumViews )
        *pdwNumViews = m_dwNumViews;

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: GetViewForObj
// Desc: Returns the index of the most appropriate view for a specific object
// Args: dwObjId - Object ID of the requested callout. This corresponds to the
//         dwType value returned by EnumDeviceObjects.
//       pdwView - Pointer to a variable which will contain an appropriate
//         view for the given object after return. May be NULL.
//  Ret: DI_OK - Success.
//       DIERR_NOTINITIALIZED - Fail; Init() must be called first.
//       DIERR_INVALIDPARAM - Fail; Null pointer.
//       DIERR_OBJECTNOTFOUND - Fail; Given ID not found in list of objects
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::GetViewForObj( DWORD dwObjId, LPDWORD pdwView )
{
    UINT i=0; // Loop variable

    // Verify initialization
    if( false == m_bInitialized )
        return DIERR_NOTINITIALIZED;

    if( NULL == pdwView )
        return DIERR_INVALIDPARAM;

    CDIDIObject* pObject = GetObject( dwObjId );
    if( NULL == pObject )
        return DIERR_OBJECTNOTFOUND;

    // For a custom UI, this method depends on the UI being calculated
    if( m_bCustomUI && m_bInvalidUI )
        BuildCustomUI();

    // The method used to determine the best view for a particular object
    // is simple: The view which has the largest overlay rectangle probably
    // has the best view for an object. If there are no overlays for the
    // given object, use the view with the largest callout rectangle, or
    // the base view (0) if there are no callouts for the given object.
    
    DWORD dwCalloutMax = 0;
    DWORD dwCalloutIndex = 0;

    DWORD dwOverlayMax = 0;
    DWORD dwOverlayIndex = 0;

    for( i=0; i < m_dwNumViews; i++ )
    {
        DWORD dwArea = 0;

        DIDIOverlay *pOverlay = pObject->GetOverlay( i );
        DIDICallout *pCallout = pObject->GetCallout( i );

        dwArea = ( pOverlay->rcScaled.right - pOverlay->rcScaled.left ) *
                 ( pOverlay->rcScaled.bottom - pOverlay->rcScaled.top );

        if( dwArea > dwOverlayMax )
        {
            dwOverlayMax = dwArea;
            dwOverlayIndex = i;
        }      

        dwArea = ( pCallout->rcScaled.right - pCallout->rcScaled.left ) *
                 ( pCallout->rcScaled.bottom - pCallout->rcScaled.top );

        if( dwArea > dwCalloutMax )
        {
            dwCalloutMax = dwArea;
            dwCalloutIndex = i;
        }      
    }
    
    // If an overlay rectangle was found, use the overlay index; otherwise,
    // use the callout index (this will be 0 if no callouts were found ).
    *pdwView = ( dwOverlayMax > 0 ) ? dwOverlayIndex : dwCalloutIndex;
    
    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: SetOutputImageSize
// Desc: Sets the size of the image that CDIDevImage will paint and output to
//       the application
// Args: dwWidth - Preferred width.
//       dwHeight - Preferred height.
//       dwFlags - Scaling flag. Must be one of following:
//         DIDISOIS_DEFAULT - dwWidth and dwHeight values are ignored. The image
//           will not be scaled after loaded from disk; for created UIs, the
//           size is determined by the global constants. This is the default
//           value.                 
//         DIDISOIS_RESIZE - Scale the image and callouts to the given width
//           and height.
//         DIDISOIS_MAINTAINASPECTUSINGWIDTH - Scale the image and callouts to
//           the given width, but maintain the aspect ratio. The dwHeight 
//           argument is ignored
//         DIDISOIS_MAINTAINASPECTUSINGHEIGHT - Scale the image and callouts to
//           the given height, but maintain the aspect ratio. The dwWidth
//           argument is ignored.
//  Ret: DI_OK - Success.
//       DIERR_NOTINITIALIZED - Fail; Init() must be called first.
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::SetOutputImageSize( DWORD dwWidth, DWORD dwHeight, DWORD dwFlags )
{
    // Verify initialization
    if( false == m_bInitialized )
        return DIERR_NOTINITIALIZED;

    // Store arguments
    m_dwWidthPref   = dwWidth;
    m_dwHeightPref  = dwHeight;
    m_dwScaleMethod = dwFlags;

    // If the image size has changed, all the stored images are no longer valid
    DestroyImages();

    // This action might change the layout for the custom UI
    if( m_bCustomUI )
        m_bInvalidUI = TRUE;

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: SetFont
// Desc: Sets the font to be used for the callout text when rendering the image
// Args: hFont - Handle to a GDI font object. The font's properties will be
//         copied and used for drawn text.
//  Ret: DI_OK - Success.
//       DIERR_INVALIDPARAM - Fail; Invalid handle.
//       DIERR_NOTINITIALIZED - Fail; Init() must be called first.
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::SetFont( HFONT hFont )
{
    LOGFONT logfont = {0};
    HFONT   hNewFont   = NULL;

    // Verify initialization
    if( false == m_bInitialized )
        return DIERR_NOTINITIALIZED;
 
    // Retrieve the logfont and create a new member font
    if( 0 == ::GetObject( hFont, sizeof(LOGFONT), &logfont ) )
        return DIERR_INVALIDPARAM;

    // Create a duplicate font
    hNewFont = CreateFontIndirect( &logfont );
    if( NULL == hNewFont )
        return E_FAIL;

    // Remove the current font
    if( m_hFont )
        DeleteObject( m_hFont );

    // Copy the font handle
    m_hFont = hNewFont;

    // This action might change the layout for the custom UI
    if( m_bCustomUI )
        m_bInvalidUI = TRUE;

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: SetColors
// Desc: Assigns a set of colors to use when painting the various items on the
//       image
// Args: Background - Specifies the color and alpha component for the
//         image background. Device images are stored in a format which allows
//         the background to be replaced. D3D surfaces allow this background
//         to contain transparency information. Alpha values of 0 thru 254 allow
//         varying transparency effects on the output surfaces. A value
//         of 255 is treated specially during alpha blending, such that the 
//         final output image will fully opaque.
//       crColorNormal - Foreground text color for callout strings in a normal
//         state.
//       crColorHigh - Foreground text color for callout strings in a
//         highlighted state.
//  Ret: DI_OK - Success.
//       DIERR_NOTINITIALIZED - Fail; Init() must be called first.
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::SetColors( D3DCOLOR Background, COLORREF crCalloutNormal, COLORREF crCalloutHigh )
{
    // Verify initialization
    if( false == m_bInitialized )
        return DIERR_NOTINITIALIZED;

    // If the background is changing colors, the images will have to be 
    // reloaded. As an optimization, the background color is only applied
    // when the image is loaded since the background won't change as often as
    // the image is rendered.
    if( m_BkColor != Background )
        DestroyImages();
    
    m_BkColor = Background;

    for( UINT i=0; i < m_dwNumObjects; i++ )
    {
        m_apObject[i]->SetCalloutColors( crCalloutNormal, crCalloutHigh );
    }

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: Render
// Desc: Renders an image of the device and its callouts onto a Direct3DTexture
// Args: pTexture - Pointer to a D3D Texture Object on which to paint the UI.
//  Ret: DI_OK - Success.
//       DIERR_INVALIDPARAM - Fail; Null pointer.
//       DIERR_NOTINITIALIZED - Fail; Init() must be called first.
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::Render( LPDIRECT3DTEXTURE9 pTexture )
{
    HRESULT hr = DI_OK;
    LPDIRECT3DSURFACE9 pSurface = NULL;

    // Verify initialization
    if( false == m_bInitialized )
        return DIERR_NOTINITIALIZED;

    // Check parameters
    if( NULL == pTexture )
        return DIERR_INVALIDPARAM;

    // Add a reference to the passed texture
    pTexture->AddRef();

    // Extract the surface
    hr = pTexture->GetSurfaceLevel( 0, &pSurface );
    if( FAILED(hr) )
        goto LCleanReturn;

    // Perform the render
    if( m_bCustomUI )
        hr = RenderCustomToTarget( (LPVOID) pSurface, DIDIRT_SURFACE );
    else
        hr = RenderToTarget( (LPVOID) pSurface, DIDIRT_SURFACE );


LCleanReturn:

    SAFE_RELEASE( pSurface );
    SAFE_RELEASE( pTexture );
    return hr;
}




//-----------------------------------------------------------------------------
// Name: RenderToDC
// Desc: Renders an image of the device and its callouts onto a GDI device
//       object
// Args: hDC - Handle to a device context on which to paint the UI.
//  Ret: DI_OK - Success.
//       DIERR_NOTINITIALIZED - Fail; Init() must be called first.
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::RenderToDC( HDC hDC )
{
    HRESULT hr = S_OK;

    // Verify initialization
    if( false == m_bInitialized )
        return DIERR_NOTINITIALIZED;

    if( m_bCustomUI )
        hr = RenderCustomToTarget( (LPVOID) hDC, DIDIRT_DC );
    else
        hr = RenderToTarget( (LPVOID) hDC, DIDIRT_DC );

    return hr;
}




//-----------------------------------------------------------------------------
// Name: RenderCustomToTarget
// Desc: Renders a custom UI
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::RenderCustomToTarget( LPVOID pvTarget, DIDIRENDERTARGET eTarget )
{
    HRESULT    hr         = DI_OK;
    UINT       i          = 0;
    HDC        hdcRender  = NULL;
    HDC        hdcAlpha   = NULL;
    RECT       rcBitmap   = {0};
    DIBSECTION info       = {0};
    HBITMAP    hbmpRender = NULL;
    HBITMAP    hbmpAlpha  = NULL;
    SIZE       size       = {0};

    // Get the UI dimensions
    GetCustomUISize( &size );

    if( m_bInvalidUI )
        BuildCustomUI();

    // Create a background image
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = size.cx;
    bmi.bmiHeader.biHeight = - (int) size.cy; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    hdcRender = CreateCompatibleDC( NULL );
    if( NULL == hdcRender )
    {
        hr = DIERR_OUTOFMEMORY;
        goto LCleanReturn;
    }

    hdcAlpha = CreateCompatibleDC( NULL );
    if( NULL == hdcAlpha )
    {
        hr = DIERR_OUTOFMEMORY;
        goto LCleanReturn;
    }
    
    hbmpRender = CreateDIBSection( hdcRender, &bmi, DIB_RGB_COLORS, NULL, NULL, NULL );
    if( hbmpRender == NULL )
    {
        hr = DIERR_OUTOFMEMORY;
        goto LCleanReturn;
    }

    if( 0 == ::GetObject( hbmpRender, sizeof( DIBSECTION ), &info ) )
    {
        hr = E_FAIL;
        goto LCleanReturn;
    }

    
    SelectObject( hdcRender, hbmpRender );
    SelectObject( hdcRender, m_hFont );
    SetBkMode( hdcRender, TRANSPARENT );

    FillBackground( hbmpRender, m_BkColor );

    // Create a bitmap to store the alpha channel for the bitmap. Since GDI
    // doesn't support alpha information, everything is drawn fully transparent.
    // As a workaround, whenever a 2D method is called on the render dc,
    // the same method will be called on the alpha dc. Before transfering the
    // image to the provided surface, the transparency information will be 
    // restored from the alpha bitmap. 
    hbmpAlpha = CreateDIBSection( hdcAlpha, &bmi, DIB_RGB_COLORS, NULL, NULL, NULL );
    if( NULL == hbmpAlpha )
    {
        hr = DIERR_OUTOFMEMORY;
        goto LCleanReturn;
    }

    // Clear the alpha channel
    DIBSECTION infoAlpha;
    if( 0 == ::GetObject( hbmpAlpha, sizeof(DIBSECTION), &infoAlpha ) )
    {
        hr = E_FAIL;
        goto LCleanReturn;
    }
    ZeroMemory( infoAlpha.dsBm.bmBits, infoAlpha.dsBm.bmWidthBytes * infoAlpha.dsBm.bmHeight );

    SelectObject( hdcAlpha, hbmpAlpha );

    SetBkMode( hdcAlpha, TRANSPARENT );
    SetTextColor( hdcAlpha, RGB(255, 255, 255) );
    SelectObject( hdcAlpha, GetStockObject( WHITE_PEN ) );
    SelectObject( hdcAlpha, m_hFont );
        

    // Draw callout and object text
    for( i=0; i < m_dwNumObjects; i++ )
    {
        COLORREF crNorm, crHigh, crCur;
        
        DIDICallout *pCallout = m_apObject[i]->GetCallout( m_dwActiveView );
        DIDIOverlay *pOverlay = m_apObject[i]->GetOverlay( m_dwActiveView );

        MAXSTRING    strCallout = {0};
        MAXSTRING    strObject  = {0};

        RECT rcFill = {0};
        
        // Callout may be invisible
        if( DIDICOS_INVISIBLE & m_apObject[i]->GetCalloutState() )
            continue;

        if( IsRectEmpty( &pOverlay->rcScaled ) ||
            IsRectEmpty( &pCallout->rcScaled ) )
            continue;

        m_apObject[i]->GetCalloutText( strCallout, MAX_PATH );
        m_apObject[i]->GetName( strObject, MAX_PATH );
        m_apObject[i]->GetCalloutColors( &crNorm, &crHigh );

        crCur = ( DIDICOS_HIGHLIGHTED & m_apObject[i]->GetCalloutState() ) ? crHigh : crNorm;
        
        SetTextColor( hdcRender, crNorm );
        
        DWORD dwFlags = DT_TOP | DT_END_ELLIPSIS | DT_NOCLIP;

        // Get the fill rect
        rcFill = pOverlay->rcScaled;
        DrawText( hdcRender, strObject, lstrlen( strObject ),
                  &rcFill, dwFlags | DT_CALCRECT );

        // Position the fill rect
        rcFill.left += pOverlay->rcScaled.right - rcFill.right;
        rcFill.right += pOverlay->rcScaled.right - rcFill.right;

        // Inflate the fill rect
        InflateRect( &rcFill, 5, 5 );

        // But make sure the rect still fits on the screen
        rcFill.left   = max( rcFill.left,   0 );
        rcFill.top    = max( rcFill.top,    0 );
        rcFill.right  = min( rcFill.right,  info.dsBm.bmWidth );
        rcFill.bottom = min( rcFill.bottom, info.dsBm.bmHeight );

        // Draw the object text
        DrawText( hdcRender, strObject, lstrlen( strObject ),
                  &(pOverlay->rcScaled), dwFlags | DT_RIGHT );

        if( hdcAlpha )
            DrawText( hdcAlpha, strObject, lstrlen( strObject ),
                      &(pOverlay->rcScaled), dwFlags | DT_RIGHT );
        
        SetTextColor( hdcRender, crCur );
        
        // Get the fill rect
        rcFill = pCallout->rcScaled;
        DrawText( hdcRender, strCallout, lstrlen( strCallout ),
                  &rcFill, dwFlags | DT_CALCRECT );

        // Inflate the fill rect
        InflateRect( &rcFill, 5, 5 );

        // But make sure the rect still fits on the screen
        rcFill.left   = max( rcFill.left,   0 );
        rcFill.top    = max( rcFill.top,    0 );
        rcFill.right  = min( rcFill.right,  info.dsBm.bmWidth );
        rcFill.bottom = min( rcFill.bottom, info.dsBm.bmHeight );

        // Draw the callout text
        DrawText( hdcRender, strCallout, lstrlen( strCallout ),
                  &(pCallout->rcScaled), dwFlags | DT_LEFT );

        if( hdcAlpha )
            DrawText( hdcAlpha, strCallout, lstrlen( strCallout ),
                      &(pCallout->rcScaled), dwFlags | DT_LEFT );

        // If the TOOLTIP flag is set and the callout text doesn't fit within
        // the scaled rect, we need to draw the full text
        if( DIDICOS_TOOLTIP & m_apObject[i]->GetCalloutState() )
        {
            SIZE TextSize = {0};
            
            // This string was modified by the first call to draw text, so we
            // need to get a fresh copy
            m_apObject[i]->GetCalloutText( strCallout, MAX_PATH-4 );
            GetTextExtentPoint32( hdcRender, strCallout, lstrlen( strCallout ), &TextSize );

            if( TextSize.cx > ( pCallout->rcScaled.right - pCallout->rcScaled.left ) )
            {
                // Yep, the text is too big and is marked as a tooltip candidate.
                RECT rcBitmap = { 0, 0, info.dsBm.bmWidth, info.dsBm.bmHeight };
                DrawTooltip( hdcRender, hdcAlpha, strCallout, &rcBitmap, &(pCallout->rcScaled),
                             CRFromColor( m_BkColor ), crNorm, crHigh );
                
            }

        }
    }

    // Finalize rendering
    GdiFlush();

    // Copy the freshly rendered image to the render target
    switch( eTarget )
    {
        case DIDIRT_SURFACE:
            // Since the image is being transfered to a Direct3D surface, the stored
            // alpha information could be used.
            ApplyAlphaChannel( hbmpRender, hbmpAlpha, ( (m_BkColor & ALPHA_MASK) == ALPHA_MASK ) );
            rcBitmap.right  = info.dsBm.bmWidth;
            rcBitmap.bottom = info.dsBm.bmHeight;

            hr = D3DXLoadSurfaceFromMemory( (LPDIRECT3DSURFACE9) pvTarget,
                                             NULL, NULL, info.dsBm.bmBits,
                                             D3DFMT_A8R8G8B8, 
                                             info.dsBm.bmWidthBytes,
                                             NULL, &rcBitmap, 
                                             D3DX_FILTER_NONE, 0 );
            break;

        case DIDIRT_DC:
            BitBlt( (HDC) pvTarget, 0, 0, info.dsBm.bmWidth, info.dsBm.bmHeight,
                          hdcRender, 0, 0, SRCCOPY );

            break;
    
        default:
            // Invalid render target
            hr = DIERR_INVALIDPARAM;
            goto LCleanReturn;
    }   


LCleanReturn:
    
    DeleteDC( hdcRender );
    DeleteDC( hdcAlpha );
    
    if( hbmpAlpha )
        DeleteObject( hbmpAlpha );

    if( hbmpRender )
        DeleteObject( hbmpRender );

    return hr;
}




//-----------------------------------------------------------------------------
// Name: RenderToTarget
// Desc: Renders an image of the device and its callouts 
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::RenderToTarget( LPVOID pvTarget, DIDIRENDERTARGET eTarget )
{
    HRESULT    hr           = DI_OK;
    UINT       i            = 0; // Loop variable
    RECT       rcBitmap     = {0};
    HDC        hdcRender    = NULL;
    HDC        hdcAlpha     = NULL;
    DIBSECTION info         = {0};
    
    LPBYTE     pSavedPixels = NULL;
    LPBYTE     pCleanPixels = NULL;
    
    HBITMAP    hbmpAlpha    = NULL;
    BITMAPINFO bmi = {0};
    
    
    // Verify initialization
    if( false == m_bInitialized )
        return DIERR_NOTINITIALIZED;

    // Verify parameters
    if( NULL == pvTarget )
        return DIERR_INVALIDPARAM;

    // Sanity check
    if( m_dwActiveView >= m_dwNumViews )
        return E_FAIL;

    // Load images if not loaded already
    if( NULL == m_ahImages[ m_dwActiveView ] )
    {  
        // File UI not yet loaded
        if( FAILED( hr = LoadImages() ) )
            return hr;
    } 
    

    // Get information about the background image.
    if( 0 == ::GetObject( m_ahImages[ m_dwActiveView ], sizeof(DIBSECTION), &info ) )
        return E_FAIL;


    // Allocate space for the saved background images
    pSavedPixels = new BYTE[ info.dsBm.bmWidthBytes * info.dsBm.bmHeight ];
    if( NULL == pSavedPixels )
    {
        hr = DIERR_OUTOFMEMORY;
        goto LCleanReturn;
    }

    pCleanPixels = new BYTE[ info.dsBm.bmWidthBytes * info.dsBm.bmHeight ];
    if( NULL == pCleanPixels )
    {
        // Could not create a copy of the background image; release memory
        // here to avoid using unitialized pixels during cleanup.
        SAFE_DELETE_ARRAY( pSavedPixels );

        hr = DIERR_OUTOFMEMORY; 
        goto LCleanReturn;
    }

    // Save the background
    CopyMemory( pSavedPixels, info.dsBm.bmBits, 
                info.dsBm.bmWidthBytes * info.dsBm.bmHeight );
    
    // Draw overlays
    for( i=0; i < m_dwNumObjects; i++ )
    {  
        DIDIOverlay *pOverlay = m_apObject[i]->GetOverlay( m_dwActiveView );
        
        if( DIDICOS_HIGHLIGHTED & m_apObject[i]->GetCalloutState() )
        {     
            // Draw overlay
            if( pOverlay->hImage )    
                ApplyOverlay( m_ahImages[ m_dwActiveView ], &pOverlay->rcScaled, pOverlay->hImage );      
        }
    }

    // Before drawing callouts and lines on top of the composed image, save
    // a copy of the image bits. This will allow us to erase portions of lines
    // which intersect with the callout text.
    CopyMemory( pCleanPixels, info.dsBm.bmBits, 
                info.dsBm.bmWidthBytes * info.dsBm.bmHeight );
  
    // Load the background image into a device context for rendering
    hdcRender = CreateCompatibleDC( NULL );
    SelectObject( hdcRender, m_ahImages[ m_dwActiveView ] );
    
    SelectObject( hdcRender, m_hFont );
    SetBkMode( hdcRender, TRANSPARENT );
    SetBkColor( hdcRender, CRFromColor(m_BkColor) );
  
    hdcAlpha = CreateCompatibleDC( NULL );

    // Create a bitmap to store the alpha channel for the bitmap. Since GDI
    // doesn't support alpha information, everything is drawn fully transparent.
    // As a workaround, whenever a 2D method is called on the render dc,
    // the same method will be called on the alpha dc. Before transfering the
    // image to the provided surface, the transparency information will be 
    // restored from the alpha bitmap. 
    
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = info.dsBm.bmWidth;
    bmi.bmiHeader.biHeight = - (int) info.dsBm.bmHeight; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    hbmpAlpha = CreateDIBSection( hdcAlpha, &bmi, DIB_RGB_COLORS, NULL, NULL, NULL );
    if( NULL == hbmpAlpha )
    {
        hr = DIERR_OUTOFMEMORY;
        goto LCleanReturn;
    }

    // Clear the alpha channel
    DIBSECTION infoAlpha;
    if( 0 == ::GetObject( hbmpAlpha, sizeof(DIBSECTION), &infoAlpha ) )
    {
        hr = E_FAIL;
        goto LCleanReturn;
    }
    ZeroMemory( infoAlpha.dsBm.bmBits, infoAlpha.dsBm.bmWidthBytes * infoAlpha.dsBm.bmHeight );

    SelectObject( hdcAlpha, hbmpAlpha );

    SetBkMode( hdcAlpha, TRANSPARENT );
    SetTextColor( hdcAlpha, RGB(255, 255, 255) );
    SelectObject( hdcAlpha, GetStockObject( WHITE_PEN ) );
    SelectObject( hdcAlpha, m_hFont );
     

    // Draw callout lines
    for( i=0; i < m_dwNumObjects; i++ )
    {
        COLORREF crNorm, crHigh, crCur;
        POINT    aptArrow[2] = {0}; 
        BOOL     bDrawArrow = FALSE;
        
        // Get the current callout
        DIDICallout *pCallout = m_apObject[i]->GetCallout( m_dwActiveView );

        // Callout may be invisible
        if( DIDICOS_INVISIBLE & m_apObject[i]->GetCalloutState() )
            continue;
        
        // Retrieve normal/highlighted colors
        m_apObject[i]->GetCalloutColors( &crNorm, &crHigh );

        // Set the current color based on callout state
        crCur = ( DIDICOS_HIGHLIGHTED & m_apObject[i]->GetCalloutState() ) ? crHigh : crNorm;
       

        DeleteObject( SelectObject( hdcRender, CreatePen( PS_SOLID, 3, CRFromColor(m_BkColor) ) ) ); 
        DeleteObject( SelectObject( hdcAlpha, CreatePen( PS_SOLID, 3, RGB(255, 255, 255) ) ) ); 

        // Draw callout lines
        MoveToEx( hdcRender, pCallout->aptLineScaled[0].x, pCallout->aptLineScaled[0].y, NULL );
        MoveToEx( hdcAlpha, pCallout->aptLineScaled[0].x, pCallout->aptLineScaled[0].y, NULL );
        
        for( UINT j=1; j < pCallout->dwNumPoints; j++ )
        {
            LineTo( hdcRender, pCallout->aptLineScaled[j].x, pCallout->aptLineScaled[j].y );
            LineTo( hdcAlpha, pCallout->aptLineScaled[j].x, pCallout->aptLineScaled[j].y );
        }

        // Draw arrow ends
        if( pCallout->dwNumPoints >= 2 )
        {
            DWORD dwEnd = pCallout->dwNumPoints-1;
            POINT p1 = pCallout->aptLineScaled[ dwEnd ];
            POINT p2 = pCallout->aptLineScaled[ dwEnd-1 ];

            aptArrow[0] = aptArrow[1] = p1;
            bDrawArrow = TRUE;

            aptArrow[0].x -= 1;
            aptArrow[0].y -= 1;
            aptArrow[1].x += 1;
            aptArrow[1].y += 1;
            
            // Adjust arrow points based on line orientation
            if( p1.y == p2.y )
            {
                if( p2.x < p1.x )
                    aptArrow[1].x -= 2;
                else
                    aptArrow[0].x += 2;
            }
            else if( p1.x == p2.x )
            {
                if( p2.y < p1.y )
                    aptArrow[1].y -= 2;
                else
                    aptArrow[0].y += 2;
            }
            else
            {
                // This is a diagonal line. Skip the arrow endpoint.
                bDrawArrow = FALSE;
            }

            if( bDrawArrow )
            {
                MoveToEx( hdcRender, aptArrow[0].x, aptArrow[0].y, NULL );
                LineTo(   hdcRender, aptArrow[1].x, aptArrow[1].y );

                MoveToEx( hdcAlpha, aptArrow[0].x, aptArrow[0].y, NULL );
                LineTo(   hdcAlpha, aptArrow[1].x, aptArrow[1].y );
            }
        }

        // Select a new pen into the DC based on current color
        DeleteObject( SelectObject( hdcRender, CreatePen( PS_SOLID, 1, crCur ) ) ); 
        DeleteObject( SelectObject( hdcAlpha, GetStockObject( WHITE_PEN ) ) ); 
        

        // Draw callout lines
        MoveToEx( hdcRender, pCallout->aptLineScaled[0].x, pCallout->aptLineScaled[0].y, NULL );
        MoveToEx( hdcAlpha, pCallout->aptLineScaled[0].x, pCallout->aptLineScaled[0].y, NULL );
        for( j=1; j < pCallout->dwNumPoints; j++ )
        {
            LineTo( hdcRender, pCallout->aptLineScaled[j].x, pCallout->aptLineScaled[j].y );
            LineTo( hdcAlpha, pCallout->aptLineScaled[j].x, pCallout->aptLineScaled[j].y );
        }

        // Draw arrow ends
        if( bDrawArrow )
        {
            DWORD dwEnd = pCallout->dwNumPoints-1;

            SetPixel( hdcRender, aptArrow[0].x, aptArrow[0].y, crCur );
            SetPixel( hdcRender, aptArrow[1].x, aptArrow[1].y, crCur );
            SetPixel( hdcRender, pCallout->aptLineScaled[ dwEnd ].x, 
                                 pCallout->aptLineScaled[ dwEnd ].y, crCur );

            SetPixel( hdcAlpha, aptArrow[0].x, aptArrow[0].y, RGB(255, 255, 255) );
            SetPixel( hdcAlpha, aptArrow[1].x, aptArrow[1].y, RGB(255, 255, 255) );
            SetPixel( hdcAlpha, pCallout->aptLineScaled[ dwEnd ].x, 
                                 pCallout->aptLineScaled[ dwEnd ].y, RGB(255, 255, 255) );
        }

        
    }
    
    // Free the pen resource
    DeleteObject( SelectObject( hdcRender, GetStockObject( WHITE_PEN ) ) );
    DeleteObject( SelectObject( hdcAlpha, GetStockObject( WHITE_PEN ) ) );

    // Draw callout text
    for( i=0; i < m_dwNumObjects; i++ )
    {
        COLORREF crNorm, crHigh, crCur;
        RECT     rcFill;
        
        DIDICallout *pCallout = m_apObject[i]->GetCallout( m_dwActiveView );
        MAXSTRING    strCallout = {0};
        
        // Callout may be invisible
        if( DIDICOS_INVISIBLE & m_apObject[i]->GetCalloutState() )
            continue;

        m_apObject[i]->GetCalloutText( strCallout, MAX_PATH-4 );
        m_apObject[i]->GetCalloutColors( &crNorm, &crHigh );
        
        
        

        if( IsRectEmpty( &pCallout->rcScaled ) )
            continue;

        // Draw callouts
        DWORD dwFormat = DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOCLIP;

        // Get text dimensions
        rcFill = pCallout->rcScaled;
        DrawText( hdcRender, strCallout, lstrlen( strCallout ),
                  &rcFill, dwFormat | DT_CALCRECT | DT_MODIFYSTRING );

      
        // Horizontal alignment
        if( pCallout->dwTextAlign & DIDAL_CENTERED      ) 
        {
            dwFormat |= DT_CENTER;
            OffsetRect( &rcFill, (pCallout->rcScaled.right - rcFill.right) / 2, 0 );
        }
        else if( pCallout->dwTextAlign & DIDAL_RIGHTALIGNED  ) 
        {
            dwFormat |= DT_RIGHT;
            OffsetRect( &rcFill, (pCallout->rcScaled.right - rcFill.right ), 0 );
        }
        else
        {
            dwFormat |= DT_LEFT;
        }

        // Vertical alignment
        if( pCallout->dwTextAlign & DIDAL_MIDDLE        ) 
        {
            dwFormat |= DT_VCENTER;
            OffsetRect( &rcFill, 0, (pCallout->rcScaled.bottom - rcFill.bottom) / 2 );
        }
        else if( pCallout->dwTextAlign & DIDAL_BOTTOMALIGNED ) 
        {
            dwFormat |= DT_BOTTOM;
            OffsetRect( &rcFill, 0, (pCallout->rcScaled.bottom - rcFill.bottom) );
        }
        else
        {
            dwFormat |= DT_TOP;
        }

        // First replace the background area behind the text to cover up
        // intersecting lines
        
        // Pad the returned rect
        InflateRect( &rcFill, 5, 5 );

        // But make sure the rect still fits on the screen
        rcFill.left   = max( rcFill.left,   0 );
        rcFill.top    = max( rcFill.top,    0 );
        rcFill.right  = min( rcFill.right,  info.dsBm.bmWidth );
        rcFill.bottom = min( rcFill.bottom, info.dsBm.bmHeight );

        RestoreRect( m_ahImages[ m_dwActiveView ], &rcFill, pCleanPixels );
        if( hdcAlpha )
            FillRect( hdcAlpha, &rcFill, (HBRUSH) GetStockObject( BLACK_BRUSH ) );

        // Fill behind the text
        SetTextColor( hdcRender, CRFromColor(m_BkColor) );
        for( int x = -1; x <= 1; x++ )
        {
            for( int y = -1; y <= 1; y++ )
            {
                RECT rcText = pCallout->rcScaled;
                OffsetRect( &rcText, x, y );

                DrawText( hdcRender, strCallout, lstrlen( strCallout ),
                  &rcText, dwFormat );

                DrawText( hdcAlpha, strCallout, lstrlen( strCallout ),
                  &rcText, dwFormat );
            }
        }

        // Now draw the actual text
        crCur = ( DIDICOS_HIGHLIGHTED & m_apObject[i]->GetCalloutState() ) ? crHigh : crNorm;     
        SetTextColor( hdcRender, crCur );

        DrawText( hdcRender, strCallout, lstrlen( strCallout ),
                  &(pCallout->rcScaled), dwFormat );
        
        DrawText( hdcAlpha, strCallout, lstrlen( strCallout ),
                  &(pCallout->rcScaled), dwFormat );
        


        // If the TOOLTIP flag is set and the callout text doesn't fit within
        // the scaled rect, we need to draw the full text
        if( DIDICOS_TOOLTIP & m_apObject[i]->GetCalloutState() )
        {
            SIZE TextSize = {0};
            
            // This string was modified by the first call to draw text, so we
            // need to get a fresh copy
            m_apObject[i]->GetCalloutText( strCallout, MAX_PATH-4 );
            GetTextExtentPoint32( hdcRender, strCallout, lstrlen( strCallout ), &TextSize );

            if( TextSize.cx > ( pCallout->rcScaled.right - pCallout->rcScaled.left ) )
            {
                // Yep, the text is too big and is marked as a tooltip candidate.
                RECT rcBitmap = { 0, 0, info.dsBm.bmWidth, info.dsBm.bmHeight };
                DrawTooltip( hdcRender, hdcAlpha, strCallout, &rcBitmap, &(pCallout->rcScaled),
                             CRFromColor( m_BkColor ), crNorm, crHigh );
                
            }

        }

    }

    

    // Finalize all rendering
    GdiFlush();

    // Copy the freshly rendered image to the render target
    switch( eTarget )
    {
        case DIDIRT_SURFACE:
            // Since the image is being transfered to a Direct3D surface, the stored
            // alpha information could be used.
            ApplyAlphaChannel( m_ahImages[ m_dwActiveView ], hbmpAlpha, ( (m_BkColor & ALPHA_MASK) == ALPHA_MASK ) );
            rcBitmap.right  = info.dsBm.bmWidth;
            rcBitmap.bottom = info.dsBm.bmHeight;

            hr = D3DXLoadSurfaceFromMemory( (LPDIRECT3DSURFACE9) pvTarget,
                                             NULL, NULL, info.dsBm.bmBits,
                                             D3DFMT_A8R8G8B8, 
                                             info.dsBm.bmWidthBytes,
                                             NULL, &rcBitmap, 
                                             D3DX_FILTER_NONE, 0 );
            break;

        case DIDIRT_DC:
            BitBlt( (HDC) pvTarget, 0, 0, info.dsBm.bmWidth, info.dsBm.bmHeight,
                          hdcRender, 0, 0, SRCCOPY );
            break;
    
        default:
            // Invalid render target
            hr = DIERR_INVALIDPARAM;
            goto LCleanReturn;
    }   

    

LCleanReturn:
    
    // Restore the background
    if( pSavedPixels )
        CopyMemory( info.dsBm.bmBits, pSavedPixels, info.dsBm.bmWidthBytes * info.dsBm.bmHeight );
  
    DeleteDC( hdcRender );
    DeleteDC( hdcAlpha );
    DeleteObject( hbmpAlpha );

    delete [] pSavedPixels;
    delete [] pCleanPixels;
    return hr;
}





//-----------------------------------------------------------------------------
// Name: AddObject
// Desc: Adds an object to the current list according to object ID. If an 
//       object with the given ID already exists, a pointer to it returned. 
//       Otherwise, a new object is created and a pointer to the new object
//       is returned. Returns NULL if memory couldn't be allocated.
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::AddObject( DWORD dwID )
{
    CDIDIObject* pObject = NULL;

    // Search through current objects
    if( GetObject( dwID ) )
        return DI_OK;

    // Did not find object. Create a new object, and store pointer
    pObject = new CDIDIObject( dwID, m_dwNumViews );
    if( NULL == pObject )
        return DIERR_OUTOFMEMORY;

    m_apObject[m_dwNumObjects++] = pObject;
    
    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: GetObject
// Desc: If an object with given ID exist, a pointer to it is returned
//-----------------------------------------------------------------------------
CDIDIObject* CDIDevImage::GetObject( DWORD dwID )
{
    for( UINT i=0; i < m_dwNumObjects; i++ )
    {
        if( m_apObject[i]->GetID() == dwID )
            return m_apObject[i];
    }

    return NULL;
}




//-----------------------------------------------------------------------------
// Name: LoadImageInfo
// Desc: helper function to retrieve callout / image data from
//       DirectInput
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::LoadImageInfo( LPDIRECTINPUTDEVICE8 pDIDevice )
{
    HRESULT hr;
    DWORD   dwBufferCount = 0;
    DIDEVICEIMAGEINFOHEADER dihImageHeader = {0};
    DIDEVICEIMAGEINFO *pInfo = NULL;

    // properly initialize the structure before it can be used
    dihImageHeader.dwSize = sizeof( DIDEVICEIMAGEINFOHEADER );
    dihImageHeader.dwSizeImageInfo = sizeof( DIDEVICEIMAGEINFO );
    
    // since m_dihImageHeader.dwBufferSize is 0, this call serves to determine
    // the minimum buffer size required to hold information for all the images
    hr = pDIDevice->GetImageInfo( &dihImageHeader );
    if( FAILED(hr) )
        return hr;

    // at this point, m_lpDidImgHeader->dwBufferUsed has been set by
    // the GetImageInfo method to minimum buffer size needed, so allocate.
    dihImageHeader.dwBufferSize = dihImageHeader.dwBufferUsed;
    dihImageHeader.lprgImageInfoArray = (DIDEVICEIMAGEINFO*) new BYTE[dihImageHeader.dwBufferSize];

    // make sure memory has been allocated
    if( NULL == dihImageHeader.lprgImageInfoArray )
        return DIERR_OUTOFMEMORY;

    // now that the dwBufferSize has been filled, and lprgImageArray allocated,
    // we call GetImageInfo again to get the image data
    hr = pDIDevice->GetImageInfo( &dihImageHeader );
    if( FAILED(hr) )
        goto LCleanReturn;
    
    // Allocate space for all the object/callouts/overlays
    m_apObject = new CDIDIObject* [dihImageHeader.dwcButtons + 
                                   dihImageHeader.dwcAxes +
                                   dihImageHeader.dwcPOVs];

    if( NULL == m_apObject )
    {
        hr = DIERR_OUTOFMEMORY;
        goto LCleanReturn;
    }

    m_dwNumViews = dihImageHeader.dwcViews;

    // Allocate space for background images
    m_atszImagePath = new TCHAR[m_dwNumViews][MAX_PATH];
    if( NULL == m_atszImagePath )
    {
        hr = DIERR_OUTOFMEMORY;
        goto LCleanReturn;
    }

    ZeroMemory( m_atszImagePath, sizeof(m_atszImagePath) );

    m_ahImages = new HBITMAP[m_dwNumViews];
    if( NULL == m_ahImages )
    {
        hr = DIERR_OUTOFMEMORY;
        goto LCleanReturn;
    }

    ZeroMemory( m_ahImages, sizeof(m_ahImages) );

    // Fill the data from the ImageHeader
    pInfo = dihImageHeader.lprgImageInfoArray;
    dwBufferCount = dihImageHeader.dwBufferUsed;

    while( dwBufferCount > 0)
    {
        if( pInfo->dwViewID > m_dwNumViews )
        {
            // Error in the input format, this is out of bounds for our array
            hr = E_FAIL;
            goto LCleanReturn;
        }

        if( pInfo->dwFlags & DIDIFT_CONFIGURATION )
        {
            lstrcpy( m_atszImagePath[pInfo->dwViewID], pInfo->tszImagePath );
        }
        else if( pInfo->dwFlags & DIDIFT_OVERLAY )
        {
            hr = AddObject( pInfo->dwObjID );
            if( FAILED(hr) )
                goto LCleanReturn;

            CDIDIObject *pDIObj = GetObject( pInfo->dwObjID ); 
            if( NULL == pDIObj )
            {
                hr = DIERR_OUTOFMEMORY;
                goto LCleanReturn;
            }

            // Overlay
            if( pInfo->tszImagePath[0] )
                pDIObj->SetOverlay( pInfo->dwViewID, pInfo->tszImagePath, pInfo->rcOverlay );
           

            // Callout
            pDIObj->SetCallout( pInfo->dwViewID, pInfo->dwcValidPts, pInfo->rgptCalloutLine, pInfo->rcCalloutRect, pInfo->dwTextAlign );
        }
        
        pInfo++;
        dwBufferCount -= dihImageHeader.dwSizeImageInfo;
    }

    // We made it this far, set the return value as success.
    hr = DI_OK;

LCleanReturn:
    
    // Release the resources used for the image info structure
    delete [] dihImageHeader.lprgImageInfoArray;
    return hr;
}




//-----------------------------------------------------------------------------
// Name: CreateCustomImageInfo
// Desc: Create a default UI for the given device, and fill in all neccesary
//       structures to support rendering
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::CreateCustomImageInfo( LPDIRECTINPUTDEVICE8 pDIDevice )
{
    HRESULT hr;
    DIDEVCAPS didc;

    // Allocate space for all the device's objects (axes, buttons, POVS)
    ZeroMemory( &didc, sizeof(DIDEVCAPS) );
    didc.dwSize = sizeof(DIDEVCAPS);
    hr = pDIDevice->GetCapabilities( &didc );
    if( FAILED(hr) )
        return hr;

    m_apObject = new CDIDIObject* [didc.dwAxes + didc.dwButtons + didc.dwPOVs];
    if( NULL == m_apObject )
        return DIERR_OUTOFMEMORY;

    
    hr = pDIDevice->EnumObjects( EnumDeviceObjectsCB, this, DIDFT_AXIS );
    if( FAILED(hr) )
        return hr;

    hr = pDIDevice->EnumObjects( EnumDeviceObjectsCB, this, DIDFT_BUTTON );
    if( FAILED(hr))
        return hr;

    hr = pDIDevice->EnumObjects( EnumDeviceObjectsCB, this, DIDFT_POV );
    if( FAILED(hr))
        return hr;

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: LoadImages
// Desc: Load all images associated with the active view
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::LoadImages()
{
    UINT               i;
    HRESULT            hr;
    SIZE               sizeInit = {0};
    SIZE               sizeScaled = {0};
    FLOAT              fxScale, fyScale;
    D3DXIMAGE_INFO     d3dxImageInfo;
    LPDIRECT3DSURFACE9 pLoadSurface = NULL;
    LPDIRECT3DSURFACE9 pScaleSurface = NULL;
    

    // Create a temporary surface
    pLoadSurface = GetCloneSurface( DIDICONST_MAX_IMAGE_WIDTH, DIDICONST_MAX_IMAGE_HEIGHT );
    
    // Load the background image onto the temporary loading surface
    hr = D3DXLoadSurfaceFromFile( pLoadSurface, NULL, NULL, 
                                  m_atszImagePath[m_dwActiveView],
                                  NULL, D3DX_FILTER_NONE, 
                                  NULL, &d3dxImageInfo );

    if( FAILED(hr) )
        goto LCleanReturn;

   
    // The actual dimensions of the render surface are determined
    // by the background image, the overlay images, and the 
    // callout rects. 
    sizeInit.cx = d3dxImageInfo.Width;
    sizeInit.cy = d3dxImageInfo.Height;

    for( i=0; i < m_dwNumObjects; i++ )
    {
        DIDICallout* pCallout = m_apObject[i]->GetCallout( m_dwActiveView );
        
        sizeInit.cx = max( sizeInit.cx, pCallout->rcInit.right );
        sizeInit.cy = max( sizeInit.cy, pCallout->rcInit.bottom );
    }

    // Determine the scaling parameters
    switch( m_dwScaleMethod )
    {
        case( DIDISOIS_RESIZE ) :
            sizeScaled.cx = m_dwWidthPref;
            sizeScaled.cy = m_dwHeightPref;
            break;

        case( DIDISOIS_MAINTAINASPECTUSINGWIDTH ) :
            sizeScaled.cx = m_dwWidthPref;
            sizeScaled.cy = (LONG) ( 0.5 + sizeInit.cy * ( (FLOAT)m_dwWidthPref  / sizeInit.cx ) );
            break;
  
        case( DIDISOIS_MAINTAINASPECTUSINGHEIGHT ) :
            sizeScaled.cx = (LONG) ( 0.5 + sizeInit.cx * ( (FLOAT)m_dwHeightPref / sizeInit.cy ) );
            sizeScaled.cy = m_dwHeightPref;
            break;

        case( DIDISOIS_DEFAULT ) :
        default :
            sizeScaled.cx = sizeInit.cx;
            sizeScaled.cy = sizeInit.cy;
            break;  
    }

    // Calculate scaling multipliers
    fxScale = (FLOAT)sizeScaled.cx / sizeInit.cx;
    fyScale = (FLOAT)sizeScaled.cy / sizeInit.cy;

    // Scale all object display parameters
    for( i=0; i < m_dwNumObjects; i++ )
    {
        m_apObject[i]->ScaleView( m_dwActiveView, fxScale, fyScale );
    }

    // Load the background image
    hr = CreateScaledSurfaceCopy( pLoadSurface, d3dxImageInfo.Width, d3dxImageInfo.Height, 
                                  fxScale, fyScale, &pScaleSurface );
    if( FAILED(hr) )
        goto LCleanReturn;


    // Create a DIB section for the loaded image
    hr = CreateDIBSectionFromSurface( pScaleSurface, &(m_ahImages[ m_dwActiveView ]), &sizeScaled );
    if( FAILED(hr) )
        goto LCleanReturn;

    SAFE_RELEASE( pScaleSurface );

    // Apply the background color
    FillBackground( m_ahImages[ m_dwActiveView ], m_BkColor );

    // Load all object images
    for( i=0; i < m_dwNumObjects; i++ )
    {
        DIDIOverlay *pOverlay = m_apObject[i]->GetOverlay( m_dwActiveView );

        // Load the file onto the temporary surface
        if( !pOverlay->strImagePath[0] )
            continue;

        hr = D3DXLoadSurfaceFromFile( pLoadSurface, 
                                      NULL, NULL, 
                                      pOverlay->strImagePath,
                                      NULL, D3DX_FILTER_NONE, 
                                      NULL, &d3dxImageInfo );
        if( FAILED(hr) )
            continue;
        
        // Since overlay rectanges are actually defined by the image size, some
        // of the image info files simply define the top-left coordinate of the
        // rectangle. We may want good data for the overlay rectangle, so set
        // the rect based on image size
        pOverlay->rcInit.bottom = pOverlay->rcInit.top  + d3dxImageInfo.Height;
        pOverlay->rcInit.right  = pOverlay->rcInit.left + d3dxImageInfo.Width;
        ScaleRect( &( pOverlay->rcInit), &( pOverlay->rcScaled ), fxScale, fyScale );

        // Scale the overlay
        hr = CreateScaledSurfaceCopy( pLoadSurface, d3dxImageInfo.Width, d3dxImageInfo.Height,
                                      fxScale, fyScale, &pScaleSurface );
        if( FAILED(hr) )
            goto LCleanReturn;
     
        // Load the stored bitmap from the scaled D3D surface
        hr = CreateDIBSectionFromSurface( pScaleSurface, &(pOverlay->hImage) );
        if( FAILED(hr) )
            goto LCleanReturn;

        SAFE_RELEASE( pScaleSurface );
    }

    hr = DI_OK;

LCleanReturn:

    SAFE_RELEASE( pLoadSurface );
    SAFE_RELEASE( pScaleSurface );
    
    return hr;
}




//-----------------------------------------------------------------------------
// Name: GetCustomUISize
// Desc: Determine the dimensions of the custom UI based on default values and
//       user-supplied sizing information.
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::GetCustomUISize( SIZE* pSize )
{
    if( pSize == NULL )
        return DIERR_INVALIDPARAM;

    // Calculate view dimensions based on values set during a call to 
    // SetOutputImageSize(), or the default values defined in the header

    switch( m_dwScaleMethod )
    {
    case DIDISOIS_RESIZE :
        pSize->cx = m_dwWidthPref;
        pSize->cy = m_dwHeightPref;
        break;

    case DIDISOIS_MAINTAINASPECTUSINGWIDTH :
        pSize->cx = m_dwWidthPref;
        pSize->cy = (LONG) ( 0.5 + DIDICONST_CUSTOM_VIEW_HEIGHT *
                               ( (FLOAT)m_dwWidthPref / DIDICONST_CUSTOM_VIEW_WIDTH ) );
        break;

    case DIDISOIS_MAINTAINASPECTUSINGHEIGHT :
        pSize->cy = m_dwHeightPref;
        pSize->cx = (LONG) ( 0.5 + DIDICONST_CUSTOM_VIEW_WIDTH *
                               ( (FLOAT)m_dwHeightPref / DIDICONST_CUSTOM_VIEW_HEIGHT ) );
        break;

    default:
        pSize->cx = DIDICONST_CUSTOM_VIEW_WIDTH;
        pSize->cy = DIDICONST_CUSTOM_VIEW_HEIGHT;
    };

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: BuildCustomUI
// Desc: Creates the callout rects for each view based on stored sizing 
// information and callout strings.
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::BuildCustomUI()
{
    HDC     hdc      = NULL;
    SIZE    size     = {0};
    UINT    i        = 0;

    int nMaxNameWidth     = 0;
    int nMaxNameHeight    = 0;
    int nMaxCalloutWidth  = 0;
    int nMaxCalloutHeight = 0;
    int nRowsPerView      = 0;
    int nColsPerView      = 0;
    int nNumVisObjects    = 0;

    const int GUTTER_SIZE    = 20;
    const int PADDING_SIZE   = 20;
    const int SPACING_WIDTH  = 10;
    const int SPACING_HEIGHT = 10;

    const int MAX_CHARS_OBJECT = 20;
    const int MAX_CHARS_ACTION = 20;

    // we need a device context in order to evaluate the text metrics
    hdc = CreateDC( TEXT("DISPLAY"), NULL, NULL, NULL );
    if( NULL == hdc )
        return E_FAIL;

    // select the font into the dc
    SelectObject( hdc, m_hFont );

    // determine the largest device name
    for( i=0; i < m_dwNumObjects; i++ )
    {
        if( DIDICOS_INVISIBLE & m_apObject[i]->GetCalloutState() )
            continue;

        nNumVisObjects++;

        TCHAR str[ MAX_PATH ] = {0};
       
        m_apObject[i]->GetName( str, MAX_PATH );
        if( lstrlen(str) > 15 )
            lstrcpy( &str[15], TEXT("...") );
        if( str && GetTextExtentPoint32( hdc, str, lstrlen( str ), &size ) )
        {
            nMaxNameWidth  = max( nMaxNameWidth,  size.cx );
            nMaxNameHeight = max( nMaxNameHeight, size.cy ); 
        }

        m_apObject[i]->GetCalloutText( str, MAX_PATH );
        if( lstrlen(str) > 15 )
            lstrcpy( &str[15], TEXT("...") );
        if( str && GetTextExtentPoint32( hdc, str, lstrlen( str ), &size ) )
        {
            nMaxCalloutWidth  = max( nMaxCalloutWidth,  size.cx );
            nMaxCalloutHeight = max( nMaxCalloutHeight, size.cy ); 
        }
       
    }
    
    // Optionally, you can help constrain the callout sizes by restricting the
    // string lengths.
    TEXTMETRIC tm = {0};
    if( GetTextMetrics( hdc, &tm ) )
    {
        nMaxCalloutWidth = min( nMaxCalloutWidth, MAX_CHARS_ACTION * tm.tmAveCharWidth );
        nMaxNameWidth    = min( nMaxNameWidth,    MAX_CHARS_OBJECT * tm.tmAveCharWidth );
    }

    // Release resources
    DeleteDC( hdc );

    // Calculate view dimensions
    GetCustomUISize( &size );

    // determine how many callouts we can fit on a single view
    nColsPerView = ( size.cx - (2 * PADDING_SIZE) + GUTTER_SIZE ) / 
                   ( nMaxNameWidth + nMaxCalloutWidth + SPACING_WIDTH + GUTTER_SIZE );

    nColsPerView = max( nColsPerView, 1 );

    nRowsPerView = ( size.cy - (2 * PADDING_SIZE) ) /
                   ( nMaxNameHeight + SPACING_HEIGHT );

    nRowsPerView = max( nRowsPerView, 1 );

    m_dwNumViews = nNumVisObjects / ( nColsPerView * nRowsPerView );
    m_dwNumViews = max( m_dwNumViews, 1 );

    // now all the dimensions are calculated and the callouts can be
    // allocated. 
    for( i=0; i < m_dwNumObjects; i++ )
    {
        m_apObject[i]->AllocateViews( m_dwNumViews );
    }

    DIDICallout* pCallout = NULL;
    DIDIOverlay* pOverlay = NULL;

    int x = 0, y = 0;
    UINT index = 0;

    // Build the view by enumerating through the callouts and 

    // For each view...
    for( UINT view=0; view < m_dwNumViews; view++ )
    {
        x = PADDING_SIZE;

        // For each column...
        for( int col=0; col < nColsPerView; col++ )
        {
            y = PADDING_SIZE;

            // For each row...
            for( int row=0; row < nRowsPerView; row++ )
            {          
                // Eat indices to invisible objects until an object is found or
                // we run out.
                while( index < m_dwNumObjects && 
                       ( DIDICOS_INVISIBLE & m_apObject[index]->GetCalloutState() ) )
                {
                    index++;
                }

                // If we're on a valid object, calculate the screen coords
                if( index < m_dwNumObjects )
                {
                    pOverlay = m_apObject[index]->GetOverlay( view );
                    pCallout = m_apObject[index]->GetCallout( view );

                    pOverlay->rcScaled.left = x;
                    pOverlay->rcScaled.top  = y;
                    pOverlay->rcScaled.right = x + nMaxNameWidth;
                    pOverlay->rcScaled.bottom = y + nMaxNameHeight;

                    pCallout->rcScaled.left = x + nMaxNameWidth + SPACING_WIDTH;
                    pCallout->rcScaled.top = y;
                    pCallout->rcScaled.right = pCallout->rcScaled.left + nMaxCalloutWidth;
                    pCallout->rcScaled.bottom = y + nMaxNameHeight;
                    pCallout->dwTextAlign = DIDAL_LEFTALIGNED | DIDAL_BOTTOMALIGNED;
                }
                index++;
             
                y += nMaxNameHeight + SPACING_HEIGHT;
            }

            x += nMaxNameWidth + nMaxCalloutWidth + SPACING_WIDTH + GUTTER_SIZE;
        }
    }

    // Clear the invalid flag
    m_bInvalidUI = FALSE;  
    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: CreateScaledSurfaceCopy
// Desc: Creates a new surface and copies the contents from the provided
//       source surface onto the newly created destination surface.
//-----------------------------------------------------------------------------
HRESULT CDIDevImage::CreateScaledSurfaceCopy( LPDIRECT3DSURFACE9 pSurfaceSrc,
                                              DWORD dwWidthSrc, DWORD dwHeightSrc, 
                                              FLOAT fxScale, FLOAT fyScale, 
                                              LPDIRECT3DSURFACE9 *ppSurfaceDest )
{
    HRESULT hr;
    RECT    rcSrc = {0}, rcDest = {0}; 

    // Verify parameters
    if( NULL == pSurfaceSrc || NULL == ppSurfaceDest )
        return DIERR_INVALIDPARAM;

    // Calculate creation arguments
    rcSrc.right  = dwWidthSrc-1; 
    rcSrc.bottom = dwHeightSrc-1;
    ScaleRect( &rcSrc, &rcDest, fxScale, fyScale );
    
    // Create the stored surface
    *ppSurfaceDest = GetCloneSurface( rcDest.right, rcDest.bottom );
                  
    // Since we're using a surface workaround, the d3d functions should only
    // be called if we're actually scaling the surface.
    if( EqualRect( &rcSrc, &rcDest ) )
    {
        D3DLOCKED_RECT d3drcSrc = {0};
        D3DLOCKED_RECT d3drcDest = {0};

        pSurfaceSrc->LockRect( &d3drcSrc, NULL, 0 );
        (*ppSurfaceDest)->LockRect( &d3drcDest, NULL, 0 );

        BYTE* pBitsSrc = (BYTE*) d3drcSrc.pBits;
        BYTE* pBitsDest = (BYTE*) d3drcDest.pBits;

        for( int y = rcDest.top; y < rcDest.bottom; y++ )
        {
            CopyMemory( pBitsDest, pBitsSrc, d3drcDest.Pitch );
            pBitsDest += d3drcDest.Pitch;
            pBitsSrc += d3drcSrc.Pitch;
        }
    }
    else
    {
        // Copy the image onto the stored surface
        hr = D3DXLoadSurfaceFromSurface( *ppSurfaceDest, 
                                          NULL, &rcDest, 
                                          pSurfaceSrc, NULL,
                                          &rcSrc, D3DX_FILTER_TRIANGLE,
                                          NULL );

        if( FAILED(hr) )
            goto LCleanReturn;

    }

    // Everything went OK. Return before cleaning up the new surface.
    return DI_OK;

LCleanReturn:
    // An error occured. Clean up the new surface before returning.
    if( *ppSurfaceDest )
    {
       (*ppSurfaceDest)->Release();
        *ppSurfaceDest = NULL;
    }
    
    return hr;
}




//-----------------------------------------------------------------------------
// Name: CleanUp
// Desc: Release all allocated memory, zero out member variables
//-----------------------------------------------------------------------------
void CDIDevImage::CleanUp()
{
    UINT i=0; //loop var

    // Release resources first
    DestroyImages();

    // Call destructors on all stored objects
    for( i=0; i < m_dwNumObjects; i++ )
    {
        delete m_apObject[i];
    }

    // Delete object array
    if( m_apObject )
        delete [] m_apObject;


    // Delete BITMAP storage array
    delete [] m_atszImagePath;
    delete [] m_ahImages;

    if( m_hFont )
        DeleteObject( m_hFont );

    m_apObject      = NULL;
    m_atszImagePath = NULL;
    m_ahImages      = NULL;
    m_dwNumObjects  = 0;
    m_dwNumViews    = 0;
    m_dwWidthPref   = 0;
    m_dwHeightPref  = 0;
    m_dwScaleMethod = 0;
    m_hFont         = NULL;
    m_bInitialized  = FALSE;
    m_bCustomUI     = FALSE;
    m_bInvalidUI    = TRUE;
}




//-----------------------------------------------------------------------------
// Name: DestroyImages
// Desc: Release all stored images
//-----------------------------------------------------------------------------
VOID CDIDevImage::DestroyImages()
{
    UINT i=0; //loop var

    // Release all background images
    if( m_ahImages )
    {
        for( i=0; i < m_dwNumViews; i++ )
        {
            if( m_ahImages[i] )
                DeleteObject( m_ahImages[i] );

            m_ahImages[i] = NULL;
        }
    }

    // Release all object images
    for( i=0; i < m_dwNumObjects; i++ )
    {
        if( m_apObject[i] )
            m_apObject[i]->DestroyImages();
    }
};




//-----------------------------------------------------------------------------
// Name: CreateFont
// Desc: Create the GDI font to use for callout text
//-----------------------------------------------------------------------------
VOID CDIDevImage::CreateFont()
{
    // Create display font
    LOGFONT lf;
    ZeroMemory( &lf, sizeof(LOGFONT) );
    lf.lfHeight = 14;
    lf.lfWeight = 700;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
    _tcscpy( lf.lfFaceName, TEXT("arial") );

    m_hFont = CreateFontIndirect( &lf );

    // Have a backup plan
    if( NULL == m_hFont )
        m_hFont = (HFONT) GetStockObject( DEFAULT_GUI_FONT );
}




//-----------------------------------------------------------------------------
// Name: CDIDIObject
// Desc: Constructor
//-----------------------------------------------------------------------------
CDIDIObject::CDIDIObject( DWORD dwID, DWORD dwNumViews )
{
    m_dwID        = dwID; 
    m_crNormColor = RGB(150, 150, 200);
    m_crHighColor = RGB(255, 255, 255);
    m_dwNumViews  = dwNumViews;


    m_aCallout = NULL;
    m_aOverlay = NULL;
    m_dwState  = 0;

    AllocateViews( dwNumViews );

    lstrcpy( m_strCallout, TEXT("_ _ _") );
    lstrcpy( m_strName,    TEXT("") );
};




//-----------------------------------------------------------------------------
// Name: ~CDIDIObject
// Desc: Destructor
//-----------------------------------------------------------------------------
CDIDIObject::~CDIDIObject() 
{
    DestroyImages();
    delete [] m_aCallout;
    delete [] m_aOverlay;
};




//-----------------------------------------------------------------------------
// Name: AllocateViews
// Desc: The number of views for a custom UI can change, requiring the need
//       to free and reallocate resources depending on the number of views.
//-----------------------------------------------------------------------------
HRESULT CDIDIObject::AllocateViews( DWORD dwNumViews ) 
{
    // Release current views
    delete [] m_aCallout;
    delete [] m_aOverlay;

    m_dwNumViews = 0;

    m_aCallout = new DIDICallout[dwNumViews];
    if( NULL == m_aCallout )
        return DIERR_OUTOFMEMORY;

    m_aOverlay = new DIDIOverlay[dwNumViews];
    if( NULL == m_aOverlay )
    {
        delete[] m_aCallout;
        m_aCallout = NULL;
        return DIERR_OUTOFMEMORY;
    }

    ZeroMemory( m_aCallout, sizeof(DIDICallout) * dwNumViews );
    ZeroMemory( m_aOverlay, sizeof(DIDIOverlay) * dwNumViews );

    m_dwNumViews = dwNumViews;
    return DI_OK;
};




//-----------------------------------------------------------------------------
// Name: SetOverlay
// Desc: Sets the values for an overlay in the object's current list
//-----------------------------------------------------------------------------
VOID CDIDIObject::SetOverlay( DWORD dwViewID, LPCTSTR strImagePath, RECT rect )
{
    m_aOverlay[dwViewID].rcInit = rect;
    m_aOverlay[dwViewID].rcScaled = rect;
    lstrcpy( m_aOverlay[dwViewID].strImagePath, strImagePath );
}




//-----------------------------------------------------------------------------
// Name: SetCallout
// Desc: Adds the values for a callout in the object's current list
//-----------------------------------------------------------------------------
VOID CDIDIObject::SetCallout( DWORD dwViewID, DWORD dwNumPoints, POINT *aptLine, RECT rect, DWORD dwTextAlign )
{
    m_aCallout[dwViewID].dwNumPoints = dwNumPoints;
    m_aCallout[dwViewID].rcInit = rect;
    m_aCallout[dwViewID].rcScaled = rect;
    m_aCallout[dwViewID].dwTextAlign = dwTextAlign;

    for( int i=0; i < 5; i++)
    {
        m_aCallout[dwViewID].aptLineInit[i] = aptLine[i];
        m_aCallout[dwViewID].aptLineScaled[i] = aptLine[i];
    }
}




//-----------------------------------------------------------------------------
// Name: DestroyImages
// Desc: Release all stored images
//-----------------------------------------------------------------------------
VOID CDIDIObject::DestroyImages()
{
    UINT i=0; //loop var

    // Release all object images
    for( i=0; i < m_dwNumViews; i++ )
    {
        if( m_aOverlay[i].hImage )
            DeleteObject( m_aOverlay[i].hImage );

        m_aOverlay[i].hImage = NULL;
    }
};




//-----------------------------------------------------------------------------
// Name: SetCalloutText
// Desc: copy as many characters as will fit, leaving room for the terminating
//       null and 3 elipses.
//-----------------------------------------------------------------------------
VOID CDIDIObject::SetCalloutText( LPCTSTR strText )
{
    _tcsncpy( m_strCallout, strText, MAX_PATH - 4 );
}




//-----------------------------------------------------------------------------
// Name: GetCalloutText
// Desc: Retrieve the current callout text from the name buffer up to the 
//       amount of characters specified by dwSize
//-----------------------------------------------------------------------------
VOID CDIDIObject::GetCalloutText( LPTSTR strText, DWORD dwSize )
{
    _tcsncpy( strText, m_strCallout, dwSize );
}




//-----------------------------------------------------------------------------
// Name: ScaleView
// Desc: Scale all rects and points to the given scaling factors
//-----------------------------------------------------------------------------
VOID CDIDIObject::ScaleView( DWORD dwViewID, FLOAT fxScale, FLOAT fyScale )
{
    UINT i=0;

    // Overlay/Callout Rects
    ScaleRect( &(m_aOverlay[dwViewID].rcInit), 
               &(m_aOverlay[dwViewID].rcScaled),
               fxScale, fyScale );

    ScaleRect( &(m_aCallout[dwViewID].rcInit), 
               &(m_aCallout[dwViewID].rcScaled),
               fxScale, fyScale );

    // Callout Lines
    for( i=0; i < 5; i++ )
    {
        ScalePoint( &(m_aCallout[dwViewID].aptLineInit[i]), 
                    &(m_aCallout[dwViewID].aptLineScaled[i]),
                    fxScale, fyScale );
    }

}




//-----------------------------------------------------------------------------
// Name: ApplyAlphaChannel
// Desc: Restore the alpha information from the source bitmap to the
//       destination bitmap. GDI uses COLORREF structures which lack an alpha
//       channel, and draws everything as fully transparent. By using a 
//       separate bitmap to store opacity information, the alpha channel can
//       be restored.
//-----------------------------------------------------------------------------
HRESULT ApplyAlphaChannel( HBITMAP hbmpDest, HBITMAP hbmpSrc, BOOL bOpaque )
{
    HRESULT        hr          = S_OK;
    BITMAP         bmpInfoSrc  = {0}; 
    BITMAP         bmpInfoDest = {0};
    
   
    // Verify parameters
    if( NULL == hbmpDest || NULL == hbmpSrc )
        return DIERR_INVALIDPARAM;

    // Get the bitmap pixel data
    if( 0 == GetObject( hbmpSrc,  sizeof(BITMAP), &bmpInfoSrc ) )
        return E_FAIL;

    if( 0 == GetObject( hbmpDest, sizeof(BITMAP), &bmpInfoDest ) )
        return E_FAIL;

    // Cast the data pointers
    DWORD* pBitsSrc  = (DWORD*) bmpInfoSrc.bmBits;
    DWORD* pBitsDest = (DWORD*) bmpInfoDest.bmBits;

    // Syncronize bitmap pixel info
    GdiFlush();

    // For each pixel in the rect...
    for( int y=0; y < bmpInfoDest.bmHeight; y++ )
    {
        for( int x=0; x < bmpInfoDest.bmWidth; x++ )
        {
            if( *pBitsSrc )
            {
                // The source bitmap contains information at this pixel, which 
                // may signal full or partial opacity. If the passed bOpaque
                // flag is set, the method should treat partially opaque pixels
                // as being fully opaque. This is used when the user has selected
                // a fully opaque background color and wants the resulting image
                // to also be fully opaque.
                *pBitsDest |= bOpaque ? ALPHA_MASK : ( GetBlue( *pBitsSrc ) << 24 );
            }  
            
            // Advance the pixel pointers.
            pBitsSrc++;
            pBitsDest++;
        }
    }

    return hr;
}




//-----------------------------------------------------------------------------
// Name: RestoreRect
// Desc: Restore the pixel data of an image for the given RECT
//-----------------------------------------------------------------------------
HRESULT RestoreRect( HBITMAP hbmpDest, CONST RECT* prcDest, LPBYTE pSrcPixels )
{
    BITMAP         bmpInfoDest = {0};

    // Verify parameters
    if( NULL == hbmpDest || NULL == pSrcPixels || NULL == prcDest )
        return DIERR_INVALIDPARAM;

    // Get the bitmap pixel data
    if( 0 == GetObject( hbmpDest, sizeof(BITMAP), &bmpInfoDest ) )
        return E_FAIL;

    // Cast the data pointers
    DWORD* pBitsSrc  = (DWORD*) pSrcPixels;
    DWORD* pBitsDest = (DWORD*) bmpInfoDest.bmBits;

    // Syncronize bitmap pixel info
    GdiFlush();

    // Advance the pixel pointers to the starting position
    pBitsDest += ( prcDest->top * bmpInfoDest.bmWidth ) + prcDest->left;
    pBitsSrc  += ( prcDest->top * bmpInfoDest.bmWidth ) + prcDest->left;

    // For each scanline in rcDest...
    for( int y = prcDest->top; y < prcDest->bottom; y++ )
    {
        CopyMemory( pBitsDest, pBitsSrc, sizeof(DWORD) * ( prcDest->right - prcDest->left ) );
        
        // Advance pointers
        pBitsDest += bmpInfoDest.bmWidth;
        pBitsSrc  += bmpInfoDest.bmWidth;
    
    }

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: ApplyOverlay
// Desc: Use manual alpha blending to paste the overlay bitmap on top of the 
//       destination bitmap.
//-----------------------------------------------------------------------------
HRESULT ApplyOverlay( HBITMAP hbmpDest, CONST RECT* prcDest, HBITMAP hbmpSrc )
{
    BITMAP         bmpInfoSrc  = {0}; 
    BITMAP         bmpInfoDest = {0};

    // Verify parameters
    if( NULL == hbmpDest || NULL == hbmpSrc || NULL == prcDest )
        return DIERR_INVALIDPARAM;

    // Get the bitmap pixel data
    if( 0 == GetObject( hbmpSrc,  sizeof(BITMAP), &bmpInfoSrc ) )
        return E_FAIL;

    if( 0 == GetObject( hbmpDest, sizeof(BITMAP), &bmpInfoDest ) )
        return E_FAIL;

    // Cast the data pointers
    DWORD* pBitsSrc  = (DWORD*) bmpInfoSrc.bmBits;
    DWORD* pBitsDest = (DWORD*) bmpInfoDest.bmBits;

    // Syncronize bitmap pixel info
    GdiFlush();

    // Advance the destination pixel to the starting position
    pBitsDest += ( prcDest->top * bmpInfoDest.bmWidth ) + prcDest->left;

    // For each pixel...
    for( int y=0; y < bmpInfoSrc.bmHeight; y++ )
    {
        for( int x=0; x < bmpInfoSrc.bmWidth; x++ )
        {
            // calculate src and dest alpha
            if( ( *pBitsSrc & ALPHA_MASK ) == ALPHA_MASK )
            {
                // Source pixel is completely opaque, have it replace the
                // current destination pixel
                *pBitsDest = *pBitsSrc;
            }            
            else if( ( *pBitsSrc & ALPHA_MASK ) == 0 )
            {
                // Source pixel is completely transparent, do nothing
            }
            else
            {
            // This formula computes the blended component value:
                // ( ALPHA * ( srcPixel - destPixel ) ) / 256 + destPixel
                
                DWORD dwMultiplier  = GetAlpha( *pBitsSrc );

                // Decompose the image into color components
                DWORD dwRed   = GetRed( *pBitsDest );
                DWORD dwGreen = GetGreen( *pBitsDest );
                DWORD dwBlue  = GetBlue( *pBitsDest );


                // Calculate the component blend
                dwRed   = ( dwMultiplier * (   GetRed( *pBitsSrc ) - dwRed   ) ) / 256 + dwRed;
                dwGreen = ( dwMultiplier * ( GetGreen( *pBitsSrc ) - dwGreen ) ) / 256 + dwGreen;
                dwBlue  = ( dwMultiplier * (  GetBlue( *pBitsSrc ) - dwBlue  ) ) / 256 + dwBlue;
                
                // Compose the blended pixel. The destination bitmap's original alpha
                // value is preserved.
                *pBitsDest = (*pBitsDest & ALPHA_MASK) | dwRed << 16 | dwGreen << 8 | dwBlue;
            }

            // Move to next pixel
            pBitsDest++;
            pBitsSrc++;
        }

        // Advance destination pointer
        pBitsDest += ( bmpInfoDest.bmWidth - bmpInfoSrc.bmWidth );
    }

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: FillBackground
// Desc: Fills the background of the given bitmap with the provided fill color.
//       The background of the image is defined by the alpha channel.
//-----------------------------------------------------------------------------
HRESULT FillBackground( HBITMAP hbmpDest, D3DCOLOR Fill )
{
    BITMAP         bmpInfo     = {0}; 
    
    // Verify parameters
    if( NULL == hbmpDest )
        return DIERR_INVALIDPARAM;

    // Get the bitmap pixel data
    if( 0 == GetObject( hbmpDest, sizeof(BITMAP), &bmpInfo ) )
        return E_FAIL;

    // Cast the data pointers
    DWORD* pBits = (DWORD*) bmpInfo.bmBits;

    // Extract the component channels of the background color.
    DWORD dwBkAlpha = GetAlpha( Fill );
    DWORD dwBkRed   = GetRed( Fill );
    DWORD dwBkGreen = GetGreen( Fill );
    DWORD dwBkBlue  = GetBlue( Fill );

    // If the background color is defined as being completely opaque, the fill
    // method is performed a little differently. Normally, the per-pixel alpha
    // value is blended along with the color components, but this can result in
    // areas of the image which are partially transparent. If the provided fill
    // color is opaque, it's assumed that the user wishes to have the final
    // rendered surface completely opaque. If the provided background color is
    // partially transparent, then the alpha channel is blended to allow for
    // an antialiased border around the device image.
    BOOL  bBkOpaque = ( dwBkAlpha == 255 );

    // Syncronize bitmap pixel info
    GdiFlush();

    // For each pixel in the rect...
    for( int y=0; y < bmpInfo.bmHeight; y++ )
    {
        for( int x=0; x < bmpInfo.bmWidth; x++ )
        {
            if( (*pBits & ALPHA_MASK) == ALPHA_MASK )
            {
                // Destination pixel is completely opaque, no fill needed.
            }            
            else if( (*pBits & ALPHA_MASK) == 0 )
            {
                // Destination pixel is completely transparent, replace with
                // the fill color.
                *pBits = Fill;
            }
            else
            {
                // This formula computes the blended component value:
                // ( ALPHA * ( srcPixel - destPixel ) ) / 256 + destPixel
                
                DWORD dwMultiplier  = GetAlpha( *pBits );

                // Calculate the component blend
                DWORD dwAlpha = bBkOpaque ? 255 : 
                                ( dwMultiplier * ( GetAlpha( *pBits ) - dwBkAlpha ) ) / 256 + dwBkAlpha;
                DWORD dwRed   = ( dwMultiplier * (   GetRed( *pBits ) - dwBkRed   ) ) / 256 + dwBkRed;
                DWORD dwGreen = ( dwMultiplier * ( GetGreen( *pBits ) - dwBkGreen ) ) / 256 + dwBkGreen;
                DWORD dwBlue  = ( dwMultiplier * (  GetBlue( *pBits ) - dwBkBlue  ) ) / 256 + dwBkBlue;
                
                // Compose the final pixel color value
                *pBits = dwAlpha << 24 | dwRed << 16 | dwGreen << 8 | dwBlue;

            }

            // Move to next pixel
            pBits++;
        }
    }

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: DrawTooltip
// Desc: Draws tooltip text on the provided device contexts. The tooltip string
//       is drawn in relation to the RECT containing the truncated text, and 
//       using the given foreground, background, and border colors.
//-----------------------------------------------------------------------------
HRESULT DrawTooltip( HDC hdcRender, HDC hdcAlpha, TCHAR* strTooltip, RECT* prcBitmap, 
                     RECT* prcTruncated, COLORREF crFore, COLORREF crBack, COLORREF crBorder )
{
    // Create the tooltip font. This should be highly readable at a small point
    // size, so a raster font has been chosen.
    HFONT hTipFont = NULL;
    LOGFONT lfTipFont = {0};
    SIZE sizeText = {0};
    
    lstrcpy( lfTipFont.lfFaceName, TEXT("MS Sans Serif") );
    lfTipFont.lfHeight = 8;
    lfTipFont.lfOutPrecision = OUT_RASTER_PRECIS;
    lfTipFont.lfQuality = PROOF_QUALITY;
    hTipFont = CreateFontIndirect( &lfTipFont );

    if( !hTipFont )
        hTipFont = (HFONT) GetStockObject( SYSTEM_FONT );
 
    HFONT hOldRenderFont = (HFONT) SelectObject( hdcRender, hTipFont );
    HFONT hOldAlphaFont = (HFONT) SelectObject( hdcAlpha, hTipFont );

    // How much screen space are we going to need?
    GetTextExtentPoint32( hdcRender, strTooltip, lstrlen( strTooltip ), &sizeText );

    // Position the rect right above the callout space
    RECT rcTooltip = { 0, 0, sizeText.cx, sizeText.cy };
    OffsetRect( &rcTooltip, prcTruncated->left + 2, prcTruncated->top - sizeText.cy - 2);

    InflateRect( &rcTooltip, 2, 2 );

    // Adjust the tooltip rect if it's beyond the screen edge
    if( rcTooltip.top < 0 )
        OffsetRect( &rcTooltip, 0, -rcTooltip.top );

    if( rcTooltip.right > prcBitmap->right )
        OffsetRect( &rcTooltip, prcBitmap->right - rcTooltip.right, 0 );


    // Draw tooltip 
    HBRUSH hOldRenderBrush = (HBRUSH) SelectObject( hdcRender, CreateSolidBrush( crBack ) );
    HPEN hOldRenderPen = (HPEN) SelectObject( hdcRender, CreatePen( PS_SOLID, 1, crBorder ) );

    COLORREF crOldRenderTextColor = SetTextColor( hdcRender, crFore );

    
    Rectangle( hdcRender, rcTooltip.left, rcTooltip.top, rcTooltip.right, rcTooltip.bottom );
    FillRect( hdcAlpha, &rcTooltip, (HBRUSH) GetStockObject( WHITE_BRUSH ) );

    InflateRect( &rcTooltip, -2, -2 );
    DrawText( hdcRender, strTooltip, lstrlen( strTooltip ), &rcTooltip, 0 );
    
    

    // Restore the DC state
    SelectObject( hdcRender, hOldRenderFont );
    SelectObject( hdcAlpha,  hOldAlphaFont );
    
    DeleteObject( (HBRUSH) SelectObject( hdcRender, hOldRenderBrush ) );
    DeleteObject( (HPEN)   SelectObject( hdcRender, hOldRenderPen ) );

    SetTextColor( hdcRender, crOldRenderTextColor );

    DeleteObject( hTipFont );

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: CreateDIBSectionFromSurface
// Desc: Extract the pixel information from the provided surface and create
//       a new DIB Section.
//-----------------------------------------------------------------------------
HRESULT CreateDIBSectionFromSurface( LPDIRECT3DSURFACE9 pSurface, HBITMAP* phBitmap, SIZE* pSize )
{
    HRESULT         hr;
    D3DSURFACE_DESC d3dDesc;
    D3DLOCKED_RECT  d3dRect;
    LPBYTE          pDIBBits;
    LPBYTE          pSurfBits;
    BITMAPINFO      bmi = {0};

    // Get the surface info
    hr = pSurface->GetDesc( &d3dDesc );
    if( FAILED( hr ) )
        return hr;

    // Lock the surface
    hr = pSurface->LockRect( &d3dRect, NULL, 0 );
    if( FAILED( hr ) )
        return hr;

    pSurfBits = (LPBYTE) d3dRect.pBits;

    // Fill in the bitmap creation info
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = pSize ? pSize->cx : d3dDesc.Width;
    bmi.bmiHeader.biHeight = pSize ? -pSize->cy : ( - (int) d3dDesc.Height ); // Top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    
    HDC hdcMem = CreateCompatibleDC( NULL );
    if( NULL == hdcMem )
        return DIERR_OUTOFMEMORY;

    // Create the DIBSection
    *phBitmap = CreateDIBSection( hdcMem, &bmi, DIB_RGB_COLORS,( LPVOID* )&pDIBBits, NULL, 0 );
    DeleteDC( hdcMem );
    if( NULL == *phBitmap )
         return DIERR_OUTOFMEMORY;

    
    // Copy the bits
    for( UINT y=0; y < d3dDesc.Height; y++ )
    {
        CopyMemory( pDIBBits, pSurfBits, ( d3dDesc.Width * sizeof(DWORD) ) );
        pDIBBits += bmi.bmiHeader.biWidth * sizeof(DWORD);
        pSurfBits += d3dRect.Pitch;
    }

    return DI_OK;
}





//-----------------------------------------------------------------------------
// Name: EnumDeviceObjectsCB
// Desc: Cycle through device objects, adding information to the passed 
//       CDIDevImage object when appropriate.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumDeviceObjectsCB( LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef )
{
    HRESULT hr;

    // Extract the passed pointer
    CDIDevImage *pDIDevImage = (CDIDevImage*) pvRef;
    CDIDIObject *pObject     = NULL;

    // Add the object to the list
    hr = pDIDevImage->AddObject( lpddoi->dwType );
    if( FAILED(hr) )
        return DIENUM_STOP;

    // Set the object's friendly name
    pObject = pDIDevImage->GetObject( lpddoi->dwType );
    if( pObject )
        pObject->SetName( lpddoi->tszName );


    return DIENUM_CONTINUE;
}




//-----------------------------------------------------------------------------
// Name: DidcvDirect3DSurface9Clone
// Desc: a clone surface
//-----------------------------------------------------------------------------
class DidcvDirect3DSurface9Clone : public IUnknown
{
private:
    int m_iRefCount;
    BYTE *m_pData;
    D3DSURFACE_DESC m_Desc;

public:
    DidcvDirect3DSurface9Clone() : m_pData( NULL ), m_iRefCount( 1 ) { }
    ~DidcvDirect3DSurface9Clone() { delete[] m_pData; }

public:     
    // IUnknown methods
    STDMETHOD( QueryInterface )( REFIID  riid, VOID  **ppvObj ) { return E_NOINTERFACE; }
    STDMETHOD_( ULONG,AddRef )() { return ++m_iRefCount; }
    STDMETHOD_( ULONG,Release )()
    {
        if( !--m_iRefCount )
        {
            delete this;
            return 0;
        }
        return m_iRefCount;
    }

    // IDirect3DResource9 methods
    STDMETHOD( GetDevice )( IDirect3DDevice9 **ppDevice ) { return E_FAIL; }
    STDMETHOD( SetPrivateData )( REFGUID riid, CONST VOID *pvData, DWORD cbData, DWORD   dwFlags ) { return E_FAIL; }
    STDMETHOD( GetPrivateData )( REFGUID riid, VOID* pvData, DWORD  *pcbData ) { return E_FAIL; }
    STDMETHOD( FreePrivateData )( REFGUID riid ) { return E_FAIL; }
    STDMETHOD( SetPriority )( DWORD PriorityNew ) { return E_FAIL; }
    STDMETHOD_( DWORD, GetPriority )() { return 0; }
    STDMETHOD( PreLoad )() { return E_FAIL; }
    STDMETHOD( GetType )() { return E_FAIL; }
    
    // IDirect3DSurface9 methods
    STDMETHOD( GetContainer )( REFIID riid, void **ppContainer ) { return E_FAIL; }
    STDMETHOD_( D3DSURFACE_DESC, GetDesc )() { return m_Desc; }     
    STDMETHOD( LockRect )( D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD dwFlags )
    {
        // Assume the entire surface is being locked.
        pLockedRect->Pitch = m_Desc.Width * 4;
        pLockedRect->pBits = m_pData;
        return S_OK;
    }
    STDMETHOD( UnlockRect )() { return S_OK; }
    STDMETHOD( GetDC )() { return E_FAIL; }
    STDMETHOD( ReleaseDC )() { return E_FAIL; }

    BOOL Create( int iWidth, int iHeight )
    {
        m_pData = new BYTE[iWidth * iHeight * 4];
        if( !m_pData ) return FALSE;

        m_Desc.Format = D3DFMT_A8R8G8B8;
        m_Desc.Type = D3DRTYPE_SURFACE;
        m_Desc.Usage = 0;
        m_Desc.Pool = D3DPOOL_SYSTEMMEM;
        m_Desc.MultiSampleType = D3DMULTISAMPLE_NONE;
        m_Desc.MultiSampleQuality = 0;
        m_Desc.Width = iWidth;
        m_Desc.Height = iHeight;
        return TRUE;
    }
};




//-----------------------------------------------------------------------------
// Name: GetCloneSurface
// Desc: Fake a Direct3D surface so we don't have to actually create a direct3d
//       device object.
//-----------------------------------------------------------------------------
IDirect3DSurface9* GetCloneSurface( int iWidth, int iHeight )
{
    DidcvDirect3DSurface9Clone *pSurf = new DidcvDirect3DSurface9Clone;

    if( !pSurf ) return NULL;
    if( !pSurf->Create( iWidth, iHeight ) )
    {
        delete pSurf;
        return NULL;
    }

    return( IDirect3DSurface9* )pSurf;
}