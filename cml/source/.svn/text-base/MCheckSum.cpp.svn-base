//#define _CHECKSUM_LEVEL	1

#include "stdafx.h"
#include "MCheckSum.h"
#include "MDebug.h"

#ifdef _RELEASEPACK
int	g_nCehckSumBackupCount = 1;
#else
int	g_nCehckSumBackupCount = 5;
#endif

void SetCheckSumBackUpCount(int i)
{
	g_nCehckSumBackupCount = i;
}
int GetCheckSumBackUpCount(void)
{
	return g_nCehckSumBackupCount;
}

bool g_bEnableCheckSum = true;
void EnableCheckSum(bool bEnable)
{
	g_bEnableCheckSum = bEnable;
}

bool IsEnableCheckSum(void)
{
	return g_bEnableCheckSum;
}


MCheckSum::MCheckSum(void)
{
	m_nCurCheckSum = 0;
}

MCheckSum::~MCheckSum(void)
{
	// clear automatically
	Initialize();
}

void MCheckSum::Initialize(void)
{
	while(1){
		list<MSUBCHECKSUM*>::iterator i=begin();
		if(i==end()) break;
		MSUBCHECKSUM* pCheckSu = *i;
		delete pCheckSu;
		erase(i);
	}
	m_nCurCheckSum = 0;
}


MCheckSums::MCheckSums(void)
{
	m_pCurCheckSum = NULL;
}

MCheckSums::~MCheckSums(void)
{
	ClearCheckSum();
}

void MCheckSums::ClearCheckSum(void)
{
	if(m_pCurCheckSum!=NULL){
		delete m_pCurCheckSum;
		m_pCurCheckSum = NULL;
	}
	while(m_CheckSumBackUps.begin()!=m_CheckSumBackUps.end()){
		delete (*m_CheckSumBackUps.begin());
		m_CheckSumBackUps.erase(m_CheckSumBackUps.begin());
	}
}

void MCheckSums::InitCheckSum(void)
{
	if(m_pCurCheckSum!=NULL){	// Backup
		if(GetCheckSumBackUpCount()>0 && m_pCurCheckSum->size()>0){
			m_CheckSumBackUps.insert(m_CheckSumBackUps.end(), m_pCurCheckSum);
			if((int)m_CheckSumBackUps.size()>GetCheckSumBackUpCount()){
				delete (*m_CheckSumBackUps.begin());
				m_CheckSumBackUps.erase(m_CheckSumBackUps.begin());
			}
		}
		else{	// 아무 내용도 없으면, 백업하지 않는다.
			delete m_pCurCheckSum;
		}
	}

	m_pCurCheckSum = new MCheckSum;
}

void MCheckSums::AddCheckSum(int nCheckSum, char* szName, MCheckSumType t)
{
	_ASSERT(m_pCurCheckSum!=NULL);	// InitCheckSum()을 미리 불러서 초기화해줘야 한다.
	MSUBCHECKSUM* pCSL = new MSUBCHECKSUM;
	if(szName==NULL) pCSL->szName[0] = 0;
	else strcpy(pCSL->szName, szName);
	pCSL->nCheckSum = nCheckSum;
	pCSL->nType = t;
	m_pCurCheckSum->insert(m_pCurCheckSum->end(), pCSL);
	m_pCurCheckSum->m_nCurCheckSum += nCheckSum;
}

int MCheckSums::GetCheckSum(void)
{
	_ASSERT(m_pCurCheckSum!=NULL);	// InitCheckSum()을 미리 불러서 초기화해줘야 한다.
	if(m_pCurCheckSum==NULL) return 0;
	return m_pCurCheckSum->m_nCurCheckSum;
}

void MCheckSums::LogSubCheckSum(const char* szCheckSumName, MCheckSum* pCheckSum, int nCheckSum)
{
	mlog("%s = %d\n", szCheckSumName, nCheckSum);
	for(MCheckSum::iterator i=pCheckSum->begin(); i!=pCheckSum->end(); i++){
		MSUBCHECKSUM* pCheckSu = *i;
		if(pCheckSu->nType==MCST_INT) mlog("\t%s = %d", pCheckSu->szName, pCheckSu->nCheckSum);
		else if(pCheckSu->nType==MCST_FLOAT) mlog("\t%s = %f", pCheckSu->szName, *((float *)&(pCheckSu->nCheckSum)));
		mlog(" ( %d )\n", pCheckSu->nCheckSum);
	}
}

void MCheckSums::LogSubCheckSum(const char* szCheckSumName)
{
	mlog("---------------------------------------------------------------------------------------\n");
	int nCount = 0;
	for(list<MCheckSum*>::iterator i=m_CheckSumBackUps.begin(); i!=m_CheckSumBackUps.end(); i++){
		MCheckSum* pCheckSum = *i;
		mlog("CheckSum(%d)\n", nCount-m_CheckSumBackUps.size());
		LogSubCheckSum(szCheckSumName, pCheckSum, pCheckSum->m_nCurCheckSum);
		nCount++;
	}
	mlog("CheckSum(0, Current)\n");
	if(m_pCurCheckSum!=NULL)
		LogSubCheckSum(szCheckSumName, m_pCurCheckSum, m_pCurCheckSum->m_nCurCheckSum);
	mlog("---------------------------------------------------------------------------------------\n");
}
