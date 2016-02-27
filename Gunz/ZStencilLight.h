#pragma	 once

#include "RTypes.h"
#include "RBaseTexture.h"
#include "RMeshUtil.h"
#include "map"
#include "MemPool.h"

using namespace std;

struct LightSource : public CMemPoolSm<LightSource>
{
	bool bAttenuation;
	rvector pos;
	float power; /* 0~1 */
	DWORD attenuationTime;
	DWORD deadTime;
};

class ZStencilLight
{
protected:
	rvector m_Position;
	float m_Radius;
	LPD3DXMESH m_pMesh;
	RealSpace2::RBaseTexture* m_pTex;
	RTLVertex m_VBuffer[4];
	int m_id;
	map<int, LightSource*> m_LightSource;

protected:
	static ZStencilLight m_instance;
    
public:
	void Destroy();

	void PreRender();
	void RenderStencil();
	void RenderStencil(rvector& p, float radius);
	void RenderLight();	
	void PostRender();

	void Render();

	void Update();
	
	void SetPosition( const rvector& p) { m_Position = p; }
	void SetRadius( const float r )	{ m_Radius = r; }
	
	int AddLightSource( rvector& p, float power );
	int AddLightSource( rvector& p, float power, DWORD lastTime );
	bool SetLightSourcePosition( int id, rvector& p );
	bool DeleteLightSource( int id );
	bool DeleteLightSource( int id, DWORD lastTime );

	static ZStencilLight* GetInstance() { return &m_instance;}

	size_t GetCount() { return m_LightSource.size(); }

public:
	ZStencilLight();
	~ZStencilLight();
};

ZStencilLight* ZGetStencilLight();