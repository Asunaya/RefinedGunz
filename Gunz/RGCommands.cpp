#include "stdafx.h"
#include "ZConfiguration.h"
#include "NewChat.h"
#include "RGMain.h"
#include "VoiceChat.h"
#include "Extensions.h"
#include <cstdint>

bool CheckDeveloperMode(const char* Name)
{
	if (ZApplication::GetInstance()->GetLaunchMode() != ZApplication::ZLAUNCH_MODE_STANDALONE_GAME)
	{
		ZChatOutputF("%s can only be used in developer mode", Name);
		return false;
	}

	return true;
};

void LoadRGCommands(ZChatCmdManager& CmdManager)
{
	CmdManager.AddCommand(0, "fpslimit", [](const char *line, int argc, char ** const argv){
		int nFPSLimit = atoi(argv[1]);
		ZGetConfiguration()->nFPSLimit = nFPSLimit;
		ZChatOutputF("FPS limit set to %d", nFPSLimit);

		ZGetConfiguration()->Save();
	},
		CCF_ALL, 1, 1, true, "/fpslimit <fps>", "");


	CmdManager.AddCommand(0, "camfix", [](const char *line, int argc, char ** const argv){
		bool bCamFix;
		if (argc == 1)
			bCamFix = !ZGetConfiguration()->GetCamFix();
		else
			bCamFix = atoi(argv[1]) != 0;

		ZGetConfiguration()->bCamFix = bCamFix;
		if (bCamFix)
			ZGetCamera()->m_fDist = CAMERA_DEFAULT_DISTANCE * RGetScreenWidth() / RGetScreenHeight() / (4.f / 3.f);
		else
			ZGetCamera()->m_fDist = CAMERA_DEFAULT_DISTANCE;

		ZGetConfiguration()->Save();

		ZChatOutputF("Cam fix %s", bCamFix ? "enabled" : "disabled");
	},
		CCF_ALL, 0, 1, true, "/camfix [0/1]", "");


	CmdManager.AddCommand(0, "backgroundcolor", [](const char *line, int argc, char ** const argv){
		DWORD BackgroundColor = strtoul(argv[1], NULL, 16);
		g_Chat.SetBackgroundColor(BackgroundColor);
		ZGetConfiguration()->ChatBackgroundColor = BackgroundColor;

		ZGetConfiguration()->Save();

		ZChatOutputF("Background color set to %08X", BackgroundColor);
	},
		CCF_ALL, 1, 1, true, "/backgroundcolor <AARRGGBB hex color>", "");


	CmdManager.AddCommand(0, "replayseek",
		[](const char *line, int argc, char ** const argv){
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
			case -1:
				ZChatOutputF("No player with %s in their name was found", argv[1]);
				break;

			case -2:
				ZChatOutputF("Too many players with %s in their name was found", argv[1]);
				break;

			default:
				ZChatOutputF("Unknown error");
			};
			
			return;
		}

		bool b = g_RGMain->MutePlayer(ret.second->GetUID());

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
		CCF_ALL, 1, 1, true, "/timescale <scale>", "");
}

#ifdef TIMESCALE
unsigned long long GetGlobalTimeMS()
{
	//if (!ZApplication::GetInstance())
		return timeGetTime();

	return ZApplication::GetInstance()->GetTime();
}
#endif