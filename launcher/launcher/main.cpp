#include <Windows.h>
#include "../../sdk/rapidxml/include/rapidxml.hpp"
#include "../../sdk/rapidxml/include/rapidxml_print.hpp"
#include <io.h>
#include <fstream>
#include "md5.h"
#include "downloadstatus.h"
#include <unordered_map>
#define CURL_STATICLIB
#include "../../sdk/curl/include/curl/curl.h"
#include "defer.h"

#define PATCH_URL "http://refinedgunz.com"
//#define _DEBUG

#pragma comment(lib, "../../sdk/curl/lib/libcurl.lib")

#define F_OK 0

static CURL* curl = nullptr;

static bool CreateCurl()
{
	auto res = curl_global_init(CURL_GLOBAL_DEFAULT);
	if (res != CURLE_OK)
		return false;
	curl = curl_easy_init();
	if (!curl)
		return false;
	return true;
}

static void DestroyCurl()
{
	if (curl)
		curl_easy_cleanup(curl);
	curl_global_cleanup();
}

static bool FileExists(const char *szFilePath)
{
	return !_access(szFilePath, F_OK);
}

static void StartProcess(const char *commandline)
{
	char buf[128];
	strcpy_s(buf, commandline);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(NULL, buf, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		printf_s("CreateProcess failed (%d)\n", GetLastError());
		return;
	}
}

struct CurlWriteData {
	const char *filename;
	FILE *stream;
};

static size_t CurlWriteFunction(void *buffer, size_t size, size_t nmemb, void *stream)
{
	CurlWriteData *out = static_cast<CurlWriteData*>(stream);
	if (out && !out->stream) {
		out->stream = nullptr;
		fopen_s(&out->stream, out->filename, "wb");
		if (!out->stream)
			return -1; /* failure, can't open file to write */
	}
	return fwrite(buffer, size, nmemb, out->stream);
}

static bool DownloadFile(const char *URL, const char *OutputPath)
{
	CurlWriteData ftpfile = { OutputPath, nullptr };

	if (!curl)
	{
		printf_s("curl is dead! can't download files\n");
		return false;
	}

	curl_easy_setopt(curl, CURLOPT_URL, URL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteFunction);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);

	auto res = curl_easy_perform(curl);

	if (res != CURLE_OK) {
		fprintf(stderr, "curl told us %d: %s\n", res, curl_easy_strerror(res));
	}
	
	if (ftpfile.stream)
		fclose(ftpfile.stream);

	return true;
}

static bool ReadFile(const char *szFilename, std::string &Output)
{
	std::ifstream File(szFilename, std::ios::in | std::ios::binary | std::ios::ate);

	if (!File.is_open())
	{
		printf_s("Failed to open %s for reading\n", szFilename);
		return false;
	}

	int Size = (int)File.tellg();
	try
	{
		Output.resize(Size);
	}
	catch (std::bad_alloc& e)
	{
		printf_s("ReadFile - Caught std::bad_alloc while trying to resize to %d bytes for file - e.what() = %s", Size, e.what());
		return false;
	}
	File.seekg(std::ios::beg);
	File.read(&Output[0], Size);

	if (File.fail())
	{
		printf_s("Failed to read %d from %s\n", Size, szFilename);
		return false;
	}

	return true;
}

static bool WriteFile(const char *Filename, const char *StrToWrite, size_t Len)
{
	std::ofstream File(Filename);

	if (!File.is_open())
	{
		printf_s("Failed to open %s for writing\n", Filename);
		return false;
	}

	File.write(StrToWrite, Len);

	if (File.fail())
	{
		printf_s("Failed to write merged changes to %s\n", Filename);
		return false;
	}

	return true;
}

static void DeleteOldLauncher()
{
	if (FileExists("launcher_old.exe"))
	{
		// Sleep so that the old launcher instance has a chance to close.
		Sleep(100);
		if (!DeleteFile("launcher_old.exe"))
			printf_s("DeleteFile failed on launcher_old.exe!\n");
	}
	if (FileExists("launcher_new.exe"))
	{
		// Sleep so that the old launcher instance has a chance to close.
		Sleep(100);
		if (!DeleteFile("launcher_new.exe"))
			printf_s("DeleteFile failed on launcher_new.exe!\n");
	}
}

static bool DownloadPatch(rapidxml::xml_document<>& patchxml, std::string& patchfile)
{
	printf_s("Downloading patch.xml...\n\n");
	if (!DownloadFile(PATCH_URL "/patch/patch.xml", "patch.xml"))
		return false;

	if (!ReadFile("patch.xml", patchfile))
		return false;

	try
	{
		patchxml.parse<0>(&patchfile[0]);
	}
	catch (rapidxml::parse_error &e)
	{
		printf_s("RapidXML threw parse_error on patch.xml - e.what() = %s, e.where() = %s\n", e.what(), e.where<char>());
		return false;
	}

	return true;
}

static bool DownloadFullFile(const char* Filename, const char* OutputPath)
{
	auto IsLauncher = !strcmp(Filename, "launcher.exe");

	printf_s("Downloading new %s...\n\n", Filename);
	char URL[512];
	sprintf_s(URL, PATCH_URL "/patch/%s", Filename);
	if (FileExists(OutputPath) && !DeleteFile(OutputPath))
	{
		printf_s("DeleteFile failed on %s!\n", OutputPath);
		return false;
	}
	return DownloadFile(URL, OutputPath);
}

static bool DownloadFullFile(const char* Filename)
{
	DownloadFullFile(Filename, Filename);
}

struct CachedFileData
{
	uint64_t LastModifiedTime;
	uint64_t Size;
	std::string MD5;
};

using CacheMapType = std::unordered_map<std::string, CachedFileData>;

static bool ReadFileCache(CacheMapType& CacheMap)
{
	std::string FileCache;
	if (!ReadFile("launcher_cache.xml", FileCache))
		return false;

	rapidxml::xml_document<> CacheXML;

	try
	{
		CacheXML.parse<0>(&FileCache[0]);
	}
	catch (rapidxml::parse_error &e)
	{
		printf_s("RapidXML threw parse_error on launcher_cache.xml - e.what() = %s, e.where() = %s\n", e.what(), e.where<char>());
		return false;
	}

	for (auto* node = CacheXML.first_node("file"); node; node = node->next_sibling())
	try
	{
		auto GetAttrString = [&](const char* Name)
		{
			auto Throw = [&]() { throw std::runtime_error(std::string("Failed to find attribute ") + Name + " in cache"); };
			auto* attr = node->first_attribute(Name);
			if (!attr)
				Throw();
			auto* val = attr->value();
			if (!val)
				Throw();
			return val;
		};
		auto GetAttrU64 = [&](const char* Name)
		{
			auto* String = GetAttrString(Name);
			char *endptr = nullptr;
			auto val = strtoull(String, &endptr, 10);
			if (endptr != String + strlen(String))
				throw std::runtime_error(std::string("Failed to convert ") + String + " to number");
			return val;
		};

		auto* Filename = GetAttrString("name");
		auto* MD5 = GetAttrString("md5");
		auto Size = GetAttrU64("size");
		auto LastModifiedTime = GetAttrU64("last_modified_time");
		CacheMap.emplace(std::make_pair(Filename, CachedFileData{ static_cast<uint64_t>(LastModifiedTime), static_cast<size_t>(Size), MD5 }));
	}
	catch (std::runtime_error& e)
	{
		printf_s("%s\n", e.what());
		continue;
	}

	return true;
}

static bool GetFileSizeAndTime(const char* Filename, CachedFileData& Data)
{
	auto GetU64 = [&](auto High, auto Low) {
		return (static_cast<uint64_t>(High) << 32) | Low;
	};
	WIN32_FILE_ATTRIBUTE_DATA wfad;
	if (!GetFileAttributesEx(Filename, GetFileExInfoStandard, &wfad))
	{
		printf_s("GetFileAttributesEx on %s failed!\n", Filename);
		return false;
	}
	Data.Size = GetU64(wfad.nFileSizeHigh, wfad.nFileSizeLow);
	Data.LastModifiedTime = GetU64(wfad.ftLastWriteTime.dwHighDateTime, wfad.ftLastWriteTime.dwLowDateTime);
	return true;
}

static bool FileUnchanged(const CacheMapType& CacheMap, const char* Filename, std::string& md5hash)
{
	auto it = CacheMap.find(Filename);
	if (it == CacheMap.end())
		return false;

	CachedFileData Data;
	if (!GetFileSizeAndTime(Filename, Data))
		return false;

	if (it->second.LastModifiedTime != Data.LastModifiedTime
		|| it->second.Size != Data.Size)
		return false;

	md5hash = it->second.MD5;
	return true;
}

static void AddCacheEntry(CacheMapType& CacheMap, const char* Filename, const char* MD5)
{
	CachedFileData Data;
	if (!GetFileSizeAndTime(Filename, Data))
	{
		printf_s("Failed to get size or time for file %s for cache!\n", Filename);
		return;
	}
	Data.MD5 = MD5;

	CacheMap[Filename] = Data;
}

static bool SaveCache(CacheMapType& CacheMap)
try
{
	rapidxml::xml_document<> doc;
	for (auto&& pair : CacheMap)
	{
		auto node = doc.allocate_node(rapidxml::node_element, "file");
		node->append_attribute(doc.allocate_attribute("name", pair.first.c_str()));
		char buf[64];
		auto AppendNumber = [&](const char* Name, const char* Format, auto val)
		{
			sprintf_s(buf, Format, val);
			node->append_attribute(doc.allocate_attribute(Name, doc.allocate_string(buf)));
		};
		AppendNumber("size", "%d", pair.second.Size);
		AppendNumber("last_modified_time", "%llu", pair.second.LastModifiedTime);
		node->append_attribute(doc.allocate_attribute("md5", pair.second.MD5.c_str()));
		doc.append_node(node);
	}
	auto* Filename = "launcher_cache.xml";
	std::ofstream file(Filename);
	file << doc;
	printf_s("Saving cache to %s %s!\n", Filename, file.fail() ? "failed" : "succeeded");
	return true;
}
catch (std::bad_alloc& e)
{
	printf_s("SaveCache - Caught std::bad_alloc - e.what() = %s", e.what());
	return false;
}

struct PatchFileRet { bool Succeeded, FileChanged, CacheChanged; };
static PatchFileRet PatchFile(const char* Filename, const char* OutputFilename,
	const char* WantedMD5, size_t WantedMD5Size, CacheMapType& CacheMap,
	std::string& contents, rapidxml::xml_node<>* node)
{
	PatchFileRet ret{};
	std::string md5hash;

	bool FoundInCache = FileUnchanged(CacheMap, Filename, md5hash);
	if (!FoundInCache)
	{
		if (FileExists(Filename))
		{
			ReadFile(Filename, contents);
			md5hash = md5(contents.c_str(), contents.size());
		}
		else
			printf_s("%s nonexistent\n", Filename);

		ret.CacheChanged = true;
		printf_s("Couldn't find %s in cache\n", Filename);
	}

	if (md5hash.length() != WantedMD5Size || strcmp(md5hash.c_str(), WantedMD5))
	{
		printf_s("Name: %s\nCurrent md5: %s\nWanted md5: %s\n", Filename, md5hash.c_str(), WantedMD5);

		DownloadFullFile(Filename, OutputFilename);
		ret.FileChanged = true;
		AddCacheEntry(CacheMap, Filename, WantedMD5);
		ret.CacheChanged = true;
	}
	else if (!FoundInCache)
		AddCacheEntry(CacheMap, Filename, md5hash.c_str());

	ret.Succeeded = true;
	return ret;
}

static bool SelfUpdate(rapidxml::xml_node<>* xml, CacheMapType& CacheMap,
	bool& CacheChanged, std::string& contents)
{
	for (auto* node = xml->first_node("file"); node; node = node->next_sibling())
	{
		auto* Filename = node->first_attribute("name")->value();
		if (strcmp(Filename, "launcher.exe"))
			continue;

		auto* WantedMD5 = node->first_attribute("md5")->value();
		auto WantedMD5Size = node->first_attribute("md5")->value_size();
		auto ret = PatchFile(Filename, "launcher_new.exe", WantedMD5, WantedMD5Size, CacheMap, contents, node);
		if (!ret.Succeeded)
		{
			printf_s("Failed to patch launcher.exe!\n");
			return false;
		}

		CacheChanged |= ret.CacheChanged;

		if (!ret.FileChanged)
			return true;

#ifdef _DEBUG
		return true;
#endif

		printf_s("Starting new launcher\n");
		MoveFile("launcher.exe", "launcher_old.exe");
		MoveFile("launcher_new.exe", "launcher.exe");
		StartProcess("launcher.exe");
		exit(0);
	}

	return false;
}

int main()
{
	// Add a delay before closing the console window 
	// so that users can actually read the final output.
	DEFER( Sleep(3000); );
	if (!CreateCurl())
	{
		printf_s("Failed to initialize curl!\n");
		return -1;
	}
	printf_s("Created curl!\n");
	DEFER( DestroyCurl(); );

	DeleteOldLauncher();

	rapidxml::xml_document<> patchxml;
	std::string patchfile;
	if (!DownloadPatch(patchxml, patchfile))
	{
		printf_s("Failed to download patch.xml!\n");
		return -1;
	}

	CacheMapType CacheMap;
	auto HasCache = ReadFileCache(CacheMap);

	auto* xml = patchxml.first_node("xml");
	if (!xml)
	{
		printf_s("Failed to find xml node in patch.xml!\n");
		return -1;
	}

	std::string contents;
	constexpr size_t InitialSize = 50 * 1024 * 1024; // 50 MiB
	try
	{
		contents.reserve(InitialSize);
	}
	catch (std::bad_alloc& e)
	{
		printf_s("Caught bad_alloc when trying to reserve %d initial bytes for files; attempting to continue - e.what() = %s\n", InitialSize, e.what());
	}

	bool CacheChanged = false;

	SelfUpdate(xml, CacheMap, CacheChanged, contents);

	for (auto* node = xml->first_node("file"); node; node = node->next_sibling())
	{
		auto* Filename = node->first_attribute("name")->value();
		auto* WantedMD5 = node->first_attribute("md5")->value();
		auto WantedMD5Size = node->first_attribute("md5")->value_size();
		auto ret = PatchFile(Filename, Filename, WantedMD5, WantedMD5Size,
			CacheMap, contents, node);
		CacheChanged |= ret.CacheChanged;
	}

	if (CacheChanged)
		SaveCache(CacheMap);

#ifdef _DEBUG
	std::cin.get();
#endif

	printf_s("Starting Gunz...");
	StartProcess("Gunz.exe");

	return 0;
}