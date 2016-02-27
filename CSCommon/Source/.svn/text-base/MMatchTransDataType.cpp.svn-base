#include "stdafx.h"
#include "MMatchTransDataType.h"
#include "MMath.h"

void Make_MTDItemNode(MTD_ItemNode* pout, MUID& uidItem, unsigned long int nItemID, int nRentMinutePeriodRemainder)
{
	pout->uidItem = uidItem;
	pout->nItemID = nItemID;

	pout->nRentMinutePeriodRemainder = nRentMinutePeriodRemainder;		// 초단위
}

void Make_MTDAccountItemNode(MTD_AccountItemNode* pout, int nAIID, unsigned long int nItemID, int nRentMinutePeriodRemainder)
{
	pout->nAIID = nAIID;
	pout->nItemID = nItemID;
	pout->nRentMinutePeriodRemainder = nRentMinutePeriodRemainder;		// 초단위
}


void Make_MTDQuestItemNode( MTD_QuestItemNode* pOut, const unsigned long int nItemID, const int nCount )
{
	if( 0 == pOut )
		return;

	pOut->m_nItemID			= nItemID;
	pOut->m_nCount			= nCount;
}


void Make_MTDWorldItem(MTD_WorldItem* pOut, MMatchWorldItem* pWorldItem)
{
	pOut->nUID = pWorldItem->nUID;
	pOut->nItemID = pWorldItem->nItemID;
	if ( (pWorldItem->nStaticSpawnIndex < 0) && (pWorldItem->nLifeTime > 0) )	
		pOut->nItemSubType = MTD_Dynamic;
	else
		pOut->nItemSubType = MTD_Static;
	
	pOut->x = (short)Roundf(pWorldItem->x);
	pOut->y = (short)Roundf(pWorldItem->y);
	pOut->z = (short)Roundf(pWorldItem->z);
}
