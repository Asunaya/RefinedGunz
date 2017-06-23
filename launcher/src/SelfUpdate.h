#include "MProcess.h"
#include "FileCache.h"
#include "Download.h"

enum class SelfUpdateResultType
{
	Updated,
	NoUpdateAvailable,
	Failed,
};

inline SelfUpdateResultType SelfUpdate(DownloadManagerType& DownloadManager,
	const XMLFile& PatchXML, rapidxml::xml_node<>& files_node,
	FileCacheType& FileCache)
{
	auto&& Range = GetFileNodeRange(files_node);

	// Find the node for the launcher.xml file.
	auto it = std::find_if(std::begin(Range), std::end(Range), [&](auto&& node) {
		return iequals(PatchXML.GetName(node), LauncherConfig::LauncherFilename);
	});

	if (it == std::end(Range))
	{
		Log(LogLevel::Error, "Failed to find launcher.exe in patch.xml!\n");
		return SelfUpdateResultType::Failed;
	}

	auto&& node = *it;
	auto Filename = PatchXML.GetName(node);
	auto WantedHash = PatchXML.GetHash(node, Filename);
	auto Size = PatchXML.GetSize(node, Filename);
	if (Filename.empty() || WantedHash.empty() || !Size.has_value())
		return SelfUpdateResultType::Failed;

	auto PatchResult = PatchFile(DownloadManager,
		Filename.data(), "launcher_new.exe",
		WantedHash, Size.value(),
		FileCache, node);

	if (!PatchResult.Succeeded)
	{
		Log(LogLevel::Error, "Failed to patch launcher.exe!\n");
		return SelfUpdateResultType::Failed;
	}

	if (!PatchResult.FileChanged)
		return SelfUpdateResultType::NoUpdateAvailable;

#ifdef _DEBUG
	// Don't self-update under debug.
	Log(LogLevel::Debug, "Ignoring available update\n");
	return SelfUpdateResultType::NoUpdateAvailable;
#endif

	MFile::Move("launcher.exe", "launcher_old.exe");
	MFile::Move("launcher_new.exe", "launcher.exe");
	MProcess::Start("launcher.exe");
	return SelfUpdateResultType::Updated;
}