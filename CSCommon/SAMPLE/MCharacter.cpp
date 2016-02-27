#include "MCharacter.h"

bool chr_LoadFromXmlFile(MCHR* pOutChr, char* szFileName, int iChrID)
{
	MXmlDocument	aXml;

	aXml.Create();
	memset(pOutChr, 0, sizeof(MCHR));

	if (!aXml.LoadFromFile(szFileName)) 
	{
		aXml.Destroy();
		return false;
	}

	bool bFound = false;
	int iCount, iCount2, i, j;
	char szBuf[65535], szBuf2[65535];
	memset(szBuf, 0, 65535);
	memset(szBuf2, 0, 65535);

	MXmlElement		aRoot, aChild, aChild2;

	aRoot = aXml.GetDocumentElement();

	iCount = aRoot.GetChildNodeCount();

	for (i = 0; i < iCount; i++)
	{
		aChild = aRoot.GetChildNode(i);
		aChild.GetTagName(szBuf);
		if (szBuf[0] == '#') continue;

		if (!strcmp(szBuf, "Character"))
		{
			int iID;

			if (!aChild.GetAttribute(&iID, "id")) continue;
			if (iID != iChrID) continue;

			// Found
			iCount2 = aChild.GetChildNodeCount();
			for (j = 0; j < iCount2; j++)
			{
				aChild2 = aChild.GetChildNode(j);
				aChild2.GetTagName(szBuf);
				if (szBuf[0] == '#') continue;

				// 식별
				if (!strcmp(szBuf, "Name"))
				{
					aChild2.GetContents(pOutChr->szName);
				}
				else if (!strcmp(szBuf, "Surname"))
				{
					aChild2.GetContents(pOutChr->szSurname);
				}
				else if (!strcmp(szBuf, "Class"))
				{
					aChild2.GetContents(&pOutChr->iClass);
				}
				else if (!strcmp(szBuf, "Race"))
				{
					aChild2.GetContents(&pOutChr->iRace);
				}
				else if (!strcmp(szBuf, "Type"))
				{
					aChild2.GetContents(&pOutChr->iType);
				}
				else if (!strcmp(szBuf, "Sex"))
				{
					aChild2.GetContents(&pOutChr->iSex);
				}
				else if (!strcmp(szBuf, "Specialization"))
				{
					aChild2.GetContents(&pOutChr->iSpecialization);
				}
				else if (!strcmp(szBuf, "BindingSpot"))
				{
					aChild2.GetContents(&pOutChr->iBindingSpot);
				}
				// 경험치
				else if (!strcmp(szBuf, "CurrentXP"))
				{
					aChild2.GetContents(&pOutChr->iCurrentXP);
				}
				else if (!strcmp(szBuf, "NextXP"))
				{
					aChild2.GetContents(&pOutChr->iNextXP);
				}
				// 성향
				else if (!strcmp(szBuf, "Relationships"))
				{
					aChild2.GetContents(&pOutChr->iRelationships);
				}
				else if (!strcmp(szBuf, "RshipRaise"))
				{
					aChild2.GetContents(&pOutChr->iRshipRaise);
				}
				else if (!strcmp(szBuf, "RshipLower"))
				{
					aChild2.GetContents(&pOutChr->iRshipLower);
				}
				// 기본특성치
				else if (!strcmp(szBuf, "HP"))
				{
					aChild2.GetContents(&pOutChr->iHP);
				}
				else if (!strcmp(szBuf, "EN"))
				{
					aChild2.GetContents(&pOutChr->iEN);
				}
				else if (!strcmp(szBuf, "STR"))
				{
					aChild2.GetContents(&pOutChr->iSTR);
				}
				else if (!strcmp(szBuf, "CON"))
				{
					aChild2.GetContents(&pOutChr->iCON);
				}
				else if (!strcmp(szBuf, "DEX"))
				{
					aChild2.GetContents(&pOutChr->iDEX);
				}
				else if (!strcmp(szBuf, "AGI"))
				{
					aChild2.GetContents(&pOutChr->iAGI);
				}
				else if (!strcmp(szBuf, "INT"))
				{
					aChild2.GetContents(&pOutChr->iINT);
				}
				else if (!strcmp(szBuf, "CHA"))
				{
					aChild2.GetContents(&pOutChr->iCHA);
				}
				// 저항력
				else if (!strcmp(szBuf, "MR"))
				{
					aChild2.GetContents(&pOutChr->iMR);
				}
				else if (!strcmp(szBuf, "FR"))
				{
					aChild2.GetContents(&pOutChr->iFR);
				}
				else if (!strcmp(szBuf, "CR"))
				{
					aChild2.GetContents(&pOutChr->iCR);
				}
				else if (!strcmp(szBuf, "PR"))
				{
					aChild2.GetContents(&pOutChr->iPR);
				}
				else if (!strcmp(szBuf, "LR"))
				{
					aChild2.GetContents(&pOutChr->iLR);
				}
				// 마법
				else if (!strcmp(szBuf, "BUFFS"))
				{
					aChild2.GetContents(&pOutChr->iBUFFS);
				}
				else if (!strcmp(szBuf, "SkillList"))
				{
					aChild2.GetContents(&pOutChr->iSkillList);
				}
				// 방어력
				else if (!strcmp(szBuf, "AC"))
				{
					aChild2.GetContents(&pOutChr->iAC);
				}
				else if (!strcmp(szBuf, "MFlee"))
				{
					aChild2.GetContents(&pOutChr->iMFlee);
				}
				else if (!strcmp(szBuf, "RFlee"))
				{
					aChild2.GetContents(&pOutChr->iRFlee);
				}
				else if (!strcmp(szBuf, "SFlee"))
				{
					aChild2.GetContents(&pOutChr->iSFlee);
				}
				else if (!strcmp(szBuf, "HPRegen"))
				{
					aChild2.GetContents(&pOutChr->iHPRegen);
				}
				else if (!strcmp(szBuf, "ENRegen"))
				{
					aChild2.GetContents(&pOutChr->iENRegen);
				}
				// 공격력
				else if (!strcmp(szBuf, "MATK"))
				{
					aChild2.GetContents(&pOutChr->iMATK);
				}
				else if (!strcmp(szBuf, "RATK"))
				{
					aChild2.GetContents(&pOutChr->iRATK);
				}
				else if (!strcmp(szBuf, "MCritical"))
				{
					aChild2.GetContents(&pOutChr->iMCritical);
				}
				else if (!strcmp(szBuf, "RCritical"))
				{
					aChild2.GetContents(&pOutChr->iRCritical);
				}
				else if (!strcmp(szBuf, "MAspd"))
				{
					aChild2.GetContents(&pOutChr->iMAspd);
				}
				else if (!strcmp(szBuf, "RAspd"))
				{
					aChild2.GetContents(&pOutChr->iRAspd);
				}
				else if (!strcmp(szBuf, "MAccu"))
				{
					aChild2.GetContents(&pOutChr->iMAccu);
				}
				else if (!strcmp(szBuf, "RAccu"))
				{
					aChild2.GetContents(&pOutChr->iRAccu);
				}
				// 이동
				else if (!strcmp(szBuf, "DSight"))
				{
					aChild2.GetContents(&pOutChr->iDSight);
				}
				else if (!strcmp(szBuf, "NSight"))
				{
					aChild2.GetContents(&pOutChr->iNSight);
				}
				else if (!strcmp(szBuf, "Spd"))
				{
					aChild2.GetContents(&pOutChr->iSpd);
				}
				// 상태
				else if (!strcmp(szBuf, "Levitate"))
				{
					aChild2.GetContents(&pOutChr->iLevitate);
				}
				else if (!strcmp(szBuf, "Invisibility"))
				{
					aChild2.GetContents(&pOutChr->iInvisibility);
				}
				else if (!strcmp(szBuf, "Invincibility"))
				{
					aChild2.GetContents(&pOutChr->iInvincibility);
				}
				else if (!strcmp(szBuf, "Stun"))
				{
					aChild2.GetContents(&pOutChr->iStun);
				}
				else if (!strcmp(szBuf, "Stealth"))
				{
					aChild2.GetContents(&pOutChr->iStealth);
				}
				else if (!strcmp(szBuf, "Poisoned"))
				{
					aChild2.GetContents(&pOutChr->iPoisoned);
				}
				// 이뮨
				else if (!strcmp(szBuf, "IVMelee"))
				{
					aChild2.GetContents(&pOutChr->iIVMelee);
				}
				else if (!strcmp(szBuf, "IVRange"))
				{
					aChild2.GetContents(&pOutChr->iIVRange);
				}
				else if (!strcmp(szBuf, "IVMagic"))
				{
					aChild2.GetContents(&pOutChr->iIVMagic);
				}
				else if (!strcmp(szBuf, "IVCold"))
				{
					aChild2.GetContents(&pOutChr->iIVCold);
				}
				else if (!strcmp(szBuf, "IVLightning"))
				{
					aChild2.GetContents(&pOutChr->iIVLightning);
				}
				else if (!strcmp(szBuf, "IVPoison"))
				{
					aChild2.GetContents(&pOutChr->iIVPoison);
				}
				else if (!strcmp(szBuf, "IVFire"))
				{
					aChild2.GetContents(&pOutChr->iIVFire);
				}
				else if (!strcmp(szBuf, "IVStun"))
				{
					aChild2.GetContents(&pOutChr->iIVStun);
				}
				else if (!strcmp(szBuf, "IVSleep"))
				{
					aChild2.GetContents(&pOutChr->iIVSleep);
				}
				else if (!strcmp(szBuf, "IVCrit"))
				{
					aChild2.GetContents(&pOutChr->iIVCrit);
				}
				else if (!strcmp(szBuf, "IVParalysis"))
				{
					aChild2.GetContents(&pOutChr->iIVParalysis);
				}
				// 전문화
				else if (!strcmp(szBuf, "OCS"))
				{
					aChild2.GetContents(&pOutChr->iOCS);
				}
				else if (!strcmp(szBuf, "DCS"))
				{
					aChild2.GetContents(&pOutChr->iDCS);
				}
				else if (!strcmp(szBuf, "RCS"))
				{
					aChild2.GetContents(&pOutChr->iRCS);
				}
				else if (!strcmp(szBuf, "ECS"))
				{
					aChild2.GetContents(&pOutChr->iECS);
				}
				else if (!strcmp(szBuf, "OMS"))
				{
					aChild2.GetContents(&pOutChr->iOMS);
				}
				else if (!strcmp(szBuf, "DMS"))
				{
					aChild2.GetContents(&pOutChr->iDMS);
				}
				else if (!strcmp(szBuf, "EMS"))
				{
					aChild2.GetContents(&pOutChr->iEMS);
				}
			}

			bFound = true;
			break;
		}
	}

	aXml.Destroy();

	return bFound;

return false;
}