#ifndef MFORMULATABLE_H
#define MFORMULATABLE_H

#include "MObjectTypes.h"

const unsigned long int	g_nHPRegenTickInteval = 1000;		// (1 sec)
const unsigned long int g_nENRegenTickInteval = 1000;		// (1 sec)


int MGetHP_LCM(int nLevel, MCharacterClass nClass);
float MGetEN_LCM(int nLevel, MCharacterClass nClass);

int MGetChangingModeTickCount(MCharacterMode mode);		///< 캐릭터 모드 진입시간(msec단위)

int MCalculateMaxHP(int nLevel, MCharacterClass nClass, int nCON, int nDCS);	///< 최대 HP 계산
int MCalculateMaxEN(int nLevel, MCharacterClass nClass, int nINT, int nDMS);	///< 최대 EN 계산
float MCalculateRegenHPPerTick(int nLevel, MCharacterRace nRace, int nDCS);		///< 초당 리젠 HP 계산
float MCalculateRegenENPerTick(int nLevel, MCharacterClass nClass, int nDMS);	///< 초당 리젠 EN 계산
float MCalculateAtkDamage();	///< Attack Damage 계산
#endif
