#include "stdafx.h"
#include "NewChat.h"
#include "RGMain.h"
#include "ZCharacterManager.h"
#include "ZInput.h"
#include "Config.h"

// Resize flags
enum
{
	RESIZE_X1 = 1 << 0,
	RESIZE_Y1 = 1 << 1,
	RESIZE_X2 = 1 << 2,
	RESIZE_Y2 = 1 << 3,
};

// Stuff crashes if this is increased
static constexpr int MAX_INPUT_LENGTH = 230;

Chat g_Chat;
#ifdef NEW_CHAT
bool g_bNewChat = true;
#else
bool g_bNewChat = false;
#endif

void Chat::Create(const std::string &strFont, int nFontSize){
	fFadeTime = 10;

	Border.x1 = 10;
	Border.y1 = double(1080 - 300) / 1080 * RGetScreenHeight();
	Border.x2 = (double)500 / 1920 * RGetScreenWidth();
	Border.y2 = double(1080 - 100) / 1080 * RGetScreenHeight();

	TextColor = CHAT_DEFAULT_TEXT_COLOR;
	InterfaceColor = CHAT_DEFAULT_INTERFACE_COLOR;
	BackgroundColor = 0;

	Action = CWA_NONE;
	dwResize = 0;

	bInputEnabled = false;

	this->strFont = strFont;
	this->nFontSize = nFontSize;

	Cursor.x = RGetScreenWidth() / 2;
	Cursor.y = RGetScreenHeight() / 2;

	pFont = std::make_unique<MFontR2>();
	pFont->Create("NewChatFont", strFont.c_str(),
		int(float(nFontSize) / 1080 * RGetScreenHeight() + 0.5), 1, true);
	pItalicFont = std::make_unique<MFontR2>();
	pItalicFont->Create("NewChatItalicFont", strFont.c_str(),
		int(float(nFontSize) / 1080 * RGetScreenHeight() + 0.5), 1, true, true);

	nFontHeight = pFont->GetHeight();

	pFromMsg = 0;
	pToMsg = 0;

	strField.assign("");
	nCaretPos = -1;

	bAlpha = false;
	bBegunDrawing = false;
}

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

void Chat::OutputChatMsg(const char *szMsg, DWORD dwColor){
	unsigned long long t = QPC();

	vMsgs.push_back(ChatLine(t, std::string(szMsg)));

	ChatLine &cl = vMsgs.back();

	cl.DefaultColor = dwColor;

	unsigned long nPos = cl.Msg.find_first_of("^[", 0);
	while (nPos != std::string::npos && nPos < cl.Msg.length() - 2){
		if (cl.Msg.at(nPos) == '^'){
			if (cl.Msg.at(nPos + 1) >= '0' && cl.Msg.at(nPos + 1) <= '9'){
				cl.vFormatSpecifiers.push_back(FormatSpecifier(nPos, MMColorSet[cl.Msg.at(nPos + 1) - '0']));
				cl.Msg.erase(nPos, 2);
				nPos = cl.Msg.find_first_of("^[", nPos);
			}
			else if (tolower(cl.Msg.at(nPos + 1)) == 'b'){
				cl.vFormatSpecifiers.push_back(FormatSpecifier(nPos, FT_BOLD));
				cl.Msg.erase(nPos, 2);
				nPos = cl.Msg.find_first_of("^[", nPos);
			}
			else if (tolower(cl.Msg.at(nPos + 1)) == 'i'){
				cl.vFormatSpecifiers.push_back(FormatSpecifier(nPos, FT_ITALIC));
				cl.Msg.erase(nPos, 2);
				nPos = cl.Msg.find_first_of("^[", nPos);
			}
			else if (tolower(cl.Msg.at(nPos + 1)) == 'n'){
				cl.vFormatSpecifiers.push_back(FormatSpecifier(nPos, FT_LINEBREAK));
				cl.Msg.erase(nPos, 2);
				nPos = cl.Msg.find_first_of("^[", nPos);
			}
			else if (tolower(cl.Msg.at(nPos + 1)) == '#'){
				char buf[9];
				int nLen = min(static_cast<int>(cl.Msg.length() - 1 - nPos - 2), 8);
				strncpy_safe(buf, cl.Msg.data() + nPos + 2, nLen);

				char *pcEnd;
				D3DCOLOR Color = strtoul(buf, &pcEnd, 16);
				if (pcEnd - buf <= 6)
					Color |= 0xFF000000;

				cl.vFormatSpecifiers.push_back(FormatSpecifier(nPos, Color));
				cl.Msg.erase(nPos, 2 + nLen);
				nPos = cl.Msg.find_first_of("^[", nPos);
			}
			else if ((cl.Msg.at(nPos + 1) == '/')){
				cl.vFormatSpecifiers.push_back(FormatSpecifier(nPos, XRGB(0xC8, 0xC8, 0xC8)));
				cl.vFormatSpecifiers.push_back(FormatSpecifier(nPos, FT_DEFAULT));
				cl.Msg.erase(nPos, 2);
				nPos = cl.Msg.find_first_of("^[", nPos);
			}
			else
				nPos = cl.Msg.find_first_of("^[", nPos + 1);
		}
		else if (cl.Msg.at(nPos) == '[' && nPos != cl.Msg.size() - 2){
			if (cl.Msg.at(nPos + 2) == ']'){
				if (tolower(cl.Msg.at(nPos + 1)) == 'b' || tolower(cl.Msg.at(nPos + 1)) == 'i'){
					int nStart = nPos;
					FormatSpecifierType t;

					if (tolower(cl.Msg.at(nPos + 1)) == 'b')
						t = FT_BOLD;
					else
						t = FT_ITALIC;

					cl.Msg.erase(nStart, 3);

					cl.vFormatSpecifiers.push_back(FormatSpecifier(nStart, t));

					cprint("FormatSpecifier: %d, %s\n", nStart, t == FT_BOLD ? "bold" : "italic");

					nPos = cl.Msg.find_first_of("^[", nPos);
					continue;
				}
			}
			else if (cl.Msg.at(nPos + 1) == '/' && nPos != cl.Msg.size() - 3 && cl.Msg.at(nPos + 3) == ']'){
				if (tolower(cl.Msg.at(nPos + 2)) == 'b' || tolower(cl.Msg.at(nPos + 2)) == 'i'){
					int nStart = nPos;
					FormatSpecifierType t;

					cl.Msg.erase(nStart, 4);

					t = FT_DEFAULT;

					cl.vFormatSpecifiers.push_back(FormatSpecifier(nStart, t));

					cprint("FormatSpecifier: %d, FT_DEFAULT\n", nStart);

					nPos = cl.Msg.find_first_of("^[", nPos);
					continue;
				}
			}
			nPos = cl.Msg.find_first_of("^[", nPos + 1);
		}
		else
			nPos = cl.Msg.find_first_of("^[", nPos + 1);
	}

	CalcLineBreaks(cl);
}

int Chat::GetLines(MFontR2 *pFont, const char *szMsg, int nWidth, int nSize)
{
	int nMsgWidth = pFont->GetWidth(szMsg, nSize);
	return nMsgWidth / nWidth + (nMsgWidth % nWidth != 0);
}

void Chat::CalcLineBreaks(ChatLine &cl){
	cl.ClearLineBreaks();

	RECT r = { Border.x1 + 5, 0, Border.x2 - 5, RGetScreenHeight() };

	int nTempLines = GetLines(pFont.get(), cl.Msg.c_str(), r.right - r.left);

	int Lines = 1;
	int StringLength = strlen(cl.Msg.c_str());
	int CurrentLineLength = 0;
	int MaxLineLength = r.right - r.left;

	for (int i = 0; i < StringLength; i++)
	{
		int CharWidth = pFont->GetWidth(cl.Msg.data() + i, 1);
		int CharHeight = pFont->GetHeight();

		if (CurrentLineLength + CharWidth > MaxLineLength)
		{
			cl.AddLineBreak(i);
			CurrentLineLength = 0;
			Lines++;
		}

		CurrentLineLength += CharWidth;
	}
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

bool Chat::CursorInRange(int x1, int y1, int x2, int y2){
	return Cursor.x > x1 && Cursor.x < x2 && Cursor.y > y1 && Cursor.y < y2;
}

#define DWSIZEOF(x) ((sizeof(x) + sizeof(DWORD) - 1) & ~(sizeof(DWORD) - 1))
#define GETVA(lastarg) ((va_list)&lastarg + DWSIZEOF(lastarg))

int Chat::GetTextLength(MFontR2 *pFont, const char *szFormat, ...) 
{
	char buf[256] = { 0 };
	vsnprintf(buf, sizeof(buf), szFormat, GETVA(szFormat));
	return pFont->GetWidth(buf);
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

			pRet->x = Output.x1 + 5 + GetTextLength(pFont.get(), "%.*s_", nPos - nOffset, &c.Msg.at(nOffset)) - GetTextLength(pFont.get(), "_");

			return 1;
		}

		nLines += cl.GetLines();
	}

	return 0;
}

void Chat::OnEvent(MEvent *pEvent){
	if (bInputEnabled){
		if (pEvent->nMessage == MWM_KEYDOWN){
			switch (pEvent->nKey){

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

				const char *PartialName = strField.data() + StartPos;

				for (auto &it : *ZGetCharacterManager())
				{
					ZCharacter &Player = *it.second;
					const char *PlayerName = Player.GetProperty()->szName;
					size_t PlayerNameLength = strlen(PlayerName);

					if (PlayerNameLength < PartialNameLength)
						continue;

					if (!_strnicmp(PartialName, PlayerName, PartialNameLength))
					{
						if (strField.size() + PlayerNameLength - PartialNameLength > MAX_INPUT_LENGTH)
							break;

						for (size_t i = 0; i < PartialNameLength; i++)
							strField.erase(strField.size() - 1);

						strField.append(PlayerName);
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

				if (nCurInputHistoryEntry > 0){
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

				if (nCurInputHistoryEntry < int(vstrInputHistory.size()) - 1){
					nCurInputHistoryEntry++;
					std::string &strEntry = vstrInputHistory.at(nCurInputHistoryEntry);
					strField.assign(strEntry);
					nCaretPos = strEntry.length() - 1;
				}
				else{
					strField.clear();
					nCaretPos = -1;
				}

				break;

			case VK_LEFT:
				if (nCaretPos >= 0)
					nCaretPos--;
				break;

			case VK_RIGHT:
				if (nCaretPos <= int(strField.length()) - 1)
					nCaretPos++;
				break;

			case 'V':
			{
						if (!pEvent->bCtrl)
							return;

						if (!IsClipboardFormatAvailable(CF_TEXT))
							return;

						if (!OpenClipboard(g_hWnd))
							return;

						HANDLE h = GetClipboardData(CF_TEXT);

						if (!h)
							return;

						const char *s = (const char *)GlobalLock(h);
						std::string str = s;
						if (strField.length() + str.length() > MAX_INPUT_LENGTH)
						{
							strField += str.substr(0, MAX_INPUT_LENGTH - strField.length());
						}
						else
							strField += s;

						GlobalUnlock(h);
						CloseClipboard();
						break;
			}

			};
		}
		else if (pEvent->nMessage == MWM_CHAR){
			switch (pEvent->nKey){

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

				if (strField.compare("")){
					vstrInputHistory.push_back(strField);
					nCurInputHistoryEntry = vstrInputHistory.size();
					ZGetGameInterface()->GetChat()->Input(strField.c_str());
					strField.clear();
				}

				EnableInput(0, 0);
				break;

			case VK_BACK:
				if (nCaretPos >= 0){
					strField.erase(nCaretPos, 1);
					nCaretPos--;
				}
				break;
			case VK_ESCAPE:
				EnableInput(0, 0);
				break;

			default:
				if (strField.length() < MAX_INPUT_LENGTH){
					if (pEvent->nKey < 27) // Ctrl + A-Z
						break;

					strField.insert(nCaretPos + 1, 1, pEvent->nKey);

					if (strField == "/r ")
					{
						strField = "/whisper ";
						strField += ZGetGameInterface()->GetChat()->m_szWhisperLastSender;
						nCaretPos += strlen("/whisper ") - strlen("/r ");
					}

					nCaretPos++;
				}
			};
		}
	}
	else if (pEvent->nMessage == MWM_CHAR && pEvent->nKey == VK_RETURN)
		EnableInput(1, 0);
}

int Chat::GetTextLen(ChatLine &cl, int nPos, int nCount){
	return GetTextLength(pFont.get(), "_%.*s_", nCount, &cl.Msg.at(nPos)) - GetTextLength(pFont.get(), "__");
}

int Chat::GetTextLen(const char *szMsg, int nCount){
	return GetTextLength(pFont.get(), "_%.*s_", nCount, szMsg) - GetTextLength(pFont.get(), "__");
}

void Chat::OnUpdate(){
	POINT PrevCursorPos = Cursor;
	GetCursorPos(&Cursor);

	if (dwResize){
		if (dwResize & RESIZE_X1 && Border.x1 + Cursor.x - PrevCursorPos.x < Border.x2 - 20)
			Border.x1 += Cursor.x - PrevCursorPos.x;
		if (dwResize & RESIZE_X2 && Border.x2 + Cursor.x - PrevCursorPos.x > Border.x1 + 20)
			Border.x2 += Cursor.x - PrevCursorPos.x;
		if (dwResize & RESIZE_Y1 && Border.y1 + Cursor.y - PrevCursorPos.y  < Border.y2 - 20)
			Border.y1 += Cursor.y - PrevCursorPos.y;
		if (dwResize & RESIZE_Y2 && Border.y2 + Cursor.y - PrevCursorPos.y > Border.y1 + 20)
			Border.y2 += Cursor.y - PrevCursorPos.y;

		for (std::vector<ChatLine>::iterator it = vMsgs.begin(); it != vMsgs.end(); it++)
			CalcLineBreaks(*it);
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
				HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, abs(nToPos - nFromPos) + 2);
				void *pMem = GlobalLock(hMem);
				strcpy_trunc(reinterpret_cast<char *>(pMem),
					abs(nToPos - nFromPos) + 1,
					&pFromMsg->Msg.at(min(nFromPos, nToPos)));
				GlobalUnlock(hMem);

				SetClipboardData(CF_TEXT, hMem);

				cprint("Copied %.*s\n", abs(nToPos - nFromPos) + 1, &pFromMsg->Msg.at(min(nFromPos, nToPos)));
			}
			else{
				std::string str;

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
							str.append("\n");
							str.append(pcl->Msg.c_str(), nPos + 2);

							break;
						}
					}

					if (bFirstFound){
						str.append("\n");
						str.append(pcl->Msg.c_str());
					}
				}

				if (bFirstFound){
					HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, str.length());
					void *pMem = GlobalLock(hMem);
					strcpy_safe((char *)pMem, str.length(), str.c_str());
					GlobalUnlock(hMem);

					SetClipboardData(CF_TEXT, hMem);
				}
			}

			CloseClipboard();
		}
	}

	const int nBorderWidth = RELWIDTH(10);

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
	D3DRECT r = { Border.x1, Border.y2 - nFontHeight, Border.x2, Border.y2 };
	return r;
}

D3DRECT Chat::GetTotalRect(){
	D3DRECT r = { Border.x1, Border.y1 - 20, Border.x2, Border.y2 };
	return r;
}

void Chat::Display(){
	//BeginDraw();

	int nLimit;
	bool bShowAll = ZIsActionKeyPressed(ZACTION_SHOW_FULL_CHAT) && !bInputEnabled;

	const D3DRECT &Output = GetOutputRect();

	if (bShowAll)
		nLimit = (Output.y2 - 5) / nFontHeight;
	else
		nLimit = (Output.y2 - Output.y1 - 10) / nFontHeight;

	unsigned long long tNow = QPC();

	unsigned long long nTPS = QPF();

	if (BackgroundColor & 0xFF000000)
	{
		RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
		RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
		RGetDevice()->SetTexture(0, NULL);
		RGetDevice()->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

		if (!bInputEnabled)
		{
			// Need to store this value instead of calculating it every frame
			int nLines = 0;
			for (int i = int(vMsgs.size() - 1); nLines < nLimit && i >= 0; i--) // i needs to be signed since it terminates on -1
			{
				ChatLine &cl = vMsgs.at(i);

				if (cl.Time + nTPS * fFadeTime < tNow && !bShowAll && !bInputEnabled)
					break;

				nLines += cl.GetLines();
			}

			if (nLines > 0)
			{
				v2 v1, v2;

				v1.x = Output.x1;
				v1.y = Output.y2 - 5 - nLines * nFontHeight;
				v2.x = Output.x2;
				v2.y = Output.y2;

				Quad(v1, v2, BackgroundColor);
			}
		}
		else
		{
			Quad(Border, BackgroundColor);
		}
	}

	int nLines = 0;

	pFont->m_Font.BeginFont();

	for (int i = vMsgs.size() - 1; nLines < nLimit && i >= 0; i--){
		ChatLine &cl = vMsgs.at(i);

		if (cl.Time + nTPS * fFadeTime < tNow && !bShowAll && !bInputEnabled)
			break;

		long nMsgOffset;
		long nTempLines;

		auto it = cl.vFormatSpecifiers.begin();

		if (cl.GetLines() + nLines > nLimit){
			nMsgOffset = cl.GetLineBreak(cl.GetLines() + nLines - nLimit - 1)->nStartPos;

			while (&*it != cl.GetLineBreak(cl.GetLines() + nLines - nLimit - 1))
				it++;

			it++;
			nTempLines = cl.GetLines() + nLines - nLimit;
			cprint("nMsgOffset = %d, nTempLines = %d, cl.GetLines() + nLines - nLimit - 1 = %d\n", nMsgOffset, nTempLines, cl.GetLines() + nLines - nLimit - 1);
		}
		else{
			nMsgOffset = 0;
			nTempLines = cl.GetLines();
		}

		if (it == cl.vFormatSpecifiers.end()){
			RECT Rect = { Output.x1 + 5, Output.y2 - 5 - (nTempLines + nLines) * nFontHeight, Output.x2 - 5, Output.y2 - 5 };
			DrawTextN(pFont.get(), cl.Msg.data() + nMsgOffset, Rect, cl.DefaultColor);
			nLines++;
		}
		else{
			MFontR2 *pFont = this->pFont.get();
			D3DCOLOR dwColor = cl.DefaultColor;
			long nTemp = nTempLines;
			int nOffsetX = 0;
			int nPos = nMsgOffset;

			while (nTemp && (unsigned long)nPos < cl.Msg.length()){
				RECT Rect = { Output.x1 + 5 + nOffsetX, Output.y2 - 5 - (nTemp + nLines) * nFontHeight, Output.x2 - 5, Output.y2 - 5 };

				int nLen;

				if (it != cl.vFormatSpecifiers.end()){
					nLen = it->nStartPos - nPos;
				}
				else{
					nLen = strlen(cl.Msg.data() + nPos);
				}

				//g_pDraw->Text(pFont, cl.Msg.data() + nPos, nLen, &Rect, DT_TOP | DT_LEFT | DT_NOCLIP, dwColor);
				//pFont->m_Font.DrawTextA(Rect.left, Rect.top, cl.Msg.data() + nPos, dwColor);
				DrawTextN(pFont, cl.Msg.data() + nPos, Rect, dwColor, nLen);

				nOffsetX += GetTextLen(cl, nPos, nLen);

				if (it != cl.vFormatSpecifiers.end()){
					nPos = it->nStartPos;

					switch (it->ft){
					case FT_COLOR:
						dwColor = it->Color;
						break;

					case FT_DEFAULT:
						pFont = this->pFont.get();
						break;

					case FT_BOLD:
						//pFont = pBoldFont;
						break;

					case FT_ITALIC:
						pFont = pItalicFont.get();
						break;

					case FT_WRAP:
					case FT_LINEBREAK:
						nTemp--;
						nOffsetX = 0;
						break;
					};

					it++;
				}
				else{
					nTemp--;
					nOffsetX = 0;
				}
			}

			nLines += cl.GetLines();
		}
	}

	pFont->m_Font.EndFont();

	if (pFromMsg && pToMsg){
		POINT p;

		if (!GetPos(*pFromMsg, nFromPos, &p))
			goto ret;

		int nFromX = p.x;
		int nFromY = p.y;

		if (!GetPos(*pToMsg, nToPos, &p))
			goto ret;

		int nToX = p.x;
		int nToY = p.y;

		bool bSwap = nFromY > nToY || nFromY == nToY && nFromX > nToX;

		if (bSwap){
			std::swap(nFromX, nToX);
			std::swap(nFromY, nToY);

			if (!GetPos(*pFromMsg, nFromPos + 1, &p))
				goto ret;
		}
		else
		if (!GetPos(*pToMsg, nToPos + 1, &p))
			goto ret;

		nToX = p.x;
		nToY = p.y;

		if (nFromY == nToY)
			Quad(nFromX, nFromY - nFontHeight / 2, nToX, nToY + nFontHeight / 2, ARGB(0xA0, 00, 0x80, 0xFF));
		else{
			Quad(nFromX, nFromY - nFontHeight / 2, Border.x2 - 5, nToY + nFontHeight / 2, ARGB(0xA0, 00, 0x80, 0xFF));
			for (int i = nFontHeight; i < nToY - nFromY; i += nFontHeight){
				Quad(Border.x1 + 5, nFromY + i - nFontHeight / 2, Border.x2 - 5, nFromY + i + nFontHeight / 2, ARGB(0xA0, 00, 0x80, 0xFF));
			}
			Quad(Border.x1, nFromY - nFontHeight / 2, nToX - 5, nToY + nFontHeight / 2, ARGB(0xA0, 00, 0x80, 0xFF));
		}
	}

ret:;

	if (!bInputEnabled)
	{
		//EndDraw();
		return;
	}

	RECT r;

	r.left = Border.x2 - 15;
	r.top = Border.y1 - 18;
	r.right = Border.x2 - 15 + 12;
	r.bottom = Border.y1 - 18 + nFontHeight;

	D3DRECT Rect = { Border.x1, Border.y1 - 20, Border.x2 + 1, Border.y1 };

	Quad(Rect, InterfaceColor);

	DrawBorder();

	DrawTextN(pFont.get(), "D", r, TextColor);

	D3DXCOLOR Color;
	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
		Color = XRGB(0xFF, 0xFF, 0xFF);
	else{
		Color = 0xFF00A5C3;
		D3DXColorScale(&Color, &Color, 1.2);
	}

	/*Line(Cursor.x, Cursor.y, Cursor.x + 15, Cursor.y + 6, Color);
	Line(Cursor.x, Cursor.y, Cursor.x + 6, Cursor.y + 15, Color);

	if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
		Color = XRGB(0, 0, 0);

	Line(Cursor.x + 1, Cursor.y + 1, Cursor.x + 15, Cursor.y + 7, Color);
	Line(Cursor.x + 1, Cursor.y + 1, Cursor.x + 7, Cursor.y + 15, Color);*/

	r.left = Border.x1 + 5;
	r.top = Border.y2 - 2 - nFontHeight;
	r.right = Border.x2;
	r.bottom = Border.y2;

	int x = r.left + GetTextLen(strField.c_str(), nCaretPos + 1) + 1;
	int y = r.top;

	if (fmod(tNow, .8f * nTPS) > .4f * nTPS)
		Line(x, y, x, y + nFontHeight, TextColor);

	y -= 2;

	DrawTextN(pFont.get(), strField.c_str(), r, TextColor);
}

int Chat::DrawTextWordWrap(MFontR2 *pFont, const TCHAR *szStr, const RECT &r, DWORD dwColor)
{
	int Lines = 1;
	int StringLength = _tcslen(szStr);
	int CurrentLineLength = 0;
	int MaxLineLength = r.right - r.left;

	for (int i = 0; i < StringLength; i++)
	{
		int CharWidth = pFont->GetWidth((const char *)szStr + i, 1);
		int CharHeight = pFont->GetHeight();

		if (CurrentLineLength + CharWidth > MaxLineLength)
		{
			CurrentLineLength = 0;
			Lines++;
		}

		TCHAR String[2] = { szStr[i], 0 };
		
		pFont->m_Font.DrawTextA(r.left + CurrentLineLength, r.top + (CharHeight + 1) * max(0, Lines - 1), String, dwColor);

		CurrentLineLength += CharWidth;
	}

	return Lines;
}

void Chat::DrawTextN(MFontR2 *pFont, const TCHAR *szStr, const RECT &r, DWORD dwColor, int nLen)
{
	const TCHAR *String;
	std::unique_ptr<TCHAR[]> p;

	if (nLen != -1)
	{
		auto size = nLen + 1;
		p = decltype(p){new TCHAR[size]};
		_tcsncpy_s(p.get(), size, szStr, nLen);
		p[nLen] = 0;
		String = p.get();
	}
	else
	{
		String = szStr;
	}

	pFont->m_Font.DrawTextA(r.left, r.top, String, dwColor);
}

void Chat::DrawBorder()
{
	v2 vs[] = { { float(Border.x1), float(Border.y1) },
	{ float(Border.x2), float(Border.y1) },
	{ float(Border.x2), float(Border.y2) },
	{ float(Border.x1), float(Border.y2) },
	};

	for (size_t i = 0; i < ArraySize(vs); i++)
		Line(vs[i], vs[(i + 1) % ArraySize(vs)]);

	Line({ float(Border.x1), float(Border.y2 - 2 - nFontHeight) },
	{ float(Border.x2), float(Border.y2 - 2 - nFontHeight) });
}

void Chat::Line(const v2 &v1, const v2 &v2, D3DCOLOR Color, float z)
{
	ScreenSpaceVertex v;

	v.x = v1.x;
	v.y = v1.y;
	v.z = z;
	v.rhw = z;
	v.color = Color;
	Lines.push_back(v);

	v.x = v2.x;
	v.y = v2.y;
	Lines.push_back(v);

	if (Color < 0xFF000000)
	{
		bAlpha = true;
	}

	if (!bBegunDrawing)
		EndDraw();
}

void Chat::Quad(const v2 &v1, const v2 &v2, D3DCOLOR Color, float z)
{
	ScreenSpaceVertex v;

	v.x = v1.x;
	v.y = v1.y;
	v.z = z;
	v.rhw = z;
	v.color = Color;
	Triangles.push_back(v);

	v.x = v2.x;
	v.y = v1.y;
	Triangles.push_back(v);

	v.x = v1.x;
	v.y = v2.y;
	Triangles.push_back(v);

	v.x = v2.x;
	v.y = v2.y;
	Triangles.push_back(v);

	v.x = v1.x;
	v.y = v2.y;
	Triangles.push_back(v);

	v.x = v2.x;
	v.y = v1.y;
	Triangles.push_back(v);

	if (Color < 0xFF000000)
	{
		bAlpha = true;
	}

	if (!bBegunDrawing)
		EndDraw();
}

void Chat::BeginDraw()
{
	bBegunDrawing = true;
}

void Chat::EndDraw()
{
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	if (bAlpha)
	{
		RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		RGetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}

	int nLines = Lines.size() / 2;

	if (nLines)
		RGetDevice()->DrawPrimitiveUP(D3DPT_LINELIST, nLines, Lines.data(), sizeof(ScreenSpaceVertex));

	int nTriangles = Triangles.size() / 3;

	if(nTriangles)
		RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, nTriangles, Triangles.data(), sizeof(ScreenSpaceVertex));

	if (bAlpha)
	{
		RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}

	Lines.clear();
	Triangles.clear();
	
	bAlpha = false;
	bBegunDrawing = false;
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