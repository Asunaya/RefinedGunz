#ifndef MCHARACTER_H
#define MCHARACTER_H

#include "MXml.h"

struct MCHR
{
	// 식별
	char		szName[24];
	char		szSurname[24];
	int			iClass;				// enum
	int			iRace;				// enum
	int			iType;				// enum
	int			iSex;				// enum
	int			iSpecialization;	// enum
	int			iBindingSpot;		// pos

	// 경험치
	int			iCurrentXP;
	int			iNextXP;

	// 성향
	int			iRelationships;		// list
	int			iRshipRaise;		// list
	int			iRshipLower;		// list

	// 기본특성치
	int			iHP;
	int			iEN;
	int			iSTR;
	int			iCON;
	int			iDEX;
	int			iAGI;
	int			iINT;
	int			iCHA;

	// 저항력
	int			iMR;
	int			iFR;
	int			iCR;
	int			iPR;
	int			iLR;

	// 마법
	int			iBUFFS;		// list
	int			iSkillList;	// list

	// 방어력
	int			iAC;
	int			iMFlee;
	int			iRFlee;
	int			iSFlee;
	int			iHPRegen;
	int			iENRegen;

	// 공격력
	int			iMATK;
	int			iRATK;
	int			iMCritical;
	int			iRCritical;
	int			iMAspd;
	int			iRAspd;
	int			iMAccu;
	int			iRAccu;

	// 이동
	int			iDSight;
	int			iNSight;
	int			iSpd;

	// 상태
	int			iLevitate;			// flag
	int			iInvisibility;		// flag
	int			iInvincibility;		// flag
	int			iStun;				// flag
	int			iStealth;			// flag
	int			iPoisoned;			// flag

	// 이뮨
	int			iIVMelee;			// flag
	int			iIVRange;			// flag
	int			iIVMagic;			// flag
	int			iIVCold;			// flag
	int			iIVLightning;		// flag
	int			iIVPoison;			// flag
	int			iIVFire;			// flag
	int			iIVStun;			// flag
	int			iIVSleep;			// flag
	int			iIVCrit;			// flag
	int			iIVParalysis;		// flag

	// 전문화
	int			iOCS;
	int			iDCS;
	int			iRCS;
	int			iECS;
	int			iOMS;
	int			iDMS;
	int			iEMS;
};


bool chr_LoadFromXmlFile(MCHR* pOutChr, char* szFileName, int iChrID);
#endif