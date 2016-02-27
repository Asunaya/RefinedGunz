#ifndef _ZCOMBAT_MENU_H
#define _ZCOMBAT_MENU_H

#include <string>
using namespace std;

// 게임중에 나오는 메뉴
class ZCombatMenu
{
public:
	enum ZCOMBAT_MENU_ITEM
	{
		ZCMI_OPTION = 0,		// 옵션
		ZCMI_CLOSE,				// 게임으로 돌아가기
		ZCMI_BATTLE_EXIT,		// 대기방으로 나가기
		ZCMI_STAGE_EXIT,		// 게임에서 나가기
		ZCMI_PROG_EXIT,			// 프로그램 종료

		ZCMI_END
	};

private:
	string		m_ItemStr[ZCMI_END];
	string		m_FrameStr;
public:
	ZCombatMenu();
	~ZCombatMenu();
	bool IsEnableItem(ZCOMBAT_MENU_ITEM nItem);					// 해당 메뉴가 활성화되었는지 여부
	void EnableItem(ZCOMBAT_MENU_ITEM nItem, bool bEnable);			// 해당 메뉴 활성화

	void ShowModal(bool bShow=true);
	bool IsVisible();
};








#endif