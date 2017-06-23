#pragma once

#include "Hash.h"
#include "GlobalTypes.h"
#include "optional.h"
#include <memory>
#include <functional>

struct DownloadManagerDeleter { void operator()(void*) const; };
using DownloadManagerType = std::unique_ptr<void, DownloadManagerDeleter>;

// Returns a non-null pointer on success, or a null pointer on error.
// DownloadManagerType is a std::unique_ptr, so it destroys itself; there's no need to call any
// kind of release function.
DownloadManagerType CreateDownloadManager();

struct DownloadInfo
{
	// Is true if the buffer provided is a range, false if it is the entire file.
	// This will always be false if you didn't provide a range when calling the function,
	// but may also be false even if you did, in case the server does not support ranges.
	bool IsRange();

	// Returns the content length if the response header included it, or an empty optional
	// otherwise.
	optional<double> ContentLength();

	// Internal data member.
	void* Data;
};

// Parameters:
//
// const u8* Buffer
//     A pointer to the data.
//
// size_t Size
//     The size of the data.
//
// const DownloadInfo& Info
//     Info about the download, see DownloadInfo for more information.
//
// Return value:
//
// False stops the download, true continues.
//
using DownloadCallbackType = bool(const u8* Buffer, size_t Size, DownloadInfo& Info);

// Downloads a file, passing each chunk of data to the provided callback as it comes in.
// This is a synchronous function: It blocks until it is complete. When it returns, the download
// will have completed.
//
// The URL must already be URL-encoded.
//
// If Range is not null, this will be passed in the HTTP message as the range request,
// allowing you to request parts of the file. The range is a string consisting of pairs of decimal values,
// the values separated by dashes, and the pairs separated by commas.
//
// The last value in the pair can be left out, which implies that the range extends to the end of
// the file.
//
// Note that HTTP servers are not required to support range requests, and you may not get a range back.
// Single range example: "100-200"
// Multiple range example: "300-400, 700-750, 900-920"
//
// If HashOutput is not null, the hash of the file will be computed and stored into it.
//
// If SizeOutput is not null, the total size of the file will be stored into it.
//
bool DownloadFile(const DownloadManagerType& DownloadManager,
	const char* URL,
	int Port,
	const std::function<DownloadCallbackType>& Callback,
	const char* Range = nullptr,
	Hash::Strong* HashOutput = nullptr,
	u64* SizeOutput = nullptr);

// Like DownloadFile, except that it saves it to a file.
bool DownloadFileToFile(const DownloadManagerType& DownloadManager,
	const char* URL,
	int Port,
	const char* OutputPath,
	const char* Range = nullptr,
	Hash::Strong* HashOutput = nullptr,
	u64* SizeOutput = nullptr);