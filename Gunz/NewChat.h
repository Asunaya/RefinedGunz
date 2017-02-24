#pragma once
#include <vector>
#include <string>
#include <memory>
#include "MUtil.h"

constexpr u32 CHAT_DEFAULT_TEXT_COLOR = XRGB(0xC8, 0xC8, 0xC8);
constexpr u32 CHAT_DEFAULT_INTERFACE_COLOR = XRGB(0, 0xA5, 0xC3);
constexpr u32 CHAT_DEFAULT_BACKGROUND_COLOR = 0;
constexpr u32 CHAT_DEFAULT_SELECTION_COLOR = ARGB(0xA0, 0, 0x80, 0xFF);

struct v2i
{
	int x, y;
};

enum ChatWindowAction {
	CWA_NONE,
	CWA_MOVING,
	CWA_RESIZING,
	CWA_SELECTING,
};

class Chat
{
public:
	Chat(const std::string &strFont, int nFontSize);
	~Chat();

	void EnableInput(bool bEnable, bool bToTeam);
	void OutputChatMsg(const char *szMsg);
	void OutputChatMsg(const char *szMsg, u32 dwColor);

	void OnUpdate();
	void OnDraw(MDrawContext* pDC);
	bool OnEvent(MEvent* pEvent);

	void Scale(double fWidthRatio, double fHeightRatio);
	void Resize(int Width, int Height);

	void ClearHistory();

	const D3DRECT &GetRect() const { return Border; }
	void SetRect(D3DRECT &r) { Border = r; }
	void SetRect(int x1, int y1, int x2, int y2) { Border.x1 = x1; Border.y1 = y1; Border.x2 = x2; Border.y2 = y2; }

	float GetFadeTime() const { return fFadeTime; }
	void SetFadeTime(float fFade) { fFadeTime = fFade; }

	const std::string &GetFont() const { return strFont; }
	int GetFontSize() const { return nFontSize; }
	void SetFont(std::string &s) { strFont.assign(s); ResetFonts(); }
	void SetFontSize(int nSize) { nFontSize = nSize; ResetFonts(); }

	D3DCOLOR GetTextColor() const { return TextColor; }
	D3DCOLOR GetInterfaceColor() const { return InterfaceColor; }
	D3DCOLOR GetBackgroundColor() const { return BackgroundColor; }
	void SetTextColor(D3DCOLOR Color) { TextColor = Color; }
	void SetBackgroundColor(D3DCOLOR Color) { BackgroundColor = Color; }
	void SetInterfaceColor(D3DCOLOR Color) { InterfaceColor = Color; }

	bool IsInputEnabled() const { return bInputEnabled; }

private:
	std::string strFont;
	int nFontSize{};
	int nFontHeight{};
	double fFadeTime = 10;
	std::vector<struct ChatLine> vMsgs;
	std::vector<struct LineSegmentInfo> LineSegments;
	bool bInputEnabled{};
	POINT Cursor{};
	D3DRECT Border{};
	std::unique_ptr<MFontR2> pFont;
	std::unique_ptr<MFontR2> pItalicFont;
	// The normal font is already bold right now.
	//MFontR2 *pBoldFont;
	//MFontR2 *pBoldItalicFont;
	D3DCOLOR TextColor = CHAT_DEFAULT_TEXT_COLOR;
	D3DCOLOR InterfaceColor = CHAT_DEFAULT_INTERFACE_COLOR;
	D3DCOLOR BackgroundColor = CHAT_DEFAULT_BACKGROUND_COLOR;
	D3DCOLOR SelectionColor = CHAT_DEFAULT_SELECTION_COLOR;
	ChatWindowAction Action = CWA_NONE;
	DWORD dwResize{};
	const ChatLine *pFromMsg{};
	int nFromPos{};
	const ChatLine *pToMsg{};
	int nToPos{};
	std::vector<std::wstring> vstrInputHistory;
	int nCurInputHistoryEntry{};
	std::wstring strField;
	int nCaretPos = -1;
	/*bool bPlayerList;
	ID3DXLine *pPlayerListLine;
	std::vector<std::string> vstrPlayerList;
	int nCurPlayer;
	int nPlayerListWidth;*/
	int InputHeight{};
	v2i CaretCoord{};

	void DrawBorder(MDrawContext* pDC);
	void DrawBackground(MDrawContext* pDC, u64 Time, u64 TPS, int nLimit, bool bShowAll);
	void DrawChatLines(MDrawContext* pDC, u64 Time, u64 TPS, int nLimit, bool bShowAll);
	void DrawSelection(MDrawContext* pDC);
	void DrawFrame(MDrawContext* pDC, u64 Time, u64 TPS);
	MFontR2* GetFont(u32 Emphasis);

	D3DRECT GetOutputRect();
	D3DRECT GetInputRect();
	D3DRECT GetTotalRect();
	template <typename T>
	void DivideIntoLines(int ChatLineIndex, T&& OutputIterator);
	bool GetPos(const ChatLine &cl, unsigned long nPos, POINT *pRet);
	bool CursorInRange(int x, int y, int nWidth, int nHeight);
	int GetTextLen(ChatLine &cl, int nPos, int nCount);
	int GetTextLen(const char *szMsg, int nCount);

	int GetLines(MFontR2 *pFont, const wchar_t *szMsg, int nWidth, int nSize = -1);
	int GetTextLength(MFontR2 *pFont, const wchar_t *szFormat, ...);
	int DrawTextWordWrap(MFontR2 *pFont, const wchar_t *szStr, const D3DRECT &r, DWORD dwColor);
	void DrawTextN(MFontR2 *pFont, const wchar_t *szStr, const D3DRECT &r, DWORD dwColor, int nLen = -1);

	void ResetFonts();
};
