/******************************************************************
   
   ZUpdater.cpp

     Corperation : MAIET entertainment
     Programmer  : Lim Dong Hwan
	 Date        : 4.July.2005

*******************************************************************/


#include "stdafx.h"												// Include stdafx.h
#include <windows.h>											// Include windows.h


// Defines
#define LAUNCHER_NAME				"Gunz Launcher"				// 건즈 런쳐 App 이름
#define LAUNCHER_FILENAME_PATCH		"GunzLauncher.ex_"			// 건즈 런쳐 패치 파일 이름
#define LAUNCHER_FILENAME_EXE		"GunzLauncher.exe"			// 건즈 런쳐 실행 파일 이름



// main
int _tmain(int argc, _TCHAR* argv[])
{
	// 화면에 텍스트 출력
	printf( "Gunz Launcher Updating...\n");


	// 실행중인 런쳐 프로그램을 찾는다
	HWND hWnd = FindWindow( NULL, LAUNCHER_NAME);
	if ( hWnd != NULL)		// 실행중인 런쳐가 있으면...
	{
		// 실행중인 런쳐에 종료 메시지를 보낸다
		PostMessage( hWnd, WM_CLOSE, 0, 0);


		// 실행중인 런쳐가 종료될 때까지 대기한다.
		while ( hWnd != NULL)
		{
			hWnd = FindWindow( NULL, LAUNCHER_NAME);

			Sleep( 50);
		}
	}


	Sleep( 1000);


	// 런쳐 업데이트 파일(GunzLauncher.ex_)을 찾는다
	WIN32_FIND_DATA FindData;
	if ( FindFirstFile( _T( LAUNCHER_FILENAME_PATCH), &FindData) != INVALID_HANDLE_VALUE)		// 파일이 있으면...
	{
		while ( 1)
		{
			// 기존에 있던 런쳐 실행 파일(GunzLauncher.exe)을 삭제한다.
			if ( DeleteFile( _T( LAUNCHER_FILENAME_EXE)) == TRUE)		// 삭제 되었으면...
			{
				while ( 1)
				{
					// 런쳐 업데이트 파일(GunzLauncher.ex_)을 런쳐 실행파일(GunzLauncher.exe)로 복사한다.
					if ( CopyFile( _T( LAUNCHER_FILENAME_PATCH), _T( LAUNCHER_FILENAME_EXE), false) == TRUE)	// 복사 했으면...
					{
						// 런쳐 업데이트 파일(GunzLauncher.ex_)을 삭제한다.
						DeleteFile( _T( LAUNCHER_FILENAME_PATCH));

					
						break;
					}
					// 복사에 실패했다면...
					else
					{
						// 대기
						Sleep( 50);
					}
				}

				break;
			}
			// 런쳐 실행 파일(GunzLauncher.exe)을 삭제를 할 수 없다면...
			else
			{
				// 대기
				Sleep( 50);
			}
		}
	}


	// 런쳐를 다시 시작한다.
	WinExec( "GunzLauncher.exe", SW_SHOW);


	return 0;
}
