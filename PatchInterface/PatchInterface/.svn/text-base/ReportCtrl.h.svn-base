#pragma once


// CReportCtrl

#define NUM_COLUMNS 12

static const char*	COLUMN_NAME[ NUM_COLUMNS ]	= { "서버이름", 
													"IP",
													"S-상태",
													"A-상태",
													"연결상태", 
													"서버오류검사",
													"마지막 작업 상태", 
													"S-자동실행", 
													"A-자동실행",
													"패치받기", 
													"패치준비", 
													"패치적용" };

static const int	COLUMN_LENTH[ NUM_COLUMNS ] = { 100, 100, 50, 50, 175, 100, 150, 75, 75, 60, 60, 60 };

static const int	REPORT_WND_STYLE = LVS_REPORT | WS_BORDER | LVS_SHOWSELALWAYS;

class CReportCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CReportCtrl)
	
public:
	CReportCtrl();
	virtual ~CReportCtrl();

	static CReportCtrl& GetInst()
	{
		static CReportCtrl Report;
		return Report;
	}

	void InsertColumns();
	void InsertItems();
	void Update( const int nID );

	BOOL Create( CWnd* pParentWnd, UINT nID );

	void SetGridLines( bool bSet );
	void SetCheckboxStyle( int nStyle = 1 );
	void SetFullrowSelectStyle( bool bSet );

private :
	int FindSelectedColumn( const CPoint Point );
	int FindSelectedItem();
	void ConvertSelectedColumnConfigState( const int nItemID, const int nColumnID );

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};

#define GetReport CReportCtrl::GetInst()