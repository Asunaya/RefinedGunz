#include "RSCameraList.h"

RSCameraList::RSCameraList()
{
	m_nCamera=0;
	m_nFrames=0;
	m_Cameras=NULL;
}

RSCameraList::~RSCameraList()
{
	int i;

	if(m_Cameras)
	{
		for(i=0;i<m_nCamera;i++)
		{
			delete m_Cameras[i].name;
			delete m_Cameras[i].m_States;
		}
	}
	delete m_Cameras;
}

bool RSCameraList::Open(FILE *file)
{
	fread(&m_nCamera,sizeof(int),1,file);
	fread(&m_nFrames,sizeof(int),1,file);
	
	m_Cameras=new RSCamera[m_nCamera];
	for(int i=0;i<m_nCamera;i++)
	{
		unsigned char ns=0;
		fread(&ns,sizeof(unsigned char),1,file);
		m_Cameras[i].name=new char[ns+1];
		fread(m_Cameras[i].name,ns,1,file);m_Cameras[i].name[ns]=0;
		fread(&m_Cameras[i].m_fov,sizeof(float),1,file);
		m_Cameras[i].m_States=new RSCameraState[m_nFrames];
		fread(m_Cameras[i].m_States,sizeof(RSCameraState),m_nFrames,file);
	}

	return true;
}

RSCameraState *RSCameraList::Get(int i,int nFrame)
{
	return &(m_Cameras[i].m_States[nFrame]);
}