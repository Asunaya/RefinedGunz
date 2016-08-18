#include "stdafx.h"
#include "ReplayControl.h"
#include "RGMain.h"
#include "Draw.h"
#include "NewChat.h"

ReplayControl g_ReplayControl;

void ReplayControl::Draw()
{
	if (!ZGetGame()->IsReplay())
		return;

	if (!g_Chat.IsInputEnabled())
		return;

	D3DXVECTOR2 v1, v2;
	v1.x = RELWIDTH(1920.f / 2 - 220);
	v1.y = RELHEIGHT(830);
	v2.x = RELWIDTH(1920.f / 2 + 220);
	v2.y = RELHEIGHT(870);

	g_Draw.Border(v1, v2);

	v1.x = RELWIDTH(1920.f / 2 - 170);
	v1.y = RELHEIGHT(850);
	v2.x = RELWIDTH(1920.f / 2 + 170);
	v2.y = RELHEIGHT(850);

	g_Draw.Line(v1, v2);

	float ReplayTime = ZGetGame()->GetReplayTime();
	float ReplayLength = ZGetGame()->GetReplayLength();

	float Index = ReplayTime / ReplayLength;

	v1.x = RELWIDTH(1920.f / 2 - 170 + Index * 340 - 5);
	v1.y = RELHEIGHT(835);
	v2.x = RELWIDTH(1920.f / 2 - 170 + Index * 340 + 5);
	v2.y = RELHEIGHT(865);

	g_Draw.Quad(v1, v2);

	char buf[64];

	sprintf_safe(buf, "%02d:%02d", int(ReplayTime / 60), int(fmod(ReplayTime, 60)));

	extern MFontR2 *g_pDefFont;
	g_pDefFont->m_Font.DrawTextA(RELWIDTH(1920.f / 2 - 215), RELHEIGHT(835), buf);

	sprintf_safe(buf, "%02d:%02d", int(ReplayLength / 60), int(fmod(ReplayLength, 60)));

	extern MFontR2 *g_pDefFont;
	g_pDefFont->m_Font.DrawTextA(RELWIDTH(1920.f / 2 + 185), RELHEIGHT(835), buf);
}

bool CursorInRange(const POINT &Cursor, int x1, int y1, int x2, int y2){
	return Cursor.x > x1 && Cursor.x < x2 && Cursor.y > y1 && Cursor.y < y2;
}

bool ReplayControl::OnEvent(MEvent *pEvent)
{
	if (!ZGetGame()->IsReplay() || !g_Chat.IsInputEnabled())
		return false;

	static bool bLastLb = false;

	if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000 && !bLastLb))
	{
		bLastLb = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
		return false;
	}

	bLastLb = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

	POINT p;
	GetCursorPos(&p);

	if (!CursorInRange(p, RELWIDTH(1920.f / 2 - 170), RELHEIGHT(835), RELWIDTH(1920.f / 2 + 170), RELHEIGHT(865)))
		return false;

	float Index = float(p.x - RELWIDTH(1920.f / 2 - 170)) / 340;

	for (auto* Player : MakePairValueAdapter(*ZGetCharacterManager()))
	{
		Player->UpdateValidShotTime(0, 0);
	}

	ZGetGame()->SetReplayTime(Index * ZGetGame()->GetReplayLength());

	return true;
}