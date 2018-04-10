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
#include <string>

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

// If size is 0, returns the length of the formatted string (not including the null terminator).
inline int vsprintf_safe(char *Dest, size_t size, const char* Format, va_list va)
{
	if (size == 0)
	{
		return vsnprintf(nullptr, 0, Format, va);
	}
	return (std::min)(vsnprintf(Dest, size, Format, va), int(size) - 1);
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

template <typename T, size_t size>
char* itoa_safe(T val, char(&dest)[size], int radix)
{
	assert(radix > 1 && radix <= 36);
	auto i = dest, end = dest + size - 1;
	bool negative = val < 0;
	std::make_unsigned_t<T> absval;
	if (negative)
	{
		--end;
		absval = 0;
		absval -= val;
	}
	else
	{
		absval = val;
	}
	while (i < end)
	{
		auto digit = absval % radix;
		auto c = digit + (digit < 10 ? '0' : 'a' - 10);
		*i = c;
		absval /= radix;
		++i;
		if (!absval)
			break;
	}
	if (negative)
	{
		*i = '-';
		++i;
	}
	*i = 0;
	std::reverse(dest, i);
	return i;
}

inline auto strcpy_unsafe(char* a, const char* b) {
	return strcpy(a, b);
}

inline std::string vstrprintf(const char* Format, va_list va)
{
	va_list va2;
	va_copy(va2, va);
	auto Size = static_cast<size_t>(vsprintf_safe(nullptr, 0, Format, va2));
	va_end(va2);
	std::string Ret(Size, 0);
	vsprintf_safe(&Ret[0], Size + 1, Format, va);
	return Ret;
}

inline std::string strprintf(const char* Format, ...)
{
	va_list va;
	va_start(va, Format);
	auto Size = static_cast<size_t>(vsprintf_safe(nullptr, 0, Format, va));
	va_end(va);
	std::string Ret(Size, 0);
	va_start(va, Format);
	vsprintf_safe(&Ret[0], Size + 1, Format, va);
	va_end(va);
	return Ret;
}

namespace detail {
inline int strerror_r_ret_impl(int ret) { return ret; }
inline int strerror_r_ret_impl(char*) { return 0; }
}

// Returns zero if ok, and a non-zero value on error.
inline int strerror_safe(int errnum, char* buf, size_t buflen)
{
#ifdef _MSC_VER
	return strerror_s(buf, buflen, errnum);
#else
	// Pass the result through an overloaded function to support the non-standard GNU strerror_r
	// version that returns char* instead of int.
	return detail::strerror_r_ret_impl(strerror_r(errnum, buf, buflen));
#endif
}

template <size_t buflen>
inline int strerror_safe(int errnum, char (&buf)[buflen]) {
	return strerror_safe(errnum, buf, buflen);
}

inline char* strlwr_safe(char* str, size_t size)
{
	size_t i = 0;
	for (; str[i] && i < size; ++i)
	{
		str[i] = tolower(str[i]);
	}
	return str + i;
}

template <size_t size>
char* strlwr_safe(char (&str)[size])
{
	return strlwr_safe(str, size);
}

#pragma warning(pop)
