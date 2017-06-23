#pragma once

#include "StringView.h"
#include "MUtil.h"
#include "optional.h"
#include <functional>
#include <cassert>

namespace MFile
{

constexpr size_t MaxPath = 260;

bool Exists(const char* Path);
bool Delete(const char* Path);
bool Move(const char* OldPath, const char* NewPath);

// Returns true if the path is an existing file.
bool IsFile(const char* Path);

// Returns true if the path is an existing directory.
bool IsDir(const char* Path);

bool CreateFile(const char* Path);
bool CreateDir(const char* Path);

namespace Attributes
{
enum Type
{
	Normal = 0,
	ReadOnly = 1 << 0,
	Hidden = 1 << 1,
	System = 1 << 2,
	Subdir = 1 << 4,
	Archive = 1 << 5,
};
}

struct FileAttributes
{
	u32 Attributes;
	u64 CreationTime;
	u64 LastAccessTime;
	u64 LastModifiedTime;
	u64 Size;
};

optional<FileAttributes> GetAttributes(const char* Path);

struct FileData
{
	u32 Attributes;
	u64 CreationTime;
	u64 LastAccessTime;
	u64 LastModifiedTime;
	u64 Size;
	char Name[260];
};

struct FileIterator;

struct FileRange
{
	FileIterator begin();
	FileIterator end();

	intptr_t First;
	FileData Data;
};

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

	FileRange& Range;
	bool End;
};

FileRange FilesAndSubdirsInDir(const char* Path);

enum class Seek
{
	Begin = SEEK_SET,
	Current = SEEK_CUR,
	End = SEEK_END,
};

// A simple class that wraps a FILE*.
// It supports 64-bit offsets and is always in read-only binary mode.
struct File
{
	File() = default;
	// Takes ownership of an existing FILE*.
	File(CFilePtr file_ptr) : file_ptr{ std::move(file_ptr) } {}
	// Wrapper for open.
	File(const char* path) { open(path); }

	// Opens a file. Returns true on success, or false on error.
	bool open(const char* path);

	// Closes the file, if it is open.
	void close();

	// Returns true if the file is open, and false if not.
	bool is_open() const;

	// Returns the size of the file, or 0 on error.
	u64 size();

	// Returns true on success, or false on error.
	bool seek(i64 offset, Seek origin);

	static constexpr i64 tell_error = EOF;

	// Returns the position in bytes on success, or tell_error on error.
	i64 tell();

	// Attempts to read size bytes from the file into the buffer provided.
	// Returns the actual amount of bytes read, which may be less than requested,
	// either because of EOF or error.
	//
	// Note: The buffer you pass must be of a trivially copyable type
	// (i.e. anything that is not a class with non-trivial constructors or
	// assignment operators).
	size_t read(void* buffer, size_t size);

	// Returns true if the file is at the end.
	bool eof() const;

	// Returns true if the last operation failed.
	// The operations that are counted by this are all the ones that can fail,
	// i.e: open (and open ctors), close, size, seek, tell and read.
	// Performing a new operation resets this flag, it ONLY tracks the very last operation.
	bool error() const;

	CFilePtr release();

protected:
	FILE* fp() const;

	bool open_impl(const char* path, const char* mode);

	CFilePtr file_ptr;

	struct state_t {
		bool eof;
		bool error;
	} state{};
};

constexpr struct ClearExistingContentsType {} ClearExistingContents;
constexpr struct PreserveExistingContentsAndAppendType {} PreserveExistingContentsAndAppend;
constexpr struct PreserveExistingContentsAndPrependType {} PreserveExistingContentsAndPrepend;

constexpr struct TextType {} Text;

// A File, except it also supports writing.
struct RWFile : public File
{
	RWFile() = default;
	// Takes ownership of an existing FILE*.
	RWFile(CFilePtr file_ptr) : File{ std::move(file_ptr) } {}
	// Wrappers for open.
	template <typename... Ts>
	RWFile(const char* Path, Ts... Options) { open(Path, Options...); }

	// Opens a file for writing.
	// Returns true on success, or false on error.
	// Must specify one of ClearExistingContents, PreserveExistingContentsAndAppend, or
	// PreserveExistingContentsAndPrepend.
	template <typename... OptionTypes>
	bool open(const char* path, OptionTypes...);

	// Writes memory to the file.
	// Returns the number of bytes written, which may be less than
	// the requested size only if an error happens.
	size_t write(const void* buffer, size_t size);
};

struct FileOutputIterator
{
	FileOutputIterator(RWFile& file) : file{ &file } {}

	auto& operator*() { return *this; }
	auto& operator++() { return *this; }
	auto& operator++(int) { return *this; }
	auto& operator=(char c) { file->write(&c, 1); return *this; }

	RWFile* file;
};

// Implementation

namespace detail
{

template <typename T, typename... Ts>
struct is_part_of
{
	static constexpr auto value = tmp::any_of<
		std::is_same<
		std::remove_const_t<std::remove_reference_t<T>>, Ts>::value...>::value;
};

}

template <typename... OptionTypes>
bool RWFile::open(const char* path, OptionTypes...)
{
	static_assert(detail::is_part_of<ClearExistingContentsType, OptionTypes...>::value +
		detail::is_part_of<PreserveExistingContentsAndAppendType, OptionTypes...>::value +
		detail::is_part_of<PreserveExistingContentsAndPrependType, OptionTypes...>::value
		<= 1,
		"ClearExistingContents, PreserveExistingContentsAndAppend, and "
		"PreserveExistingContentsAndPrepend are mutually exclusive. You can't use more than "
		"one of these as open flags.");

	static_assert(
		tmp::all_of<
			detail::is_part_of<OptionTypes,
				TextType,
				ClearExistingContentsType,
				PreserveExistingContentsAndAppendType,
				PreserveExistingContentsAndPrependType
			>::value...
		>::value,
		"Invalid option types");

	static_assert(
		tmp::any_of<
			detail::is_part_of<OptionTypes,
				ClearExistingContentsType,
				PreserveExistingContentsAndAppendType,
				PreserveExistingContentsAndPrependType
			>::value...
		>::value,
		"Must specify either ClearExistingContents, PreserveExistingContentsAndAppend, or "
		"PreserveExistingContentsAndPrepend in options");

	auto mode = [&] {
		if (!detail::is_part_of<TextType, OptionTypes...>::value)
		{
			if (detail::is_part_of<ClearExistingContentsType, OptionTypes...>::value)
				return "wb+";
			else if (detail::is_part_of<PreserveExistingContentsAndAppendType, OptionTypes...>::value)
				return "ab+";
			else if (detail::is_part_of<PreserveExistingContentsAndPrependType, OptionTypes...>::value)
				return "rb+";
		}
		else
		{
			if (detail::is_part_of<ClearExistingContentsType, OptionTypes...>::value)
				return "w+";
			else if (detail::is_part_of<PreserveExistingContentsAndAppendType, OptionTypes...>::value)
				return "a+";
			else if (detail::is_part_of<PreserveExistingContentsAndPrependType, OptionTypes...>::value)
				return "r+";
		}
	}();

	return open_impl(path, mode);
}

}