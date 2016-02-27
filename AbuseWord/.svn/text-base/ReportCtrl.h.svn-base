#pragma once

#include <string>
using std::string;


// CReportCtrl

class CReportCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CReportCtrl)

public:
	CReportCtrl();
	virtual ~CReportCtrl();

	const int FindPosition( const string& strAbuseWord );

	void ScrollUp( const int nPos = -10 );
	void ScrollDown( const int nPos = 10 );

	const int FindSelectItem();
	const string GetSelectedItemText();

private :
	const int FindChageCheckState();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};


