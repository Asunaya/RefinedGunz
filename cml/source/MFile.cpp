#include "stdafx.h"
#include "MFile.h"
#include "defer.h"

#ifdef WIN32
#include <Windows.h>
#include <io.h>
#undef DeleteFile
#endif

namespace MFile
{

#ifdef WIN32

bool DeleteFile(const char* Path)
{
	return ::DeleteFileA(Path) != FALSE;
}

static void CopyFileData(FileData& dest, const _finddata_t& src)
{
	dest.Attributes = src.attrib;
	dest.CreationTime = src.time_create;
	dest.LastAccessTime = src.time_access;
	dest.LastModifiedTime = src.time_write;
	dest.Size = src.size;
	dest.Name = src.name;
}

template <bool Recursive>
FileIterator<Recursive>& FileIterator<Recursive>::operator++()
{
	_finddata_t fd;
	auto find_ret = _findnext(Range.First, &fd);
	if (find_ret == 0)
		End = true;

	CopyFileData(Range.Data, fd);
	return *this;
}

template <bool Recursive>
FileIterator<Recursive> FileRange<Recursive>::begin() {
	return{ *this, First == -1 ? true : false };
}

template <bool Recursive>
FileIterator<Recursive> FileRange<Recursive>::end() {
	return{ *this, true };
}

template <bool Recursive>
FileRange<Recursive> FilesInDirImpl(const char* Path)
{
	FileRange<Recursive> ret;

	_finddata_t fd;
	ret.First = _findfirst(Path, &fd);

	CopyFileData(ret.Data, fd);

	return ret;
}

FileRange<true> FilesInDirRecursive(const char* Path) { return FilesInDirImpl<true>(Path); }
FileRange<false> FilesInDir(const char* Path) { return FilesInDirImpl<false>(Path); }

#else

bool DeleteFile(const char* Path)
{
	// TODO
	return false;
}


FileRange<true> FilesInDirRecursive(const char* Path) { return{}; }
FileRange<false> FilesInDir(const char* Path) { return{}; }

#endif

template FileIterator<true>;
template FileIterator<false>;
template FileRange<true>;
template FileRange<false>;

}