#include "stdafx.h"
#include "MFile.h"
#include "defer.h"

#ifdef WIN32
#include "MWindows.h"
#include <io.h>
#undef DeleteFile
#endif

namespace MFile
{

optional<u64> Size(const char* Path)
{
	auto Attr = MFile::GetAttributes(Path);
	if (!Attr)
		return nullopt;
	return Attr->Size;
}

#ifdef _WIN32

bool Exists(const char* Path)
{
	constexpr auto F_OK = 0;
	return !_access(Path, F_OK);
}

bool Delete(const char* Path)
{
	return ::DeleteFileA(Path) != FALSE;
}

bool Move(const char* OldPath, const char* NewPath)
{
	return ::MoveFileA(OldPath, NewPath) != FALSE;
}

static bool EntityExists(const char* Path, u32 SubdirValue)
{
	if (!Exists(Path))
		return false;

	auto MaybeAttributes = GetAttributes(Path);
	if (!MaybeAttributes.has_value())
		return false;

	return (MaybeAttributes.value().Attributes & Attributes::Subdir) == SubdirValue;
}

bool IsFile(const char* Path)
{
	return EntityExists(Path, 0);
}

bool IsDir(const char* Path)
{
	return EntityExists(Path, Attributes::Subdir);
}

bool CreateFile(const char* Path)
{
	auto FileHandle = ::CreateFileA(Path, 0, 0, nullptr, 0, 0, nullptr);
	if (FileHandle == nullptr)
		return false;

	CloseHandle(FileHandle);
	return true;
}

bool CreateDir(const char* Path)
{
	return ::CreateDirectoryA(Path, nullptr) != FALSE;
}

static constexpr u64 GetU64(u32 High, u32 Low) {
	return (static_cast<uint64_t>(High) << 32) | Low;
}

static constexpr u64 GetU64(FILETIME filetime) {
	return GetU64(filetime.dwHighDateTime, filetime.dwLowDateTime);
}

optional<FileAttributes> GetAttributes(const char* Path)
{
	WIN32_FILE_ATTRIBUTE_DATA wfad;
	if (!GetFileAttributesExA(Path, GetFileExInfoStandard, &wfad))
		return nullopt;
	
	FileAttributes ret{};

	ret.Attributes = wfad.dwFileAttributes;
	ret.CreationTime = GetU64(wfad.ftCreationTime);
	ret.LastAccessTime = GetU64(wfad.ftLastAccessTime);
	ret.LastModifiedTime = GetU64(wfad.ftLastWriteTime);
	ret.Size = GetU64(wfad.nFileSizeHigh, wfad.nFileSizeLow);

	return ret;
}

static void CopyFileData(FileData& dest, const _finddata_t& src)
{
	dest.Attributes = src.attrib;
	dest.CreationTime = src.time_create;
	dest.LastAccessTime = src.time_access;
	dest.LastModifiedTime = src.time_write;
	dest.Size = src.size;
	strcpy_safe(dest.Name, src.name);
}

FileIterator& FileIterator::operator++()
{
	_finddata_t fd;
	auto find_ret = _findnext(Range.First, &fd);
	if (find_ret != 0)
		End = true;
	else
		CopyFileData(Range.Data, fd);

	return *this;
}

FileIterator FileRange::begin() {
	return{ *this, First == -1 ? true : false };
}

FileIterator FileRange::end() {
	return{ *this, true };
}

FileRange FilesAndSubdirsInDir(const char* Path)
{
	FileRange ret;

	_finddata_t fd;
	ret.First = _findfirst(Path, &fd);

	if (ret.First != -1)
		CopyFileData(ret.Data, fd);

	return ret;
}

#else

bool DeleteFile(const char* Path)
{
	// TODO
	return false;
}

FileRange FilesAndSubdirsInDir(const char* Path)
{
	return false;
}

#endif

bool File::open_impl(const char* path, const char* mode)
{
#pragma warning(push)
#pragma warning(disable: 4996)
	file_ptr = CFilePtr{ fopen(path, mode) };
#pragma warning(pop)

	state.error = file_ptr == nullptr;
	return !state.error;
}

bool File::open(const char* path)
{
	return open_impl(path, "rb");
}

bool File::open(const char* path, TextType)
{
	return open_impl(path, "r");
}

void File::close()
{
	if (!file_ptr)
		return;

	// Close the FILE*.
	auto fclose_ret = fclose(fp());

	// Since we've already closed the file, we need to set the pointer to null
	// without triggering the fclose on reset, so we release it without doing
	// anything with the return value.
	(void)file_ptr.release();

	state.error = fclose_ret != 0;
}

bool File::is_open() const
{
	return file_ptr != nullptr;
}

u64 File::size()
{
#define E do { if (error()) return 0; } while (false)

	const auto prev_pos = tell();
	E;

	seek(0, Seek::End);
	E;

	const auto ret = tell();
	E;

	seek(prev_pos, Seek::Begin);

	return ret;

#undef E
}

bool File::seek(i64 offset, Seek origin)
{
	assert(fp());

#ifdef _MSC_VER
	auto fseek_ret = _fseeki64(fp(), offset, static_cast<int>(origin));
#else
	static_assert(false, "Port me!");
#endif

	state.error = fseek_ret != 0;
	if (state.error)
		clearerr(fp());

	return !state.error;
}

i64 File::tell()
{
	assert(fp());

#ifdef _MSC_VER
	auto ftell_ret = _ftelli64(fp());
#else
	static_assert(false, "Port me!");
#endif

	state.error = ftell_ret == tell_error;

	return ftell_ret;
}

size_t File::read(void* buffer, size_t size)
{
	assert(fp());

	auto fread_ret = fread(buffer, 1, size, fp());

	const auto error_or_eof = fread_ret != size;
	if (error_or_eof)
	{
		state.error = ferror(fp()) != 0;
		state.eof = feof(fp()) != 0;
		if (state.error)
			clearerr(fp());
	}
	else
	{
		state.error = false;
		state.eof = false;
	}

	return fread_ret;
}

bool File::flush()
{
	auto fflush_ret = fflush(fp());
	state.error = fflush_ret == EOF;
	if (state.error)
		clearerr(fp());
	return !state.error;
}

bool File::eof() const
{
	assert(fp());

	return state.eof;
}

bool File::error() const
{
	return state.error;
}

FILE* File::fp() const
{
	return file_ptr.get();
}

CFilePtr File::release()
{
	return std::move(file_ptr);
}

size_t RWFile::write(const void* buffer, size_t size)
{
	assert(fp());

	auto fwrite_ret = fwrite(buffer, 1, size, fp());

	state.error = fwrite_ret != size;
	if (state.error)
		clearerr(fp());

	return fwrite_ret;
}

}