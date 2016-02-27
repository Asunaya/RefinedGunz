#include <windows.h>
#include "MDebug.h"
#include "RealSpace2.h"
#include "RMaterialList.h"
#include "RRoam.h"

_USING_NAMESPACE_REALSPACE2

Landscape g_map;
const int size=1024;

unsigned char buffer[size*size+size*2];

RRESULT InitScene(void *param)
{

	InitLog();

	FILE *file=fopen("Height1024.raw","rb");
	fread(buffer+size,size*size,1,file);
	fclose(file);

	// Copy the last row of the height map into the extra first row.
	memcpy( buffer, buffer + size * size, size );

	// Copy the first row of the height map into the extra last row.
	memcpy( buffer + size * size + size, buffer + size, size );

	g_map.Init(buffer);
	RSetCamera(rvector(10,10,100),rvector(0,0,0),rvector(0,0,1));
	RSetProjection(pi/3,0.1f,3000.f);
	return R_OK;
}

RRESULT CloseScene(void *param)
{
	g_map.Destroy();
	return R_OK;
}

#define IsKeyDown(key) ((GetAsyncKeyState(key) & 0x8000)!=0)
#pragma comment(lib,"winmm.lib")

_NAMESPACE_REALSPACE2_BEGIN	
extern float gClipAngle ;
_NAMESPACE_REALSPACE2_END

void Update()
{
static DWORD thistime,lasttime=timeGetTime(),elapsed;

	thistime = timeGetTime();
	elapsed = (thistime - lasttime)*(IsKeyDown(VK_SHIFT)?5:1);
	lasttime = thistime;

	static float rotatez=0.7f,rotatex=2.5f;

	float fRotateStep=elapsed*0.001f;
	float fMoveStep=elapsed*0.1f;


	if(IsKeyDown(VK_LEFT)) rotatez-=fRotateStep;
	if(IsKeyDown(VK_RIGHT)) rotatez+=fRotateStep;
	if(IsKeyDown(VK_UP)) rotatex-=fRotateStep;
	if(IsKeyDown(VK_DOWN)) rotatex+=fRotateStep;

	rvector pos=RCameraPosition,dir=rvector(cosf(rotatez)*sinf(rotatex),sinf(rotatez)*sinf(rotatex),cosf(rotatex));
	D3DXVec3Normalize(&dir,&dir);
	rvector right;
	D3DXVec3Cross(&right,&dir,&rvector(0,0,1));
	D3DXVec3Normalize(&right,&right);

	if(IsKeyDown('W')) pos+=fMoveStep*dir;
	if(IsKeyDown('S')) pos-=fMoveStep*dir;
	if(IsKeyDown('A')) pos+=fMoveStep*right;
	if(IsKeyDown('D')) pos-=fMoveStep*right;
	if(IsKeyDown(VK_SPACE)) pos+=fMoveStep*rvector(0,0,1);

	gClipAngle +=2.1f;
	gClipAngle=fmodf(gClipAngle,180);
	
	rvector at=pos+dir;
	//at.z=0;
	RSetCamera(pos,at,rvector(0,0,1));
//	mlog("%3.3f %3.3f\n",rotatex,rotatez);

}

RRESULT RenderScene(void *param)
{
	Update();

	g_map.Reset();
	g_map.Tessellate();
	g_map.Render();

	return R_OK;
}

int PASCAL WinMain(HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow)
{
	RSetFunction(RF_CREATE,InitScene);
	RSetFunction(RF_RENDER,RenderScene);
	RSetFunction(RF_DESTROY ,CloseScene);
	return RMain("rs2test",this_inst,prev_inst,cmdline,cmdshow);
}
