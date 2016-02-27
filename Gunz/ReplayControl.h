#pragma once
#include "MEvent.h"

class ReplayControl
{
public:
	void Draw();
	bool OnEvent(MEvent *pEvent);
private:
};

extern ReplayControl g_ReplayControl;