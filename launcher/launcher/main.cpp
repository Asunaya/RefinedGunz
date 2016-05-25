#include <Windows.h>
#include <boost/property_tree/detail/rapidxml.hpp>
#include <urlmon.h>
#include <io.h>
#include <fstream>
#include "md5.h"
#include "downloadstatus.h"
#include "../zdelta/zdlib.h"

using namespace boost::property_tree::detail;

//#define DEBUG

#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "../Release/zdelta.lib")

#define F_OK 0

bool FileExists(const char *szFilePath)
{
	return !_access(szFilePath, F_OK);
}

void StartProcess(const char *commandline)
{
	char buf[128];
	strcpy_s(buf, commandline);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process. 
	if (!CreateProcess(NULL,   // No module name (use command line)
		buf,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		printf_s("CreateProcess failed (%d)\n", GetLastError());
		return;
	}
}

HRESULT DownloadStatus::OnProgress(
	/* [in] */ ULONG ulProgress,
	/* [in] */ ULONG ulProgressMax,
	/* [in] */ ULONG ulStatusCode,
	/* [in] */ LPCWSTR wszStatusText)
{
	static ULONG ulLastProgress = -1;
	if (ulProgressMax == 0 || ulProgress == ulLastProgress)
		return S_OK;

	float Progress = float(ulProgress) / ulProgressMax * 100;
	std::cout << ulProgress << "/" << ulProgressMax << " -- " << Progress << "%" << "       " << "\r" << std::flush;

	ulLastProgress = ulProgress;

	return S_OK;
}

DownloadStatus g_DownloadStatus;

bool DownloadFile(const char *szURL, const char *szOutputPath)
{
	HRESULT hr = URLDownloadToFile(0, szURL, szOutputPath, 0, &g_DownloadStatus);
	switch (hr)
	{
	case S_OK:
		printf_s("Download succeeded!            \n");
		break;
	case E_OUTOFMEMORY:
		printf_s("Download failed: Out of memory.\n");
		break;
	case INET_E_DOWNLOAD_FAILURE:
		printf_s("Download failed: The specified resource or callback interface was invalid.\n");
		break;
	default:
		printf_s("Download failed: Unknown error.\n");
	};

	return hr == S_OK;
}

bool ReadFile(const char *szFilename, std::string &Output)
{
	std::ifstream File(szFilename, std::ios::in | std::ios::binary | std::ios::ate);

	if (!File.is_open())
	{
		printf_s("Failed to open %s for reading\n", szFilename);
		return false;
	}

	int Size = (int)File.tellg();

	Output.resize(Size);

	File.seekg(std::ios::beg);

	File.read(&Output[0], Size);

	if (File.fail())
	{
		printf_s("Failed to read %d from %s\n", Size, szFilename);
		return false;
	}

	return true;
}

bool WriteFile(const char *szFilename, const char *szStrToWrite, int len)
{
	std::ofstream File(szFilename);

	if (!File.is_open())
	{
		printf_s("Failed to open %s for writing\n", szFilename);
		return false;
	}

	File.write(szStrToWrite, len);

	if (File.fail())
	{
		printf_s("Failed to write merged changes to %s\n", szFilename);
		return false;
	}

	return true;
}

int main(int argc, char *argv[])
{
	if (FileExists("launcher_old.exe"))
	{
		Sleep(100);
		DeleteFile("launcher_old.exe");
	}

	printf_s("Downloading patch.xml...\n\n");
	DownloadFile("http://51.254.130.130/patch/patch.xml", "patch.xml");


	std::string patchfile;
	if (!ReadFile("patch.xml", patchfile))
	{
		Sleep(3000);
		return -1;
	}

	rapidxml::xml_document<> patchxml;

	try
	{
		patchxml.parse<0>(&patchfile[0]);
	}
	catch (rapidxml::parse_error &e)
	{
		printf_s("RapidXML threw parse_error (%s) on patch.xml at %s\n", e.what(), e.where<char>());
		Sleep(3000);
		return -1;
	}

	rapidxml::xml_node<> *xml = patchxml.first_node("xml");

	if (!xml)
	{
		printf_s("nope!\n");
		Sleep(3000);
		return -1;
	}

	char *contents = new char[30000000]; // 30 MB
	int contents_size = 30000000;

	for (rapidxml::xml_node<> *node = xml->first_node("file"); node; node = node->next_sibling())
	{
		const char *szFilename = node->first_attribute("name")->value();
		const char *szWantedMD5 = node->first_attribute("md5")->value();

		std::string md5hash("");

		if (FileExists(szFilename))
		{
			std::ifstream in(szFilename, std::ios::in | std::ios::binary);
			if (!in)
			{
				printf_s("Couldn't open %s\n", szFilename);
				in.close();
				continue;
			}

			in.seekg(0, std::ios::end);
			int size = (int)in.tellg();

			if (size > contents_size)
			{
				contents_size = size * 2;
				delete[] contents;
				contents = new char[contents_size];
			}

			in.seekg(0, std::ios::beg);
			in.read(contents, size);
			in.close();

			md5hash = md5(contents, size);
		}
		else
		{
			printf_s("%s inexistent\n", szFilename);
		}

		if (strcmp(md5hash.c_str(), szWantedMD5))
		{
			printf_s("name: %s, current md5: %s, wanted md5: %s\n", szFilename, md5hash.c_str(), szWantedMD5);

			bool bDeltaSuccess = false;
			const char *szDeltaFile = nullptr;

			for (auto oldnode = node->first_node("old"); oldnode; oldnode = oldnode->next_sibling("old"))
			{
				auto md5attr = oldnode->first_attribute("md5");

				if (!md5attr)
					continue;

				if (!strcmp(md5hash.c_str(), md5attr->value()))
				{
					printf_s("Found old version %s, downloading delta file\n", md5attr->value());

					auto deltaattr = oldnode->first_attribute("delta");

					if (!deltaattr)
					{
						printf_s("Location of delta file wasn't embedded");
						continue;
					}

					auto dfile = deltaattr->value();

					char szURL[512];
					sprintf_s(szURL, "http://51.254.130.130/diff/%s", dfile);

					if (!DownloadFile(szURL, dfile))
					{
						printf_s("Failed to download %s\n", dfile);
						break;
					}

					szDeltaFile = dfile;

					std::string RefFileData;
					if (!ReadFile(szFilename, RefFileData))
						break;

					std::string DeltaFileData;
					if (!ReadFile(dfile, DeltaFileData))
						break;

					Bytef *OutputData;
					uLongf OutputSize;

					int ret = zd_uncompress1((const Bytef *)&RefFileData[0], RefFileData.length(), &OutputData, &OutputSize, (const Bytef *)&DeltaFileData[0], DeltaFileData.length());

					if (ret >= ZD_OK)
					{
						if (!WriteFile(szFilename, (const char *)OutputData, OutputSize))
							break;

						printf_s("Successfully merged changes and wrote %d characters to %s\n", OutputSize, szFilename);

						bDeltaSuccess = true;
					}
					else
					{
						printf_s("Failed to merge changes, downloading whole file (zd_uncompress1 error code %d)\n", ret);
					}

					break;
				}
			}

			if (szDeltaFile)
			{
				if (!DeleteFile(szDeltaFile))
				{
					printf_s("Failed to delete delta file %s\n", szDeltaFile);
				}
			}

			if (bDeltaSuccess)
				continue;

			bool bIsLauncher = !strcmp(szFilename, "launcher.exe");
			const char *szOutputPath = szFilename;

			if (bIsLauncher)
			{
#ifdef DEBUG
				continue;
#endif
				szOutputPath = "launcher_new.exe";
			}

			printf_s("Downloading new %s...\n\n", szFilename);
			char szURL[512];
			sprintf_s(szURL, "http://51.254.130.130/patch/%s", szFilename);
			DeleteFile(szOutputPath);
			DownloadFile(szURL, szOutputPath);

			if (bIsLauncher)
			{
				MoveFile("launcher.exe", "launcher_old.exe");
				MoveFile("launcher_new.exe", "launcher.exe");
				StartProcess("launcher.exe");
				return 0;
			}
		}
	}

	printf_s("Starting Gunz...");
	StartProcess("Gunz.exe");

	Sleep(3000);

	return 0;
}