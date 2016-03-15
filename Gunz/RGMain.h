#pragma once

// Sort this stuff sometime maybe '__'

#include <mutex>
#include <queue>

#define ARGB(a, r, g, b) ((DWORD)D3DCOLOR_ARGB(a, r, g, b))
#define XRGB(r, g, b) ARGB(0xFF, r, g, b)
#define RELWIDTH(x) float(x) / 1920 * RGetScreenWidth()
#define RELHEIGHT(x) float(x) / 1080 * RGetScreenHeight()
#define QPC(t) QueryPerformanceCounter(PLARGE_INTEGER(&t))
#define SAFE_DESTROY(obj) do{ if(obj) obj->Destroy(); } while(false)
#define cprint(a, ...) __noop

#define RG_REPLAY_MAGIC_NUMBER 0x00DEFBAD
#define RG_REPLAY_BINARY_VERSION 1

#define RG_SLASH_RADIUS 320
#define RG_MASSIVE_RADIUS 280

class ZChatCmdManager;
void OnAppCreate();
void OnCreateDevice();
void OnGameDraw();
HRESULT GenerateTexture(IDirect3DDevice9 *pD3Ddev, IDirect3DTexture9 **ppD3Dtex, DWORD colour32);
void LoadRGCommands(ZChatCmdManager &CmdManager);
std::pair<bool, std::string> ReadFile(const char *szPath);

class RGMain
{
public:
	void OnUpdate(double Elapsed);

	// Invokes a callback on the main thread
	template<typename T>
	void Invoke(T Callback)
	{
		std::lock_guard<std::mutex> lock(QueueMutex);
		QueuedInvokations.push(Callback);
	}

private:
	std::queue<std::function<void()>> QueuedInvokations;
	std::mutex QueueMutex;
};

extern RGMain g_RGMain;