#pragma once
#include <vector>
#include <string>
#include <memory>

#define CHAT_DEFAULT_TEXT_COLOR 0xFFC8C8C8
#define CHAT_DEFAULT_INTERFACE_COLOR 0xFF00A5C3

// Note that while FT_WRAP and FT_LINEBREAK are both linebreaks, the former is placed by the line-wrapping mechanism and the latter is explicitly placed by the message creator.
enum FormatSpecifierType{
	FT_WRAP,
	FT_LINEBREAK,
	FT_COLOR,
	FT_DEFAULT,
	FT_BOLD,
	FT_ITALIC,
	FT_BOLDITALIC,
	FT_UNDERLINE,
	FT_STRIKETHROUGH,
};

struct FormatSpecifier{
	int nStartPos;
	FormatSpecifierType ft;
	D3DCOLOR Color;

	FormatSpecifier(int nStart, D3DCOLOR c) : nStartPos(nStart), ft(FT_COLOR), Color(c) { }
	FormatSpecifier(int nStart, FormatSpecifierType type) : nStartPos(nStart), ft(type) { }
};

struct ChatLine{
	unsigned long long Time;
	std::wstring Msg;
	D3DCOLOR DefaultColor;
	std::vector<FormatSpecifier> vFormatSpecifiers;

	ChatLine(unsigned long long Time, std::wstring Msg, D3DCOLOR DefaultColor)
		: Time{ Time }, Msg{ std::move(Msg) }, DefaultColor{ DefaultColor } {
		SubstituteFormatSpecifiers();
	}

	void SubstituteFormatSpecifiers();

	int GetLines() const {
		int i = 1;
		for (auto it = vFormatSpecifiers.begin(); it != vFormatSpecifiers.end(); it++)
		if (it->ft == FT_WRAP || it->ft == FT_LINEBREAK)
			i++;

		return i;
	}

	void ClearLineBreaks(){
		for (auto it = vFormatSpecifiers.begin(); it != vFormatSpecifiers.end();)
		if (it->ft == FT_WRAP)
			it = vFormatSpecifiers.erase(it);
		else
			it++;
	}

	int GetNumLineBreaks() const {
		int i = 0;
		for (auto it = vFormatSpecifiers.begin(); it != vFormatSpecifiers.end(); it++)
			if (it->ft == FT_WRAP)
				i++;

		return i;
	}

	const FormatSpecifier *GetLineBreak(int n) const {
		int i = 0;
		for (auto it = vFormatSpecifiers.begin(); it != vFormatSpecifiers.end(); it++){
			if (it->ft == FT_WRAP || it->ft == FT_LINEBREAK){
				if (i == n)
					return &*it;

				i++;
			}
		}

		return 0;
	}

	void AddLineBreak(int n){
		if (vFormatSpecifiers.begin() == vFormatSpecifiers.end()){
			vFormatSpecifiers.push_back(FormatSpecifier(n, FT_WRAP));
			return;
		}

		for (auto it = vFormatSpecifiers.crbegin(); it != vFormatSpecifiers.crend(); it++)
		if (it->nStartPos < n){
			vFormatSpecifiers.insert(it.base(), FormatSpecifier(n, FT_WRAP));
			return;
		}

		if (vFormatSpecifiers.cbegin()->nStartPos < n)
			vFormatSpecifiers.insert(vFormatSpecifiers.cbegin() + 1, FormatSpecifier(n, FT_WRAP));
		else
			vFormatSpecifiers.insert(vFormatSpecifiers.cbegin(), FormatSpecifier(n, FT_WRAP));
	}
};

enum ChatWindowAction{
	CWA_NONE,
	CWA_MOVING,
	CWA_RESIZING,
	CWA_SELECTING,
};

struct v2i
{
	int x, y;
};

class Chat{
public:
	void Create(const std::string &strFont, int nFontSize);
	void Destroy();

	void EnableInput(bool bEnable, bool bToTeam);
	void OutputChatMsg(const char *szMsg);
	void OutputChatMsg(const char *szMsg, DWORD dwColor);
	void Display();
	void Scale(double fWidthRatio, double fHeightRatio);
	void Resize(int Width, int Height);
	void OnEvent(MEvent *pEvent);
	void OnUpdate();

	void ClearHistory() { vMsgs.clear(); }

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
	int nFontSize;
	int nFontHeight;
	double fFadeTime;
	std::vector<ChatLine> vMsgs;
	bool bInputEnabled;
	POINT Cursor;
	D3DRECT Border;
	std::unique_ptr<MFontR2> pFont;
	std::unique_ptr<MFontR2> pItalicFont;
	// The normal font is already bold right now.
	//MFontR2 *pBoldFont;
	//MFontR2 *pBoldItalicFont;
	D3DCOLOR TextColor;
	D3DCOLOR InterfaceColor;
	D3DCOLOR BackgroundColor;
	ChatWindowAction Action;
	DWORD dwResize;
	const ChatLine *pFromMsg;
	int nFromPos;
	const ChatLine *pToMsg;
	int nToPos;
	std::vector<std::wstring> vstrInputHistory;
	int nCurInputHistoryEntry;
	std::wstring strField;
	int nCaretPos;
	/*bool bPlayerList;
	ID3DXLine *pPlayerListLine;
	std::vector<std::string> vstrPlayerList;
	int nCurPlayer;
	int nPlayerListWidth;*/
	int InputHeight{};
	v2i CaretCoord{};

	struct ScreenSpaceVertex {
		float x, y, z, rhw;
		DWORD color;
	};
	struct LineListIndex
	{
		short v1;
		short v2;
	};
	struct TriangleListIndex
	{
		short v1;
		short v2;
		short v3;
	};

	void DrawBorder();

	void Line(float x1, float y1, float x2, float y2, D3DCOLOR Color = CHAT_DEFAULT_INTERFACE_COLOR, float z = 0) {
		Line(v2(x1, y1), v2(x2, y2), Color, z);
	}
	void Line(const v2 &v1, const v2 &v2, D3DCOLOR Color = CHAT_DEFAULT_INTERFACE_COLOR, float z = 0);
	void Quad(float x1, float y1, float x2, float y2, D3DCOLOR Color = CHAT_DEFAULT_INTERFACE_COLOR) {
		Quad(v2(x1, y1), v2(x2, y2), Color);
	}
	void Quad(const D3DRECT &r, D3DCOLOR Color = CHAT_DEFAULT_INTERFACE_COLOR, float z = 0) {
		Quad(v2(r.x1, r.y1), v2(r.x2, r.y2), Color, z);
	}
	void Quad(const v2 &v1, const v2 &v2, D3DCOLOR Color = CHAT_DEFAULT_INTERFACE_COLOR, float z = 0);

	std::vector<ScreenSpaceVertex> Lines;
	std::vector<ScreenSpaceVertex> Triangles;
	bool bAlpha;
	bool bBegunDrawing;

	void BeginDraw();
	void EndDraw();

	D3DRECT GetOutputRect();
	D3DRECT GetInputRect();
	D3DRECT GetTotalRect();
	void CalcLineBreaks(ChatLine &cl);
	bool GetPos(const ChatLine &cl, unsigned long nPos, POINT *pRet);
	bool CursorInRange(int x, int y, int nWidth, int nHeight);
	int GetTextLen(ChatLine &cl, int nPos, int nCount);
	int GetTextLen(const char *szMsg, int nCount);

	int GetLines(MFontR2 *pFont, const wchar_t *szMsg, int nWidth, int nSize = -1);
	int GetTextLength(MFontR2 *pFont, const wchar_t *szFormat, ...);
	int DrawTextWordWrap(MFontR2 *pFont, const wchar_t *szStr, const RECT &r, DWORD dwColor);
	void DrawTextN(MFontR2 *pFont, const wchar_t *szStr, const RECT &r, DWORD dwColor, int nLen = -1);

	void ResetFonts();
};

extern Chat g_Chat;
extern bool g_bNewChat;