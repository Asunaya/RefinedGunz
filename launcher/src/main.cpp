#include "Patch.h"
#include "Log.h"
#include "XML.h"
#include "Download.h"
#include "FileCache.h"
#include "StringAllocator.h"
#include "SelfUpdate.h"
#include "LauncherConfig.h"

#include "defer.h"
#include "SafeString.h"
#include "MDebug.h"
#include "MProcess.h"
#include "MFile.h"
#include "Hash.h"
#include "StringView.h"
#include "ArrayView.h"

#include <thread>
#include <chrono>
#include <string>
#include <vector>

using namespace std::chrono_literals;
using namespace std::string_literals;

// Deletes any temporary files left over from a previous run of the program.
static void DeleteResidualTemporaryFiles()
{
	// launcher_new.exe should not normally exist at this point, but if it does, delete it anyway.
	if (MFile::Exists("launcher_new.exe"))
	{
		Log(LogLevel::Debug, "Deleting launcher_new.exe\n");

		if (!MFile::Delete("launcher_new.exe"))
			Log(LogLevel::Error, "DeleteFile failed on launcher_new.exe!\n");
	}
	
	if (MFile::Exists("launcher_old.exe"))
	{
		// Sleep so that the old launcher instance has a chance to close.
		std::this_thread::sleep_for(100ms);

		Log(LogLevel::Debug, "Deleting launcher_old.exe\n");

		if (!MFile::Delete("launcher_old.exe"))
			Log(LogLevel::Error, "DeleteFile failed on launcher_old.exe!\n");
	}
}

static void PatchFiles(XMLFile& xml, rapidxml::xml_node<>& ParentNode,
	DownloadManagerType& DownloadManager, FileCacheType& FileCache)
{
	for (auto&& node : GetFileNodeRange(ParentNode))
	{
		auto Filename = xml.GetName(node);
		if (Filename.empty() || Filename == LauncherConfig::LauncherFilename)
			continue;

		auto ExpectedHash = xml.GetHash(node, Filename);
		auto ExpectedSize = xml.GetSize(node, Filename);
		if (ExpectedHash.empty() || !ExpectedSize.has_value())
			continue;

		auto ret = PatchFile(DownloadManager,
			Filename.data(), Filename.data(),
			ExpectedHash, ExpectedSize.value(),
			FileCache,
			node);

		Log(LogLevel::Debug, "PatchFile on %s returned Succeeded = %d, FileChanged = %d\n",
			Filename.data(), ret.Succeeded, ret.FileChanged);
	}
}

struct Options
{
	bool IgnoreSelfUpdate{};
};

static bool HandleArguments(Options& Opt, int argc, char** argv)
{
	// Start i at 1 because argv[0] is the program name.
	for (int i = 1; i < argc; ++i)
	{
		static constexpr char VerbosityOpt[] = "--verbosity=";
		static constexpr char IgnoreSelfUpdateOpt[] = "--ignore-self-update";

		auto Check = [&](auto&& String) {
			return strncmp(argv[i], String, strlen(String)) == 0;
		};

		if (Check(VerbosityOpt))
		{
			auto MaybeVerbosity = StringToInt<int>(argv[i] + strlen(VerbosityOpt));
			if (!MaybeVerbosity.has_value())
			{
				Log.Fatal("Invalid --verbosity option value. "
					"Expected integral value, got %s\n",
					argv[i]);
				return false;
			}

			auto Verbosity = MaybeVerbosity.value();
			Log.DebugVerbosity = Verbosity;
		}
		else if (Check(IgnoreSelfUpdateOpt))
		{
			Opt.IgnoreSelfUpdate = true;
		}
		else
		{
			Log.Fatal("Unknown option %s\n", argv[i]);
			return false;
		}
	}

	return true;
}

int main(int argc, char** argv)
{
	// Add a three-second delay before closing the console window 
	// so that users can actually read the final output.
	// Can be disabled by setting ShouldSleep to false, in order to exit instantly.
	bool ShouldSleep = true;
	auto OnExit = [&] {
#ifndef _DEBUG
		if (ShouldSleep)
			std::this_thread::sleep_for(3s);
#else
		Log(LogLevel::Debug, "Enter anything to end the program\n");
		getchar();
#endif
	};
	DEFER(OnExit);

	Log.InitFile("launcher_log.txt");

	Options Opt;
	if (!HandleArguments(Opt, argc, argv))
		return -1;

	auto DownloadManager = CreateDownloadManager();
	if (!DownloadManager)
	{
		Log(LogLevel::Fatal, "Failed to initialize download manager!\n");
		return -1;
	}

	Log(LogLevel::Debug, "Created download manager!\n");

	DeleteResidualTemporaryFiles();

	// The patch.xml file's memory must be in scope for the rest of the program,
	// since references to parts of it are retained in many places.
	XMLFile PatchXML;
	if (!GetPatchXML(PatchXML, DownloadManager))
		return -1;

	// Load the file cache.
	FileCacheType FileCache;
	FileCache.Load();

	// Find the <files> node.
	auto* files = PatchXML.Doc.first_node("files");
	if (!files)
	{
		Log(LogLevel::Fatal, "Failed to find files node in patch.xml!\n");
		return false;
	}

	// Self-update.
	if (!Opt.IgnoreSelfUpdate)
	{
		auto SelfUpdateResult = SelfUpdate(DownloadManager, PatchXML, *files, FileCache);

		switch (SelfUpdateResult)
		{
		case SelfUpdateResultType::Updated:
			Log(LogLevel::Info, "Self-updated! Starting new launcher and terminating\n");
			// Exit immediately.
			ShouldSleep = false;
			return 0;

		case SelfUpdateResultType::NoUpdateAvailable:
			Log(LogLevel::Info, "No self-update available\n");
			break;

		case SelfUpdateResultType::Failed:
			Log(LogLevel::Fatal, "Can't self-update!\n");
			return -1;
		}
	}

	PatchFiles(PatchXML, *files, DownloadManager, FileCache);

	FileCache.Save();

#ifdef _DEBUG
	Log(LogLevel::Debug, "Enter anything to start Gunz\n");
	getchar();
#endif

	Log(LogLevel::Info, "Starting Gunz...\n");
	MProcess::Start("Gunz.exe");
}