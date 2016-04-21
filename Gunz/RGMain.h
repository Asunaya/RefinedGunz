#pragma once

// Sort this stuff sometime maybe '__'

#include <mutex>
#include <queue>
#include <memory>
#include "VoiceChat.h"
#include "Tasks.h"
#include "MeshManager.h"
#include "Hitboxes.h"

class ZChatCmdManager;
bool OnGameInput();
HRESULT GenerateTexture(IDirect3DDevice9 *pD3Ddev, IDirect3DTexture9 **ppD3Dtex, DWORD colour32);
void LoadRGCommands(ZChatCmdManager &CmdManager);
std::pair<bool, std::vector<unsigned char>> ReadMZFile(const char *szPath);
std::pair<bool, std::vector<unsigned char>> ReadZFile(const char *szPath);
void Invoke(std::function<void()> fn);
// 0 = ok, -1 = not found, -2 = too many found
std::pair<int, ZCharacter*> FindSinglePlayer(const char* NameSubstring);

HRESULT GenerateMassiveTexture(IDirect3DTexture9 **Tex, D3DCOLOR Color);

class MEvent;
class ZIDLResource;

struct ReplayInfo
{
	ReplayVersion Version;
	REPLAY_STAGE_SETTING_NODE StageSetting;
	std::string VersionString;

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

	void OnUpdate(double Elapsed);
	bool OnEvent(MEvent *pEvent);
	void OnReset();
	void OnInitInterface(ZIDLResource &IDLResource);
	void OnReplaySelected();
	void OnCreateDevice();
	void OnAppCreate();
	decltype(auto) MutePlayer(const MUID& UID)
	{
		return m_VoiceChat.MutePlayer(UID);
	}

	void OnDrawLobby();
	void OnRender();
	void OnGameDraw();
	void OnGameCreate();

	void OnSlash(ZCharacter* Char, const D3DXVECTOR3& Pos, const D3DXVECTOR3& Dir);
	void OnMassive(ZCharacter* Char, const D3DXVECTOR3& Pos, const D3DXVECTOR3& Dir);

	void SetSwordColor(const MUID& UID, uint32_t Hue);

	void AddMapBanner(const char* MapName, MBitmap* Bitmap)
	{
		MapBanners.insert({ MapName, Bitmap });
	}

	void OnReceiveVoiceChat(ZCharacter *Char, const uint8_t *Buffer, int Length);

	void OnDestroyObject(void* Obj)
	{
		if (!m_MeshManager)
			return;

		m_MeshManager->OnDestroyObject(Obj);
	}

	void ReleaseMeshNode(RMeshNode* Node)
	{
		if (!m_MeshManager)
			return;

		m_MeshManager->Release(Node);
	}

	void GetMeshNode(const char *MeshName, const char *NodeName, void* Obj, std::function<void(RMeshNode*)> Callback)
	{
		m_MeshManager->GetAsync(MeshName, NodeName, Obj, Callback);
	}

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

	VoiceChat m_VoiceChat;
	TaskManager m_TaskManager;
	std::unique_ptr<MeshManager> m_MeshManager = nullptr;
	HitboxManager m_HitboxManager;
};

extern RGMain* g_RGMain;

struct rgb
{
	double r;       // percent
	double g;       // percent
	double b;       // percent
};

struct hsv
{
	double h;       // angle in degrees
	double s;       // percent
	double v;       // percent
};

hsv   rgb2hsv(rgb in);
rgb   hsv2rgb(hsv in);

inline hsv rgb2hsv(rgb in)
{
	hsv         out;
	double      min, max, delta;

	min = in.r < in.g ? in.r : in.g;
	min = min  < in.b ? min : in.b;

	max = in.r > in.g ? in.r : in.g;
	max = max  > in.b ? max : in.b;

	out.v = max;                                // v
	delta = max - min;
	if (delta < 0.00001)
	{
		out.s = 0;
		out.h = 0; // undefined, maybe nan?
		return out;
	}
	if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
		out.s = (delta / max);                  // s
	}
	else {
		// if max is 0, then r = g = b = 0              
		// s = 0, v is undefined
		out.s = 0.0;
		out.h = NAN;                            // its now undefined
		return out;
	}
	if (in.r >= max)                           // > is bogus, just keeps compilor happy
		out.h = (in.g - in.b) / delta;        // between yellow & magenta
	else
		if (in.g >= max)
			out.h = 2.0 + (in.b - in.r) / delta;  // between cyan & yellow
		else
			out.h = 4.0 + (in.r - in.g) / delta;  // between magenta & cyan

	out.h *= 60.0;                              // degrees

	if (out.h < 0.0)
		out.h += 360.0;

	return out;
}


inline rgb hsv2rgb(hsv in)
{
	double      hh, p, q, t, ff;
	long        i;
	rgb         out;

	if (in.s <= 0.0) {       // < is bogus, just shuts up warnings
		out.r = in.v;
		out.g = in.v;
		out.b = in.v;
		return out;
	}
	hh = in.h;
	if (hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	i = (long)hh;
	ff = hh - i;
	p = in.v * (1.0 - in.s);
	q = in.v * (1.0 - (in.s * ff));
	t = in.v * (1.0 - (in.s * (1.0 - ff)));

	switch (i) {
	case 0:
		out.r = in.v;
		out.g = t;
		out.b = p;
		break;
	case 1:
		out.r = q;
		out.g = in.v;
		out.b = p;
		break;
	case 2:
		out.r = p;
		out.g = in.v;
		out.b = t;
		break;

	case 3:
		out.r = p;
		out.g = q;
		out.b = in.v;
		break;
	case 4:
		out.r = t;
		out.g = p;
		out.b = in.v;
		break;
	case 5:
	default:
		out.r = in.v;
		out.g = p;
		out.b = q;
		break;
	}
	return out;
}

inline ZMyCharacter* MyChar()
{
	return ZGetGame()->m_pMyCharacter;
}

IDirect3DTexture9* HueShiftTexture(IDirect3DTexture9* Tex, float Hue);