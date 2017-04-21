#pragma once

#include "StringView.h"
#include <functional>
#include <cassert>

namespace MFile
{

bool DeleteFile(const char* Path);

struct FileData
{
	u32 Attributes;
	u64 CreationTime;
	u64 LastAccessTime;
	u64 LastModifiedTime;
	u64 Size;
	char Name[260];
};

template <bool Recursive>
struct FileRange;

template <bool Recursive>
struct FileIterator
{
	bool operator==(const FileIterator& rhs) const { assert(&Range == &rhs.Range); return End == rhs.End; }
	bool operator!=(const FileIterator& rhs) const { return !(*this == rhs); }
	const FileData& operator*() const { return Range.Data; }
	FileIterator& operator++();
	FileIterator operator++(int) {
		auto temp = *this;
		++*this;
		return temp;
	}

	FileRange<Recursive>& Range;
	bool End;
};

template <bool Recursive>
struct FileRange
{
	FileIterator<Recursive> begin();
	FileIterator<Recursive> end();

	intptr_t First;
	FileData Data;
};

FileRange<true> FilesInDirRecursive(const char* Path);
FileRange<false> FilesInDir(const char* Path);

}