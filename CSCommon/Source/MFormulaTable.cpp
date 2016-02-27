#include "MFormulaTable.h"
#include "crtdbg.h"
#include <windows.h>


// HP Level/Class Modifier Table
static int		gHPLevelClassModifierTbl[8][4] = {
//	0 : Fighter, 1 : Rogue, 2 : Acolyte, 3 : Mage
			{ 40, 35, 30, 25 },		// 01 - 10 Level
			{ 40, 35, 30, 25 },		// 11 - 20 Level
			{ 45, 38, 32, 26 },		// 21 - 30 Level
			{ 45, 38, 32, 26 },		// 31 - 40 Level
			{ 50, 41, 34, 27 },		// 41 - 50 Level
			{ 50, 41, 34, 27 },		// 51 - 60 Level
			{ 55, 44, 36, 28 },		// 61 - 70 Level
			{ 55, 44, 36, 28 }		// 71 - 80 Level
};


// EN Level/Class Modifier Table
static float	gENLevelClassModifierTbl[8][4] = {
// 0 : Fighter, 1 : Rogue, 2 : Acolyte, 3 : Mage
			{ 0.6f, 0.7f, 1.0f, 1.2f },		// 01 - 10 Level
			{ 0.6f, 0.7f, 1.0f, 1.2f },		// 11 - 20 Level
			{ 0.6f, 0.7f, 1.0f, 1.2f },		// 21 - 30 Level
			{ 0.6f, 0.7f, 1.0f, 1.2f },		// 31 - 40 Level
			{ 0.6f, 0.7f, 1.0f, 1.2f },		// 41 - 50 Level
			{ 0.6f, 0.7f, 1.0f, 1.2f },		// 51 - 60 Level
			{ 0.6f, 0.7f, 1.0f, 1.2f },		// 61 - 70 Level
			{ 0.6f, 0.7f, 1.0f, 1.2f },		// 71 - 80 Level
};

// 캐릭터 모드 진입시간 - msec 단위이다.
static int	gChangingModeTickInteval[6] = { 1000, 0, 0, 1000, 2000, 0 };


int MGetHP_LCM(int nLevel, MCharacterClass nClass)
{
	_ASSERT((nLevel > 0) && (nLevel < 255));

	return gHPLevelClassModifierTbl[nLevel % 10][nClass];
}

float MGetEN_LCM(int nLevel, MCharacterClass nClass)
{
	_ASSERT((nLevel > 0) && (nLevel < 255));

	return gENLevelClassModifierTbl[nLevel % 10][nClass];
}

int MGetChangingModeTickCount(MCharacterState nState) 
{
	return gChangingModeTickInteval[nState];
}


int MCalculateMaxHP(int nLevel, MCharacterClass nClass, int nCON, int nDCS)
{
	// Basic HP = TRUNC[(CON x LCM)/50 + LVL x LCM] + (MAX(0,DCS-150) x 10)
	return (int)((nCON * MGetHP_LCM(nLevel, nClass)) / 50.0f + nLevel * MGetHP_LCM(nLevel, nClass)) 
		   + (max(0, (nDCS-150)) * 10);

}

int MCalculateMaxEN(int nLevel, MCharacterClass nClass, int nINT, int nDMS)
{
	// Basic EN = TRUNC[TRUNC[INT/4] x LCM x LVL] + (MAX(0,DMS-150) x 10)
	return (int)((int)(nINT / 4.0f) * MGetEN_LCM(nLevel, nClass) * nLevel) 
		   + (max(0, (nDMS-150)) * 10);
}

float MCalculateRegenHPPerTick(int nLevel, MCharacterRace nRace, int nDCS)
{
	float c1, c2;	// 상수 1, 상수 2

	switch (nRace)
	{
	case MCR_HUMAN:
		c1 = 0.5f; c2 = 0.4f; break;
	case MCR_ELF:
		c1 = 0.8f; c2 = 0.38f; break;
	case MCR_DWARF:
		c1 = 0.6f; c2 = 0.45f; break;
	case MCR_DARKELF:
		c1 = 0.7f; c2 = 0.46f; break;
	case MCR_OGRE:
		c1 = 1.0f; c2 = 0.45f; break;
	default:
		c1 = 0.0f; c2 = 0.0f; 
	}

	return (c1 + (int(nLevel / 10) * c2)) * (max(0, (nDCS-150)) /300.0f + 1);
}

float MCalculateRegenENPerTick(int nLevel, MCharacterClass nClass, int nDMS)
{
	float c1;
	switch (nClass)
	{
	case MCC_FIGHTER:
		c1 = 0.4f; break;
	case MCC_ROGUE:
		c1 = 0.8f; break;
	case MCC_ACOLYTE:
		c1 = 0.45f; break;
	case MCC_MAGE:
		c1 = 0.46f; break;
	}
	return ((int)(nLevel / 10) * c1) * (max(0,(nDMS-150)) / 300.0f +1);

}


float MCalculateAtkDamage()
{

	return 0.0f;
}