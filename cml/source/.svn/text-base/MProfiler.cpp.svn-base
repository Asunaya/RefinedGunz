#include "stdafx.h"
#include "MProfiler.h"
#include <windows.h>
#include <mmsystem.h>
#include <crtdbg.h>
#include <time.h>


MProfiler	g_DefaultProfiler;	// Default Global Profiler



MProfileStack::~MProfileStack(void)
{
	while(empty()==false){
		MPROFILEITEM* pProfileItem = top();
		delete pProfileItem;
		pop();
	}
}


void MProfileLoop::AddProfile(MPROFILELOG* pPL)
{
	// 맨 뒤에서 부터 같은 Depth와 같은 이름을 가진 로그를 찾아 증가시키거나 없으면 새로 생성
	for(MProfileLoop::reverse_iterator i=rbegin(); i!=rend(); i++){
		MPROFILELOG* pLog = *i;

		if(pLog->nDepth==pPL->nDepth && strcmp(pLog->szName, pPL->szName)==0){
			pLog->nCount++;
			pLog->nMaxTime = max(pLog->nMaxTime, pPL->nMaxTime);
			pLog->nMinTime = min(pLog->nMinTime, pPL->nMinTime);
			pLog->nTotalTime += pPL->nTotalTime;
			delete pPL;
			return;
		}
	}

	/*
	for(MProfileLoop::reverse_iterator ri=rbegin(); ri!=rend(); ri++){
		MPROFILELOG* pLog = *ri;
		if(pLog->nDepth==pPL->nDepth){
			pPL->nCount = 1;
			MProfileLoop::iterator i(ri.base());
			insert(i, pPL);
			return;
		}
	}
	*/

	pPL->nCount = 1;
	insert(begin(), pPL);
}
/*
void MProfileLoop::AddProfile(char* szName, int nDepth)
{
	// 중복된 Item은 리스트에 마지막에만 나타난다.
	MPROFILELOG* pLog = NULL;
	if(rbegin()!=rend()) pLog = *rbegin();

	if(pLog==NULL || strcmp(pLog->szName, szName)!=0){
		MPROFILELOG* pPL = new MPROFILELOG;
		strcpy(pPL->szName, szName);
		pPL->nCount = 0;
		pPL->nDepth = nDepth;
		pPL->nMaxTime = -1;
		pPL->nMinTime = -1;
		pPL->nTotalTime = -1;
		insert(end(), pPL);
	}
}

void MProfileLoop::SetProfile(int nTime)
{
	MPROFILELOG* pProfileLog = NULL;
	if(rbegin()!=rend()) pProfileLog = *rbegin();

	if(pProfileLog->nCount==0){	// 초기 생성되는 Profile인 경우
		pProfileLog->nCount = 1;
		pProfileLog->nMaxTime = nTime;
		pProfileLog->nMinTime = nTime;
		pProfileLog->nTotalTime = nTime;
	}
	else{	// 누적되는 Profile
		pProfileLog->nCount++;
		pProfileLog->nMaxTime = max(pProfileLog->nMaxTime, nTime);
		pProfileLog->nMinTime = min(pProfileLog->nMinTime, nTime);
		pProfileLog->nTotalTime += nTime;
	}
}
*/

int MProfileLoop::GetTotalTime(void)
{
	int nMinDepth = 9999;
	int nTotalTime = 0;

	for(MProfileLoop::iterator i=begin(); i!=end(); i++){
		MPROFILELOG* pLog = *i;
		nMinDepth = min(pLog->nDepth, nMinDepth);
	}

	for(i=begin(); i!=end(); i++){
		MPROFILELOG* pLog = *i;
		if(pLog->nDepth==nMinDepth){
			if(pLog->nTotalTime==-1){
				nMinDepth++;
				nTotalTime = 0;
				continue;
			}
			nTotalTime += pLog->nTotalTime;
		}
	}

	return nTotalTime;
}


MProfiler::MProfiler(void)
{
	m_pOneLoopProfile = NULL;
	m_bEnableOneLoopProfile = false;
	m_szFirstProfileName = NULL;
	m_pOneLoopProfileResult = NULL;
}
MProfiler::~MProfiler(void)
{
	if(m_pOneLoopProfile!=NULL){
		delete m_pOneLoopProfile;
		m_pOneLoopProfile = NULL;
	}
	if(m_pOneLoopProfileResult!=NULL){
		delete m_pOneLoopProfileResult;
		m_pOneLoopProfileResult = NULL;
	}

	if(m_szFirstProfileName!=NULL){
		delete m_szFirstProfileName;
		m_szFirstProfileName = NULL;
	}

	while(m_ProfileLoop.empty()==false){
		MPROFILELOG* pPL = *(m_ProfileLoop.begin());
		m_ProfileLoop.erase(m_ProfileLoop.begin());
		delete pPL;
	}
}

void MProfiler::BeginProfile(char* szProfileName)
{
	if(m_bEnableOneLoopProfile==false) return;

	MPROFILEITEM* pProfileItem = new MPROFILEITEM;

	// Safe String Copy
	int nLen = strlen(szProfileName);
	memcpy(pProfileItem->szName, szProfileName, min(MPROFILE_ITEM_NAME_LENGTH, nLen+1));
	pProfileItem->szName[min(MPROFILE_ITEM_NAME_LENGTH-1, nLen)] = NULL;

	pProfileItem->nStartTime = timeGetTime();
	pProfileItem->nEndTime = 0;

	if(m_ProfileStack.empty()==true && m_szFirstProfileName!=NULL && strcmp(m_szFirstProfileName, szProfileName)==0){	// 한 루프가 끝나면 m_pOneLoopProfile 리셋
		if(m_pOneLoopProfile!=NULL){
			// m_pOneLoopProfileResult 으로 결과값 이양
			if(m_pOneLoopProfileResult!=NULL) delete m_pOneLoopProfileResult;
			m_pOneLoopProfileResult = m_pOneLoopProfile;
			m_pOneLoopProfile = NULL;
		}
		delete[] m_szFirstProfileName;
		m_szFirstProfileName = NULL;
	}

	if(m_szFirstProfileName==NULL){
		m_szFirstProfileName = new char[strlen(szProfileName)+2];
		strcpy(m_szFirstProfileName, szProfileName);
	}

	m_ProfileStack.push(pProfileItem);

	//m_ProfileLoop.AddProfile(pProfileItem->szName, m_ProfileStack.size()-1);
}

void MProfiler::EndProfile(char* szProfileName)
{
	if(m_bEnableOneLoopProfile==false) return;

	if(m_ProfileStack.empty()==true) return;
	MPROFILEITEM* pProfileItem = m_ProfileStack.top();
//	if(pProfileItem==NULL) return;	// 중간 Depth에서 프로파일링을 Enable 시키는 경우 최상위 _BP()가 불리기 전에 _EP()가 불려진다.

	// 속도상의 이유로 Debug에서만 비교한다.
	// 페어가 깨진 경우가 절대 있어서 안된다.
	if(strcmp(pProfileItem->szName, szProfileName)!=0){	// 현재 스택 Top의 값과 같은 이름의 Profile이여야 한다.
		_ASSERT(FALSE);
		return;
	}
	//_ASSERT(strcmp(pProfileItem->szName, szProfileName)==0);	// 현재 스택 Top의 값과 같은 이름의 Profile이여야 한다.
	pProfileItem->nEndTime = timeGetTime();

	/*
	m_ProfileLoop.SetProfile(pProfileItem->nEndTime - pProfileItem->nStartTime);
	delete pProfileItem;
	*/
	MPROFILELOG* pProfileLog = new MPROFILELOG;
	strcpy(pProfileLog->szName, pProfileItem->szName);
	pProfileLog->nCount = 1;
	pProfileLog->nDepth = m_ProfileStack.size()-1;
	pProfileLog->nMaxTime = pProfileItem->nEndTime - pProfileItem->nStartTime;
	pProfileLog->nMinTime = pProfileLog->nMaxTime;
	pProfileLog->nTotalTime = pProfileLog->nMaxTime;
	delete pProfileItem;

	//if(m_bEnableOneLoopProfile==true){
		if(m_pOneLoopProfile==NULL)
			m_pOneLoopProfile = new MProfileLoop;
		MPROFILELOG* pCopy = new MPROFILELOG;
		memcpy(pCopy, pProfileLog, sizeof(MPROFILELOG));
		m_pOneLoopProfile->AddProfile(pCopy);
	//}

	m_ProfileLoop.AddProfile(pProfileLog);

	m_ProfileStack.pop();
}

/*
 
----------------------------------------------------------------------------------------------------------------------------------------
 Total             | Average           | Min               | Max               | Count             | Scope
----------------------------------------------------------------------------------------------------------------------------------------
 12345 (100%)      | %d                | %d                | %d                | %d                | Scope
----------------------------------------------------------------------------------------------------------------------------------------
*/

bool MProfiler::FinalAnalysis(char* szFileName)
{
	FILE* fp = fopen(szFileName, "wt");
	if(fp==NULL) return false;

	static char szLog[1024];

	int nTotalTime = GetTotalTime();
	if(nTotalTime==0) nTotalTime = 1;

	char szTime[128], szDate[128];
    _strtime(szTime);
    _strdate(szDate);

	sprintf(szLog, "%s %s\n\n", szDate, szTime);
	fputs(szLog, fp);
	//fputs(Profiling Data\n, fp);
	sprintf(szLog, "Total Profiling Time : %8.3f sec\n", nTotalTime/1000.0f);
	fputs(szLog, fp);

	// Total Profile Loop Count
	//fputs(temp, fp);
	fputs("----------------------------------------------------------------------------------------------------------------------------------------\n", fp);
	fputs("       Total        |   Count    |       Average      |         Min        |         Max        | Scope\n", fp);
	fputs("----------------------------------------------------------------------------------------------------------------------------------------\n", fp);

	for(MProfileLoop::iterator i=m_ProfileLoop.begin(); i!=m_ProfileLoop.end(); i++){
		MPROFILELOG* pLog = *i;
		float fTotalTime = pLog->nTotalTime/1000.0f;
		float fTotalTimePercent = min(pLog->nTotalTime*100/(float)nTotalTime, 100);
		float fAverageTime = (pLog->nTotalTime/(float)pLog->nCount)/1000.0f;
		float fAverageTimePercent = min((pLog->nTotalTime*100/(float)pLog->nCount)/(float)nTotalTime, 100);
		float fMinTime = pLog->nMinTime/1000.0f;
		float fMinTimePercent = min(pLog->nMinTime*100/(float)nTotalTime, 100);
		float fMaxTime = pLog->nMaxTime/1000.0f;
		float fMaxTimePercent = min(pLog->nMaxTime*100/(float)nTotalTime, 100);
		sprintf(szLog, " %8.3f (%6.2f%%) | %8d   | %8.3f (%6.2f%%) | %8.3f (%6.2f%%) | %8.3f (%6.2f%%) |",
			fTotalTime, fTotalTimePercent,
			pLog->nCount,
			fAverageTime, fAverageTimePercent,
			fMinTime, fMinTimePercent,
			fMaxTime, fMaxTimePercent);
		fputs(szLog, fp);
		for(int d=0; d<pLog->nDepth; d++) fputs("  ", fp);
		fputs(pLog->szName, fp);
		fputs("\n", fp);
	}

	fputs("----------------------------------------------------------------------------------------------------------------------------------------\n", fp);

	if(m_ProfileStack.size()>0){
		sprintf(szLog, "Remained Profile Stack = %d\n", m_ProfileStack.size());
		fputs(szLog, fp);
	}

	fputs("\nGenerated by MProfiler ", fp);
	fputs("ⓒ 1998-2001, MAIET entertainment, Inc. all rights reserved.\n", fp);

	fclose(fp);
	return true;
}

int MProfiler::GetTotalTime(void)
{
	return m_ProfileLoop.GetTotalTime();
}

void MProfiler::EnableOneLoopProfile(bool bEnable)
{
	if(m_bEnableOneLoopProfile!=bEnable && m_pOneLoopProfile!=NULL){
		delete m_pOneLoopProfile;
		m_pOneLoopProfile = NULL;
	}

	m_bEnableOneLoopProfile = bEnable;
}

bool MProfiler::IsOneLoopProfile(void)
{
	return m_bEnableOneLoopProfile;
}

MProfileLoop* MProfiler::GetOneLoopProfile(void)
{
	return m_pOneLoopProfileResult;
}

MProfileLoop* MProfiler::GetProfile(void)
{
	return m_pOneLoopProfile;
}
