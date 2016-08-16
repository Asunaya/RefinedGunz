#include "stdafx.h"
#include "MMatchStringResManager.h"

MMatchStringResManager::MMatchStringResManager() : MBaseStringResManager()
{

}

MMatchStringResManager::~MMatchStringResManager()
{

}

void MMatchStringResManager::MakeInstance()
{
	_ASSERT(m_pInstance == NULL);
	m_pInstance = new MMatchStringResManager();
}
