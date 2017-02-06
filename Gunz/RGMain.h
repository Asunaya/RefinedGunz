#pragma once

// Sort this stuff sometime maybe '__'

#include <mutex>
#include <queue>
#include <memory>
#include "Config.h"
#include "VoiceChat.h"
#include "Tasks.h"
#include "Hitboxes.h"
#include "ZReplay.h"

class ZChatCmdManager;
bool OnGameInput();
HRESULT GenerateTexture(IDirect3DDevice9 *pD3Ddev, IDirect3DTexture9 **ppD3Dtex, DWORD colour32);
void LoadRGCommands(ZChatCmdManager &CmdManager);
std::pair<bool, std::vector<unsigned char>> ReadMZFile(const char *szPath);
std::pair<bool, std::vector<unsigned char>> ReadZFile(const char *szPath);
void Invoke(std::function<void()> fn);
// 0 = ok, -1 = not found, -2 = too many found
std::pair<int, ZCharacter*> FindSinglePlayer(const char* NameSubstring);
IDirect3DTexture9* HueShiftTexture(IDirect3DTexture9* Tex, float Hue);
HRESULT GenerateMassiveTexture(IDirect3DTexture9 **Tex, D3DCOLOR Color);

class MEvent;
class ZIDLResource;

struct ReplayInfo
{
	ReplayVersion Version;
	REPLAY_STAGE_SETTING_NODE StageSetting;
	std::string VersionString;
	time_t Timestamp = 0;
	bool Dead = true;

	struct PlayerInfo
	{
		char Name[MATCHOBJECT_NAME_LENGTH];
		int Kills = 0;
		int Deaths = 0;
	};

	std::unordered_map<MUID, PlayerInfo> PlayerInfos;
};

class RGMain
{
public:
	RGMain();
	RGMain(const RGMain&) = delete;
	~RGMain();

	void OnUpdate(double Elapsed);
	bool OnEvent(MEvent *pEvent);
	void OnReset();
	void OnInitInterface(ZIDLResource &IDLResource);
	void OnReplaySelected();
	void OnCreateDevice();
	void OnAppCreate();
#ifdef VOICECHAT
	auto MutePlayer(const MUID& UID)
	{
		return m_VoiceChat.MutePlayer(UID);
	}
#endif

	void OnDrawLobby();
	void OnRender();
	void OnGameDraw();
	void OnGameCreate();

	void OnSlash(ZCharacter* Char, const rvector& Pos, const rvector& Dir);
	void OnMassive(ZCharacter* Char, const rvector& Pos, const rvector& Dir);

	void SetSwordColor(const MUID& UID, uint32_t Hue);

	void AddMapBanner(const char* MapName, MBitmap* Bitmap)
	{
		MapBanners.insert({ MapName, Bitmap });
	}

	void OnReceiveVoiceChat(ZCharacter *Char, const uint8_t *Buffer, int Length);

	bool IsCursorEnabled() const;

	double GetTime() const { return Time; }
	double GetElapsedTime() const { return Time - LastTime; }

	std::pair<bool, uint32_t> GetPlayerSwordColor(const MUID& UID);

	// Invokes a callback on the main thread
	template<typename T>
	void Invoke(T Callback, double Delay = 0)
	{
		std::lock_guard<std::mutex> lock(QueueMutex);
		QueuedInvokations.push_back({ Callback, Time + Delay });
	}

private:
	void DrawReplayInfo() const;

	double Time = 0;
	double LastTime = 0;

	struct Invokation
	{
		std::function<void()> fn;
		double Time;
	};

	std::vector<Invokation> QueuedInvokations;
	std::mutex QueueMutex;

	bool Selected = false;
	ReplayInfo SelectedReplayInfo;

	std::unordered_map<std::string, MBitmap *> MapBanners;

	std::unordered_map<MUID, uint32_t> SwordColors;

#ifdef VOICECHAT
	VoiceChat m_VoiceChat;
#endif
	HitboxManager m_HitboxManager;
};

extern std::unique_ptr<RGMain> g_RGMain;

inline ZMyCharacter* MyChar()
{
	return ZGetGame()->m_pMyCharacter;
}

inline auto FixedFOV(float x) {
	return 2.f * atan(tan((2.f * atan(tan(x / 2.f) / (4.f / 3.f))) / 2.f) * RGetAspect());
}