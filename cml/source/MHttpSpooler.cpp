#include "stdafx.h"
#include "MHttpSpooler.h"


void MHttpSpooler::OnCreate()
{
}

void MHttpSpooler::OnDestroy()
{
	Shutdown();
}

void MHttpSpooler::Run()
{
	while(CheckShutdown() == false) {
		MHttpSpoolerNode* pNode = m_RequestQueue.Pop();
		if (pNode) {
			if (m_AsyncHttp.Get(pNode->GetURL().c_str()))
				m_ResultQueue.Post(pNode);
			else
				delete pNode;
		}
		Sleep(100);
	}
}
