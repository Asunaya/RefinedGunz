// MScheduleMaker.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "PatchInterface.h"
#include "MScheduleMaker.h"
#include ".\mschedulemaker.h"
#include "KeeperManager.h"
#include "MMatchGlobal.h"
#include "MMatchSchedule.h"


// MScheduleMaker 대화 상자입니다.

IMPLEMENT_DYNAMIC(MScheduleMaker, CDialog)
MScheduleMaker::MScheduleMaker(CWnd* pParent /*=NULL*/)
	: CDialog(MScheduleMaker::IDD, pParent), m_nScheduleListCount( 0 )
{
}

MScheduleMaker::~MScheduleMaker()
{
}


void MScheduleMaker::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCHEDULE_TYPE, m_cbScheduleType);
	DDX_Control(pDX, IDC_SCHEDULE_DAY, m_cbScheduleDay);
	DDX_Control(pDX, IDC_SCHEDULE_YEAR, m_cbScheduleYear);
	DDX_Control(pDX, IDC_SCHEDULE_MONTH, m_cbScheduleMonth);
	DDX_Control(pDX, IDC_SCHEDULE_HOUR, m_cbScheduleHour);
	DDX_Control(pDX, IDC_SCHEDULE_MIN, m_cbScheduleMin);
	DDX_Control(pDX, IDC_SCHEDULE_COMMAND, m_cbScheduleCommand);
	DDX_Control(pDX, IDC_ANNOUNCE, m_edtAnnounce);
	DDX_Control(pDX, IDC_SCHEDULE_COUNT, m_edtCount);
}


void MScheduleMaker::InitScheduleTypeComboBox()
{
	for( int i = 0; i < SCHEDULE_TYPE_MAX_COUNT; ++i )
		m_cbScheduleType.InsertString( i, SCHEDULE_TYPE[i] );
}


void MScheduleMaker::InitScheduleRelativeYearComboBox()
{
	m_cbScheduleYear.InsertString( 0, SCHEDULE_YEAR[0] );

	for( int i = 1; i < 7; ++i )
		m_cbScheduleYear.InsertString( i, SCHEDULE_YEAR[i] );
}


void MScheduleMaker::InitScheduleYearComboBox()
{
	m_cbScheduleYear.InsertString( 0, SCHEDULE_YEAR[0] );

	for( int i = 7; i < SCHEDULE_YEAR_COUNT; ++i )
		m_cbScheduleYear.InsertString( i - 6, SCHEDULE_YEAR[i] );
}


void MScheduleMaker::InitScheduleMonthComboBox()
{
	for( int i = 0; i < SCHEDULE_MONTH_COUNT; ++i )
		m_cbScheduleMonth.InsertString( i, SCHEDULE_MONTH[i] );
}


void MScheduleMaker::InitScheduleDayComboBox()
{
	const int nMaxDay = static_cast< int >( GetMaxDay() ) + 2;
	for( int i = 0; i < nMaxDay; ++i )
		m_cbScheduleDay.InsertString( i, SCHEDULE_DAY[i] );
}


void MScheduleMaker::InitScheduleHourComboBox()
{
	for( int i = 0; i < SCHEDULE_HOUR_COUNT; ++i )
		m_cbScheduleHour.InsertString( i, SCHEDULE_HOUR[i] );
}


void MScheduleMaker::InitScheduleMinComboBox()
{
	for( int i = 0; i < SCHEDULE_MIN_COUNT; ++i )
		m_cbScheduleMin.InsertString( i, SCHEDULE_MIN[i] );
}


void MScheduleMaker::InitScheduleCmdComboBox()
{
	for( int i = 0; i < SCHEDULE_CMD_COUNT; ++i )
		m_cbScheduleCommand.InsertString( i, SCHEDULE_CMD[i] );
}


bool MScheduleMaker::Init()
{
	InitScheduleCmdComboBox();

	SetDefaultDataAllMemberWnd();
	return true;
}


const CScheduleInfo* MScheduleMaker::GetScheduleInfo()
{
	if( CheckInputData() )
	{
		CString strCount;
		CString strAnnounce;

		m_edtCount.GetWindowText( strCount );
		m_edtAnnounce.GetWindowText( strAnnounce );

		const int nType	 = GetScheduleType();
		const int nYear	 = GetYear(); 
		const int nMonth = GetMonth();
		const int nDay	 = GetDay();
		const int nHour	 = GetHour();
		const int nMin	 = GetMin();
		const int nCount = atoi( strCount.GetBuffer() );
		const int nCmd	 = GetCommandType();

		return new CScheduleInfo( nType, nYear, nMonth,
			nDay, nHour, nMin, nCount, 
			nCmd, strAnnounce.GetBuffer() );
	}

	return 0;
}


bool MScheduleMaker::CheckInputData()
{
	// 선행 선택 조건 검사.
	if( (0 == m_cbScheduleType.GetCurSel()) || (0 == m_cbScheduleCommand.GetCurSel())  )
	{
		AfxMessageBox( "커맨드 선택이나 스케줄 종류선택을 확인해 주세요." );
		return false;
	}

	// 날짜 검색.
	if( (0 == m_cbScheduleYear.GetCurSel()) || (0 == m_cbScheduleMonth.GetCurSel()) ||
		(0 == m_cbScheduleDay.GetCurSel()) || (0 == m_cbScheduleHour.GetCurSel()) ||
		(0 == m_cbScheduleMin.GetCurSel()) )
	{
		AfxMessageBox( "선택하지 않은 시간이 있는지 확인해 주세요." );
		return false;
	}

	// 커맨드에 관련된 부분 검사.
	if( KMSC_ANNOUNCE == GetCommandType() )
	{
		if( !IsAnnounceValid() )
			return false;
	
		if( KMST_REPEAT == GetScheduleType() )
		{
			if( !CheckDateForCountAndRepeatSchedule() )
				return false;
		}
		else if( KMST_COUNT == GetScheduleType() )
		{
			if( !CheckDateForCountAndRepeatSchedule() )
				return false;

			if( !IsCountRangeValid() )
				return false;
		}
		else if( KMST_ONCE == GetScheduleType() )
		{
			if( !CheckDateForOnceSchedule() )
				return false;
		}
	}
	else if( KMSC_STOP_SERVER )
	{
		if( !CheckDateForOnceSchedule() )
			return false;
	}
	else if( KMSC_RESTART_SERVER )
	{
		if( !CheckDateForOnceSchedule() )
			return false;
	}
	else
	{
		AfxMessageBox( "커맨드 선택을 확인해 주세요." );
		return false;
	}

	return true;
}


bool MScheduleMaker::CheckDateForCountAndRepeatSchedule()
{
	// 반복작업은 적어도 하나의 날자가 0보다 커야 함.
	if( (1 == m_cbScheduleYear.GetCurSel()) && (1 == m_cbScheduleMonth.GetCurSel()) &&
		(1 == m_cbScheduleDay.GetCurSel()) && (1 == m_cbScheduleHour.GetCurSel()) &&
		(1 == m_cbScheduleMin.GetCurSel()) )
	{
		AfxMessageBox( "하나라도 0보다 큰 시간이 있어야 합니다." );
		return false;
	}

	return true;
}


const SYSTEMTIME MScheduleMaker::GetCurTime()
{
	SYSTEMTIME stTime;
	GetSystemTime( &stTime );

	stTime.wHour += 9;
	if( 24 <= stTime.wHour )
	{
		stTime.wHour -= 24;
		++stTime.wDay;
	}
	if( GetMaxDay() <= stTime.wDay )
	{
		stTime.wDay -= GetMaxDay();
		++stTime.wMonth;
	}
	if( 12 < stTime.wMonth )
	{
		stTime.wMonth -= 12;
		++stTime.wYear;
	}

	return stTime;
}


bool MScheduleMaker::CheckDateForOnceSchedule()
{
	// 선택한 시간중에 0은 사용할수 없음.
	if( (2 > m_cbScheduleMonth.GetCurSel()) &&	(2 > m_cbScheduleDay.GetCurSel()) && 
		(2 > m_cbScheduleHour.GetCurSel()) && (2 > m_cbScheduleMin.GetCurSel()) )
	{
		AfxMessageBox( "0은 해당 스켖줄에서 사용할수 없습니다." );
		return false;
	}

	if( CheckTimeIsPastForOnceSchedule() )
		return false;

	if( !CheckTimeIsSameForOnceSchedule() )
		return false;

	return true;
}


bool MScheduleMaker::CheckTimeIsPastForOnceSchedule()
{
	const SYSTEMTIME stTime = GetCurTime();

	// 현제 시간보다 작은지 검사.
	bool bPastTime = false;
	bool bContinue = false;

	if( GetYear() < (stTime.wYear % 100) )
		bPastTime = true;
	else
		bContinue = (GetYear() == (stTime.wYear % 100));

	if( bContinue && (GetMonth() < stTime.wMonth) )
	{
		bPastTime = true;
		bContinue = false;
	}
	else
		bContinue = (GetMonth() == stTime.wMonth);

	if( bContinue && (GetDay() < stTime.wDay) )
	{
		bPastTime = true;
		bContinue = false;
	}
	else
		bContinue = (GetDay() == stTime.wDay);

	if( bContinue && (GetHour() < stTime.wHour) )
	{
		bPastTime = true;
		bContinue = false;
	}
	else
		bContinue = (GetHour() == stTime.wHour);

	if( bContinue && (GetMin() < stTime.wMinute) )
		bPastTime = true;

	if( bPastTime )
	{
		AfxMessageBox( "이미 지난 시간은 선택할수 없습니다." );
		return true;
	}

	return false;
}


bool MScheduleMaker::CheckTimeIsSameForOnceSchedule()
{
	const SYSTEMTIME stTime = GetCurTime();

	// 만약 현제 시간과 같으면 스케줄 등록이 실패할수 있다고 알려줌.
	if( (GetYear() == (stTime.wYear % 100)) && (GetMonth() == stTime.wMonth) && (GetDay() == stTime.wDay) &&
		(GetHour() == stTime.wHour) && (GetMin() == stTime.wMinute) )
	{
		if( IDYES == AfxMessageBox("현제 시간과 같은 스케줄은 등록이 되어도 실행되지 않을 수 있습니다.\n진행하시겠습니까?", MB_YESNO, 0) )
			return true;
	}

	return true;
}


bool MScheduleMaker::IsCountRangeValid()
{
	// 입력된 수가 정상적인 범위에 있는지 검사.
	if( (0 >= m_edtCount.GetWindowTextLength()) || (127 < m_edtCount.GetWindowTextLength()) )
	{
		AfxMessageBox( "입력한 수를 확인해 주세요. 너무 클수 있습니다." );
		return false;
	}

	char szVal[ 128 ] = {0,};
	m_edtCount.GetWindowText( szVal, 127 );

	if( 0 >= atoi( szVal ) )
	{
		AfxMessageBox( "0보다 작은수는 사용할수 없습니다." );
		return false;
	}

	return true;
}


bool MScheduleMaker::IsAnnounceValid()
{
	// 공지사항 길이 검사.
	if( (0 >= m_edtAnnounce.GetWindowTextLength()) || (127 < m_edtAnnounce.GetWindowTextLength()) )
	{
		AfxMessageBox( "공지사항 내용을 확인해 주세요. 내용이 너무 길수도 있습니다." );
		return false;
	}

	// 공지사항 입력값 검사.
	char szVal[ 128 ] = {0,};
	m_edtAnnounce.GetWindowText( szVal, 127 );

	if( 0 == strcmp(szVal, "공지사항입력") )
	{
		AfxMessageBox( "공지사항을 입력해 주세요." );
		return false;
	}

	return true;
}


void MScheduleMaker::DisableAllMemberWnd()
{
	m_cbScheduleType.EnableWindow( false );
	m_cbScheduleYear.EnableWindow( false );
	m_cbScheduleMonth.EnableWindow( false );
	m_cbScheduleDay.EnableWindow( false );
	m_cbScheduleHour.EnableWindow( false );
	m_cbScheduleMin.EnableWindow( false );
	m_cbScheduleCommand.EnableWindow( false );
	m_edtCount.EnableWindow( false );
	m_edtAnnounce.EnableWindow( false );

	m_edtCount.SetWindowText( "" );
	m_edtAnnounce.SetWindowText( "" );
}


void MScheduleMaker::DesableDateWnd()
{
	m_cbScheduleYear.EnableWindow( false );
	m_cbScheduleMonth.EnableWindow( false );
	m_cbScheduleDay.EnableWindow( false );
	m_cbScheduleHour.EnableWindow( false );
	m_cbScheduleMin.EnableWindow( false );
	m_edtCount.EnableWindow( false );

	m_edtCount.SetWindowText( "" );
}


void MScheduleMaker::SetDefaultDataAllMemberWnd()
{
	m_cbScheduleType.SetCurSel( 0 );
	m_cbScheduleYear.SetCurSel( 0 );
	m_cbScheduleMonth.SetCurSel( 0 );
	m_cbScheduleDay.SetCurSel( 0 );
	m_cbScheduleHour.SetCurSel( 0 );
	m_cbScheduleMin.SetCurSel( 0 );
	m_cbScheduleCommand.SetCurSel( 1 );
	m_edtCount.SetWindowText( "COUNT" );
	m_edtAnnounce.SetWindowText( "공지사항입력" );
}


void MScheduleMaker::MakeAnnounceScheduleType()
{
	m_cbScheduleType.ResetContent();

	// repeat, count, once.
	for( int i = 0; i < SCHEDULE_TYPE_MAX_COUNT; ++i )
		m_cbScheduleType.InsertString( i, SCHEDULE_TYPE[i] );

	m_nScheduleListCount = SCHEDULE_TYPE_MAX_COUNT;

	m_cbScheduleType.EnableWindow( true );
	m_edtAnnounce.EnableWindow( true );
	m_cbScheduleType.SetCurSel( 0 );
}


void MScheduleMaker::MakeStopServerScheduleType()
{
	m_cbScheduleType.ResetContent();

	// once.
	m_cbScheduleType.InsertString( 0, SCHEDULE_TYPE[0] );
	m_cbScheduleType.InsertString( 1, SCHEDULE_TYPE[3] );

	m_nScheduleListCount = 2;

	m_cbScheduleType.EnableWindow( true );
	m_edtAnnounce.EnableWindow( true );
	m_cbScheduleType.SetCurSel( 0 );
}


void MScheduleMaker::MakeRestartServerScheduleType()
{
	m_cbScheduleType.ResetContent();

	// once.
	m_cbScheduleType.InsertString( 0, SCHEDULE_TYPE[0] );
	m_cbScheduleType.InsertString( 1, SCHEDULE_TYPE[3] );

	m_nScheduleListCount = 2;

	m_cbScheduleType.EnableWindow( true );
	m_cbScheduleType.SetCurSel( 0 );
}


void MScheduleMaker::ResetScheduleEnv()
{
	m_cbScheduleYear.ResetContent();
	m_cbScheduleMonth.ResetContent();
	m_cbScheduleDay.ResetContent();
	m_cbScheduleHour.ResetContent();
	m_cbScheduleMin.ResetContent();

	m_cbScheduleYear.EnableWindow( true );
	m_cbScheduleMonth.EnableWindow( true );
	m_cbScheduleDay.EnableWindow( true );
	m_cbScheduleHour.EnableWindow( true );
	m_cbScheduleMin.EnableWindow( true );
	
	m_edtCount.EnableWindow( false );
}


void MScheduleMaker::DefaultScheduleEnv()
{
	
}


void MScheduleMaker::SetDefaultScheduleEnv()
{
	m_cbScheduleYear.SetCurSel( 0 );
	m_cbScheduleMonth.SetCurSel( 0 );
	m_cbScheduleDay.SetCurSel( 0 );
	m_cbScheduleHour.SetCurSel( 0 );
	m_cbScheduleMin.SetCurSel( 0 );
}


void MScheduleMaker::MakeRepeatScheduleEnv()
{
	ResetScheduleEnv();

	InitScheduleRelativeYearComboBox();
	InitScheduleMonthComboBox();
	InitScheduleDayComboBox();
	InitScheduleHourComboBox();
	InitScheduleMinComboBox();

	SetDefaultScheduleEnv();
}


void MScheduleMaker::MakeCountScheduleEnv()
{
	ResetScheduleEnv();

	InitScheduleRelativeYearComboBox();
	InitScheduleMonthComboBox();
	InitScheduleDayComboBox();
	InitScheduleHourComboBox();
	InitScheduleMinComboBox();

	m_edtCount.EnableWindow( true );

	SetDefaultScheduleEnv();
}


void MScheduleMaker::MakeOnceScheduleEnv()
{
	ResetScheduleEnv();
	
	InitScheduleYearComboBox();
	InitScheduleMonthComboBox();
	InitScheduleDayComboBox();
	InitScheduleHourComboBox();
	InitScheduleMinComboBox();

	SetDefaultScheduleEnv();
}


void MScheduleMaker::Reset()
{
	void DisableAllMemberWnd();

	m_cbScheduleCommand.EnableWindow( true );
	m_cbScheduleCommand.SetCurSel( 0 );
}


const KMS_COMMAND_TYPE MScheduleMaker::GetCommandType() const
{
	return static_cast<KMS_COMMAND_TYPE>(m_cbScheduleCommand.GetCurSel());
}


const KMS_SCHEDULE_TYPE MScheduleMaker::GetScheduleType() const
{
	if( 0 == m_cbScheduleType.GetCurSel() )
		return KMST_NO;

	if( KMSC_ANNOUNCE == GetCommandType() )
	{
		return static_cast<KMS_SCHEDULE_TYPE>(m_cbScheduleType.GetCurSel());
	}
	else if( KMSC_STOP_SERVER == GetCommandType() )
	{
		return KMST_ONCE;
	}
	else if( KMSC_RESTART_SERVER == GetCommandType() )
	{
		return KMST_ONCE;
	}
	else
	{
		ASSERT( 0 && "잘못된 커맨드 타입" );
	}

	return KMST_NO;
}


const int MScheduleMaker::GetMonth() const
{
	int a = m_cbScheduleMonth.GetCurSel();

	if( 0 < m_cbScheduleMonth.GetCurSel() )
		return atoi( SCHEDULE_MONTH[m_cbScheduleMonth.GetCurSel()] );

	return -1;
}


const int MScheduleMaker::GetDay() const
{
	int a = m_cbScheduleDay.GetCurSel();

	if( 0 < m_cbScheduleDay.GetCurSel() )
		return atoi( SCHEDULE_DAY[m_cbScheduleDay.GetCurSel()] );

	return -1;
}


const int MScheduleMaker::GetHour() const
{
	int a = m_cbScheduleHour.GetCurSel();

	if( 0 < m_cbScheduleHour.GetCurSel() )
		return atoi( SCHEDULE_HOUR[m_cbScheduleHour.GetCurSel()] );

	return -1;
}


const int MScheduleMaker::GetMin() const
{
	int a = m_cbScheduleMin.GetCurSel();

	if( 0 < m_cbScheduleMin.GetCurSel() )
		return atoi( SCHEDULE_MIN[m_cbScheduleMin.GetCurSel()] );

	return -1;
}


const int MScheduleMaker::GetYear() const
{
	if( 0 == m_cbScheduleYear.GetCurSel() )
		return KMST_NO;

	if( KMST_ONCE == GetScheduleType() )
	{
		return atoi( SCHEDULE_YEAR[m_cbScheduleYear.GetCurSel() + 6] ) % 100;
	}
	else
	{
		int a = m_cbScheduleYear.GetCurSel();
		return atoi( SCHEDULE_YEAR[m_cbScheduleYear.GetCurSel()] ) % 100;
	}

	return -1;
}


BEGIN_MESSAGE_MAP(MScheduleMaker, CDialog)
	ON_CBN_SELCHANGE(IDC_SCHEDULE_TYPE, OnCbnSelchangeScheduleType)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_SCHEDULE_MONTH, OnCbnSelchangeScheduleMonth)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
ON_EN_SETFOCUS(IDC_SCHEDULE_COUNT, OnEnSetfocusScheduleCount)
ON_EN_SETFOCUS(IDC_ANNOUNCE, OnEnSetfocusAnnounce)
ON_CBN_SELCHANGE(IDC_SCHEDULE_COMMAND, OnCbnSelchangeScheduleCommand)
ON_EN_CHANGE(IDC_ANNOUNCE, OnEnChangeAnnounce)
END_MESSAGE_MAP()


// MScheduleMaker 메시지 처리기입니다.

void MScheduleMaker::OnCbnSelchangeScheduleType()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	DesableDateWnd();

	// switch( m_cbScheduleType.GetCurSel() )
	switch( GetScheduleType() )
	{
	case 0 :
		{
		}
		break;

	case KMST_REPEAT :
		{
			MakeRepeatScheduleEnv();
		}
		break;

	case KMST_COUNT :
		{
			MakeCountScheduleEnv();
		}
		break;

	case KMST_ONCE :
		{
			MakeOnceScheduleEnv();
		}
		break;

	default :
		{
			ASSERT( 0 );
		}
		break;
	}
}


void MScheduleMaker::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	ShowWindow( SW_SHOW );

	const CScheduleInfo* pScheduleInfo = GetScheduleInfo();
	if( 0 != pScheduleInfo )
	{
		GetKeeperMgr.RequestKeeperManagerSchedule( pScheduleInfo );
		delete pScheduleInfo;
	}
	else
		AfxMessageBox( "공지 생성 실패." );

	// OnOK();
}


void MScheduleMaker::OnCbnSelchangeScheduleMonth()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void MScheduleMaker::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	ShowWindow( SW_HIDE );
	// OnCancel();
}


BOOL MScheduleMaker::Create(UINT nIDTemplate, CWnd* pParentWnd)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	if( CDialog::Create(nIDTemplate, pParentWnd) )
	{
		DisableAllMemberWnd();

		return TRUE;
	}

	return FALSE;
}


void MScheduleMaker::OnEnSetfocusScheduleCount()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	m_edtCount.SetWindowText( "" );
}	
void MScheduleMaker::OnEnSetfocusAnnounce()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_edtAnnounce.SetWindowText( "" );
}

void MScheduleMaker::OnCbnSelchangeScheduleCommand()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	enum COMM_TYPE
	{
		CMT_ANNOUNCE = 1,
		CMT_STOP_SERVER,
		CMT_RESTART_SERVER,
	};

	DisableAllMemberWnd();

	m_cbScheduleCommand.EnableWindow( true );

	switch( m_cbScheduleCommand.GetCurSel() )
	{
	case CMT_ANNOUNCE :
		{
			MakeAnnounceScheduleType();
		}
		break;

	case CMT_STOP_SERVER :
		{
			MakeStopServerScheduleType();
		}
		break;

	case CMT_RESTART_SERVER :
		{
			MakeRestartServerScheduleType();
		}
		break;

	default :
		{
			ASSERT( 0 && "잘못된 커맨드." );
		}
		break;
	}
}

void MScheduleMaker::OnEnChangeAnnounce()
{
	// TODO:  RICHEDIT 컨트롤인 경우 이 컨트롤은
	// CDialog::마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여
	// CRichEditCtrl().SetEventMask()를 호출하도록 OnInitDialog() 함수를 재지정하지 않으면
	// 이 알림을 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
