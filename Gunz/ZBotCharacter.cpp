#include "stdafx.h"
#include "ZBotCharacter.h"
#include "ZGameConst.h"
#include "ZRuleSkillmap.h"

static auto NewCmdFrom(int ID, const MUID& UID)
{
	MCommandDesc* pCmdDesc = ZGetGameClient()->GetCommandManager()->GetCommandDescByID(ID);

	MUID uidTarget;
	if (pCmdDesc->IsFlag(MCDT_PEER2PEER) == true)
		uidTarget = MUID(0, 0);
	else
		uidTarget = ZGetGameClient()->GetServerUID();

	MCommand* pCmd = new MCommand(ID,
		UID,
		uidTarget,
		ZGetGameClient()->GetCommandManager());
	return pCmd;
}

template <typename... T>
void ZBotCharacter::PostBotCmd(int ID, T&&... Args)
{
	auto NewCmd = NewCmdFrom(ID, GetUID());
	ZPostCmd_AddParameters(NewCmd, std::forward<T>(Args)...);
	ZPostCommand(NewCmd);
}

void ZBotCharacter::PostBasicInfo()
{
	CharacterInfo CharInfo;
	CharInfo.Pos = GetPosition();
	CharInfo.Dir = GetDirection();
	CharInfo.Vel = GetVelocity();
	CharInfo.LowerAni = GetStateLower();
	CharInfo.UpperAni = GetStateUpper();
	CharInfo.Slot = GetItems()->GetSelectedWeaponParts();
	CharInfo.HasCamDir = false;

	auto Blob = PackNewBasicInfo(CharInfo, BasicInfoState, ZGetGame()->GetTime());
	assert(Blob);

	auto Cmd = NewCmdFrom(MC_PEER_BASICINFO_RG, GetUID());
	Cmd->AddParameter(Blob);
	ZPostCommand(Cmd);
}

void ZBotCharacter::OnUpdate(float Delta)
{
	ZCharacter::OnUpdate(Delta);

	auto NowTime = GetGlobalTimeMS();
	if (NowTime - LastTime >= 100)
	{
		LastTime = NowTime;

		PostBasicInfo();
	}

	CheckDead();

	UpdateAnimation();
	UpdateVelocity(Delta);

	if (GetDistToFloor() < 0 && !IsDie())
	{
		float fAdjust = 400.f * Delta;
		rvector diff = rvector(0, 0, min(-GetDistToFloor(), fAdjust));
		Move(diff);
	}
}

void ZBotCharacter::OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType,
	MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeType)
{
	HandleDamage(pAttacker, srcPos, damageType, weaponType, fDamage, fPiercingRatio, nMeleeType);
}

void ZBotCharacter::CheckDead()
{
	if (IsDie())
		return;

	MUID uidAttacker = MUID(0, 0);

	if (GetPosition().z < DIE_CRITICAL_LINE)
	{
		if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_SKILLMAP)
		{
			static_cast<ZRuleSkillmap *>(ZGetGame()->GetMatch()->GetRule())->OnFall();
		}
		else
		{
			uidAttacker = GetLastThrower();

			ZObject *pAttacker = ZGetObjectManager()->GetObject(uidAttacker);
			if (pAttacker == NULL || !ZGetGame()->IsAttackable(pAttacker, this))
			{
				uidAttacker = ZGetMyUID();
				pAttacker = this;
			}
			OnDamaged(pAttacker, GetPosition(),
				ZD_FALLING, MWT_NONE, GetHP());
			ZChatOutput(ZMsg(MSG_GAME_FALL_NARAK));
		}
	}

	if (ZGetGameClient()->GetMatchStageSetting()->GetNetcode() == NetcodeType::ServerBased)
		return;

	if ((IsDie() == false) && (GetHP() <= 0))
	{
		if (uidAttacker == MUID(0, 0) && GetLastAttacker() != MUID(0, 0))
			uidAttacker = GetLastAttacker();

		ActDead();
		Die();

		PostBotCmd(MC_PEER_DIE, MCmdParamUID(uidAttacker));

		if (!ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
		{
			// We would normally send a MC_MATCH_GAME_KILL, but that has to be handled by the
			// server, which doesn't exist in local dev mode, so we just call OnPeerDead to
			// simulate receiving it.
			//PostBotCmd(MC_MATCH_GAME_KILL, MCommandParameterUID(uidAttacker));
			ZGetGame()->OnPeerDead(uidAttacker, 0, GetUID(), 0);
		}
		else
		{
			PostBotCmd(MC_MATCH_QUEST_REQUEST_DEAD);
		}

		Revival();
	}
}