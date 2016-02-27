#include "winsock2.h"
#include "MSkillDesc.h"
#include "MXml.h"


MSkillDescManager	g_SkillDescManager;		///< 캐릭터 Desc 리스트



MSkillDesc::MSkillDesc(int id, char* name)
{ 
	ZeroMemory(this, sizeof(MSkillDesc));
	nID = id;
	strcpy(Name,name); 
}

MSkillDescManager::MSkillDescManager()
{

}
MSkillDescManager::~MSkillDescManager()
{
}

bool MSkillDescManager::Read(const char* szFileName)
{
	MXmlDocument	xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(szFileName))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement		rootElement, chrElement, attrElement;
	char szTemp[256];
	char szName[256];

	rootElement = xmlIniData.GetDocumentElement();

	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTemp);
		if (szTemp[0] == '#') continue;

		if (!strcmp(szTemp, "SKILL"))
		{
			int id = -1;
			memset(szName, 0, 256);

			chrElement.GetAttribute(&id, "id");
			chrElement.GetAttribute(szName, "name");

			MSkillDesc*	pSkillDesc = new MSkillDesc(id, szName);

			int nRead = 0;

			if (chrElement.FindChildNode("IDENT", &attrElement)) {				// 식별
				attrElement.GetAttribute(pSkillDesc->Name, "name");

			}
			if (chrElement.FindChildNode("CLASS", &attrElement)) {		// 분류
				attrElement.GetAttribute(&nRead, "skilltype");
				pSkillDesc->SkillType = (MSkillType)nRead;

				attrElement.GetAttribute(&nRead, "skillfomulatype");
				pSkillDesc->SkillFomulaType = (MSkillFomulaType)nRead;

				attrElement.GetAttribute(&nRead, "primaskill");
				pSkillDesc->PrimaSkill = (MPrimaSkill)nRead;

			}
			if (chrElement.FindChildNode("COST", &attrElement)) {		// 비용
				attrElement.GetAttribute(&nRead, "hp");
				pSkillDesc->nHP = nRead;

				attrElement.GetAttribute(&nRead, "en");
				pSkillDesc->nEN = nRead;

			}
			if (chrElement.FindChildNode("TIME", &attrElement)) {		// 시간
				attrElement.GetAttribute(&nRead, "activationtime");
				pSkillDesc->nActivationTime = nRead;

				attrElement.GetAttribute(&nRead, "reactivationtime");
				pSkillDesc->nReactivationTime = nRead;

				attrElement.GetAttribute(&nRead, "duration");
				pSkillDesc->nDuration = nRead;

			}
			if (chrElement.FindChildNode("TARGET", &attrElement)) {		// 대상
				attrElement.GetAttribute(&nRead, "targettype");
				pSkillDesc->TargetType = (MTargetType)nRead;

				attrElement.GetAttribute(&nRead, "species");
				pSkillDesc->Species = (MSpecies)nRead;

			}
			if (chrElement.FindChildNode("RANGE", &attrElement)) {		// 거리
				attrElement.GetAttribute(&nRead, "min");
				pSkillDesc->nRangeMin = nRead;

				attrElement.GetAttribute(&nRead, "max");
				pSkillDesc->nRangeMax = nRead;

				attrElement.GetAttribute(&nRead, "radius");
				pSkillDesc->nRadius = nRead;

			}
			if (chrElement.FindChildNode("REQUISITE", &attrElement)) {	// 조건
				attrElement.GetAttribute(&nRead, "class");
				pSkillDesc->Class = (MCharacterClass)nRead;

				attrElement.GetAttribute(&nRead, "level");
				pSkillDesc->nLevel = nRead;

				attrElement.GetAttribute(&nRead, "weapontype");
				pSkillDesc->WeaponType = (MWeaponType)nRead;

				attrElement.GetAttribute(&nRead, "difficulty");
				pSkillDesc->nDifficulty = nRead;

				attrElement.GetAttribute(&nRead, "prerequisitetype");
				pSkillDesc->PrerequisiteType = nRead;

				attrElement.GetAttribute(&nRead, "prerequesiteid");
				pSkillDesc->nPrerequisiteID = nRead;

			}
			if (chrElement.FindChildNode("HATE", &attrElement)) {		// 적대감
				attrElement.GetAttribute(&nRead, "value");
				pSkillDesc->nHateValue = nRead;

			}
			if (chrElement.FindChildNode("MODHEALTH", &attrElement)) {	// 건강수정치
				pSkillDesc->SetMODFlag(MSF_MOD_HEALTH);

				attrElement.GetAttribute(&nRead, "hp");
				pSkillDesc->MOD_HP = nRead;

				attrElement.GetAttribute(&nRead, "en");
				pSkillDesc->MOD_EN = nRead;

			}
			if (chrElement.FindChildNode("MODSTAT", &attrElement)) {		// 특성치수정
				pSkillDesc->SetMODFlag(MSF_MOD_STAT);

				attrElement.GetAttribute(&nRead, "str");
				pSkillDesc->MOD_STR = nRead;

				attrElement.GetAttribute(&nRead, "con");
				pSkillDesc->MOD_CON = nRead;

				attrElement.GetAttribute(&nRead, "dex");
				pSkillDesc->MOD_DEX = nRead;

				attrElement.GetAttribute(&nRead, "agi");
				pSkillDesc->MOD_AGI = nRead;

				attrElement.GetAttribute(&nRead, "int");
				pSkillDesc->MOD_INT = nRead;

				attrElement.GetAttribute(&nRead, "cha");
				pSkillDesc->MOD_CHA = nRead;

			}
			if (chrElement.FindChildNode("MODRESIST", &attrElement)) {	// 저항수정치
				pSkillDesc->SetMODFlag(MSF_MOD_RESIST);

				attrElement.GetAttribute(&nRead, "mr");
				pSkillDesc->MOD_MR = nRead;

				attrElement.GetAttribute(&nRead, "fr");
				pSkillDesc->MOD_FR = nRead;

				attrElement.GetAttribute(&nRead, "cr");
				pSkillDesc->MOD_CR = nRead;

				attrElement.GetAttribute(&nRead, "pr");
				pSkillDesc->MOD_PR = nRead;

				attrElement.GetAttribute(&nRead, "lr");
				pSkillDesc->MOD_LR = nRead;

			}
			if (chrElement.FindChildNode("MODDEFENSE", &attrElement)) {	// 방어수정치
				pSkillDesc->SetMODFlag(MSF_MOD_DEFENSE);

				attrElement.GetAttribute(&nRead, "ac");
				pSkillDesc->MOD_AC = nRead;

				attrElement.GetAttribute(&nRead, "abs");
				pSkillDesc->MOD_ABS = nRead;

				attrElement.GetAttribute(&nRead, "mflee");
				pSkillDesc->MOD_MFlee = nRead;

				attrElement.GetAttribute(&nRead, "rflee");
				pSkillDesc->MOD_RFlee = nRead;

				attrElement.GetAttribute(&nRead, "sflee");
				pSkillDesc->MOD_SFlee = nRead;

				attrElement.GetAttribute(&nRead, "hpregen");
				pSkillDesc->MOD_HPRegen = nRead;

				attrElement.GetAttribute(&nRead, "enregen");
				pSkillDesc->MOD_ENRegen = nRead;

			}
			if (chrElement.FindChildNode("MODOFFENSE", &attrElement)) {	// 공격수정치
				pSkillDesc->SetMODFlag(MSF_MOD_OFFENSE);

				attrElement.GetAttribute(&nRead, "priatk");
				pSkillDesc->MOD_PriATK = nRead;

				attrElement.GetAttribute(&nRead, "secatk");
				pSkillDesc->MOD_SecATK = nRead;

				attrElement.GetAttribute(&nRead, "aspd");
				pSkillDesc->MOD_Aspd = nRead;

				attrElement.GetAttribute(&nRead, "mcritical");
				pSkillDesc->MOD_MCritical = nRead;

				attrElement.GetAttribute(&nRead, "rcritical");
				pSkillDesc->MOD_RCritical = nRead;

				attrElement.GetAttribute(&nRead, "scritical");
				pSkillDesc->MOD_SCritical = nRead;

				attrElement.GetAttribute(&nRead, "maccu");
				pSkillDesc->MOD_MAccu = nRead;

				attrElement.GetAttribute(&nRead, "raccu");
				pSkillDesc->MOD_RAccu = nRead;

				attrElement.GetAttribute(szTemp, "damageamp");
				sscanf(szTemp, "%f", &pSkillDesc->MOD_DamageAmp);

			}
			if (chrElement.FindChildNode("MODNORMAL", &attrElement)) {	// 일반수정치
				pSkillDesc->SetMODFlag(MSF_MOD_NORMAL);

				attrElement.GetAttribute(&nRead, "dsight");
				pSkillDesc->MOD_DSight = nRead;

				attrElement.GetAttribute(&nRead, "nsight");
				pSkillDesc->MOD_NSight = nRead;

				attrElement.GetAttribute(&nRead, "speed");
				pSkillDesc->MOD_Spd = nRead;

			}
			if (chrElement.FindChildNode("MODSTATUS", &attrElement)) {	// 상태수정치
				pSkillDesc->SetMODFlag(MSF_MOD_STATUS);

				attrElement.GetAttribute(&nRead, "mode");
				pSkillDesc->MOD_Mode = nRead;

				attrElement.GetAttribute(&nRead, "state");
				pSkillDesc->MOD_State = nRead;

				attrElement.GetAttribute(&nRead, "ability");
				pSkillDesc->MOD_Ability = nRead;

			}
			if (chrElement.FindChildNode("MODKEY", &attrElement)) {		// 열쇠
				pSkillDesc->SetMODFlag(MSF_MOD_KEY);

				attrElement.GetAttribute(&nRead, "keyflag");
				pSkillDesc->MOD_KeyFlag = nRead;

				attrElement.GetAttribute(&nRead , "keyskill");
				pSkillDesc->MOD_KeySkill = nRead;

			}
			if (chrElement.FindChildNode("MODPRIMA", &attrElement)) {	// 프리마스킬 수정치
				pSkillDesc->SetMODFlag(MSF_MOD_PRIMA);

				attrElement.GetAttribute(&nRead, "ocs");
				pSkillDesc->MOD_OCS = nRead;

				attrElement.GetAttribute(&nRead, "dsc");
				pSkillDesc->MOD_DCS = nRead;

				attrElement.GetAttribute(&nRead, "rcs");
				pSkillDesc->MOD_RCS = nRead;

				attrElement.GetAttribute(&nRead, "ecs");
				pSkillDesc->MOD_ECS = nRead;

				attrElement.GetAttribute(&nRead, "oms");
				pSkillDesc->MOD_OMS = nRead;

				attrElement.GetAttribute(&nRead, "dms");
				pSkillDesc->MOD_DMS = nRead;

				attrElement.GetAttribute(&nRead, "ems");
				pSkillDesc->MOD_EMS = nRead;

				attrElement.GetAttribute(&nRead, "mcs");
				pSkillDesc->MOD_MCS = nRead;

				attrElement.GetAttribute(&nRead, "scs");
				pSkillDesc->MOD_SCS = nRead;

			}

			insert(value_type(id, pSkillDesc));
		}
	}

	xmlIniData.Destroy();
	return true;
}

MSkillDesc* MSkillDescManager::Find(int nDescID)
{
	iterator itor = find(nDescID);
	if (itor != end())
	{
		return (*itor).second;
	}
	return NULL;

}

void MSkillDescManager::Clear()
{
	if (!empty())
	{
		MSkillDesc* pSkillDesc = (*begin()).second;
		delete pSkillDesc; pSkillDesc = NULL;
		erase(begin());
	}

}