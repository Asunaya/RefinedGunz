#pragma once

#include "Log.h"

#include "MFile.h"
#include "defer.h"

template <typename OutputContainerType>
inline bool ReadFile(OutputContainerType& OutputContainer, const char* Filename)
{
	static_assert(sizeof(OutputContainerType::value_type) == sizeof(char), "Wrong value type");

	MFile::File File{ Filename };

	if (File.error())
	{
		Log(LogLevel::Error, "Failed to open %s for reading\n", Filename);
		return false;
	}

	size_t FileSize;
	{
		const u64 FileSize64 = File.size();
		constexpr u64 MaxSize = (std::numeric_limits<size_t>::max)();
		if (FileSize64 > MaxSize)
		{
			Log(LogLevel::Error, "File %s is too large to be stored in memory\n"
				"File size is %llu, can hold at most %llu\n",
				FileSize64, MaxSize);
			return false;
		}
		FileSize = static_cast<size_t>(FileSize64);
	}

	// Allocate memory for the output, if necessary.
	if (OutputContainer.size() < FileSize)
	{
		try
		{
			OutputContainer.resize(FileSize);
		}
		catch (std::bad_alloc& e)
		{
			Log(LogLevel::Error,
				"ReadFile -- Caught std::bad_alloc while trying to resize to %zu bytes for file %s\n"
				"e.what() = %s\n",
				FileSize, Filename,
				e.what());
			return false;
		}
	}

	File.read(&OutputContainer[0], FileSize);

	if (File.error())
	{
		Log(LogLevel::Error, "Failed to read %zu from %s\n",
			FileSize, Filename);
		return false;
	}

	return true;
}

template <typename CharType>
CharType* find_first_of(CharType* Haystack, const StringView& Needle)
{
	for (auto p = Haystack; *p != 0; ++p)
	{
		for (auto&& c : Needle)
		{
			if (*p == c)
				return p;
		}
	}

	return nullptr;
}

// Create all directories leading up to a file if any of them don't exist.
inline bool CreateDirectoriesIfNonexistent(const StringView& PathArgument)
{
	auto FirstIndex = PathArgument.find_first_of("/\\");
	if (FirstIndex == PathArgument.npos)
		return true; // No directories.

					 // Create our own copy so we can add zeroes.
	char Path[MFile::MaxPath];
	strcpy_safe(Path, PathArgument);

	char* p = Path + FirstIndex;
	while (true)
	{
		// Set the current slash to zero, and revert this change before the next loop iteration.
		auto OldValue = *p;
		*p = 0;
		// Capture by value to ignore the change to p at the end.
		DEFER([=] { *p = OldValue; });

		if (!MFile::IsDir(Path))
		{
			Log(LogLevel::Debug, "Creating directory %s\n", Path);
			if (!MFile::CreateDir(Path))
			{
				Log(LogLevel::Error, "Failed to create directory %s\n", Path);
				return false;
			}
		}

		p = find_first_of(p + 1, "/\\");
		if (p == nullptr)
			break;
	}

	return true;
}