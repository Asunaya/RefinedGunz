#pragma once

#include "LauncherConfig.h"
#include "XML.h"
#include "Download.h"
#include "File.h"
#include "Sync.h"
#include "SafeString.h"
#include "FileCache.h"
#include "defer.h"
#include "MInetUtil.h"

template <size_t OutputSize>
void GetPatchFileURL(char(&Output)[OutputSize], const StringView& Path, bool Client = true)
{
	char URLEncodedPath[4096];
	URLEncode(URLEncodedPath, Path);

	sprintf_safe(Output, "%s/patch/%s%s",
		LauncherConfig::PatchDomain,
		Client ? "client/" : "",
		URLEncodedPath);
}

template <size_t OutputSize>
void GetSyncFileURL(char(&Output)[OutputSize], const StringView& Path)
{
	char URLEncodedPath[4096];
	URLEncode(URLEncodedPath, Path);

	sprintf_safe(Output, "%s/patch/sync/%s.sync",
		LauncherConfig::PatchDomain,
		URLEncodedPath);
}

inline bool GetPatchXML(XMLFile& PatchXML, const DownloadManagerType& DownloadManager)
{
	Log(LogLevel::Info, "Downloading patch.xml...\n\n");

	const char PatchXMLFilename[] = "patch.xml";

	char URL[1024];
	GetPatchFileURL(URL, PatchXMLFilename, false);

	return PatchXML.CreateFromDownload(DownloadManager,
		URL, LauncherConfig::PatchPort,
		PatchXMLFilename);
}

inline bool DownloadWholeFile(const DownloadManagerType& DownloadManager,
	const char* Filename, const char* OutputPath = nullptr,
	Hash::Strong* HashOutput = nullptr, u64* SizeOutput = nullptr)
{
	if (OutputPath == nullptr)
		OutputPath = Filename;

	Log(LogLevel::Info, "Downloading new %s...\n\n", Filename);

	char URL[1024];
	GetPatchFileURL(URL, Filename);

	if (MFile::Exists(OutputPath))
	{
		const auto DeletionSucceeded = MFile::Delete(OutputPath);
		if (!DeletionSucceeded)
		{
			Log(LogLevel::Error, "DeleteFile failed on %s!\n", OutputPath);
			return false;
		}
	}

	return DownloadFileToFile(DownloadManager,
		URL, LauncherConfig::PatchPort,
		OutputPath, nullptr, HashOutput, SizeOutput);
}

struct PatchFileResult
{
	bool Succeeded;
	bool FileChanged;
};

inline PatchFileResult PatchFile(DownloadManagerType& DownloadManager,
	const char* Filename,
	const char* OutputFilename,
	const StringView& WantedHashString,
	u64 ExpectedSize,
	FileCacheType& FileCache,
	rapidxml::xml_node<>& node)
{
	assert(Filename != nullptr);
	if (OutputFilename == nullptr)
		OutputFilename = Filename;

	PatchFileResult ret{};

	StringView ActualHashString;

	// A buffer that ActualHashString can reference,
	// in case we need to create the string.
	char HashStringMemory[Hash::Strong::MinimumStringSize];

	const auto CachedHash = FileCache.GetHash(Filename);
	bool FileExistsLocally = MFile::Exists(Filename);

	// If it was found in the cache, ActualHashString holds the correct hash.
	// Otherwise, we need to calculate the hash of the file.
	if (!CachedHash.empty())
	{
		ActualHashString = CachedHash;
	}
	else
	{
		Log(LogLevel::Info, "Couldn't find %s in cache\n", Filename);

		if (FileExistsLocally)
		{
			Hash::Strong ActualHash;
			const auto HashSuccess = ActualHash.HashFile(Filename);
			if (!HashSuccess)
			{
				Log(LogLevel::Error, "PatchFile -- HashFile on %s failed!\n", Filename);
				return ret;
			}

			ActualHash.ToString(HashStringMemory);
			ActualHashString = HashStringMemory;

			FileCache.Add(Filename, ActualHashString);
		}
		else
		{
			// The file doesn't exist, so we can't calculate the hash.
			// ActualHashString will just stay empty.
			Log(LogLevel::Info, "%s nonexistent\n", Filename);
		}
	}

	if (WantedHashString != ActualHashString)
	{
		Log(LogLevel::Info,
			"Patching file...\n"
			"Name: %s\n"
			"Wanted hash: %.*s\n"
			"Current hash: %.*s\n",
			Filename,
			WantedHashString.size(), WantedHashString.data(),
			ActualHashString.size(), ActualHashString.data());

		CreateDirectoriesIfNonexistent(OutputFilename);

		Hash::Strong NewHash;
		u64 NewFileSize;

		char FileURL[4096];
		GetPatchFileURL(FileURL, Filename);

		if (FileExistsLocally)
		{
			char SyncFileURL[4096];
			GetSyncFileURL(SyncFileURL, Filename);

			const auto Success = Sync::SynchronizeFile(Filename,
				FileURL, SyncFileURL,
				ExpectedSize,
				DownloadManager,
				&NewHash, &NewFileSize);
			if (!Success)
			{
				Log.Error("Failed to synchronize file!\n"
					"Local file path: %s\n"
					"Remote file URL: %s\n"
					"Sync file URL: %s\n",
					Filename, FileURL, SyncFileURL);
				return ret;
			}
		}
		else
		{
			const auto Success = DownloadWholeFile(DownloadManager,
				Filename, OutputFilename, &NewHash, &NewFileSize);
			if (!Success)
			{
				Log.Error("Downloading new file %s from URL %s failed!\n",
					Filename, FileURL);
				return ret;
			}
		}

		NewHash.ToString(HashStringMemory);
		ActualHashString = HashStringMemory;

		Log.Debug("NewFileSize = %llu, NewHash = %s\n",
			NewFileSize, ActualHashString.data());

		// Verify the hash and size.
		// If wrong, we log it, delete the invalid file, and return.
		const auto WrongHash = WantedHashString != ActualHashString;
		const auto WrongSize = ExpectedSize != NewFileSize;
		if (WrongHash || WrongSize)
		{
			if (WrongHash)
			{
				Log(LogLevel::Error, "Hash of downloaded file %s doesn't match expected hash\n"
					"Expected: %.*s\n"
					"Actual: %.*s\n",
					Filename,
					WantedHashString.size(), WantedHashString.data(),
					ActualHashString.size(), ActualHashString.data());
			}
			else if (WrongSize)
			{
				Log(LogLevel::Error, "Size of downloaded file %s doesn't match expected size\n"
					"Expected: %llu\n"
					"Actual: %llu\n",
					Filename,
					ExpectedSize,
					NewFileSize);
			}

			MFile::Delete(OutputFilename);
			return ret;
		}

		ret.FileChanged = true;

		FileCache.Add(Filename, ActualHashString);
	}

	ret.Succeeded = true;
	return ret;
}