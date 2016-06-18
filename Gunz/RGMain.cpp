#include "stdafx.h"
#include "RGMain.h"
#include "Portal.h"
#include "NewChat.h"
#include "ZConfiguration.h"
#include "Hitboxes.h"
#include "Draw.h"
#include "ZRule.h"
#include "ZRuleSkillmap.h"
#include "VoiceChat.h"
#include "ReplayControl.h"
#include "FileInfo.h"
#include "ZReplay.inl"
#include "ZInput.h"

RGMain *g_RGMain = nullptr;

void RGMain::OnAppCreate()
{
	ZRuleSkillmap::CourseMgr.Init();

#ifdef PORTAL
	g_pPortal = new Portal();
#endif
}

void RGMain::OnCreateDevice()
{
	g_Chat.Create("Arial", 16);
	g_Chat.SetBackgroundColor(ZGetConfiguration()->GetChatBackgroundColor());

	g_Draw.OnCreateDevice();

#ifdef VOICECHAT
	m_VoiceChat.OnCreateDevice();
#endif

	m_HitboxManager.Create();
}

void RGMain::OnGameDraw()
{
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SKILLMAP)
		((ZRuleSkillmap *)ZGetGame()->GetMatch()->GetRule())->Draw();

	if(ZGetConfiguration()->GetShowHitboxes())
		m_HitboxManager.Draw();

#ifdef VOICECHAT
	m_VoiceChat.Draw();
#endif

	/*for (auto pair : *ZGetCharacterManager())
	{
		auto Char = pair.second;

		if (Char == ZGetGame()->m_pMyCharacter)
			continue;

		if (ZGetGame()->CheckWall(Char, ZGetGame()->m_pMyCharacter))
		{
			g_Draw.Text("Evading", 10, 500);
		}
		else
		{
			g_Draw.Text("Not evading", 10, 500);
		}
	}*/

	for (auto pair : *ZGetCharacterManager())
	{
		auto& Char = *pair.second;

		auto GetItemDesc = [&](MMatchCharItemParts slot)
		{
			return Char.GetItems()->GetDesc(slot);
		};

		auto Time = ZGetGame()->GetTime();

		auto Observer = ZGetGameInterface()->GetCombatInterface()->GetObserver();
		if (Observer->IsVisible())
			Time -= Observer->GetDelay();

		v3 Head, Foot;
		Char.BasicInfoHistory.GetPositions(&Head, &Foot, nullptr,
			Time,
			GetItemDesc, (MMatchSex)!Char.IsMan(), Char.IsDie());

		v3 OldHead = Head;
		v3 OldFoot = Foot;

		RGetDevice()->SetTexture(0, NULL);
		RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
		RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		RGetDevice()->SetRenderState(D3DRS_ZENABLE, FALSE);

		RDrawSphere(Head, 15, 20);
		RDrawLine(Head, Foot, 0xFFFF0000);

		Head = Char.m_pVMesh->GetHeadPosition();
		Foot = Char.GetPosition();
		RDrawSphere(Head, 15, 20, 0xFF0000FF);
		RDrawLine(Head, Foot, 0xFF0000FF);

		DMLog("Time: %f, diff: %f, %f\n", Time, Magnitude(Head - OldHead), Magnitude(Foot - OldFoot));

		RGetDevice()->SetRenderState(D3DRS_ZENABLE, TRUE);
	}
}

bool OnGameInput()
{
	if (g_Chat.IsInputEnabled())
	{
		g_Chat.OnUpdate();
		return true;
	}

	return false;
}

HRESULT GenerateTexture(IDirect3DDevice9 *pD3Ddev, IDirect3DTexture9 **ppD3Dtex, DWORD colour32){
	if (pD3Ddev->CreateTexture(8, 8, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, ppD3Dtex, NULL))
		return E_FAIL;

	WORD colour16 = ((WORD)((colour32 >> 28) & 0xF) << 12) | (WORD)(((colour32 >> 20) & 0xF) << 8) | (WORD)(((colour32 >> 12) & 0xF) << 4) | (WORD)(((colour32 >> 4) & 0xF) << 0);

	D3DLOCKED_RECT d3dlr;
	(*ppD3Dtex)->LockRect(0, &d3dlr, 0, 0);
	WORD *pDst16 = (WORD*)d3dlr.pBits;

	for (int xy = 0; xy < 8 * 8; xy++)
		*pDst16++ = colour16;

	(*ppD3Dtex)->UnlockRect(0);

	return S_OK;
}

std::pair<bool, std::vector<unsigned char>> ReadMZFile(const char *szPath)
{
	MZFile File;

	if (!File.Open(szPath, ZApplication::GetFileSystem()))
	{
		return{ false, {} };
	}

	int FileLength = File.GetLength();

	if (FileLength <= 0)
	{
		return{ false, {} };
	}

	std::vector<unsigned char> InflatedFile;
	InflatedFile.resize(FileLength);

	File.Read(&InflatedFile[0], FileLength);

	return { true, InflatedFile };
}

std::pair<bool, std::vector<unsigned char>> ReadZFile(const char *szPath)
{
	ZFile File;

	if (!File.Open(szPath))
	{
		return{ false,{} };
	}

	std::vector<unsigned char> InflatedFile;
	int FileLength = 0;
	int ret = 0;

	do
	{
		InflatedFile.resize(FileLength + 1024);

		ret = File.Read(&InflatedFile[FileLength], 1024);

		FileLength += 1024;
	} while (ret == 1024);

	FileLength -= 1024 - ret;
	InflatedFile.resize(FileLength);

	return{ true, InflatedFile };
}

//void Invoke(std::function<void()> fn)
//{
//	g_RGMain->Invoke(fn);
//}

std::pair<int, ZCharacter*> FindSinglePlayer(const char * NameSubstring)
{
	bool Found = false;
	ZCharacter* FoundChar = nullptr;

	for (auto Item : *ZGetCharacterManager())
	{
		auto Char = Item.second;

		if (!strstr(Char->GetUserNameA(), NameSubstring))
			continue;

		if (Found)
			return{ -2, nullptr };

		Found = true;
		FoundChar = Char;
	}

	if (Found)
		return{ 0, FoundChar };

	return{ -1, nullptr };
}

RGMain::RGMain()
{
}

void RGMain::OnUpdate(double Elapsed)
{
	LastTime = Time;

	Time += Elapsed;

	std::lock_guard<std::mutex> lock(QueueMutex);

	/*while (QueuedInvokations.size())
	{
		QueuedInvokations.front()();
		QueuedInvokations.pop();
	}*/

	for (auto it = QueuedInvokations.begin(); it != QueuedInvokations.end(); it++)
	{
		auto item = *it;

		if (Time < item.Time)
			continue;

		item.fn();

		it = QueuedInvokations.erase(it);

		if (it == QueuedInvokations.end())
			break;
	}
}

bool RGMain::OnEvent(MEvent *pEvent)
{
	g_Chat.OnEvent(pEvent);

	if (g_Chat.IsInputEnabled())
	{
		if (ZGetGame()->IsReplay())
			g_ReplayControl.OnEvent(pEvent);

		return true;
	}

	bool ret = false;

#ifdef VOICECHAT
	static bool LastState = false;
	bool CurState = ZIsActionKeyPressed(ZACTION_VOICE_CHAT);

	ret = [&]
	{
		if (CurState && !LastState)
		{
			m_VoiceChat.StartRecording();

			return true;
		}
		else if (!CurState && LastState)
		{
			m_VoiceChat.StopRecording();

			return true;
		}

		return false;
	}();

	LastState = CurState;
#endif

#ifdef PORTAL
	g_pPortal->OnShot();
#endif

	return ret;
}

void RGMain::OnReset()
{
	g_Draw.OnReset();
}

void RGMain::OnInitInterface(ZIDLResource &IDLResource)
{
}

void RGMain::OnDrawLobby()
{
}

void RGMain::OnReplaySelected()
{
	SelectedReplayInfo.PlayerInfos.clear();

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	auto ReplayWidget = pResource->FindWidget("Replay");

	if (!ReplayWidget->IsVisible())
		return;

	auto ReplayFileList = (MListBox *)pResource->FindWidget("Replay_FileList");

	auto SelectedReplay = ReplayFileList->GetSelItem();

	if (!SelectedReplay)
		return;

	TCHAR Path[MAX_PATH];
	if (!GetMyDocumentsPath(Path))
	{
		return;
	}

	strcat(Path, GUNZ_FOLDER);
	strcat(Path, REPLAY_FOLDER);

	strcat(Path, "/");
	strcat(Path, SelectedReplay->GetString());

	auto PerCommand = [&](MCommand *Command, float Time)
	{
		switch (Command->GetID())
		{
		case MC_MATCH_RESPONSE_PEERLIST:
		{
			auto Param = Command->GetParameter(1);

			if (!Param || Param->GetType() != MPT_BLOB)
				break;

			auto pBlob = Param->GetPointer();

			int Size = MGetBlobArrayCount(pBlob);

			for (int i = 0; i < Size; i++) {
				MTD_PeerListNode* pNode = (MTD_PeerListNode*)MGetBlobArrayElement(pBlob, i);

				auto it = SelectedReplayInfo.PlayerInfos.find(pNode->uidChar);

				if (it != SelectedReplayInfo.PlayerInfos.end())
					continue;

				SelectedReplayInfo.PlayerInfos.insert({ pNode->uidChar, {} });
			}
		}
		break;
		case MC_PEER_DIE:
		{
			MUID Attacker;
			if (!Command->GetParameter(&Attacker, 0, MPT_UID))
				break;

			auto it = SelectedReplayInfo.PlayerInfos.find(Attacker);

			if (it == SelectedReplayInfo.PlayerInfos.end())
				break;

			it->second.Kills++;

			it = SelectedReplayInfo.PlayerInfos.find(Command->GetSenderUID());

			if (it == SelectedReplayInfo.PlayerInfos.end())
				break;

			it->second.Deaths++;
		}
		break;
		};
	};

	try
	{
		ZReplayLoader Loader;
		Loader.LoadFile(Path);
		SelectedReplayInfo.Version = Loader.GetVersion();
		SelectedReplayInfo.Timestamp = Loader.GetTimestamp();
		Loader.GetStageSetting(SelectedReplayInfo.StageSetting);
		Loader.GetDuelQueueInfo();
		auto InitialCharInfos = Loader.GetCharInfo();

		for (const auto &CharInfo : InitialCharInfos)
		{
			ReplayInfo::PlayerInfo Player;
			strcpy_safe(Player.Name, CharInfo.Info.szName);
			SelectedReplayInfo.PlayerInfos.insert({ CharInfo.State.UID, Player });
		}

		uint32_t WantedCommands[] = { MC_MATCH_RESPONSE_PEERLIST, MC_PEER_DIE };

		Loader.GetCommands(PerCommand, false, &ArrayView<u32>(WantedCommands));

		SelectedReplayInfo.VersionString = SelectedReplayInfo.Version.GetVersionString();
		SelectedReplayInfo.Dead = false;
	}
	catch (EOFException& e)
	{
		MLog("Unexpected EOF while reading replay %s at position %d\n", SelectedReplay->GetString(), e.GetPosition());
		SelectedReplayInfo.Dead = true;
	}
	catch (...)
	{
		MLog("Something went wrong while reading replay %s\n", SelectedReplay->GetString());
		SelectedReplayInfo.Dead = true;
	}
}

void RGMain::DrawReplayInfo() const
{
	extern MFontR2 *g_pDefFont;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	auto ReplayWidget = pResource->FindWidget("Replay");

	if (!ReplayWidget->IsVisible())
		return;

	auto ReplayFileList = (MListBox *)pResource->FindWidget("Replay_FileList");

	if (!ReplayFileList->GetSelItem())
		return;

	auto ReplayGroupWidget = pResource->FindWidget("ReplayGroup");

	auto Offset = ReplayGroupWidget->GetPosition();

	//MLog("Offset: %d, %d\n", Offset.x, Offset.y);

	Offset.x += 310;
	Offset.y += 50;

	auto Print = [&](const char *Format, ...)
	{
		char buf[256];

		va_list args;

		va_start(args, Format);
		int ret = _vsnprintf_s(buf, sizeof(buf) - 1, Format, args);
		va_end(args);

		g_pDefFont->m_Font.DrawTextA(Offset.x, Offset.y, buf);

		Offset.y += 12;
	};

	if (SelectedReplayInfo.Dead)
	{
		Print("Failed to load replay");
		return;
	}

	[&]
	{
		auto BannerName = MGetBannerName(SelectedReplayInfo.StageSetting.szMapName);

		if (!BannerName)
			return;

		auto it = MapBanners.find(BannerName);

		if (it == MapBanners.end())
			return;

		auto Bitmap = it->second;
		((MBitmapR2 *)Bitmap->GetSourceBitmap())->Draw(Offset.x, Offset.y, 260, 30, Bitmap->GetX(), Bitmap->GetY(), Bitmap->GetWidth(), Bitmap->GetHeight(), 0xFFFFFFFF);

		Offset.y += 30;
	}();

	Print("%s", SelectedReplayInfo.VersionString.c_str());

	[&]()
	{
		if (SelectedReplayInfo.Timestamp == 0)
			return;

		tm Tm;
		auto err = localtime_s(&Tm, &SelectedReplayInfo.Timestamp);
		if (err != 0)
			return;

		char buf[64];
		strftime(buf, sizeof(buf), "%x", &Tm);

		Print("%s", buf);
	}();

	Print("Map: %s", SelectedReplayInfo.StageSetting.szMapName);

	if(SelectedReplayInfo.StageSetting.szStageName[0])
		Print("Stage name: %s", SelectedReplayInfo.StageSetting.szStageName);

	Print("Gametype: %s", ZGetGameInterface()->GetGameTypeManager()->GetGameTypeStr(SelectedReplayInfo.StageSetting.nGameType));

	for (auto &Item : SelectedReplayInfo.PlayerInfos)
	{
		auto& Player = Item.second;

		Print("%s - %d/%d", Player.Name, Player.Kills, Player.Deaths);
	}
}

void RGMain::OnRender()
{
	DrawReplayInfo();
}

std::pair<bool, uint32_t> RGMain::GetPlayerSwordColor(const MUID& UID)
{
	auto it = SwordColors.find(UID);

	if (it == SwordColors.end())
		return{ false, 0 };

	return{ true, it->second };
}

HRESULT GenerateMassiveTexture(IDirect3DTexture9 **Tex, D3DCOLOR Color)
{
	return S_OK;
}

#include "dxerr9.h"

#define DXERR(func) DXErr(func, __func__, #func)

inline bool DXErr(HRESULT hr, const char* CallingFunction, const char* DXFunction)
{
	if (SUCCEEDED(hr))
		return false;

	MLog("In %s, %s failed -- error code: %s, description: %s\n", CallingFunction, DXFunction, DXGetErrorString9(hr), DXGetErrorDescription9(hr));

	return true;
}

IDirect3DTexture9* HueShiftTexture(IDirect3DTexture9* Tex, float Hue)
{
	int Levels = Tex->GetLevelCount();

	D3DSURFACE_DESC Desc;
	Tex->GetLevelDesc(0, &Desc);

	IDirect3DTexture9 *NewTex;
	RGetDevice()->CreateTexture(Desc.Width, Desc.Height, Levels, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &NewTex, nullptr);

	int Level = 0;

	for (int Level = 0; Level < Levels; Level++)
	{
		Tex->GetLevelDesc(Level, &Desc);

		D3DPtr<IDirect3DSurface9> OrigSurface = nullptr;
		Tex->GetSurfaceLevel(Level, &OrigSurface.ptr);

		D3DPtr<IDirect3DSurface9> NewSurface = nullptr;
		NewTex->GetSurfaceLevel(Level, &NewSurface.ptr);

		if (FAILED(D3DXLoadSurfaceFromSurface(NewSurface, nullptr, nullptr, OrigSurface, nullptr, nullptr, D3DX_FILTER_NONE, 0)))
		{
			MLog("Failed to load surface from material\n");
			return nullptr;
		}

		D3DLOCKED_RECT Rect;
		auto hr = NewSurface->LockRect(&Rect, nullptr, 0);

		if (FAILED(hr))
		{
			MLog("Failed to lock texture\n");
			return nullptr;
		}

		for (UINT j = 0; j < Desc.Height; j++)
		{
			for (int i = 0; i < Rect.Pitch; i += 4)
			{
				uint32_t& Pixel = *(uint32_t *)(PBYTE(Rect.pBits) + i + j * Rect.Pitch);
				rgb color;
				color.r = ((Pixel & 0x0000FF00) >> 8) / 2.55;
				color.g = ((Pixel & 0x00FF0000) >> 16) / 2.55;
				color.b = ((Pixel & 0xFF000000) >> 24) / 2.55;
				hsv HSV = rgb2hsv(color);
				HSV.h += fmod(Hue, 360.0);
				color = hsv2rgb(HSV);
				Pixel = (int(color.r * 2.55) << 8) | (int(color.g * 2.55) << 16) | (int(color.b * 2.55) << 24) | (Pixel & 0xFF);
			}
		}

		hr = NewSurface->UnlockRect();

		if (FAILED(hr))
		{
			MLog("Failed to unlock texture\n");
			return false;
		}
	}

	return NewTex;
}

void RGMain::SetSwordColor(const MUID& UID, uint32_t Color)
{
	SwordColors[UID] = Color;

	auto Char = ZGetCharacterManager()->Find(UID);

	if (!Char)
		return;

	Char->m_pVMesh->SetCustomColor(Color & 0x80FFFFFF, Color & 0x0FFFFFFF);
}

void RGMain::OnReceiveVoiceChat(ZCharacter *Char, const uint8_t *Buffer, int Length)
{
#ifdef VOICECHAT
	m_VoiceChat.OnReceiveVoiceChat(Char, Buffer, Length);
#endif
}

void RGMain::OnGameCreate()
{
	g_Rules.OnGameCreate();
}

void RGMain::OnSlash(ZCharacter * Char, const D3DXVECTOR3 & Pos, const D3DXVECTOR3 & Dir)
{
	m_HitboxManager.OnSlash(Pos, Dir);
}

void RGMain::OnMassive(ZCharacter * Char, const D3DXVECTOR3 & Pos, const D3DXVECTOR3 & Dir)
{
	m_HitboxManager.OnMassive(Pos);
}

bool RGMain::IsCursorEnabled() const
{
	return g_Chat.IsInputEnabled();
}