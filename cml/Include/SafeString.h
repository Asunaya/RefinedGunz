#pragma once
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

template<size_t size>
inline char *strcpy_safe(char(&Dest)[size], const char *Source)
{
	int len = strlen(Source);

	if (size - 1 < len)
	{
		len = size - 1;
	}

	memcpy(Dest, Source, len);

	Dest[len] = 0;

	return Dest;
}

inline char *strcpy_safe(char *Dest, int size, const char *Source)
{
	int len = strlen(Source);

	if (size - 1 < len && size > 0)
	{
		len = size - 1;
	}

	memcpy(Dest, Source, len);

	Dest[len] = 0;

	return Dest;
}

template<size_t size>
inline char *strcat_safe(char(&Dest)[size], const char *Source)
{
	int DestLen = strlen(Dest);
	int SourceLen = strlen(Source);

	int BytesToCopy = SourceLen;
	if (DestLen + BytesToCopy + 1 > size)
	{
		BytesToCopy = size - 1 - DestLen;
	}

	memcpy(Dest + DestLen, Source, BytesToCopy);

	Dest[DestLen + BytesToCopy] = 0;

	return Dest;
}

inline char *strcat_safe(char *Dest, int size, const char *Source)
{
	int DestLen = strlen(Dest);
	int SourceLen = strlen(Source);

	int BytesToCopy = SourceLen;
	if (DestLen + BytesToCopy + 1 > size && size > 0)
	{
		BytesToCopy = size - 1 - DestLen;
	}

	memcpy(Dest + DestLen, Source, BytesToCopy);

	Dest[DestLen + BytesToCopy] = 0;

	return Dest;
}

#pragma warning(push)
#pragma warning(disable:4996)
template<size_t size>
inline int sprintf_safe(char(&Dest)[size], const char *Format, ...)
{
	va_list args;

	va_start(args, Format);
	int ret = _vsnprintf(Dest, size, Format, args);
	va_end(args);

	Dest[size - 1] = 0;

	return ret;
}

inline int sprintf_safe(char *Dest, int size, const char *Format, ...)
{
	va_list args;

	va_start(args, Format);
	int ret = _vsnprintf(Dest, size, Format, args);
	va_end(args);

	Dest[size - 1] = 0;

	return ret;
}
#pragma warning(pop)