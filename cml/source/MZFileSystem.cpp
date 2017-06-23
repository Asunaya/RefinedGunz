#include "stdafx.h"
#include "MZFileSystem.h"
#include "MXml.h"
#include <io.h>
#include <crtdbg.h>
#include "MZip.h"
#include "FileInfo.h"
#include "zip/zlib.h"
#include "MDebug.h"
#include "MUtil.h"
#include <algorithm>
#include <cassert>
#include "Arena.h"

static void ReplaceBackSlashToSlash(char* szPath)
{
	int nLen = strlen(szPath);
	for(int i=0; i<nLen; i++){
		if(szPath[i]=='\\') szPath[i]='/';
	}
}

void GetRefineFilename(char *szRefine, int maxlen, const char *szSource)
{
	char pBasePath[256];
	GetCurrentDirectory(sizeof(pBasePath),pBasePath);
	strcat_s(pBasePath,"\\");

	GetRelativePath(szRefine, maxlen, pBasePath,szSource);

	ReplaceBackSlashToSlash(szRefine);
}

// Asserts that the capacity of vec remains unchanged.
template <typename T>
static auto CapacityChecker(T& vec)
{
#ifdef _DEBUG
	struct CapacityChecker
	{
		CapacityChecker(T& vec) : vec{ vec } { Capacity = vec.capacity(); };
		~CapacityChecker() { assert(Capacity == vec.capacity()); }
		T& vec;
		size_t Capacity;
	};
	return CapacityChecker{ vec };
#else
	return 0;
#endif
}

void MZFileSystem::ClearFileList()
{
	Files.clear();
	Dirs.clear();
	NodeMap.clear();
}

StringView MZFileSystem::AllocateString(const StringView& Src)
{
	const auto size = Src.size() + 1;
	Strings.emplace_back(std::make_unique<char[]>(size));

	auto ptr = Strings.back().get();
	strcpy_safe(ptr, size, Src);
	
	return{ ptr, Src.size() };
}

static bool IsArchivePath(const StringView& Path)
{
	const auto ExtIndex = Path.find_last_of('.');
	if (ExtIndex == Path.npos)
		return false;

	const auto ExtLen = Path.size() - ExtIndex;
	if (ExtLen < 4)
		return false;

	const auto Ext = Path.data() + ExtIndex;
	return Ext && (_stricmp(Ext, "." DEF_EXT) == 0 || _stricmp(Ext, ".zip") == 0);
}

template <typename T>
using AllocType = ArenaAllocator<T>;

struct PreprocessedDir
{
	explicit PreprocessedDir(const AllocType<char>& Alloc) : Subdirs{ Alloc }, Files{ Alloc } {}

	// Path to the directory.
	// If the directory is an archive, this is formatted as a directory (i.e. no archive extension).
	StringView Path;

	// If this directory is an archive, this is the path to the archive file.
	// Otherwise, it's empty.
	StringView ArchivePath;

	std::vector<PreprocessedDir, AllocType<PreprocessedDir>> Subdirs;
	std::vector<MZFileDesc, AllocType<MZFileDesc>> Files;
};

struct PreprocessedFileTree
{
	explicit PreprocessedFileTree(const AllocType<char>& Alloc) : Alloc{ Alloc }, Root { Alloc } {}

	AllocType<char> Alloc;
	int NumFiles{};
	int NumDirectories{};
	int NumArchives{};
	PreprocessedDir Root;
};

void MZFileSystem::AddFilesInArchive(PreprocessedFileTree& Tree, PreprocessedDir& ArchiveDir, MZip& Zip)
{
	const auto Count = int(Zip.GetFileCount());

	Tree.NumFiles += Count;

	for (int i = 0; i < Count; i++)
	{
		const auto Filename = Zip.GetFileName(i);

		{
			const auto lastchar = Filename.back();
			if (lastchar == '\\' || lastchar == '/') {
				continue;
			}
		}

		ArchiveDir.Files.emplace_back();
		auto&& Child = ArchiveDir.Files.back();

		char Path[_MAX_PATH];
		sprintf_safe(Path, "%.*s%.*s",
			ArchiveDir.Path.size(), ArchiveDir.Path.data(),
			Filename.size(), Filename.data());
		Child.Path = AllocateString(Path);
		Child.ArchivePath = ArchiveDir.ArchivePath;

		Child.Size = Zip.GetFileLength(i);
		Child.ArchiveOffset = Zip.GetFileArchiveOffset(i);
		Child.CompressedSize = Zip.IsFileCompressed(i) ? Zip.GetFileCompressedSize(i) : 0;
	}
}

void MZFileSystem::MakeDirectoryTree(PreprocessedFileTree& Tree,
	const StringView& FullPath, PreprocessedDir& Dir)
{
	char szFilter[_MAX_PATH];
	sprintf_safe(szFilter, "%.*s*",
		Dir.Path.size(), Dir.Path.data());

	_finddata_t FileData;
	auto hFile = _findfirst(szFilter, &FileData);
	if (hFile == -1)
	{
		MLog("MakeDirectoryTree -- _findfirst failed on filter %s\n", szFilter);
		return;
	}

	do {
		if (strcmp(FileData.name, ".") == 0 || strcmp(FileData.name, "..") == 0)
			continue;

		auto AddSubdir = [&](const StringView& Filename) -> auto&
		{
			char Path[_MAX_PATH];
			sprintf_safe(Path, "%.*s%.*s/",
				Dir.Path.size(), Dir.Path.data(),
				Filename.size(), Filename.data());

			Dir.Subdirs.emplace_back(Tree.Alloc);
			auto&& Subdir = Dir.Subdirs.back();

			Subdir.Path = AllocateString(Path);

			return Subdir;
		};

		const auto IsDirectory = (FileData.attrib & _A_SUBDIR) != 0;

		if (IsDirectory)
		{
			auto&& Subdir = AddSubdir(FileData.name);

			char SubdirFullPath[_MAX_PATH];
			sprintf_safe(SubdirFullPath, "%.*s%s/",
				FullPath.size(), FullPath.data(),
				FileData.name);

			MakeDirectoryTree(Tree, SubdirFullPath, Subdir);

			++Tree.NumDirectories;
		}
		else if (IsArchivePath(FileData.name))
		{

			StringView Filename{ FileData.name };
			Filename = Filename.substr(0, Filename.find_last_of('.'));
			auto&& Subdir = AddSubdir(Filename);

			char ArchivePath[_MAX_PATH];
			sprintf_safe(ArchivePath, "%.*s%s",
				Dir.Path.size(), Dir.Path.data(),
				FileData.name);

			Subdir.ArchivePath = AllocateString(ArchivePath);

			FILE* fp{};
			fopen_s(&fp, Subdir.ArchivePath.data(), "rb");
			if (!fp)
			{
				MLog("fopen on %.*s failed\n",
					Subdir.ArchivePath.size(), Subdir.ArchivePath.data());
				assert(false);
				continue;
			}

			MZip Zip;
			Zip.Initialize(fp, MZFile::GetReadMode());

			AddFilesInArchive(Tree, Subdir, Zip);

			++Tree.NumArchives;
		}
		else
		{
			Dir.Files.emplace_back();
			auto&& File = Dir.Files.back();

			char Path[_MAX_PATH];
			sprintf_safe(Path, "%.*s%s",
				Dir.Path.size(), Dir.Path.data(),
				FileData.name);

			File.Path = AllocateString(Path);
			File.ArchiveOffset = 0;
			File.CompressedSize = 0;
			File.Size = FileData.size;

			++Tree.NumFiles;
		}
	} while (_findnext(hFile, &FileData) == 0);

	_findclose(hFile);
}

void MZFileSystem::UpdateFileList(PreprocessedDir& Src, MZDirDesc& Dest)
{
	if (Src.Subdirs.empty() && Src.Files.empty())
		return;

	auto&& chk1 = CapacityChecker(Files);
	auto&& chk2 = CapacityChecker(Dirs);

	Dest.Path = Src.Path;

	auto AddNode = [&](auto& x, bool IsDirectory) {
		NodeMap.emplace(x.Path, MZNode{ &x, IsDirectory });
	};

	if (Src.Files.empty())
	{
		Dest.Files = nullptr;
		Dest.NumFiles = 0;
	}
	else
	{
		size_t FilesIndex = Files.size();
		Files.insert(std::end(Files), std::begin(Src.Files), std::end(Src.Files));
		Dest.Files = &Files[FilesIndex];
		Dest.NumFiles = Src.Files.size();

		for (auto&& File : Dest.FilesRange())
			AddNode(File, false);
	}

	if (Src.Subdirs.empty())
	{
		Dest.Subdirs = nullptr;
		Dest.NumSubdirs = 0;
	}
	else
	{
		size_t SubdirsIndex = Dirs.size();
		Dest.NumSubdirs = Src.Subdirs.size();
		Dirs.resize(Dirs.size() + Dest.NumSubdirs);
		Dest.Subdirs = &Dirs[SubdirsIndex];

		for (size_t i = 0; i < Dest.NumSubdirs; ++i)
		{
			auto&& SrcSubdir = Src.Subdirs[i];
			auto&& DestSubdir = Dirs[SubdirsIndex + i];
			DestSubdir.Parent = &Dest;
			UpdateFileList(SrcSubdir, DestSubdir);
		}

		for (auto&& Subdir : Dest.SubdirsRange())
			AddNode(Subdir, true);
	}
}

unsigned MGetCRC32(const char *data, int nLength)
{
	uLong crc = crc32(0, Z_NULL, 0);
	crc = crc32(crc, reinterpret_cast<const Byte*>(data), nLength);
	return crc;
}

MZFileSystem::MZFileSystem() = default;
MZFileSystem::~MZFileSystem() = default;

bool MZFileSystem::Create(std::string BasePathArgument)
{
	BasePath = std::move(BasePathArgument);

	// Add slash to end of path if missing.
	if (BasePath.back() != '/' && BasePath.back() != '\\')
		BasePath += '/';

	u8 ArenaBuffer[1024 * 16];
	Arena arena{ ArenaBuffer, std::size(ArenaBuffer) };

	PreprocessedFileTree Tree{ ArenaAllocator<char>{ &arena } };
	Tree.NumDirectories++; // Root.
	MakeDirectoryTree(Tree, "", Tree.Root);

	DMLog("Number of files: %d\n"
		"Number of archives: %d\n"
		"Number of directories: %d\n",
		Tree.NumFiles, Tree.NumArchives, Tree.NumDirectories);

	const auto NumNodes = Tree.NumArchives + Tree.NumDirectories + Tree.NumFiles;
	const auto NumDirs = Tree.NumArchives + Tree.NumDirectories;
	NodeMap.reserve(NumNodes);
	Files.reserve(Tree.NumFiles);
	Dirs.reserve(NumDirs);

	Strings.shrink_to_fit();

	Dirs.emplace_back();
	auto&& Root = Dirs.back();
	UpdateFileList(Tree.Root, Root);

	return true;
}

int MZFileSystem::GetFileCount() const
{
	return static_cast<int>(Files.size());
}

const StringView& MZFileSystem::GetFileName(int i) const
{
	assert(i >= 0 && i < int(GetFileCount()));
	return GetFileDesc(i)->Path;
}

const MZFileDesc* MZFileSystem::GetFileDesc(int i) const
{
	assert(i >= 0 && i < int(GetFileCount()));
	return &Files[i];
}

const MZFileDesc* MZFileSystem::GetFileDesc(const StringView& Path) const
{
	auto Node = GetNode(Path);
	if (!Node || !Node->IsFile())
		return nullptr;

	return &Node->AsFile();
}

const MZDirDesc* MZFileSystem::GetDirectory(const StringView& Path) const
{
	auto Node = GetNode(Path);
	if (!Node || !Node->IsDirectory())
		return nullptr;

	return &Node->AsDirectory();
}

const MZNode* MZFileSystem::GetNode(const StringView& Path) const
{
	auto it = NodeMap.find(Path);
	if (it == NodeMap.end())
		return nullptr;

	return &it->second;
}

int MZFileSystem::GetFileLength(int i)
{
	auto pDesc = GetFileDesc(i);
	if (!pDesc)
		return 0;

	return pDesc->Size;
}

int MZFileSystem::GetFileLength(const StringView& szFileName)
{
	auto pDesc = GetFileDesc(szFileName);
	if (!pDesc)
		return 0;

	return pDesc->Size;
}

#ifdef WIN32
class MMappedFile
{
public:
	MMappedFile() {}
	MMappedFile(const char* Filename);
	MMappedFile(const MMappedFile&) = delete;
	MMappedFile(MMappedFile&&);
	~MMappedFile();
	MMappedFile& operator =(MMappedFile&& src)
	{
		this->~MMappedFile();
		new (this) MMappedFile(std::move(src));
		return *this;
	}

	bool Dead() const { return bDead; }
	auto GetPointer() const { return reinterpret_cast<const char*>(View); }
	auto GetSize() const { return Size; }

private:
	bool bDead = true;
	HANDLE File = INVALID_HANDLE_VALUE;
	HANDLE Mapping = INVALID_HANDLE_VALUE;
	HANDLE View = INVALID_HANDLE_VALUE;
	size_t Size = 0;
};
#endif

#ifdef WIN32
#define ARCHIVE_CACHE_MMAP
#endif

void MZFileSystem::CacheArchive(const StringView& Filename)
{
	char FilenameWithExtension[_MAX_PATH];
	sprintf_safe(FilenameWithExtension, "%.*s.%s", Filename.size(), Filename.data(), DEF_EXT);

	MZip Zip;

#ifdef ARCHIVE_CACHE_MMAP
	MMappedFile File{ FilenameWithExtension };
	if (File.Dead())
	{
		MLog("MZFileSystem::CacheArchive -- Failed to load file %s!\n", FilenameWithExtension);
		return;
	}

	const auto ZipInitialized = Zip.Initialize(File.GetPointer(), File.GetSize(), MZFile::GetReadMode());
#else
	FILE* fp = nullptr;
	auto ret = fopen_s(&fp, FilenameWithExtension, "rb");
	if (!fp || ret != 0)
	{
		MLog("MZFileSystem::CacheArchive -- fopen_s failed on %s\n", Filename);
		return -1;
	}

	const auto ZipInitialized = Zip.Initialize(fp, MZFile::GetReadMode());
#endif

	if (!ZipInitialized)
	{
		MLog("MZFileSystem::CacheArchive -- MZip::Initialize on %s failed!\n", Filename);
		return;
	}

	auto FileCount = Zip.GetFileCount();
	for (int i = 0; i < FileCount; i++)
	{
		char Name[64];
		Zip.GetFileName(i, Name);
		char AbsName[64];
		sprintf_safe(AbsName, "%.*s/%s", Filename.size(), Filename.data(), Name);
		auto Desc = GetFileDesc(AbsName);
		if (!Desc)
		{
			DMLog("Couldn't find file desc for %s\n", AbsName);
			continue;
		}
		if (Desc->ArchivePath == FilenameWithExtension)
			continue;
		auto Size = Zip.GetFileLength(i);

		auto p = std::make_unique<char[]>(Size);
		if (!Zip.ReadFile(i, p.get(), Size)) {
			MLog("MZFileSystem::CacheArchive - MZip::ReadFile on %s failed!\n", Name);
			continue;
		}

		CachedFileMap.emplace(Desc, std::move(p));
	}

	DMLog("Cached %d files for archive %s\n", FileCount, Filename);
}

void MZFileSystem::ReleaseCachedArchives()
{
	CachedFileMap.clear();
}

MMappedFile::MMappedFile(const char * Filename)
{
	File = CreateFileA(Filename, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (File == INVALID_HANDLE_VALUE)
	{
		MLog("MMappedFile::MMappedFile - CreateFile failed on %s! GetLastError() = %d\n",
			Filename, GetLastError());
		return;
	}

	Size = GetFileSize(File, nullptr);

	char FileMappingName[256];
	sprintf_safe(FileMappingName, "GunzCache_%s", Filename);
	Mapping = CreateFileMapping(File, 0, PAGE_READONLY, 0, 0, FileMappingName);
	if (Mapping == NULL)
	{
		assert(false);
		return;
	}

	View = MapViewOfFile(Mapping, FILE_MAP_READ, 0, 0, 0);
	if (View == NULL)
	{
		assert(false);
		return;
	}

	bDead = false;
}

MMappedFile::~MMappedFile()
{
	if (View != INVALID_HANDLE_VALUE) {
		auto Success = UnmapViewOfFile(View);
		assert(Success);
	}
	if (Mapping != INVALID_HANDLE_VALUE) {
		auto Success = CloseHandle(Mapping);
		assert(Success);
	}
	if (File != INVALID_HANDLE_VALUE) {
		auto Success = CloseHandle(File);
		assert(Success);
	}
}

MMappedFile::MMappedFile(MMappedFile && src)
	: bDead(src.bDead), View(src.View), Mapping(src.Mapping), File(src.File)
{
	src.bDead = true;
	src.View = INVALID_HANDLE_VALUE;
	src.Mapping = INVALID_HANDLE_VALUE;
	src.File = INVALID_HANDLE_VALUE;
}

void RecursiveMZFileIterator::AdvanceToNextFile()
{
	if (FileIndex < int(CurrentSubdir->NumFiles) - 1)
	{
		++FileIndex;
		return;
	}

	if (CurrentSubdir == &Dir)
	{
		FileIndex = Dir.NumFiles;
		return;
	}

	auto ParentRange = CurrentSubdir->Parent->SubdirsRange();

	auto CurrentSubdirIt = std::find_if(std::begin(ParentRange), std::end(ParentRange),
		[&](auto&& x) { return &x == CurrentSubdir; });

	assert(CurrentSubdirIt != std::end(ParentRange));

	if (CurrentSubdirIt == std::prev(std::end(ParentRange)))
	{
		CurrentSubdir = CurrentSubdir->Parent;
		FileIndex = 0;
	}

	FileIndex = CurrentSubdirIt - std::begin(ParentRange);

	AdvanceToNextFile();
}

Range<RecursiveMZFileIterator> FilesInDirRecursive(MZFileSystem& FS, const MZDirDesc& Dir)
{
	// Find first file
	auto* CurrentSubdir = &Dir;
	int FileIndex = 0;
	while (CurrentSubdir->NumFiles == 0)
	{
		CurrentSubdir = CurrentSubdir->Subdirs;
	}

	return{
		RecursiveMZFileIterator{ FS, Dir, CurrentSubdir, FileIndex },
		RecursiveMZFileIterator{ FS, Dir, &Dir, int(Dir.NumFiles) },
	};
}