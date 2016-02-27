/*
 CDDBAniBmp 클래스(CDDBAniBmp.h)

  desc : DDB Animation Bitmap을 생성하고 처리하는 클래스
  date : 2004년 5월 30일
  comp : 임동환
*/


#include <stdafx.h>
#include "CDDBAniBmp.h"


#define TIMER_ANIMATE		1


/************************************************************************************
  CDDBAniBmp window
*************************************************************************************/
// CDDBAniBmp
CDDBAniBmp::CDDBAniBmp()
{
	m_nNumOfScene		= 0;
	m_nCurrentScene		= 0;
	m_fAnimate			= false;
	m_fLoop				= true;
}


//~CDDBAniBmp
CDDBAniBmp::~CDDBAniBmp()
{
	ddbSkin.DeleteDDB();
	ddbRendering.DeleteDDB();
}


// Message map
BEGIN_MESSAGE_MAP(CDDBAniBmp, CStatic)
	//{{AFX_MSG_MAP(CDDBAniBmp)
	ON_WM_PAINT()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/************************************************************************************
  CDDBAniBmp message handlers
*************************************************************************************/

/*************************************************************
 InitDDBAniBmp

  desc : Animation bitmap을 생성한다.
  arg  : nIDResource = 이미지 리소스 ID
         nNumOfScene = 이미지 장면(scene) 수
  ret  : true(=성공) or false(=실패)
  ex   : InitDDBAniBmp( IDR_ANIBMP, 5);
**************************************************************/
BOOL CDDBAniBmp::InitDDBAniBmp( UINT nIDResource, UINT nNumOfScene)
{
	if ( nNumOfScene < 1)
		return false;

	// KillTimer
	KillTimer( TIMER_ANIMATE);

	// Delete DDB
	ddbSkin.DeleteDDB();
	ddbRendering.DeleteDDB();
	
	// CreateDDB
	if ( ! ddbSkin.CreateDDB( GetDC(), nIDResource))
		return false;
	m_nWidth  = (ddbSkin.GetWidth() - nNumOfScene + 1) / nNumOfScene;
	m_nHeight = ddbSkin.GetHeight();
	if ( ! ddbRendering.CreateDDB( 0, 0, GetDC(), m_nWidth, m_nHeight, RGB(255,255,255)))
		return false;

	// Set varialbes
	for ( UINT i = 0;  i < 50;  i++)
		m_nTimerValue[ i] = 1000;
	m_nNumOfScene    = nNumOfScene;
	m_nStartSceneNum = 0;
	m_nEndSceneNum   = nNumOfScene - 1;

	// Set dialog rectangle
	CRect parent_rect, rect;
	GetParent()->GetWindowRect( &parent_rect);
	GetWindowRect( &rect);
	rect.left   = rect.left - parent_rect.left;
	rect.top    = rect.top  - parent_rect.top;
	rect.right  = rect.left + m_nWidth;
	rect.bottom = rect.top  + m_nHeight;
	MoveWindow( rect);

	// Show scene
	ShowScene( 0);

	return true;
}


/*************************************************************
 EnableAnimate

  desc : 에니메이션을 시작함
  arg  : fAnimate
  ret  : true(=성공) or false(=실패)
  ex   : EnableAnimate();
**************************************************************/
void CDDBAniBmp::EnableAnimate( BOOL fAnimate)
{
	// Set timer
	if ( fAnimate)
	{
		ShowScene( m_nStartSceneNum);
		SetTimer( TIMER_ANIMATE, m_nTimerValue[ m_nStartSceneNum], NULL);
	}
	// Reset timer
	else
		KillTimer( TIMER_ANIMATE);

	m_fAnimate = fAnimate;
}


/*************************************************************
 SetAnimateScene()

  desc : 에니메이션할 이미지를 설정
  arg  : nStartSceneNum = 시작 이미지 장면 번호
         nEndtSceneNum = 종료 이미지 장면 번호
  ret  : true(=성공) or false(=실패)
  ex   : SetAnimateScene( 0, 5);
**************************************************************/
BOOL CDDBAniBmp::SetAnimateScene(UINT nStartSceneNum, UINT nEndtSceneNum)
{
	if ( (nStartSceneNum >= m_nNumOfScene) || (nEndtSceneNum >= m_nNumOfScene))
		return false;
	if ( nStartSceneNum > m_nNumOfScene)
		return false;

	// Kill timer
	KillTimer( TIMER_ANIMATE);

	// Set start/end scene number
	m_nStartSceneNum = nStartSceneNum;
	m_nEndSceneNum   = nEndtSceneNum;

	// Show scene
	if ( m_fAnimate)
	{
		ShowScene( m_nStartSceneNum);
		// Set timer
		SetTimer( TIMER_ANIMATE, m_nTimerValue[ m_nStartSceneNum], NULL);
	}

	return true;
}


/*************************************************************
 ShowScene

  desc : 지정한 번호의 장면을 보인다.
  arg  : nSceneNum = 장면 번호
  ret  : true(=성공) or false(=실패)
  ex   : ShowScene( 1);
**************************************************************/
BOOL CDDBAniBmp::ShowScene( UINT nSceneNum)
{
	// Check scene number
	if ( nSceneNum > m_nNumOfScene)
		return false;

	// Draw scene
	UINT pos;
	if ( nSceneNum == 0)
		pos = 0;
	else
		pos = m_nWidth * nSceneNum + nSceneNum;
	ddbRendering.PutBitmap( 0, 0, ddbSkin.GetDC(), pos, 0, m_nWidth, m_nHeight, SRCCOPY);

	// Paint dialog screen
	ddbRendering.Rendering();
	m_nCurrentScene = nSceneNum;

	// Set timer
	if ( m_fAnimate)
		SetTimer( TIMER_ANIMATE, m_nTimerValue[ nSceneNum], NULL);

	return true;
}


/*************************************************************
 GetCurrentScene

  desc : 현재 보이는 장면의 이미지 번호를 구한다.
  arg  : none
  ret  : 장면 번호
  ex   : GetCurrentScene();
**************************************************************/
UINT CDDBAniBmp::GetCurrentScene()
{
	return m_nCurrentScene;
}


/*************************************************************
 SetLoop

  desc : 반복(loop) 에니메이션을 함
  arg  : fLoop = true(반복함) or false(반복하지 않음)
  ret  : none
  ex   : SetLoop( true);
**************************************************************/
void CDDBAniBmp::SetLoop( BOOL fLoop)
{
	m_fLoop = fLoop;
}


/*************************************************************
 SetTimerValue

  desc : 각 장면의 지연시간(mili second)을 설정한다.
  arg  : ...
  ret  : none
  ex   : SetTimerValue( 200);
**************************************************************/
void CDDBAniBmp::SetTimerValue( UINT nTimerValue)
{
	for ( UINT i = 0;  i < m_nNumOfScene;  i++)
		m_nTimerValue[ i] = nTimerValue;
}


void CDDBAniBmp::SetTimerValue( UINT nSceneNum, UINT nTimerValue)
{
	m_nTimerValue[ nSceneNum] = nTimerValue;
}


void CDDBAniBmp::SetTimerValue( UINT nStartSceneNum, UINT nEndSceneNum, UINT nTimerValue)
{
	for ( UINT i = nStartSceneNum;  i <= nEndSceneNum;  i++)
		m_nTimerValue[ i] = nTimerValue;
}


void CDDBAniBmp::SetTimerValue( UINT *nTimerValue)
{
	for ( UINT i = 0;  i < m_nNumOfScene;  i++)
		m_nTimerValue[ i] = *( nTimerValue + i);
}


/*************************************************************
 PreSubclassWindow (protected)

  desc : 그리기 설정
**************************************************************/
void CDDBAniBmp::PreSubclassWindow() 
{
	ModifyStyle( 0, BS_OWNERDRAW);
	
	CStatic::PreSubclassWindow();
}


/*************************************************************
 OnPaint (protected)

  desc : 화면 복구
**************************************************************/
void CDDBAniBmp::OnPaint() 
{
	CPaintDC dc(this);

	// Paint dialog screen
	ddbRendering.Rendering();
}


/*************************************************************
 OnTimer (Protected)

  desc : 타이머 핸들러
**************************************************************/
void CDDBAniBmp::OnTimer( UINT nIDEvent) 
{
	if ( nIDEvent == TIMER_ANIMATE)
	{
		// Kill timer
		KillTimer( TIMER_ANIMATE);

		// Show next scene
		BOOL bLoop = true;
		m_nCurrentScene++;
		if ( (m_nCurrentScene > m_nEndSceneNum) || (m_nCurrentScene == m_nNumOfScene))
		{
			m_nCurrentScene = m_nStartSceneNum;

			if ( ! m_fLoop)
				bLoop = false;
		}

		ShowScene( m_nCurrentScene);

		// Set timer
		if ( bLoop)
			SetTimer( TIMER_ANIMATE, m_nTimerValue[ m_nCurrentScene], NULL);
	}

	CStatic::OnTimer(nIDEvent);
}
