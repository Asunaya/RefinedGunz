#pragma once

typedef void(ZCOUNTDOWNEVENTCALLBACK)(void);

// 카운트다운 하다가 0이 되면 콜백을 실행한다
struct ZCOUNTDOWN {
	int nSeconds;				// 남은시간
	const char *szLabelWidget;	// 숫자를 갱신해줄 위젯
	const char *szTargetWidget;	// 시간이 다되면 이 위젯을 숨긴다. 또 이 위젯이 숨겨지면 카운트를 중지한다.
	ZCOUNTDOWNEVENTCALLBACK *pCallBack;	// 0이 되었을때 실행할 펑션
};

void OnTimer_CountDown(void* pParam);
