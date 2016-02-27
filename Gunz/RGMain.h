// Sort this stuff sometime maybe '__'

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

void OnCreateDevice();
HRESULT GenerateTexture(IDirect3DDevice9 *pD3Ddev, IDirect3DTexture9 **ppD3Dtex, DWORD colour32);