// # SafeString.h
//
// Functions for interacting with C strings (null-terminated arrays of characters) with
// bounds checking.
//
// # Return values
//
// If the return type of a function in this file is a pointer to a character, the value is the
// address of the null terminator in the destination string after writing.
//
// If the return type is instead integral, the value is the number of characters written, not
// including the null terminator.
//
#pragma once
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <cassert>
#include <algorithm>

#pragma warning(push)
#pragma warning(disable:4996)

template <size_t size1, size_t size2>
inline char* strcpy_literal(char(&Dest)[size1], const char(&Source)[size2])
{
	static_assert(size2 < size1, "Literal must be smaller than the destination string");
	memcpy(Dest, Source, size2);
	return Dest + size2;
}

inline char* strcpy_trunc(char* Dest, size_t size, const char* Source)
{
	auto len = strlen(Source);

	if (size - 1 < len)
	{
		len = size - 1;
	}

	memcpy(Dest, Source, len);

	Dest[len] = 0;

	return Dest + len;
}

template <size_t size>
inline char* strcpy_trunc(char(&Dest)[size], const char *Source)
{
	return strcpy_trunc(Dest, size, Source);
}

inline char* strcpy_safe(char *Dest, size_t size, const char *Source)
{
	auto len = strlen(Source);

	if (size - 1 < len && size > 0)
	{
		len = size - 1;
		assert(false);
	}

	memcpy(Dest, Source, len);

	Dest[len] = 0;

	return Dest + len;
}

template <size_t size>
inline char* strcpy_safe(char(&Dest)[size], const char *Source)
{
	return strcpy_safe(Dest, size, Source);
}

inline char* strncpy_safe(char *Dest, size_t size, const char *Source, size_t Count)
{
	auto len = Count;

	if (size - 1 < len)
	{
		len = size - 1;
	}

	memcpy(Dest, Source, len);

	Dest[len] = 0;

	return Dest + len;
}

template <size_t size>
inline char* strncpy_safe(char(&Dest)[size], const char *Source, size_t Count)
{
	return strncpy_safe(Dest, size, Source, Count);
}

inline char* strcat_safe(char *Dest, size_t size, const char *Source)
{
	auto DestLen = strlen(Dest);
	auto SourceLen = strlen(Source);

	auto BytesToCopy = SourceLen;
	if (DestLen + BytesToCopy + 1 > size && size > 0)
	{
		BytesToCopy = size - 1 - DestLen;
	}

	memcpy(Dest + DestLen, Source, BytesToCopy);

	Dest[DestLen + BytesToCopy] = 0;

	return Dest + DestLen + BytesToCopy;
}

template <size_t size>
inline char* strcat_safe(char(&Dest)[size], const char *Source)
{
	return strcat_safe(Dest, size, Source);
}

inline char* strncat_safe(char* Dest, size_t size, const char* Source, size_t Count)
{
	auto DestLen = strlen(Dest);
	auto SourceLen = Count;

	auto BytesToCopy = SourceLen;
	if (DestLen + BytesToCopy + 1 > size)
	{
		BytesToCopy = size - 1 - DestLen;
	}

	memcpy(Dest + DestLen, Source, BytesToCopy);

	Dest[DestLen + BytesToCopy] = 0;

	return Dest + DestLen + BytesToCopy;
}

template <size_t size>
inline char* strncat_safe(char(&Dest)[size], const char* Source, size_t Count)
{
	return strncat_safe(Dest, size, Source, Count);
}

inline int vsprintf_safe(char *Dest, size_t size, const char* Format, va_list va)
{
	return (std::min)(vsnprintf(Dest, size, Format, va), int(size));
}

template <size_t size>
inline int vsprintf_safe(char(&Dest)[size], const char* Format, va_list va)
{
	return vsprintf_safe(Dest, size, Format, va);
}

template<size_t size>
inline int sprintf_safe(char(&Dest)[size], const char *Format, ...)
{
	va_list args;
	va_start(args, Format);
	int ret = vsprintf_safe(Dest, Format, args);
	va_end(args);
	return ret;
}

inline int sprintf_safe(char *Dest, int size, const char *Format, ...)
{
	va_list args;
	va_start(args, Format);
	int ret = vsprintf_safe(Dest, size, Format, args);
	va_end(args);
	return ret;
}

template <size_t size1, size_t size2>
inline wchar_t* strcpy_literal(wchar_t(&Dest)[size1], const wchar_t(&Source)[size2])
{
	static_assert(size2 < size1, "Literal must be smaller than the destination string");
	memcpy(Dest, Source, size2 * sizeof(wchar_t));
	return Dest + size2;
}

inline wchar_t* strcpy_trunc(wchar_t* Dest, size_t size, const wchar_t* Source)
{
	auto len = wcslen(Source);

	if (size - 1 < len)
	{
		len = size - 1;
	}

	memcpy(Dest, Source, len * sizeof(wchar_t));

	Dest[len] = 0;

	return Dest + len;
}

template <size_t size>
inline wchar_t* strcpy_trunc(wchar_t(&Dest)[size], const wchar_t *Source)
{
	return strcpy_trunc(Dest, size, Source);
}

inline wchar_t* strcpy_safe(wchar_t *Dest, size_t size, const wchar_t *Source)
{
	auto len = wcslen(Source);

	if (size - 1 < len && size > 0)
	{
		len = size - 1;
		assert(false);
	}

	memcpy(Dest, Source, len * sizeof(wchar_t));

	Dest[len] = 0;

	return Dest + len;
}

template <size_t size>
inline wchar_t* strcpy_safe(wchar_t(&Dest)[size], const wchar_t *Source)
{
	return strcpy_safe(Dest, size, Source);
}

inline wchar_t* strncpy_safe(wchar_t *Dest, size_t size, const wchar_t *Source, size_t Count)
{
	auto len = Count;

	if (size - 1 < len)
	{
		len = size - 1;
	}

	memcpy(Dest, Source, len * sizeof(wchar_t));

	Dest[len] = 0;

	return Dest + len;
}

template <size_t size>
inline wchar_t* strncpy_safe(wchar_t(&Dest)[size], const wchar_t *Source, size_t Count)
{
	return strncpy_safe(Dest, size, Source, Count);
}

inline wchar_t* strcat_safe(wchar_t *Dest, size_t size, const wchar_t *Source)
{
	auto DestLen = wcslen(Dest);
	auto SourceLen = wcslen(Source);

	auto BytesToCopy = SourceLen;
	if (DestLen + BytesToCopy + 1 > size && size > 0)
	{
		BytesToCopy = size - 1 - DestLen;
	}

	memcpy(Dest + DestLen, Source, BytesToCopy * sizeof(wchar_t));

	Dest[DestLen + BytesToCopy] = 0;

	return Dest + DestLen + BytesToCopy;
}

template <size_t size>
inline wchar_t* strcat_safe(wchar_t(&Dest)[size], const wchar_t *Source)
{
	return strcat_safe(Dest, size, Source);
}

inline wchar_t* strncat_safe(wchar_t* Dest, size_t size, const wchar_t* Source, size_t Count)
{
	auto DestLen = wcslen(Dest);
	auto SourceLen = Count;

	auto BytesToCopy = SourceLen;
	if (DestLen + BytesToCopy + 1 > size)
	{
		BytesToCopy = size - 1 - DestLen;
	}

	memcpy(Dest + DestLen, Source, BytesToCopy * sizeof(wchar_t));

	Dest[DestLen + BytesToCopy] = 0;

	return Dest + DestLen + BytesToCopy;
}

template <size_t size>
inline wchar_t* strncat_safe(wchar_t(&Dest)[size], const wchar_t* Source, size_t Count)
{
	return strncat_safe(Dest, size, Source, Count);
}

inline int vswprintf_safe(wchar_t *Dest, size_t size, const wchar_t* Format, va_list va)
{
	return vswprintf(Dest, size, Format, va);
}

template <size_t size>
inline int vswprintf_safe(wchar_t(&Dest)[size], const wchar_t* Format, va_list va)
{
	return vswprintf_safe(Dest, size, Format, va);
}

inline int swprintf_safe(wchar_t *Dest, int size, const wchar_t *Format, ...)
{
	va_list args;
	va_start(args, Format);
	int ret = vswprintf_safe(Dest, size, Format, args);
	va_end(args);
	return ret;
}

template<size_t size>
inline int swprintf_safe(wchar_t(&Dest)[size], const wchar_t *Format, ...)
{
	va_list args;
	va_start(args, Format);
	int ret = vswprintf_safe(Dest, size, Format, args);
	va_end(args);
	return ret;
}

inline int vprintf_safe(const char* Format, va_list va)
{
	return vprintf(Format, va);
}

template <size_t size>
void itoa_safe(int val, char(&dest)[size], int radix)
{
	if (radix == 10)
		sprintf_safe(dest, "%d", val);
	else if (radix == 16)
		sprintf_safe(dest, "%x", val);
	else
		assert(false);
}

inline auto strcpy_unsafe(char* a, const char* b) {
	return strcpy(a, b);
}

#pragma warning(pop)