// File: GetDXVer.cpp
//	original from dx8 sdk. dubble omit almost

#include <windows.h>
#include <windowsx.h>
#include <basetsd.h>

DWORD GetDXVersion()
{
    HINSTANCE            hD3D8DLL           = NULL;
    DWORD                dwDXVersion        = 0;

    // Simply see if D3D8.dll exists.
    hD3D8DLL = LoadLibrary( "D3D8.DLL" );
    if( hD3D8DLL == NULL )
    {
        return dwDXVersion;
    }

    // D3D8.dll exists. We must be at least DX8.0
    dwDXVersion = 0x800;

    // Close open libraries and return
    FreeLibrary( hD3D8DLL );
    
    return dwDXVersion;
}



