#pragma once

#include "afxwin.h"
#include "MMatchGlobal.h"

#include <string>
using std::string;

// MScheduleMaker 대화 상자입니다.


/* MMatchGlobal.h로 이동.
enum KMS_SCHEDULE_TYPE
{
	KMST_REPEAT = 1,
	KMST_COUNT,
	KMST_ONCE,

	KMS_SCHEDULE_TYPE_END,
};

enum KMS_COMMAND_TYPE
{
	KMSC_ANNOUNCE = 1,
	KMSC_STOP_SERVER,

	KMS_COMMAND_TYPE_END,
};
*/

static const int SCHEDULE_TYPE_MAX_COUNT	= 4;
static const int SCHEDULE_YEAR_COUNT		= 13;
static const int SCHEDULE_MONTH_COUNT		= 14;
static const int SCHEDULE_DAY_COUNT			= 33;
static const int SCHEDULE_HOUR_COUNT		= 26;
static const int SCHEDULE_MIN_COUNT			= 31;
static const int SCHEDULE_CMD_COUNT			= 4;

static const char*	SCHEDULE_TYPE[]		= { "TYPE", "REPEAT", "COUNT", "ONCE" };
static const char*	SCHEDULE_YEAR[]		= { "YEAR", "0", "1", "2", "3", "4", "5", "2005", "2006", "2007", "2008", "2009", "2010" };
static const char*	SCHEDULE_MONTH[]	= { "MONTH", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12" };
static const char*	SCHEDULE_DAY[]		= { "DAY", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16",
										    "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31" };
static const char*	SCHEDULE_HOUR[]		= { "HOUR", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24" };
static const char*	SCHEDULE_MIN[]		= { "MIN", "0", "2", "4", "6", "8", "10", "12", "14", "16", "18", "20", 
											"22", "24", "26", "28", "30", "32", "34", "36", "38", "40",
											"42", "44", "46", "48", "50", "52", "54", "56", "58" };
static const char*	SCHEDULE_CMD[]		= { "COMMAND", "공지", "서버종료", "서버재시작" };


class CScheduleInfo
{
public :
	CScheduleInfo( const int nType, const int nYear, const int nMonth, 
		const int nDay, const int nHour, const int nMin, const int nCount, 
		const int nCmd, const string& strAnnounce ) :
	m_nType( nType ), m_nYear( nYear ), m_nMonth( nMonth ), m_nDay( nDay ), m_nHour( nHour ),
		m_nMin( nMin ), m_nCount( nCount ), m_nCmd( nCmd ), m_strAnnounce( strAnnounce ) 
	{
	}

	int		m_nType;
	int		m_nYear;
	int		m_nMonth;
	int		m_nDay;
	int		m_nHour;
	int		m_nMin;
	int		m_nCount;
	int		m_nCmd;
	string	m_strAnnounce;

private :
	CScheduleInfo() {}
};


class MScheduleMaker : public CDialog
{
	DECLARE_DYNAMIC(MScheduleMaker)

public:
	MScheduleMaker(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~MScheduleMaker();

	bool Init();
	const CScheduleInfo* GetScheduleInfo();

	void Reset();

private :
	const KMS_COMMAND_TYPE GetCommandType() const;
	const KMS_SCHEDULE_TYPE GetScheduleType() const;

	const int GetYear() const;
	const int GetMonth() const;
	const int GetDay() const;
	const int GetHour() const;
	const int GetMin() const;

	const SYSTEMTIME GetCurTime();

	void InitScheduleTypeComboBox();

	void InitScheduleRelativeYearComboBox();
	void InitScheduleYearComboBox();
	void InitScheduleMonthComboBox();
	void InitScheduleDayComboBox();
	void InitScheduleHourComboBox();
	void InitScheduleMinComboBox();
	void InitScheduleCmdComboBox();

	void DisableAllMemberWnd();
	void DesableDateWnd();
	void SetDefaultDataAllMemberWnd();

	bool CheckInputData();
	bool CheckDateForCountAndRepeatSchedule();
	bool CheckDateForOnceSchedule();
	bool CheckTimeIsPastForOnceSchedule();
	bool CheckTimeIsSameForOnceSchedule();
	bool IsCountRangeValid();
	bool IsAnnounceValid();
	
	void MakeAnnounceScheduleType();
	void MakeStopServerScheduleType();
	void MakeRestartServerScheduleType();
	void DefaultScheduleEnv();

	void ResetScheduleEnv();
	void SetDefaultScheduleEnv();

	void MakeRepeatScheduleEnv();
	void MakeCountScheduleEnv();
	void MakeOnceScheduleEnv();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_SCHEDULE_MAKER_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

private:
	CComboBox	m_cbScheduleType;
	CComboBox	m_cbScheduleYear;
	CComboBox	m_cbScheduleMonth;
	CComboBox	m_cbScheduleDay;
	CComboBox	m_cbScheduleHour;
	CComboBox	m_cbScheduleMin;
	CComboBox	m_cbScheduleCommand;
	CEdit		m_edtCount;
	CEdit		m_edtAnnounce;

	int			m_nScheduleListCount;

public:
	afx_msg void OnCbnSelchangeScheduleType();
	afx_msg void OnBnClickedOk();
	afx_msg void OnCbnSelchangeScheduleMonth();
	afx_msg void OnBnClickedCancel();
	virtual BOOL Create(UINT nIDTemplate, CWnd* pParentWnd = NULL);
	afx_msg void OnEnSetfocusScheduleCount();
	afx_msg void OnEnSetfocusAnnounce();
	afx_msg void OnCbnSelchangeScheduleCommand();
	afx_msg void OnEnChangeAnnounce();
};
