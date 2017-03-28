#include "stdafx.h"
#include "ZConfiguration.h"
#include "NewChat.h"
#include "RGMain.h"
#include "VoiceChat.h"
#include "Config.h"
#include "RBspObject.h"
#include "RS2.h"
#include <cstdint>
#include "ZOptionInterface.h"
#include "ZTestGame.h"

bool CheckDeveloperMode(const char* Name)
{
	if (ZApplication::GetInstance()->GetLaunchMode() != ZApplication::ZLAUNCH_MODE_STANDALONE_GAME)
	{
		ZChatOutputF("%s can only be used in developer mode", Name);
		return false;
	}

	return true;
}

struct BoolResult
{
	bool Success;
	bool Value;
};
static BoolResult ParseBool(int argc, char ** argv)
{
	auto Argument = argv[1];
	if (!_stricmp(Argument, "true") || !strcmp(Argument, "1")) {
		return{ true, true };
	}
	else if (!_stricmp(Argument, "false") || !strcmp(Argument, "0")) {
		return{ true, false };
	}

	return{ false, false };
}

// Sets the value of a bool according to the given arguments, and outputs feedback to user in chat.
//
// If no arguments are given, the value is toggled.
// If one argument is given, it's parsed by ParseBool: 
// "true" or "1" sets the bool to true,
// and "false" or "0" sets it to false.
//
// Returns false on invalid arguments. No change to the variable is applied in this case.
static bool SetBool(const char* Name, bool& Value, int argc, char ** argv)
{
	if (argc < 1 || argc > 2) {
		assert(false);
		return false;
	}

	if (argc > 1)
	{
		// We got an argument. Parse it into a bool.
		auto ret = ParseBool(argc, argv);
		if (!ret.Success)
		{
			ZChatOutputF("%s is not a valid bool argument.", argv[1]);
			return false;
		}

		Value = ret.Value;
	}
	else
	{
		// No arguments. Toggle the value.
		Value = !Value;
	}

	ZChatOutputF("%s %s.", Name, Value ? "enabled" : "disabled");
	return true;
}

void LoadRGCommands(ZChatCmdManager& CmdManager)
{
	auto VisualFPSLimit = [](const char *line, int argc, char ** const argv) {
		int nFPSLimit = atoi(argv[1]);
		ZGetConfiguration()->VisualFPSLimit = nFPSLimit;
		ZChatOutputF("Visual FPS limit set to %d", nFPSLimit);

		ZGetConfiguration()->Save();
	};

	auto LogicalFPSLimit = [](const char *line, int argc, char ** const argv) {
		int nFPSLimit = atoi(argv[1]);
		ZGetConfiguration()->LogicalFPSLimit = nFPSLimit;
		ZChatOutputF("Logical FPS limit set to %d", nFPSLimit);

		ZGetConfiguration()->Save();
	};

	CmdManager.AddCommand(0, "visualfpslimit", VisualFPSLimit,
		CCF_ALL, 1, 1, true, "/visualfpslimit <fps>", "");
	CmdManager.AddCommand(0, "logicalfpslimit", LogicalFPSLimit,
		CCF_ALL, 1, 1, true, "/logicalfpslimit <fps>", "");

	// Alias /fpslimit to /logicalfpslimit
	CmdManager.AddAlias("fpslimit", "logicalfpslimit");


	CmdManager.AddCommand(0, "camfix", [](const char *line, int argc, char ** const argv) {
		if (SetBool("Cam fix", ZGetConfiguration()->bCamFix, argc, argv)) {
			ZGetConfiguration()->Save();
		}
	},
		CCF_ALL, 0, 1, true, "/camfix [0/1]", "");


	CmdManager.AddCommand(0, "interfacefix", [](const char *line, int argc, char ** const argv) {
		if (SetBool("Interface fix", ZGetConfiguration()->InterfaceFix, argc, argv)) {
			ZGetConfiguration()->Save();
			Mint::GetInstance()->SetStretch(!ZGetConfiguration()->InterfaceFix);
		}
	},
		CCF_ALL, 0, 1, true, "/interfacefix [0/1]", "");


	CmdManager.AddCommand(0, "backgroundcolor", [](const char *line, int argc, char ** const argv){
		DWORD BackgroundColor = strtoul(argv[1], nullptr, 16);
		GetRGMain().GetChat().SetBackgroundColor(BackgroundColor);
		ZGetConfiguration()->ChatBackgroundColor = BackgroundColor;

		ZGetConfiguration()->Save();

		ZChatOutputF("Background color set to %08X", BackgroundColor);
	},
		CCF_ALL, 1, 1, true, "/backgroundcolor <AARRGGBB hex color>", "");


	CmdManager.AddCommand(0, "replayseek", [](const char *line, int argc, char ** const argv) {
		ZGetGame()->SetReplayTime(atof(argv[1]));
	},
		CCF_ALL, 1, 1, true, "/replayseek <time>", "");


	CmdManager.AddCommand(0, "spec", [](const char *line, int argc, char ** const argv){
		if (ZGetGameInterface()->GetState() != GUNZ_GAME)
			return;

		ZCharacter &mychar = *ZGetGame()->m_pMyCharacter;
		if (mychar.GetTeamID() == MMT_SPECTATOR)
		{
			ZChatOutputF("Spectator mode is already enabled");
			return;
		}

		mychar.SetTeamID(MMT_SPECTATOR);
		mychar.ForceDie();
		ZPostStageTeam(ZGetGame()->m_pMyCharacter->GetUID(), ZGetGameClient()->GetStageUID(), MMT_SPECTATOR);
		ZPostSpec();

		ZChatOutputF("Spectator mode enabled.");
	}, CCF_ALL, 0, 0, true, "/spec", "");

	CmdManager.AddCommand(0, "fullscreen", [](const char *line, int argc, char ** const argv){
		ZGetConfiguration()->GetVideo()->FullscreenMode = static_cast<FullscreenType>(atoi(argv[1]));

		RMODEPARAMS ModeParams = { RGetScreenWidth(), RGetScreenHeight(),
			ZGetConfiguration()->GetVideo()->FullscreenMode, RGetPixelFormat() };

		RResetDevice(&ModeParams);

		ZGetConfiguration()->Save();
	}, CCF_ALL, 1, 1, true, "/fullscreen", "");

	CmdManager.AddCommand(0, "resolution", [](const char *line, int argc, char ** const argv) {
		auto Width = atoi(argv[1]);
		auto Height = atoi(argv[2]);
		if (Width == 0 || Height == 0)
		{
			ZChatOutput("Invalid resolution");
			return;
		}
		ZGetConfiguration()->GetVideo()->nWidth = Width;
		ZGetConfiguration()->GetVideo()->nHeight = Height;

		RMODEPARAMS ModeParams = { Width, Height,
			ZGetConfiguration()->GetVideo()->FullscreenMode, RGetPixelFormat() };

		RResetDevice(&ModeParams);

		Mint::GetInstance()->SetWorkspaceSize(ModeParams.nWidth, ModeParams.nHeight);
		Mint::GetInstance()->GetMainFrame()->SetSize(ModeParams.nWidth, ModeParams.nHeight);

		ZGetOptionInterface()->Resize(ModeParams.nWidth, ModeParams.nHeight);

		ZGetConfiguration()->Save();
	}, CCF_ALL, 2, 2, true, "/resolution <width> <height>", "");

	auto Sens = [](const char *line, int argc, char ** const argv) {
		float fSens = atof(argv[1]);

		ZGetConfiguration()->GetMouse()->fSensitivity = fSens;

		ZGetConfiguration()->Save();

		ZChatOutputF("Sensitivity set to %f", fSens);
	};

	CmdManager.AddCommand(0, "sensitivity", Sens, CCF_ALL, 1, 1, true, "/sensitivity <value>", "");
	CmdManager.AddCommand(0, "sens", Sens, CCF_ALL, 1, 1, true, "/sens <value>", "");

	CmdManager.AddCommand(0, "admin_mute", [](const char *line, int argc, char ** const argv){
		ZPostAdminMute(argv[1], argv[3] ? argv[3] : "", atoi(argv[3]) * 60);
	}, CCF_ADMIN, 2, 3, true, "/admin_mute <name> <minutes> [reason]", "");

	CmdManager.AddCommand(0, "argv", [](const char *line, int argc, char ** const argv) {
		for (int i = 0; i < argc; i++)
			ZChatOutputF("%s", argv[i]);
	}, CCF_ALL, ARGVNoMin, ARGVNoMax, true, "/argv", "");

	CmdManager.AddCommand(0, "swordcolor", [](const char *line, int argc, char ** const argv) {
		uint32_t Color = strtoul(argv[1], NULL, 16);
		ZPOSTCMD1(MC_PEER_SET_SWORD_COLOR, MCmdParamUInt(Color));
	}, CCF_ALL, 1, 1, true, "/swordcolor <AARRGGBB>", "");

#ifdef VOICECHAT
	CmdManager.AddCommand(0, "mute", [](const char *line, int argc, char ** const argv) {
		auto ret = FindSinglePlayer(argv[1]);

		if (!ret.second)
		{
			switch (ret.first)
			{
			case PlayerFoundStatus::NotFound:
				ZChatOutputF("No player with %s in their name was found", argv[1]);
				break;

			case PlayerFoundStatus::TooManyFound:
				ZChatOutputF("Too many players with %s in their name was found", argv[1]);
				break;

			default:
				ZChatOutputF("Unknown error %d", static_cast<int>(ret.first));
			};
			
			return;
		}

		bool b = GetRGMain().MutePlayer(ret.second->GetUID());

		ZChatOutputF("%s has been %s", ret.second->GetUserName(), b ? "muted" : "unmuted");
	}, CCF_ALL, 1, 1, true, "/swordcolor <AARRGGBB>", "");
#endif

	CmdManager.AddCommand(0, "debug", [](const char *line, int argc, char ** const argv) {
		ZGetConfiguration()->HitRegistrationDebugOutput = !ZGetConfiguration()->HitRegistrationDebugOutput;
		ZGetConfiguration()->Save();

		ZGetGameClient()->ClientSettings.DebugOutput = ZGetConfiguration()->HitRegistrationDebugOutput;
		ZPostClientSettings(ZGetGameClient()->ClientSettings);

		ZChatOutputF("Debug output %s",
			ZGetConfiguration()->HitRegistrationDebugOutput ? "enabled" : "disabled");
	},
		CCF_ALL, 0, 0, true, "/debug", "");

	CmdManager.AddCommand(0, "hello", [](const char *line, int argc, char ** const argv) {
		ZChatOutput("Hi! ^__^", ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
	},
		CCF_ALL, 0, 0, true, "/hello", "");

	CmdManager.AddCommand(0, "clear", [](const char *line, int argc, char ** const argv) {
		GetRGMain().GetChat().ClearHistory();
	},
		CCF_ALL, 0, 0, true, "/clear", "");

	CmdManager.AddCommand(0, "fov", [](const char *line, int argc, char ** const argv) {
		float fov_radians = DEFAULT_FOV;
		if (argc > 1)
		{
			auto arg = atof(argv[1]);
			if (arg != 0)
				fov_radians = ToRadian(arg);
		}

		g_fFOV = fov_radians;
		ZChatOutputF("Field of view set to %d degrees", int(ToDegree(fov_radians)));
	},
		CCF_ALL, 0, 1, true, "/fov [value, in degrees]", "");

	CmdManager.AddCommand(0, "monochrome", [](const char *line, int argc, char ** const argv) {
		static bool Enabled = false;
		if (SetBool("Monochrome", Enabled, argc, argv)) {
			auto Success = GetRenderer().PostProcess.EnableEffect("Monochrome", Enabled);
			assert(Success);
		}
	},
		CCF_ALL, 0, 1, true, "/monochrome [0/1]", "");

	CmdManager.AddCommand(0, "colorinvert", [](const char *line, int argc, char ** const argv) {
		static bool Enabled = false;
		if (SetBool("Color invert", Enabled, argc, argv)) {
			auto Success = GetRenderer().PostProcess.EnableEffect("ColorInvert", Enabled);
			assert(Success);
		}
	},
		CCF_ALL, 0, 1, true, "/colorinvert [0/1]", "");

	CmdManager.AddCommand(0, "setparts", [](const char *line, int argc, char ** const argv) {
		if (!CheckDeveloperMode("setparts"))
			return;

		ZGetGame()->m_pMyCharacter->m_pVMesh->SetParts((RMeshPartsType)atoi(argv[1]), argv[2]);
	}, CCF_ALL, 0, 0, true, "/setparts", "");

	CmdManager.AddCommand(0, "timescale", [](const char *line, int argc, char ** const argv) {
		if (!CheckDeveloperMode("Timescale"))
			return;

		float NewTimescale = atof(argv[1]);
		ZApplication::GetInstance()->SetTimescale(NewTimescale);

		ZChatOutputF("Set timescale to %f, %s.", NewTimescale, argv[1]);
	},
		CCF_ALL, 1, 1, true, "/timescale <scale>", "");

	CmdManager.AddCommand(0, "scale", [](const char *line, int argc, char ** const argv) {
		if (!CheckDeveloperMode("Mesh scaling"))
			return;

		auto Scale = static_cast<float>(atof(argv[1]));
		ZGetGame()->m_pMyCharacter->SetScale(Scale);
		ZChatOutputF("Set visual mesh scale to %f\n", Scale);
	},
		CCF_ALL, 1, 1, true, "/scale", "");

	CmdManager.AddCommand(0, "scalenode", [](const char *line, int argc, char ** const argv) {
		if (!CheckDeveloperMode("Node scaling"))
			return;

#ifdef SCALE_RMESHNODE
		auto Parts = static_cast<RMeshPartsPosInfoType>(atoi(argv[1]));//RMeshPartsType
		auto Scale = static_cast<float>(atof(argv[2]));
		v3 v{ Scale, Scale, Scale };
		auto* Node = ZGetGame()->m_pMyCharacter->m_pVMesh->m_pMesh->FindNode(Parts);
		if (!Node)
		{
			ZChatOutputF("Couldn't find node %d", Parts);
			return;
		}
		Node->SetScale(v);
		//ZGetGame()->m_pMyCharacter->m_pVMesh->m_pMesh->FindNode(Parts)->SetScale(v);
		ZChatOutputF("Set visual mesh node %d scale to %f", static_cast<int>(Parts), Scale);
#else
		ZChatOutputF("Unsupported. Compile with SCALE_RMESHNODE defined for support.");
#endif
	},
		CCF_ALL, 2, 2, true, "/scalenode", "");

	CmdManager.AddCommand(0, "camdist", [](const char *line, int argc, char ** const argv) {
		if (!CheckDeveloperMode("camdist"))
			return;

		auto val = static_cast<float>(atof(argv[1]));
		ZGetCamera()->m_fDist = val;

		ZChatOutputF("Set camdist to %f", val);
	},
		CCF_ALL, 1, 1, true, "/camdist <dist>", "");

	CmdManager.AddCommand(0, "vsync", [](const char *line, int argc, char ** const argv) {
		bool Value = false;
		if (SetBool("Vertical synchronization", Value, argc, argv)) {
			SetVSync(Value);
			RMODEPARAMS Params{ RGetScreenWidth(), RGetScreenHeight(),
				RGetFullscreenMode(), RGetPixelFormat()
			};
			RResetDevice(&Params);
		}
	},
		CCF_ALL, 1, 1, true, "/vsync <0/1>", "");

	CmdManager.AddCommand(0, "freelook", [](const char *line, int argc, char ** const argv) {
		if (!CheckDeveloperMode("freelook"))
			return;

		bool Value = ZGetCamera()->GetLookMode() == ZCAMERA_FREELOOK;
		if (SetBool("Freelook", Value, argc, argv)) {
			ZGetCamera()->SetLookMode(Value ? ZCAMERA_FREELOOK : ZCAMERA_DEFAULT);
		}
	},
		CCF_ALL, 1, 1, true, "/freelook <0/1>", "");

	CmdManager.AddCommand(0, "showrts", [](const char *line, int argc, char ** const argv) {
		if (!CheckDeveloperMode("showrts") || !GetRS2().UsingD3D9())
			return;

		SetBool("Show RTs", GetRenderer().ShowRTsEnabled, argc, argv);
	},
		CCF_ALL, 0, 1, true, "/showrts [0/1]", "");

	CmdManager.AddCommand(0, "map", [](const char *line, int argc, char ** const argv) {
		if (!CheckDeveloperMode("map"))
			return;

		ZGetGameInterface()->SetState(GUNZ_LOBBY);
		CreateTestGame(argv[1]);
	},
		CCF_ALL, 0, 1, true, "/map <name>", "");

	CmdManager.AddCommand(0, "equip", [](const char *line, int argc, char ** const argv) {
		if (!CheckDeveloperMode("equip"))
			return;

		int id = -1;
		{
			auto pair = StringToInt(argv[1]);
			if (!pair.first)
			{
				ZChatOutput("Malformed item ID");
				return;
			}
			id = pair.second;
		}

		auto desc = MGetMatchItemDescMgr()->GetItemDesc(id);
		if (!desc)
		{
			ZChatOutput("Non-existent item ID");
			return;
		}

		auto GetSlot = [&]
		{
			MMatchCharItemParts slot = MMCIP_END;
			switch (desc->m_nSlot)
			{
			case MMIST_MELEE:
				return MMCIP_MELEE;
			case MMIST_HEAD:
				return MMCIP_HEAD;
			case MMIST_CHEST:
				return MMCIP_CHEST;
			case MMIST_HANDS:
				return MMCIP_HANDS;
			case MMIST_LEGS:
				return MMCIP_LEGS;
			case MMIST_FEET:
				return MMCIP_FEET;
			case MMIST_RANGE:
				slot = MMCIP_PRIMARY;
				break;
			case MMIST_FINGER:
				slot = MMCIP_FINGERL;
				break;
			case MMIST_EXTRA:
				slot = MMCIP_CUSTOM1;
				break;
			}
			if (argc > 2)
				slot = MMatchCharItemParts(int(slot) + 1);
			return slot;
		};

		auto slot = GetSlot();
		if (slot == MMCIP_END)
		{
			ZChatOutputF("Item ID %d has invalid slot type %d", id, desc->m_nSlot);
			return;
		}

		auto success = ZGetGame()->m_pMyCharacter->m_Items.EquipItem(slot, id);
		if (!success)
		{
			ZChatOutputF("Failed to equip item %d in slot %d", id, slot);
			return;
		}
	},
		CCF_ALL, 1, 2, true, "/equip <zitem id> [1]", "");

	void SetPortalFrustraDrawLock(bool Value);
	bool GetPortalFrustraDrawLock();
	void SetPortalFrustraDrawEnabled(bool Value);
	bool GetPortalFrustraDrawEnabled();

	CmdManager.AddCommand(0, "lockfrustra", [](const char *line, int argc, char ** const argv) {
		if (!CheckDeveloperMode("lockfrustra"))
			return;

		bool Value = GetPortalFrustraDrawLock();
		if (SetBool("Frustra draw lock", Value, argc, argv)) {
			SetPortalFrustraDrawLock(Value);
		}
	},
		CCF_ALL, 0, 1, true, "", "");

	CmdManager.AddCommand(0, "drawfrustra", [](const char *line, int argc, char ** const argv) {
		if (!CheckDeveloperMode("drawfrustra"))
			return;

		bool Value = GetPortalFrustraDrawEnabled();
		if (SetBool("Frustra draw", Value, argc, argv)) {
			SetPortalFrustraDrawEnabled(Value);
		}
	},
		CCF_ALL, 0, 1, true, "", "");
}

#ifdef TIMESCALE
unsigned long long GetGlobalTimeMS()
{
	if (!ZApplication::GetInstance())
		return timeGetTime();

	return ZApplication::GetInstance()->GetTime();
}
#endif