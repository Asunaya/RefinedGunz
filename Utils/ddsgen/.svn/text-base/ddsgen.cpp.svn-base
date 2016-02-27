// ddsgen.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <conio.h>
#include <direct.h>
#include <crtdbg.h>
#include <d3dx9.h>

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")


LPDIRECT3D9				g_pD3D=NULL;
LPDIRECT3DDEVICE9		g_pd3dDevice=NULL;
D3DPRESENT_PARAMETERS	g_d3dpp; 

bool g_bGenMip=true;
bool g_bUndds=false;

bool init_d3d()
{
	HRESULT hr;

	if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
		return false;

	ZeroMemory( &g_d3dpp, sizeof(g_d3dpp) );
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferWidth = 640;
	g_d3dpp.BackBufferHeight = 480;
	g_d3dpp.BackBufferCount = 1;
	g_d3dpp.Windowed   = true;
	g_d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	g_d3dpp.Flags=NULL;

	DWORD BehaviorFlags=D3DCREATE_FPU_PRESERVE | D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	hr = g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,GetDesktopWindow(),BehaviorFlags,&g_d3dpp,&g_pd3dDevice);
	if( FAILED( hr ) )
	{
		g_pD3D->Release();
		return false;
	}

	return true;
}

void close_d3d()
{
	g_pd3dDevice->Release();
	g_pD3D->Release();
}

int nLevel=0;

bool checkandgen(char *name)
{
	int nLength=(int)strlen(name);
	if(nLength<4) return false;

	char *extpos=name+nLength-4;

	if(g_bUndds)
	{
		if(stricmp(extpos,".dds")==0)
		{
			LPDIRECT3DTEXTURE9 pTex;
			if( FAILED(D3DXCreateTextureFromFileEx(

				g_pd3dDevice,name, 
				D3DX_DEFAULT, D3DX_DEFAULT, 
				g_bGenMip ? D3DX_DEFAULT : 1, 
				0, D3DFMT_UNKNOWN,D3DPOOL_SYSTEMMEM,
				D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
				D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
				0, NULL, NULL, &pTex )))
			{
				printf("%s loading fail\n",name);
				return false;
			}

			char tgatexturefile[MAX_PATH];
			strcpy(tgatexturefile,name);
			strcpy(tgatexturefile+nLength-3,"bmp");
			HRESULT hr=D3DXSaveTextureToFile(tgatexturefile,D3DXIFF_BMP,pTex,NULL);
			_ASSERT(hr==D3D_OK);

			pTex->Release();
			return true;
		}
	}
	else
	{
		int nMode = 0;

		D3DFORMAT format = D3DFMT_DXT3;

		if( stricmp(extpos,".bmp")==0 ) {
			nMode = 1;
			format = D3DFMT_DXT1;
		}
		else if( stricmp(extpos,".tga")==0 ) nMode = 2;
		else if( stricmp(extpos,".png")==0 ) nMode = 3;

		if( nMode )
		{
			LPDIRECT3DTEXTURE9 pTex;
			if( FAILED(D3DXCreateTextureFromFileEx(

				g_pd3dDevice,name, 
				D3DX_DEFAULT, D3DX_DEFAULT, 
				g_bGenMip ? D3DX_DEFAULT : 1, 
				0, format,D3DPOOL_SYSTEMMEM,
				D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
				D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
				0, NULL, NULL, &pTex )))
			{
				printf("%s loading fail\n",name);
				return false;
			}

			char ddstexturefile[MAX_PATH];
			sprintf(ddstexturefile,"%s.dds",name);
			D3DXSaveTextureToFile(ddstexturefile,D3DXIFF_DDS,pTex,NULL);
		
			pTex->Release();
			return true;
		}
	}

	return false;
}

void gen_dds(const char *szPath)
{
	nLevel++;
	WIN32_FIND_DATA FileData; 
	HANDLE hSearch; 

	BOOL fFinished = FALSE; 

	char filetofind[MAX_PATH];
	sprintf(filetofind,"%s/*.*",szPath);

	hSearch = FindFirstFile(filetofind, &FileData); 
	if (hSearch == INVALID_HANDLE_VALUE) 
	{ 
		nLevel--;
		return;
	} 

	while (!fFinished) 
	{ 
		char childpath[MAX_PATH];
		sprintf(childpath,"%s/%s",szPath,FileData.cFileName);

		if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
			strcmp(".",FileData.cFileName)!=0 &&
			strcmp("..",FileData.cFileName)!=0)
		{
//			chdir(FileData.cFileName);
			gen_dds(childpath);
//			chdir("..");
		}

		if(checkandgen(childpath))
		{
			for(int i=0;i<nLevel;i++)
				printf("   ");
			printf("%s\n",FileData.cFileName);
		}

		if (!FindNextFile(hSearch, &FileData)) 
		{
			if (GetLastError() == ERROR_NO_MORE_FILES) 
			{ 
				fFinished = TRUE; 
			} 
			else 
			{ 
				printf("Couldn't find next file."); 
				nLevel--;
				return;
			} 
		}
	} 

	nLevel--;
}

int _tmain(int argc, _TCHAR* argv[])
{
	char szPath[MAX_PATH]=".";

	if(argc>1)
	{
		for(int i=1;i<argc;i++)
		{
			if(stricmp("-r",argv[i])==0)
				g_bUndds=true;
			else
			if(stricmp("-nomip",argv[i])==0)
			{
				g_bGenMip=false;
			}else
			if(stricmp("-path",argv[i])==0)
			{
				if(argc<=i+1)
					goto usage;
				strcpy(szPath,argv[i+1]);
				i++;
			}else
			goto usage;
		}
	}

	if(!init_d3d()) return 0;
	printf("d3d init ok."); 

	gen_dds(szPath);

	close_d3d();

	return 0;

usage:
	printf(" usage : ddsgen [-nomip] [-path TARGETPATH]\n :) thanx");
	return 0;
}

