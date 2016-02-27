/*
 CSound 클래스(CSound.cpp)

  desc : Wave sound를 출력하는 클래스
  date : 2004년 5월 30일
  comp : 임동환

  반드시 Project Settings에서 Link->Object/library modules에
  winmm.lib을 추가해줘야 함.
*/

#include <stdafx.h>
#include "CSound.h"
#include <MMSystem.h>



// PlayWavSound
void CSound::PlayWavSound( UINT nIDResource)
{
	// Play sound
	PlayWavSound( MAKEINTRESOURCE( nIDResource));
}


// PlayWavSound
inline void CSound::PlayWavSound( LPCTSTR lpszSound)
{
	HRSRC hRes;
	HGLOBAL hData;
	BOOL bOk = false;
	if (( hRes  = ::FindResource( AfxGetResourceHandle(), lpszSound, _T( "WAVE"))) != NULL &&
	    ( hData = ::LoadResource( AfxGetResourceHandle(), hRes)) != NULL)
	{
		bOk = sndPlaySound( (LPCTSTR)::LockResource( hData), SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
		FreeResource( hData);
	}
}
