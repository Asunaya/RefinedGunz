#ifndef __RSCAMERALIST_H
#define __RSCAMERALIST_H

#include <stdio.h>
#include "rutils.h"

struct RSCameraState {
	rvector dir,up,pos;
};

struct RSCamera {
	char *name;
	float m_fov;
	RSCameraState *m_States;
};

class RSCameraList {
public:
	RSCameraList();
	virtual ~RSCameraList();

	bool Open(FILE *file);
	RSCameraState *Get(int i,int nFrame);
	char *GetName(int i) { return m_Cameras[i].name; }
	float GetFOV(int i) { return m_Cameras[i].m_fov; }
	int GetCameraCount() { return m_nCamera; }
	int GetFrameCount() { return m_nFrames; }

private:
	int m_nCamera,m_nFrames;
	RSCamera *m_Cameras;
};

#endif