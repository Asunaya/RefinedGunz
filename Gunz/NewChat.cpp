#include "stdafx.h"
#include "NewChat.h"
#include "RGMain.h"
#include "ZCharacterManager.h"
#include "ZInput.h"
#include "Config.h"
#include <wchar.h>

// Resize flags
enum
{
	RESIZE_X1 = 1 << 0,
	RESIZE_Y1 = 1 << 1,
	RESIZE_X2 = 1 << 2,
	RESIZE_Y2 = 1 << 3,
};

// Note that while FT_WRAP and FT_LINEBREAK are both linebreaks, 
// the former is placed by the line-wrapping mechanism and
// the latter is explicitly placed by the message creator.
enum FormatSpecifierType {
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

struct FormatSpecifier {
	int nStartPos;
	FormatSpecifierType ft;
	D3DCOLOR Color;

	FormatSpecifier(int nStart, D3DCOLOR c) : nStartPos(nStart), ft(FT_COLOR), Color(c) { }
	FormatSpecifier(int nStart, FormatSpecifierType type) : nStartPos(nStart), ft(type) { }
};

struct ChatLine {
	u64 Time{};
	std::wstring Msg;
	D3DCOLOR DefaultColor;
	std::vector<FormatSpecifier> vFormatSpecifiers;
	int Lines{};

	void SubstituteFormatSpecifiers();

	int GetLines() const {
		return Lines;
	}

	void ClearWrappingLineBreaks() {
		erase_remove_if(vFormatSpecifiers, [&](auto&& x) { return x.ft == FT_WRAP; });
	}

	int GetNumWrappingLineBreaks() const {
		std::count_if(vFormatSpecifiers.begin(), vFormatSpecifiers.end(),
			[&](auto&& x) { return x.ft == FT_WRAP; });
	}

	const FormatSpecifier *GetLineBreak(int n) const {
		int i = 0;
		for (auto it = vFormatSpecifiers.begin(); it != vFormatSpecifiers.end(); it++) {
			if (it->ft == FT_WRAP || it->ft == FT_LINEBREAK) {
				if (i == n)
					return &*it;

				i++;
			}
		}

		return 0;
	}

	// Returns an iterator to the format specifier that was inserted.
	auto AddWrappingLineBreak(int n) {
		assert(n >= 0);
		if (n < 0)
			n = 0;

		if (vFormatSpecifiers.begin() == vFormatSpecifiers.end()) {
			vFormatSpecifiers.emplace_back(n, FT_WRAP);
			// Return the last iterator, since we appended to the end.
			return std::prev(vFormatSpecifiers.end());
		}

		for (auto it = vFormatSpecifiers.rbegin(); it != vFormatSpecifiers.rend(); it++) {
			if (it->nStartPos < n) {
				// it.base() is AFTER it (in terms of normal order, not reversed),
				// and insert inserts BEFORE the passed iterator, so this inserts
				// after the current iterator, which is correct since the
				// desired index n is after the current format specifier.
				return vFormatSpecifiers.insert(it.base(), FormatSpecifier(n, FT_WRAP));
			}
		}

		// The loop was unable to find a format specifier that precedes
		// the position, so we must add it at the start.
		return vFormatSpecifiers.insert(vFormatSpecifiers.begin(), FormatSpecifier(n, FT_WRAP));
	}
};

namespace EmphasisType
{
enum
{
	Default = 0,
	Italic = 1 << 0,
	Bold = 1 << 1,
	Underline = 1 << 2,
	Strikethrough = 1 << 3,
};
}

// A substring of a line to be displayed.
// The substring may be the entire line, but cannot span more than one line.
struct LineSegmentInfo
{
	// Index into Chat::vMsgs.
	int ChatLineIndex;
	// Offset into ChatLine::Msg at which the substring to be displayed begins.
	u16 Offset;
	// Length of the substring, in characters.
	u16 LengthInCharacters;
	// Pixel offset on the X axis at which this segment starts.
	u16 PixelOffsetX;
	struct {
		// Is this the start of the line?
		u16 IsStartOfLine : 1;
		// Emphasis, i.e. italic, bold, etc.
		// This is a bitmask; it can hold a combination of multiple emphases.
		u16 Emphasis : 15;
	};
	u32 TextColor;
};

// Stuff crashes if this is increased
static constexpr int MAX_INPUT_LENGTH = 230;

#ifdef NEW_CHAT
bool g_bNewChat = true;
#else
bool g_bNewChat = false;
#endif

static std::wstring GetClipboard()
{
	if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
		return{};

	if (!OpenClipboard(g_hWnd))
		return{};

	auto ClipboardData = GetClipboardData(CF_UNICODETEXT);
	if (!ClipboardData)
		return{};

	auto ptr = static_cast<const wchar_t*>(GlobalLock(ClipboardData));
	if (!ptr)
		return{};

	std::wstring ret = ptr;
	GlobalUnlock(ClipboardData);
	CloseClipboard();

	return ret;
}

static bool SetClipboard(const std::wstring& str)
{
	auto MemSize = (str.length() + 1) * sizeof(wchar_t);
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, MemSize);
	if (!hMem)
		return false;

	auto pMem = static_cast<wchar_t*>(GlobalLock(hMem));
	if (!pMem)
	{
		GlobalUnlock(pMem);
		GlobalFree(pMem);
		return false;
	}

	strcpy_safe(pMem, MemSize, str.c_str());
	GlobalUnlock(hMem);

	auto Success = SetClipboardData(CF_UNICODETEXT, hMem) != NULL;
	if (!Success)
		GlobalFree(hMem);

	return Success;
}

void ChatLine::SubstituteFormatSpecifiers()
{
	static const std::unordered_map<char, FormatSpecifierType> Map {
		{ 'b', FT_BOLD },
		{ 'i', FT_ITALIC },
		{ 's', FT_STRIKETHROUGH },
		{ 'u', FT_UNDERLINE },
		//{ 'n', FT_LINEBREAK }, // Linebreak specifier is disabled.
	};

	const auto npos = std::wstring::npos;

	for (auto Pos = Msg.find_first_of(L"^[", 0);
		Pos != npos && Pos < Msg.length() - 2;
		Pos = Pos < Msg.length() ? Msg.find_first_of(L"^[", Pos + 1) : npos)
	{
		auto Erase = [&](std::wstring::size_type Count) {
			Msg.erase(Pos, Count);
			--Pos;
		};

		auto RemainingLength = Msg.length() - Pos;
		auto CurrentChar = Msg[Pos];

		if (CurrentChar == '^')
		{
			// Handles color specifiers, like "Normal text ^1Red text"
			auto NextChar = Msg[Pos + 1];
			if (isdigit(NextChar))
			{
				// Simple specifier, e.g. "^1Red text"

				vFormatSpecifiers.emplace_back(Pos, MMColorSet[NextChar - '0']);
				Erase(2);
			}
			else if (NextChar == '#')
			{
				// Elaborate specifier, e.g. "^#80FF0000Transparent red text"

				auto ishexdigit = [&](auto c) {
					c = tolower(c);
					return isdigit(c) || (c >= 'a' && c <= 'f');
				};

				auto ColorStart = Pos + 2;
				auto ColorEnd = ColorStart;
				while (ColorEnd < Msg.length() &&
					ColorEnd - ColorStart < 8 &&
					ishexdigit(Msg[ColorEnd])) {
					++ColorEnd;
				}

				auto Distance = ColorEnd - ColorStart;

				// Must be 8 digits
				if (Distance != 8)
					continue;

				wchar_t ColorString[32];
				strncpy_safe(ColorString, &Msg[ColorStart], Distance);

				wchar_t* endptr;
				auto Color = static_cast<D3DCOLOR>(wcstoul(ColorString, &endptr, 16));
				assert(endptr == ColorString + Distance);

				vFormatSpecifiers.emplace_back(Pos, Color);
				Erase(ColorEnd - Pos);
			}
		}
		else if (CurrentChar == '[')
		{
			// Handles specifiers like "Normal text [b]Bold text[/b]"
			auto EndBracket = Msg.find_first_of(L"]", Pos + 1);

			if (EndBracket == npos)
				continue; // Malformed specifier

			auto Distance = EndBracket - Pos;

			if (Msg[Pos + 1] == '/' && (Distance == 2 || Distance == 3))
			{
				// End of sequence
				// Matches e.g. [/], [/b], [/i]

				// Go back to default text
				vFormatSpecifiers.emplace_back(Pos, FT_DEFAULT);
			}
			else
			{
				// Beginning of sequence
				// Matches e.g. [b], [i]
				auto it = Map.find(Msg[Pos + 1]);
				if (it == Map.end())
					continue;

				vFormatSpecifiers.emplace_back(Pos, it->second);
			}

			Erase(Distance + 1);
		}
	}
}

Chat::Chat(const std::string &strFont, int nFontSize)
	: strFont{ strFont }, nFontSize{ nFontSize }
{
	auto ScreenWidth = RGetScreenWidth();
	auto ScreenHeight = RGetScreenHeight();

	Border.x1 = 10;
	Border.y1 = double(1080 - 300) / 1080 * ScreenHeight;
	Border.x2 = (double)500 / 1920 * ScreenWidth;
	Border.y2 = double(1080 - 100) / 1080 * ScreenHeight;

	Cursor.x = ScreenWidth / 2;
	Cursor.y = ScreenHeight / 2;

	pFont = std::make_unique<MFontR2>();
	pFont->Create("NewChatFont", strFont.c_str(),
		int(float(nFontSize) / 1080 * RGetScreenHeight() + 0.5), 1, true);
	pItalicFont = std::make_unique<MFontR2>();
	pItalicFont->Create("NewChatItalicFont", strFont.c_str(),
		int(float(nFontSize) / 1080 * RGetScreenHeight() + 0.5), 1, true, true);

	nFontHeight = pFont->GetHeight();
}

Chat::~Chat() = default;

void Chat::EnableInput(bool bEnable, bool bToTeam){
	bInputEnabled = bEnable;

	if (bEnable){
		strField.clear();

		nCaretPos = -1;

		SetCursorPos(RGetScreenWidth() / 2, RGetScreenHeight() / 2);
	}
	else{
		cprint("RotationDelta: %f, %f\n", ZGetInput()->m_fRotationDeltaX, ZGetInput()->m_fRotationDeltaY);
		// These are too buggy, they change on different systems and compilations.
		//ZGetInput()->m_fRotationDeltaX = 0.f;
		//ZGetInput()->m_fRotationDeltaY = 0.f;

		pFromMsg = 0;
		pToMsg = 0;
	}

	ZGetGameInterface()->SetCursorEnable(bEnable);

	ZPostPeerChatIcon(bEnable);
}

void Chat::OutputChatMsg(const char *szMsg){
	OutputChatMsg(szMsg, TextColor);
}

static std::wstring UTF8toUTF16(const char* szMsg)
{
	wchar_t WideBuffer[1024];
	auto len = MultiByteToWideChar(CP_UTF8, 0,
		szMsg, -1,
		WideBuffer, std::size(WideBuffer));
	if (len == 0)
	{
		assert(false);
		return L"";
	}
	return std::wstring{ WideBuffer, static_cast<size_t>(len) };
}

void Chat::OutputChatMsg(const char *szMsg, u32 dwColor)
{
	vMsgs.emplace_back();
	auto&& Msg = vMsgs.back();
	Msg.Time = QPC();
	Msg.Msg = UTF8toUTF16(szMsg);
	Msg.DefaultColor = dwColor;

	Msg.SubstituteFormatSpecifiers();
	DivideIntoLines(vMsgs.size() - 1, std::back_inserter(LineSegments));
}

int Chat::GetLines(MFontR2 *pFont, const wchar_t *szMsg, int nWidth, int nSize)
{
	int nMsgWidth = pFont->GetWidth(szMsg, nSize);
	return nMsgWidth / nWidth + (nMsgWidth % nWidth != 0);
}

void Chat::Scale(double fWidthRatio, double fHeightRatio){
	//MLog("Scale: %f, %f\n", fWidthRatio, fHeightRatio);
	Border.x1 *= fWidthRatio;
	Border.x2 *= fWidthRatio;
	Border.y1 *= fHeightRatio;
	Border.y2 *= fHeightRatio;

	//nFontSize *= fHeightRatio;
	ResetFonts();
}

void Chat::Resize(int nWidth, int nHeight)
{
	Border.x1 = 10;
	Border.y1 = double(1080 - 300) / 1080 * RGetScreenHeight();
	Border.x2 = (double)500 / 1920 * RGetScreenWidth();
	Border.y2 = double(1080 - 100) / 1080 * RGetScreenHeight();

	ResetFonts();
}

void Chat::ClearHistory()
{
	vMsgs.clear();
}

bool Chat::CursorInRange(int x1, int y1, int x2, int y2){
	return Cursor.x > x1 && Cursor.x < x2 && Cursor.y > y1 && Cursor.y < y2;
}

int Chat::GetTextLength(MFontR2 *pFont, const wchar_t *szFormat, ...)
{
	wchar_t buf[256];
	va_list va;
	va_start(va, szFormat);
	vswprintf_safe(buf, szFormat, va);
	va_end(va);
	return pFont->GetWidth(buf);
}

struct CaretType
{
	int TotalTextHeight;
	v2i CaretPos;
};
static CaretType GetCaretPos(MFontR2* pFont, const wchar_t* Text, int CaretPos, int Width)
{
	CaretType ret{ 1, { 0, 1 } };
	v2i Cursor{ 0, 1 };
	for (auto c = Text; *c != 0; ++c)
	{
		auto CharWidth = pFont->GetWidth(c, 1);

		Cursor.x += CharWidth;
		if (Cursor.x > Width)
		{
			++Cursor.y;
			Cursor.x = CharWidth;
		}
		
		auto Distance = c - Text;
		if (Distance == CaretPos)
			ret.CaretPos = Cursor;
	}
	ret.TotalTextHeight = Cursor.y;
	return ret;
}

bool Chat::GetPos(const ChatLine &c, unsigned long nPos, POINT *pRet){
	if (nPos > c.Msg.length())
		return 0;

	D3DRECT Output = GetOutputRect();

	int nLimit = (Output.y2 - Output.y1 - 10) / nFontHeight;

	int nLines = 0;

	for (int i = vMsgs.size() - 1; nLines < nLimit && i >= 0; i--){
		ChatLine &cl = vMsgs.at(i);

		if (&c == &cl){
			int nOffset = 0;

			if (c.GetLines() == 1){
				pRet->y = Output.y2 - 5 - (nLines) * nFontHeight - nFontHeight * .5;
			}
			else{
				int nLine = 0;

				for (int i = 0; i < c.GetLines() - 1; i++){
					if (int(nPos) < c.GetLineBreak(i)->nStartPos)
						break;

					nLine++;
				}

				pRet->y = Output.y2 - 5 - (nLines - nLine) * nFontHeight - nFontHeight * .5;

				if (nLine > 0)
					nOffset = c.GetLineBreak(nLine - 1)->nStartPos;
			}

			pRet->x = Output.x1 + 5 + GetTextLength(pFont.get(), L"%.*s_", nPos - nOffset, &c.Msg.at(nOffset)) - GetTextLength(pFont.get(), L"_");

			return 1;
		}

		nLines += cl.GetLines();
	}

	return 0;
}

bool Chat::OnEvent(MEvent* pEvent){
	if (!bInputEnabled) {
		if (pEvent->nMessage == MWM_CHAR && pEvent->nKey == VK_RETURN) {
			EnableInput(1, 0);
		}

		return false;
	}

	if (pEvent->nMessage == MWM_KEYDOWN) {
		switch (pEvent->nKey) {

		case VK_HOME:
			nCaretPos = -1;
			break;

		case VK_END:
			nCaretPos = strField.length() - 1;
			break;

		case VK_TAB:
			/*bPlayerList = !bPlayerList;
			if (bPlayerList){
			#ifdef DEBUG
			vstrPlayerList.push_back(std::string("test1"));
			vstrPlayerList.push_back(std::string("test2"));
			#endif DEBUG
			nPlayerListWidth = 0;
			for (auto &it : *ZGetCharacterManager()){
			ZCharacter &Player = *it.second;
			vstrPlayerList.push_back(std::string(Player.GetProperty()->szName));

			int nLen = GetTextLen(vstrPlayerList.back().c_str(), -1);
			if (nLen > nPlayerListWidth)
			nPlayerListWidth = nLen;
			}

			nCurPlayer = 0;
			}
			else{
			std::string &strEntry = vstrPlayerList.at(nCurPlayer);
			strField.insert(nCaretPos + 1, strEntry);
			nCaretPos += strEntry.length();
			vstrPlayerList.clear();
			}*/

		{
			size_t StartPos = strField.rfind(0x20);
			if (StartPos == std::string::npos)
				StartPos = 0;
			else
				StartPos++;

			size_t PartialNameLength = strField.size() - StartPos;

			auto PartialName = strField.data() + StartPos;

			for (auto &it : *ZGetCharacterManager())
			{
				ZCharacter &Player = *it.second;
				const char *PlayerName = Player.GetProperty()->szName;
				size_t PlayerNameLength = strlen(PlayerName);

				if (PlayerNameLength < PartialNameLength)
					continue;

				wchar_t WidePlayerName[256];
				auto len = MultiByteToWideChar(CP_ACP, 0,
					PlayerName, -1,
					WidePlayerName, std::size(WidePlayerName));
				assert(len != 0);

				if (!_wcsnicmp(PartialName, WidePlayerName, PartialNameLength))
				{
					if (strField.size() + PlayerNameLength - PartialNameLength > MAX_INPUT_LENGTH)
						break;

					for (size_t i = 0; i < PartialNameLength; i++)
						strField.erase(strField.size() - 1);

					strField.append(WidePlayerName);
					nCaretPos += PlayerNameLength - PartialNameLength;
					break;
				}
			}
		}

		break;

		case VK_UP:
			/*if (bPlayerList){
				if (nCurPlayer > 0)
					nCurPlayer--;
				break;
			}*/

			if (nCurInputHistoryEntry > 0) {
				nCurInputHistoryEntry--;
				strField.assign(vstrInputHistory.at(nCurInputHistoryEntry));
				nCaretPos = vstrInputHistory.at(nCurInputHistoryEntry).length() - 1;
			}
			break;

		case VK_DOWN:
			/*if (bPlayerList){
				if (nCurPlayer < int(vstrPlayerList.size()) - 1)
					nCurPlayer++;
				break;
			}*/

			if (nCurInputHistoryEntry < int(vstrInputHistory.size()) - 1) {
				nCurInputHistoryEntry++;
				auto&& strEntry = vstrInputHistory.at(nCurInputHistoryEntry);
				strField.assign(strEntry);
				nCaretPos = strEntry.length() - 1;
			}
			else {
				strField.clear();
				nCaretPos = -1;
			}

			break;

		case VK_LEFT:
			if (nCaretPos >= 0)
				nCaretPos--;
			break;

		case VK_RIGHT:
			if (nCaretPos < int(strField.length()) - 1)
				nCaretPos++;
			break;

		case 'V':
		{
			if (!pEvent->bCtrl)
				break;

			auto Clipboard = GetClipboard();
			if (strField.length() + Clipboard.length() > MAX_INPUT_LENGTH)
			{
				strField += Clipboard.substr(0, MAX_INPUT_LENGTH - strField.length());
			}
			else
				strField += Clipboard;

			break;
		}

		};
	}
	else if (pEvent->nMessage == MWM_CHAR) {
		switch (pEvent->nKey) {

		case VK_TAB:
			break;

		case VK_RETURN:
			/*if (bPlayerList){
				std::string &strEntry = vstrPlayerList.at(nCurPlayer);
				strField.insert(nCaretPos + 1, strEntry);
				nCaretPos += strEntry.length();
				vstrPlayerList.clear();
				bPlayerList = 0;
				break;
			}*/

			if (strField.compare(L"")) {
				vstrInputHistory.push_back(strField);
				nCurInputHistoryEntry = vstrInputHistory.size();

				char MultibyteString[1024];
				WideCharToMultiByte(CP_UTF8, 0,
					strField.c_str(), -1,
					MultibyteString, std::size(MultibyteString),
					nullptr, nullptr);

				ZGetGameInterface()->GetChat()->Input(MultibyteString);
				strField.clear();
			}

			EnableInput(0, 0);
			break;

		case VK_BACK:
			if (nCaretPos >= 0) {
				strField.erase(nCaretPos, 1);
				nCaretPos--;
			}
			break;
		case VK_ESCAPE:
			EnableInput(0, 0);
			break;

		default:
			if (strField.length() < MAX_INPUT_LENGTH) {
				if (pEvent->nKey < 27) // Ctrl + A-Z
					break;

				strField.insert(nCaretPos + 1, 1, pEvent->nKey);

				auto SlashR = L"/r ";
				auto SlashWhisper = L"/whisper ";
				if (strField == SlashR)
				{
					std::wstring LastSenderWide;
					LastSenderWide.resize(MATCHOBJECT_NAME_LENGTH);
					auto* LastSender = ZGetGameInterface()->GetChat()->m_szWhisperLastSender;
					auto len = MultiByteToWideChar(CP_ACP, 0,
						LastSender, -1,
						&LastSenderWide[0], std::size(LastSenderWide));
					if (len == 0)
						break;

					LastSenderWide.resize(len);

					strField = SlashWhisper;
					strField += LastSenderWide;
					nCaretPos = strField.length() - 1;
				}
				else
					nCaretPos++;
			}
		};
	}

	auto ret = GetCaretPos(pFont.get(), strField.c_str(), nCaretPos, Border.x2 - (Border.x1 + 5));
	InputHeight = ret.TotalTextHeight;
	CaretCoord = ret.CaretPos;

	return true;
}

int Chat::GetTextLen(ChatLine &cl, int nPos, int nCount){
	return GetTextLength(pFont.get(), L"_%.*s_", nCount, &cl.Msg.at(nPos)) - GetTextLength(pFont.get(), L"__");
}

int Chat::GetTextLen(const char *szMsg, int nCount){
	return GetTextLength(pFont.get(), L"_%.*s_", nCount, szMsg) - GetTextLength(pFont.get(), L"__");
}

void Chat::OnUpdate(){
	POINT PrevCursorPos = Cursor;
	GetCursorPos(&Cursor);

	v2 MinimumSize{ 192.f * RGetScreenWidth() / 1920.f, 108.f * RGetScreenHeight() / 1080.f };

	if (dwResize){
		if (dwResize & RESIZE_X1 && Border.x1 + Cursor.x - PrevCursorPos.x < Border.x2 - MinimumSize.x)
			Border.x1 += Cursor.x - PrevCursorPos.x;
		if (dwResize & RESIZE_X2 && Border.x2 + Cursor.x - PrevCursorPos.x > Border.x1 + MinimumSize.x)
			Border.x2 += Cursor.x - PrevCursorPos.x;
		if (dwResize & RESIZE_Y1 && Border.y1 + Cursor.y - PrevCursorPos.y < Border.y2 - MinimumSize.y)
			Border.y1 += Cursor.y - PrevCursorPos.y;
		if (dwResize & RESIZE_Y2 && Border.y2 + Cursor.y - PrevCursorPos.y > Border.y1 + MinimumSize.y)
			Border.y2 += Cursor.y - PrevCursorPos.y;

		LineSegments.clear();
		for (int i = 0; i < int(vMsgs.size()); ++i)
			DivideIntoLines(i, std::back_inserter(LineSegments));
	}

	if (Action == CWA_MOVING){
		Border.x1 += Cursor.x - PrevCursorPos.x;
		Border.y1 += Cursor.y - PrevCursorPos.y;
		Border.x2 += Cursor.x - PrevCursorPos.x;
		Border.y2 += Cursor.y - PrevCursorPos.y;
	}

	if (pFromMsg && pToMsg && GetAsyncKeyState(VK_CONTROL) & 0x8000 && GetAsyncKeyState('C') & 0x8000){
		if (OpenClipboard(g_hWnd)){
			EmptyClipboard();

			if (pFromMsg == pToMsg){
				auto str = pFromMsg->Msg.substr(min(nFromPos, nToPos));
				SetClipboard(str);
			}
			else{
				std::wstring str;

				bool bFirstFound = 0;

				for (std::vector<ChatLine>::const_iterator it = vMsgs.begin(); it != vMsgs.end(); it++){
					const ChatLine *pcl = &*it;

					if (pcl == pFromMsg || pcl == pToMsg){
						if (!bFirstFound){
							int nPos = pcl == pFromMsg ? nFromPos : nToPos;
							str.append(&pcl->Msg.at(nPos));

							bFirstFound = 1;

							continue;
						}
						else{
							int nPos = pcl == pFromMsg ? nFromPos : nToPos;
							str.append(L"\n");
							str.append(pcl->Msg.c_str(), nPos + 2);

							break;
						}
					}

					if (bFirstFound){
						str.append(L"\n");
						str.append(pcl->Msg.c_str());
					}
				}

				if (bFirstFound){
					SetClipboard(str);
				}
			}

			CloseClipboard();
		}
	}

	const int nBorderWidth = 5;

	//dwResize = 0;
	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000){
		if (Action == CWA_NONE){
			D3DRECT tr = GetTotalRect();

			if (CursorInRange(tr.x1 - nBorderWidth, tr.y1 - nBorderWidth, tr.x1 + nBorderWidth, tr.y2 + nBorderWidth))
				dwResize |= RESIZE_X1;
			if (CursorInRange(tr.x1 - nBorderWidth, tr.y1 - nBorderWidth, tr.x2 + nBorderWidth, tr.y1 + nBorderWidth))
				dwResize |= RESIZE_Y1;
			if (CursorInRange(tr.x2 - nBorderWidth, tr.y1 - nBorderWidth, tr.x2 + nBorderWidth, tr.y2 + nBorderWidth))
				dwResize |= RESIZE_X2;
			if (CursorInRange(tr.x1 - nBorderWidth, tr.y2 - nBorderWidth, tr.x2 + nBorderWidth, tr.y2 + nBorderWidth))
				dwResize |= RESIZE_Y2;

			if (dwResize)
				Action = CWA_RESIZING;
		}

		if (CursorInRange(Border.x2 - 15, Border.y1 - 18, Border.x2 - 15 + 12, Border.y1 - 18 + nFontHeight) && Action == CWA_NONE){
			Border.x1 = 10;
			Border.y1 = double(1080 - 300) / 1080 * RGetScreenHeight();
			Border.x2 = (double)500 / 1920 * RGetScreenWidth();
			Border.y2 = double(1080 - 100) / 1080 * RGetScreenHeight();
		}
		else if (CursorInRange(Border.x1 + 5, Border.y1 + 5, Border.x2 - 5, Border.y2 - 5)){
			if (Action != CWA_SELECTING){
				D3DRECT Output = GetOutputRect();

				int nLimit = (Output.y2 - Output.y1 - 10) / nFontHeight;
				cprint("nLimit: %d\n", nLimit);

				int nLine = nLimit - ((Output.y2 - 5) - Cursor.y) / nFontHeight;
				cprint("nLine: %d\n", nLine);

				int i = vMsgs.size() - 1;
				int nCurLine = nLimit + 1;

				while (i >= 0){
					ChatLine &cl = vMsgs.at(i);

					if (nCurLine - cl.GetLines() <= nLine){
						pFromMsg = &cl;
						Action = CWA_SELECTING;

						unsigned long nPos = nCurLine - cl.GetLines() == nLine ? 0 : cl.GetLineBreak(nLine - (nCurLine - cl.GetLines()) - 1)->nStartPos;
						int x = Cursor.x - (Output.x1 + 5);
						int nLen = 0;

						while (x > nLen && nPos < cl.Msg.length()){
							nLen += GetTextLen(cl, nPos, 1);
							nPos++;

							/*if(nPos == cl.Msg.size()){
							pFromMsg = 0;
							pToMsg = 0;

							goto doublebreak;
							}*/
						}

						nPos--;

						if (nLen - GetTextLen(cl, nPos, 1) / 2 > x)
							nFromPos = nPos - 1;
						else
							nFromPos = nPos;

						//cprint("nFromMsg: %s\nnFromPos = %d\n", cl.Msg.c_str(), nFromPos);

						break;
					}

					nCurLine -= cl.GetLines();
					i--;
				}

				if (i < 0){
					pFromMsg = 0;
					pToMsg = 0;
				}
			}
			else{
				D3DRECT Output = GetOutputRect();

				int nLimit = (Output.y2 - Output.y1 - 10) / nFontHeight;
				//cprint("nLimit: %d\n", nLimit);

				int nLine = nLimit - ((Output.y2 - 5) - Cursor.y) / nFontHeight;
				//cprint("nLine: %d\n", nLine);

				int i = vMsgs.size() - 1;
				int nCurLine = nLimit + 1;

				while (i >= 0){
					ChatLine &cl = vMsgs.at(i);

					if (nCurLine - cl.GetLines() <= nLine || i == 0){
						pToMsg = &cl;

						unsigned long nPos;

						if (nCurLine - cl.GetLines() <= nLine)
							nPos = nCurLine - cl.GetLines() == nLine ? 0 : cl.GetLineBreak(nLine - (nCurLine - cl.GetLines()) - 1)->nStartPos;
						else
							nPos = 0;

						int x = Cursor.x - (Output.x1 + 5);
						int nLen = 0;

						while (x > nLen && nPos < cl.Msg.length()){
							nLen += GetTextLen(cl, nPos, 1);
							nPos++;
						}

						nPos--;

						if (nLen - GetTextLen(cl, nPos, 1) / 2 > x)
							nToPos = nPos - 1;
						else
							nToPos = nPos;

						//cprint("nToMsg: %s\nnToPos = %d\n", cl.Msg.c_str(), nToPos);

						break;
					}

					nCurLine -= cl.GetLines();
					i--;
				}

				/*if(i < 0){
				pFromMsg = 0;
				pToMsg = 0;
				}*/
			}
		}
		else if (Action != CWA_SELECTING){
			pFromMsg = 0;
			pToMsg = 0;
		}

		if (Action == CWA_NONE && CursorInRange(Border.x1, Border.y1 - 20, Border.x2 + 1, Border.y1))
			Action = CWA_MOVING;
	}
	else if (GetAsyncKeyState(VK_RBUTTON) & 0x8000){
	}
	else{
		Action = CWA_NONE;
		dwResize = 0;
		/*if(bSelecting && pFromMsg && pToMsg){
		cprint("Message selected! %d\n", pFromMsg == pToMsg);
		if(pFromMsg == pToMsg)
		cprint("%.*s\nnFromPos = %d; nToPos = %d; pToMsg->Msg.c_str(): %s; pToMsg->Msg.length(): %d\n",
		abs(nToPos - nFromPos), &pFromMsg->Msg.at(min(nFromPos, nToPos)), nFromPos, nToPos, pToMsg->Msg.c_str(), pToMsg->Msg.length());
		}*/
	}
}

D3DRECT Chat::GetOutputRect(){
	D3DRECT r = { Border.x1, Border.y1, Border.x2, Border.y2 - nFontHeight };
	return r;
}

D3DRECT Chat::GetInputRect(){
	D3DRECT r = { Border.x1, Border.y2 - nFontHeight, Border.x2, Border.y2 + (InputHeight - 1) * nFontHeight };
	return r;
}

D3DRECT Chat::GetTotalRect(){
	D3DRECT r = { Border.x1, Border.y1 - 20, Border.x2, Border.y2 };
	return r;
}

static MRECT MakeMRECT(const D3DRECT& src)
{
	return{
		src.x1,
		src.y1,
		src.x2 - src.x1,
		src.y2 - src.y1,
	};
}

void Chat::OnDraw(MDrawContext* pDC)
{
	bool bShowAll = ZIsActionKeyPressed(ZACTION_SHOW_FULL_CHAT) && !bInputEnabled;
	auto&& Output = GetOutputRect();

	int nLimit;
	if (bShowAll)
		nLimit = (Output.y2 - 5) / nFontHeight;
	else
		nLimit = (Output.y2 - Output.y1 - 10) / nFontHeight;

	auto Time = QPC();
	auto TPS = QPF();

	DrawBackground(pDC, Time, TPS, nLimit, bShowAll);
	DrawChatLines(pDC, Time, TPS, nLimit, bShowAll);
	DrawSelection(pDC);
	
	if (IsInputEnabled()) {
		DrawFrame(pDC, Time, TPS);
	}
}

int Chat::DrawTextWordWrap(MFontR2 *pFont, const wchar_t *szStr, const D3DRECT &r, DWORD dwColor)
{
	int Lines = 1;
	int StringLength = wcslen(szStr);
	int CurrentLineLength = 0;
	int MaxLineLength = r.x2 - r.x1;

	for (int i = 0; i < StringLength; i++)
	{
		int CharWidth = pFont->GetWidth(szStr + i, 1);
		int CharHeight = pFont->GetHeight();

		if (CurrentLineLength + CharWidth > MaxLineLength)
		{
			CurrentLineLength = 0;
			Lines++;
		}

		auto x = r.x1 + CurrentLineLength;
		auto y = r.y1 + (CharHeight + 1) * max(0, Lines - 1);
		pFont->m_Font.DrawTextN(x, y,
			szStr + i, 1,
			dwColor);

		CurrentLineLength += CharWidth;
	}

	return Lines;
}

void Chat::DrawTextN(MFontR2 *pFont, const wchar_t *szStr, const D3DRECT &r, DWORD dwColor, int nLen)
{
	pFont->m_Font.DrawTextN(r.x1, r.y1, szStr, nLen, dwColor);
}

void Chat::DrawBorder(MDrawContext* pDC)
{
	auto rect = Border;
	rect.y2 += (InputHeight - 1) * nFontHeight;

	// Draw the box outline
	v2 vs[] = {
		{ float(rect.x1), float(rect.y1) },
		{ float(rect.x2), float(rect.y1) },
		{ float(rect.x2), float(rect.y2) },
		{ float(rect.x1), float(rect.y2) },
	};

	for (size_t i = 0; i < std::size(vs); i++)
	{
		auto a = vs[i];
		auto b = vs[(i + 1) % std::size(vs)];
		pDC->Line(a.x, a.y, b.x, b.y);
	}

	// Draw the line between the output and input
	rect.y2 -= 2;
	rect.y2 -= InputHeight * nFontHeight;
	pDC->Line(rect.x1, rect.y2, rect.x2, rect.y2);
}

void Chat::DrawBackground(MDrawContext* pDC, u64 Time, u64 TPS, int nLimit, bool bShowAll)
{
	if (BackgroundColor & 0xFF000000)
	{
		if (!bInputEnabled)
		{
			// Need to store this value instead of calculating it every frame
			int nLines = 0;
			// i needs to be signed since it terminates on -1
			for (int i = int(vMsgs.size() - 1); nLines < nLimit && i >= 0; i--)
			{
				ChatLine &cl = vMsgs.at(i);

				if (cl.Time + TPS * fFadeTime < Time && !bShowAll && !bInputEnabled)
					break;

				nLines += cl.GetLines();
			}

			if (nLines > 0)
			{
				auto&& Output = GetOutputRect();
				D3DRECT Rect = {
					Output.x1,
					Output.y2 - 5 - nLines * nFontHeight,
					Output.x2,
					Output.y2,
				};

				pDC->SetColor(BackgroundColor);
				pDC->FillRectangle(MakeMRECT(Rect));
			}
		}
		else
		{
			auto Rect = Border;
			Rect.y2 += (InputHeight - 1) * nFontHeight;

			pDC->SetColor(BackgroundColor);
			pDC->FillRectangle(MakeMRECT(Rect));
		}
	}
}

template <typename T>
struct LineDivisionState
{
	T&& OutputIterator;
	LineSegmentInfo CurLineSegmentInfo;
	int ChatLineIndex = 0;
	int MsgIndex = 0;
	int Lines = 0;
	int CurrentLinePixelLength = 0;
	u32 CurTextColor;
	u32 CurEmphasis = EmphasisType::Default;

	LineDivisionState(T&& OutputIterator, int ChatLineIndex, u32 CurTextColor) :
		OutputIterator{ std::forward<T>(OutputIterator) },
		ChatLineIndex{ ChatLineIndex },
		CurTextColor{ CurTextColor }
	{}

	void AddSegment(bool IsEndOfLine)
	{
		// Compute the length from the distance from the current character index
		// to the one the substring started at.
		CurLineSegmentInfo.LengthInCharacters = MsgIndex - int(CurLineSegmentInfo.Offset);

		// Add this LineSegmentInfo to the vector.
		OutputIterator++ = CurLineSegmentInfo;

		if (IsEndOfLine)
		{
			CurrentLinePixelLength = 0;
			Lines++;
		}

		// Reset to zero-initialized LineSegmentInfo.
		CurLineSegmentInfo = LineSegmentInfo{};
		// Set data.
		CurLineSegmentInfo.ChatLineIndex = ChatLineIndex;
		CurLineSegmentInfo.Offset = MsgIndex;
		CurLineSegmentInfo.PixelOffsetX = CurrentLinePixelLength;
		CurLineSegmentInfo.IsStartOfLine = CurrentLinePixelLength == 0;
		CurLineSegmentInfo.TextColor = CurTextColor;
		CurLineSegmentInfo.Emphasis = CurEmphasis;
	}

	void HandleFormatSpecifier(FormatSpecifier& FormatSpec)
	{
		bool IsLine = false;
		switch (FormatSpec.ft) {
		case FT_COLOR:
			CurTextColor = FormatSpec.Color;
			break;

		case FT_DEFAULT:
			CurEmphasis = EmphasisType::Default;
			break;

		case FT_BOLD:
			CurEmphasis |= EmphasisType::Bold;
			break;

		case FT_ITALIC:
			CurEmphasis |= EmphasisType::Italic;
			break;

		case FT_UNDERLINE:
			CurEmphasis |= EmphasisType::Underline;
			break;

		case FT_STRIKETHROUGH:
			CurEmphasis |= EmphasisType::Strikethrough;
			break;

		case FT_LINEBREAK:
			IsLine = true;
			break;
		};

		// If MsgIndex - int(CurLineSegmentInfo.Offset) equals zero,
		// the substring would be empty if we were to add a line.
		if (IsLine || MsgIndex - int(CurLineSegmentInfo.Offset) != 0)
			AddSegment(IsLine);
	}
};

template <typename T>
void Chat::DivideIntoLines(int ChatLineIndex, T&& OutputIterator)
{
	auto&& cl = vMsgs[ChatLineIndex];
	// Clear the previous wrapping line breaks, since we're going to add new ones.
	cl.ClearWrappingLineBreaks();

	auto MaxLineLength = (Border.x2 - 5) - (Border.x1 + 5);

	LineDivisionState<T> State{ std::forward<T>(OutputIterator), ChatLineIndex, TextColor };

	// Initialize the first segment.
	State.CurLineSegmentInfo.ChatLineIndex = ChatLineIndex;
	State.CurLineSegmentInfo.Offset = 0;
	State.CurLineSegmentInfo.PixelOffsetX = 0;
	State.CurLineSegmentInfo.IsStartOfLine = true;
	State.CurLineSegmentInfo.TextColor = TextColor;
	State.CurLineSegmentInfo.Emphasis = EmphasisType::Default;

	auto FormatIterator = cl.vFormatSpecifiers.begin();
	for (State.MsgIndex = 0; State.MsgIndex < int(cl.Msg.length()); ++State.MsgIndex)
	{
		// Process all the format specifiers at this index.
		// There may be more than one, so we do a loop.
		while (FormatIterator != cl.vFormatSpecifiers.end() &&
			FormatIterator->nStartPos == State.MsgIndex)
		{
			State.HandleFormatSpecifier(*FormatIterator);
			++FormatIterator;
		}

		auto CharWidth = pFont->GetWidth(cl.Msg.data() + State.MsgIndex, 1);

		// If adding this character would make the line length exceed the max,
		// we add a new line for this character to go on.
		if (State.CurrentLinePixelLength + CharWidth > MaxLineLength)
		{
			// ChatLine::AddWrappingLineBreak returns an iterator to the line break
			// that was inserted, so we want to set FormatIterator to the next one.
			// We do this since the current iterator may have been invalidated by the mutation.
			FormatIterator = std::next(cl.AddWrappingLineBreak(State.MsgIndex));
			State.AddSegment(true);
		}

		State.CurrentLinePixelLength += CharWidth;
	}

	// Add the final segment.
	State.AddSegment(true);

	cl.Lines = State.Lines;
}

MFontR2* Chat::GetFont(u32 Emphasis)
{
	if (Emphasis & EmphasisType::Italic)
		return pItalicFont.get();

	return pFont.get();
}

template <typename T>
static auto Reverse(T&& Container) {
	return MakeRange(Container.rbegin(), Container.rend());
}

static auto GetDrawLinesRect(const D3DRECT& OutputRect, int LinesDrawn,
	int PixelOffsetX, int FontHeight)
{
	return D3DRECT{
		OutputRect.x1 + 5 + PixelOffsetX,
		OutputRect.y2 - 5 - ((LinesDrawn + 1) * FontHeight),
		OutputRect.x2 - 5,
		OutputRect.y2 - 5
	};
}

void Chat::DrawChatLines(MDrawContext* pDC, u64 Time, u64 TPS, int nLimit, bool bShowAll)
{
	pFont->m_Font.BeginFont();

	int LinesDrawn = 0;
	for (auto&& LineSegment : Reverse(LineSegments))
	{
		auto&& Rect = GetDrawLinesRect(GetOutputRect(), LinesDrawn, LineSegment.PixelOffsetX, nFontHeight);
		auto&& cl = vMsgs[LineSegment.ChatLineIndex];

		if (!bShowAll && !bInputEnabled && Time > cl.Time + TPS * fFadeTime)
			break;

		auto String = cl.Msg.data() + LineSegment.Offset;
		auto Length = LineSegment.LengthInCharacters;
		auto Font = GetFont(LineSegment.Emphasis);
		auto Color = LineSegment.TextColor;

		DrawTextN(Font, String, Rect, Color, Length);

		if (LineSegment.IsStartOfLine)
		{
			++LinesDrawn;
			if (LinesDrawn > nLimit)
				break;
		}
	}

	pFont->m_Font.EndFont();
}

void Chat::DrawSelection(MDrawContext * pDC)
{
	if (pFromMsg && pToMsg) {
		POINT p;

		if (!GetPos(*pFromMsg, nFromPos, &p))
			return;

		int nFromX = p.x;
		int nFromY = p.y;

		if (!GetPos(*pToMsg, nToPos, &p))
			return;

		int nToX = p.x;
		int nToY = p.y;

		bool bSwap = nFromY > nToY || nFromY == nToY && nFromX > nToX;

		if (bSwap) {
			std::swap(nFromX, nToX);
			std::swap(nFromY, nToY);

			if (!GetPos(*pFromMsg, nFromPos + 1, &p))
				return;
		}
		else
			if (!GetPos(*pToMsg, nToPos + 1, &p))
				return;

		nToX = p.x;
		nToY = p.y;

		auto Fill = [&](auto&&... Coords) {
			pDC->FillRectangle(MakeMRECT({ Coords... }));
		};

		pDC->SetColor(ARGB(0xA0, 00, 0x80, 0xFF));
		if (nFromY == nToY)
		{
			Fill(nFromX,
				nFromY - nFontHeight / 2,
				nToX,
				nToY + nFontHeight / 2);
		}
		else {
			Fill(nFromX,
				nFromY - nFontHeight / 2,
				Border.x2 - 5,
				nToY + nFontHeight / 2);

			for (int i = nFontHeight; i < nToY - nFromY; i += nFontHeight) {
				Fill(Border.x1 + 5,
					nFromY + i - nFontHeight / 2,
					Border.x2 - 5,
					nFromY + i + nFontHeight / 2);
			}
			Fill(Border.x1,
				nFromY - nFontHeight / 2,
				nToX - 5,
				nToY + nFontHeight / 2);
		}
	}
}

void Chat::DrawFrame(MDrawContext * pDC, u64 Time, u64 TPS)
{
	// Draw top of border
	{
		D3DRECT Rect = { Border.x1, Border.y1 - 20, Border.x2 + 1, Border.y1 };

		pDC->SetColor(InterfaceColor);
		pDC->FillRectangle(MakeMRECT(Rect));
	}

	DrawBorder(pDC);

	// Draw D button
	{
		D3DRECT Rect;

		Rect.x1 = Border.x2 - 15;
		Rect.y1 = Border.y1 - 18;
		Rect.x2 = Border.x2 - 15 + 12;
		Rect.y2 = Border.y1 - 18 + nFontHeight;

		DrawTextN(pFont.get(), L"D", Rect, TextColor);
	}

	D3DXCOLOR Color;
	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
		Color = XRGB(0xFF, 0xFF, 0xFF);
	else {
		Color = 0xFF00A5C3;
		D3DXColorScale(&Color, &Color, 1.2);
	}

	D3DRECT Rect;
	Rect.x1 = Border.x1 + 5;
	Rect.y1 = Border.y2 - 2 - nFontHeight;
	Rect.x2 = Border.x2;
	Rect.y2 = Border.y2;

	int x = Rect.x1 + CaretCoord.x;
	int y = Rect.y1 + (CaretCoord.y - 1) * nFontHeight;

	if (fmod(Time, .8f * TPS) > .4f * TPS)
	{
		// Draw caret
		pDC->SetColor(TextColor);
		pDC->Line(x, y, x, y + nFontHeight);
	}

	DrawTextWordWrap(pFont.get(), strField.c_str(), Rect, TextColor);
}

void Chat::ResetFonts(){
	SafeDestroy(pFont.get());
	SafeDestroy(pItalicFont.get());

	char buf[64];
	sprintf_safe(buf, "Font size: %d\n", int(float(nFontSize) / 1080 * RGetScreenHeight() + 0.5));
	OutputDebugString(buf);
	pFont->Create("NewChatFont", strFont.c_str(),
		int(float(nFontSize) / 1080 * RGetScreenHeight() + 0.5), 1, true);
	pItalicFont->Create("NewChatItalicFont", strFont.c_str(),
		int(float(nFontSize) / 1080 * RGetScreenHeight() + 0.5), 1, true, true);

	nFontHeight = pFont->GetHeight();
}
