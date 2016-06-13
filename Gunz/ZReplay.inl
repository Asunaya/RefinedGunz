#pragma once

#include "ZRuleDuel.h"
#include <boost/mpl/has_xxx.hpp>
#include "stack_allocator.hpp"

inline ReplayVersion ZReplayLoader::GetVersion()
{
	ReplayVersion Version;

	unsigned int version = 0;
	unsigned int header;

	bool FoundServer = false;

	Read(header);

	if (header == RG_REPLAY_MAGIC_NUMBER)
	{
		Version.Server = SERVER::REFINED_GUNZ;
		FoundServer = true;
	}
	else if (header != GUNZ_REC_FILE_ID)
	{
		Version.Server = SERVER::NONE;
		return Version;
	}

	Read(version);

	Version.nVersion = version;
	Version.nSubVersion = 0;

	if (!FoundServer)
	{
		u8 Something;
		ReadAt(Something, 0x4A);

		if (Version.nVersion >= 6 && Version.nVersion <= 9 && Something <= 0x01)
		{
			ReadAt(Something, 0x91);

			if (Something == 0x00)
				Version.Server = SERVER::DarkGunz;
			else
				Version.Server = SERVER::FREESTYLE_GUNZ;
		}
		else
		{
			Version.Server = SERVER::OFFICIAL;
		}
	}

	if (Version.Server == SERVER::REFINED_GUNZ && Version.nVersion >= 2)
	{
		u32 ClientVersion = 0;
		Read(ClientVersion);
		Version.nSubVersion = ClientVersion;

		i64 Time;
		Read(Time);
		Timestamp = Time;
	}
	else if(Version.Server == SERVER::FREESTYLE_GUNZ && Version.nVersion == 7)
	{
		REPLAY_STAGE_SETTING_NODE_FG Setting;
		Peek(Setting);

		int offset = 527;

		if (Setting.nGameType == MMATCH_GAMETYPE_DUEL)
			offset += sizeof(MTD_DuelQueueInfo);

		u8 Something;
		ReadAt(Something, offset);

		if (Version.nVersion == 7 && (Something == 0x00 || Something == 0x01))
		{
			Version.nSubVersion = 1;
		}
	}

	this->Version = Version;

	return Version;
}

BOOST_MPL_HAS_XXX_TRAIT_DEF(szStageName);

template <typename T>
typename std::enable_if<has_szStageName<T>::value>::type CopyStageName(REPLAY_STAGE_SETTING_NODE &m_StageSetting, const T &Setting) {
	memcpy(m_StageSetting.szStageName, Setting.szStageName, min(sizeof(m_StageSetting.szStageName), sizeof(Setting.szStageName)));
	m_StageSetting.szStageName[sizeof(m_StageSetting.szStageName) - 1] = 0;
}

template <typename T>
typename std::enable_if<!has_szStageName<T>::value>::type CopyStageName(REPLAY_STAGE_SETTING_NODE &m_StageSetting, const T &Setting) {
	m_StageSetting.szStageName[0] = 0;
}

inline void ZReplayLoader::GetStageSetting(REPLAY_STAGE_SETTING_NODE& ret)
{
#define COPY_SETTING(member) ret.member = Setting.member;
	auto CopySetting = [&](const auto &Setting)
	{
		COPY_SETTING(uidStage);
		// We don't want to do strcpy in case it's not null-terminated
		memcpy(ret.szMapName, Setting.szMapName, sizeof(ret.szMapName));
		ret.szMapName[sizeof(ret.szMapName) - 1] = 0;
		COPY_SETTING(nMapIndex);
		COPY_SETTING(nGameType);
		COPY_SETTING(nRoundMax);
		COPY_SETTING(nLimitTime);
		COPY_SETTING(nLimitLevel);
		COPY_SETTING(nMaxPlayers);
		COPY_SETTING(bTeamKillEnabled);
		COPY_SETTING(bTeamWinThePoint);
		COPY_SETTING(bForcedEntryEnabled);

		CopyStageName(ret, Setting);
	};
#undef COPY_SETTING

	switch (Version.Server)
	{
	case SERVER::OFFICIAL:
	{
		if (Version.nVersion <= 5)
		{
			REPLAY_STAGE_SETTING_NODE_OLD Setting;
			Read(Setting);

			CopySetting(Setting);
		}
		else
		{
			REPLAY_STAGE_SETTING_NODE_V11 Setting;
			Read(Setting);

			CopySetting(Setting);
		}
	}
	break;
	case SERVER::REFINED_GUNZ:
	{
		Read(ret);
	}
	break;
	case SERVER::FREESTYLE_GUNZ:
	{
		REPLAY_STAGE_SETTING_NODE_FG Setting;
		Read(Setting);
		CopySetting(Setting);
	}
	break;
	case SERVER::DarkGunz:
	{
		REPLAY_STAGE_SETTING_NODE_DG Setting;
		Read(Setting);
		CopySetting(Setting);
	}
	break;
	};

	IsDojo = !_stricmp(ret.szMapName, "Dojo");
	GameType = ret.nGameType;
}

inline void ZReplayLoader::GetDuelQueueInfo(MTD_DuelQueueInfo* QueueInfo)
{
	if (GameType != MMATCH_GAMETYPE_DUEL)
		return;

	if (!QueueInfo)
	{
		Position += sizeof(*QueueInfo);
		return;
	}

	Read(*QueueInfo);
}

inline std::vector<ReplayPlayerInfo> ZReplayLoader::GetCharInfo()
{
	std::vector<ReplayPlayerInfo> ret;

	int nCharacterCount;
	Read(nCharacterCount);

	for (int i = 0; i < nCharacterCount; i++)
	{
		bool IsHero;
		if (Version.Server != SERVER::DarkGunz)
			Read(IsHero);

		MTD_CharInfo CharInfo;

#define COPY_CHARINFO(member) CharInfo.member = oldinfo.member
		auto CopyCharInfo = [&](const auto& oldinfo)
		{
			strcpy_safe(CharInfo.szName, oldinfo.szName);
			strcpy_safe(CharInfo.szClanName, oldinfo.szClanName);
			COPY_CHARINFO(nClanGrade);
			COPY_CHARINFO(nClanContPoint);
			COPY_CHARINFO(nCharNum);
			COPY_CHARINFO(nLevel);
			COPY_CHARINFO(nSex);
			COPY_CHARINFO(nHair);
			COPY_CHARINFO(nFace);
			COPY_CHARINFO(nXP);
			COPY_CHARINFO(nBP);
			COPY_CHARINFO(fBonusRate);
			COPY_CHARINFO(nPrize);
			COPY_CHARINFO(nHP);
			COPY_CHARINFO(nAP);
			COPY_CHARINFO(nMaxWeight);
			COPY_CHARINFO(nSafeFalls);
			COPY_CHARINFO(nFR);
			COPY_CHARINFO(nCR);
			COPY_CHARINFO(nER);
			COPY_CHARINFO(nWR);
			for (size_t i = 0; i < min(MMCIP_END, ArraySize(oldinfo.nEquipedItemDesc)); i++)
				COPY_CHARINFO(nEquipedItemDesc[i]);
			COPY_CHARINFO(nUGradeID);
		};
#undef COPY_CHARINFO

#define READ_CHARINFO(type) do { type info; Read(info); CopyCharInfo(info); } while(false)

		if (Version.Server == SERVER::OFFICIAL)
		{
			if (Version.nVersion <= 5)
			{
				MTD_CharInfo_V5 oldinfo;
				if (Version.nVersion < 2)
				{
					ReadN(&oldinfo, sizeof(oldinfo) - sizeof(oldinfo.nClanCLID));
					oldinfo.nClanCLID = 0;
				}
				else
				{
					Read(oldinfo);
				}
				CopyCharInfo(oldinfo);
			}
			else if (Version.nVersion == 6)
			{
				READ_CHARINFO(MTD_CharInfo_V6);
			}
			else if (Version.nVersion == 11)
			{
				READ_CHARINFO(MTD_CharInfo_V11);
			}
		}
		else if (Version.Server == SERVER::FREESTYLE_GUNZ)
		{
			if (Version.nVersion == 7)
			{
				if (Version.nSubVersion == 0)
				{
					READ_CHARINFO(MTD_CharInfo_FG_V7_0);
				}
				else if (Version.nSubVersion == 1)
				{
					READ_CHARINFO(MTD_CharInfo_FG_V7_1);
				}
			}
			else if (Version.nVersion = 8)
			{
				READ_CHARINFO(MTD_CharInfo_FG_V8);
			}
			else if (Version.nVersion == 9)
			{
				READ_CHARINFO(MTD_CharInfo_FG_V9);
			}

			CharInfo.nEquipedItemDesc[MMCIP_MELEE] = 2; // Rusty Sword
		}
		else if (Version.Server == SERVER::REFINED_GUNZ)
		{
			Read(CharInfo);
		}
#undef READ_CHARINFO

		ZCharacterReplayState CharState;

#define COPY_CHARSTATE(member) CharState.member = src.member
		auto CopyCharState = [&](const auto& src)
		{
			COPY_CHARSTATE(UID);
			COPY_CHARSTATE(Property);
			COPY_CHARSTATE(HP);
			COPY_CHARSTATE(AP);
			COPY_CHARSTATE(Status);

			for (size_t i = 0; i < min(MMCIP_END, ArraySize(src.BulletInfos)); i++)
				COPY_CHARSTATE(BulletInfos[i]);

			COPY_CHARSTATE(Position);
			COPY_CHARSTATE(Direction);
			COPY_CHARSTATE(Team);
			COPY_CHARSTATE(Dead);
			COPY_CHARSTATE(HidingAdmin);
		};
#undef COPY_CHARSTATE

#define READ_CHARSTATE(type) do { type state; Read(state); CopyCharState(state); } while (false)

		if (Version.Server == SERVER::FREESTYLE_GUNZ)
		{
			if (Version.nVersion == 7)
			{
				if (Version.nSubVersion == 0)
				{
					READ_CHARSTATE(ZCharacterReplayState_FG_V7_0);
				}
				else
				{
					READ_CHARSTATE(ZCharacterReplayState_FG_V7_1);
				}
			}
			else if (Version.nVersion == 8)
			{
				READ_CHARSTATE(ZCharacterReplayState_FG_V8);
			}
			else if (Version.nVersion == 9)
			{
				READ_CHARSTATE(ZCharacterReplayState_FG_V9);
			}
		}
		else if (Version.Server == SERVER::REFINED_GUNZ)
		{
			Read(CharState);
		}
		else if (Version.Server == SERVER::OFFICIAL)
		{
			if (Version.nVersion >= 6)
			{
				if (Version.nVersion == 11)
				{
					READ_CHARSTATE(ZCharacterReplayState_Official_V11);
				}
				else
				{
					READ_CHARSTATE(ZCharacterReplayState_Official_V6);
				}
			}
			else
			{
				Read(CharState);
			}
		}
#undef READ_CHARSTATE

		if (Version.Server == SERVER::DarkGunz)
		{
			ReplayPlayerInfo_DG rpi;
			Read(rpi);
			IsHero = rpi.IsHero;
			CopyCharInfo(rpi.Info);
			CopyCharState(rpi.State);
		}

		MLog("Offset: %X\n", Position);

		ret.push_back({ IsHero, CharInfo, CharState });
	}

	return ret;
}

template <typename T>
void ZReplayLoader::ReadAt(T& Obj, int Position)
{
	if (Position + sizeof(Obj) > InflatedFile.size())
		throw EOFException(Position);

	Obj = *((decltype(&Obj))&InflatedFile[Position]);
};

template <typename T>
void ZReplayLoader::Peek(T& Obj)
{
	ReadAt(Obj, Position);
};

template <typename T>
void ZReplayLoader::Read(T& Obj)
{
	Peek(Obj);
	Position += sizeof(Obj);
};

template <typename T>
bool ZReplayLoader::TryRead(T& Obj)
{
	if (Position + sizeof(Obj) > InflatedFile.size())
		return false;

	Obj = *((decltype(&Obj))&InflatedFile[Position]);
	Position += sizeof(Obj);

	return true;
};

inline void ZReplayLoader::ReadN(void* Obj, size_t Size)
{
	if (Position + Size > InflatedFile.size())
		throw EOFException(Position);

	memcpy(Obj, &InflatedFile[Position], Size);
	Position += Size;
};

inline bool ZReplayLoader::FixCommand(MCommand& Command)
{
	if (Version.Server == SERVER::FREESTYLE_GUNZ && IsDojo)
	{
		auto Transform = [](float pos[3])
		{
			pos[0] -= 600;
			pos[1] += 2800;
			pos[2] += 400;
		};

		if (Command.GetID() == MC_PEER_BASICINFO)
		{
			MCommandParameter* pParam = Command.GetParameter(0);
			if (pParam->GetType() != MPT_BLOB)
				return false;

			ZPACKEDBASICINFO* ppbi = (ZPACKEDBASICINFO*)pParam->GetPointer();

			float pos[3] = { (float)ppbi->posx, (float)ppbi->posy, (float)ppbi->posz };

			if (pos[2] < 0)
			{
				Transform(pos);

				ppbi->posx = pos[0];
				ppbi->posy = pos[1];
				ppbi->posz = pos[2];
			}
		}
	}
	else if (Version.Server == SERVER::OFFICIAL && Version.nVersion == 11)
	{
		if (Command.GetID() == MC_PEER_BASICINFO)
		{
			auto Param = (MCommandParameterBlob*)Command.GetParameter(0);
			if (Param->GetType() != MPT_BLOB)
				return false;

			auto pbi = Param->GetPointer();

			[pbi = (short*)pbi]
			{
				for (int i = 0; i < 3; i++)
				{
					pbi[8 + i] = pbi[11 + i];
				}
			}();

			[pbi = (BYTE*)pbi]
			{
				for (int i = 0; i < 3; i++)
				{
					pbi[22 + i] = pbi[28 + i];
				}
			}();

			[pbi = (ZPACKEDBASICINFO*)pbi]
			{
				pbi->posz -= 125;
				pbi->velx = 0;
				pbi->vely = 0;
				pbi->velz = 0;

				ZBasicInfo bi;
				pbi->Unpack(bi);

				if (fabs(fabs(Magnitude(bi.direction)) - 1.f) > 0.001)
				{
					bi.direction = rvector(1, 0, 0);
					pbi->Pack(bi);
				}
			}();
		}
	}

	return true;
}

template <typename T>
bool ZReplayLoader::GetCommandsImpl(T fn, ArrayView<u32>* WantedCommandIDs)
{
	char CommandBuffer[1024];

	float fGameTime;
	Read(fGameTime);
	m_fGameTime = fGameTime;

	int nSize;
	float fTime;

	while (TryRead(fTime))
	{
		MUID uidSender;
		Read(uidSender);
		Read(nSize);

		if (nSize <= 0 || nSize > sizeof(CommandBuffer))
			return false;

		ReadN(CommandBuffer, nSize);

		if (WantedCommandIDs)
		{
			u32 CommandID = *(u16 *)(CommandBuffer + sizeof(u16));

			bool OuterContinue = true;

			for (auto id : *WantedCommandIDs)
			{
				if (id == CommandID)
				{
					OuterContinue = false;
					break;
				}
			}

			if (OuterContinue)
				continue;
		}
		
		fn(CommandBuffer, uidSender, fTime);
	}

	return true;
}

template <typename T>
bool ZReplayLoader::GetCommands(T ForEachCommand, bool PersistentMCommands, ArrayView<u32>* WantedCommandIDs)
{
	auto DoStuff = [&](MCommand& Command, const MUID& Sender, auto fTime)
	{
		Command.m_Sender = Sender;

		if (!FixCommand(Command))
			return;

		ForEachCommand(&Command, fTime);
	};

	if (PersistentMCommands)
	{
		auto Stuff = [&](const char *CommandBuffer, const MUID& Sender, auto fTime)
		{
			MCommand* Command = new MCommand;

			if (!CreateCommandFromStream(CommandBuffer, *Command))
				return;

			DoStuff(*Command, Sender, fTime);
		};

		if (!GetCommandsImpl(Stuff, WantedCommandIDs))
			return false;
	}
	else
	{
		MCommand StackCommand;
		u8 Stack[512];
		stack_allocator<u8, ArraySize(Stack)> Alloc(Stack);

		auto Stuff = [&](const char *CommandBuffer, const MUID& Sender, auto fTime)
		{
			auto ClearStackCommandParams = [&]()
			{
				for (auto Param : StackCommand.m_Params)
				{
					Alloc.destroy(Param);

					size_t size = 0;

					switch (Param->GetType())
					{
					case MPT_INT: size = sizeof(MCommandParameterInt); break;
					case MPT_UINT: size = sizeof(MCommandParameterUInt); break;
					case MPT_FLOAT: size = sizeof(MCommandParameterFloat); break;
					case MPT_STR: size = sizeof(MCommandParameterStringCustomAlloc<decltype(Alloc)>); break;
					case MPT_VECTOR: size = sizeof(MCommandParameterVector); break;
					case MPT_POS: size = sizeof(MCommandParameterPos); break;
					case MPT_DIR: size = sizeof(MCommandParameterDir); break;
					case MPT_BOOL: size = sizeof(MCommandParameterBool); break;
					case MPT_COLOR: size = sizeof(MCommandParameterColor); break;
					case MPT_UID: size = sizeof(MCommandParameterUID); break;
					case MPT_BLOB: size = sizeof(MCommandParameterBlobCustomAlloc<decltype(Alloc)>); break;
					case MPT_CHAR: size = sizeof(MCommandParameterChar); break;
					case MPT_UCHAR: size = sizeof(MCommandParameterUChar); break;
					case MPT_SHORT: size = sizeof(MCommandParameterShort); break;
					case MPT_USHORT: size = sizeof(MCommandParameterUShort); break;
					case MPT_INT64: size = sizeof(MCommandParameterInt64); break;
					case MPT_UINT64: size = sizeof(MCommandParameterUInt64); break;
					case MPT_SVECTOR: size = sizeof(MCommandParameterShortVector); break;
					};

					Alloc.deallocate((uint8_t*)Param, size);
				}

				StackCommand.m_Params.clear();
			};

			if (!StackCommand.m_Params.empty())
			{
				MLog("Not empty! size %d, command id %d\n", StackCommand.m_Params.size(), StackCommand.GetID());
				StackCommand.m_Params.clear();
			}

			if (CreateCommandFromStream(CommandBuffer, StackCommand, Alloc))
			{
				DoStuff(StackCommand, Sender, fTime);
			}
			else
			{
				//MLog("Failed to read command ID %d, total size %d\n", *(u16 *)(CommandBuffer + sizeof(u16)), *(u16*)CommandBuffer);
			}

			ClearStackCommandParams();
		};

		if (!GetCommandsImpl(Stuff, WantedCommandIDs))
			return false;
	}

	return true;
}
