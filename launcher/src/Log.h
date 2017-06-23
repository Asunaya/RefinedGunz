#pragma once

#include "SafeString.h"
#include "MFile.h"
#include "defer.h"
#include <cstdarg>
#include <cassert>

enum class LogLevel
{
	Debug,
	Info,
	Warning,
	Error,
	Fatal,
};

struct Logger
{
	int DebugVerbosity = -1;
	// 0 = base
	// 4 = tight loops
	// 5 = very tight loops

	void InitFile(const char* LogFilePath)
	{
		LogFile.open(LogFilePath, MFile::ClearExistingContents, MFile::Text);
	}

#define LOGGER_LOG_FUNCTION(Level)\
	void Level(const char* Format, ...){\
		va_list va;\
		va_start(va, Format);\
		LogVA(LogLevel::Level, Format, va);\
		va_end(va);\
	}

	LOGGER_LOG_FUNCTION(Info);
	LOGGER_LOG_FUNCTION(Warning);
	LOGGER_LOG_FUNCTION(Error);
	LOGGER_LOG_FUNCTION(Fatal);

#undef LOGGER_LOG_FUNCTION

	void Debug(const char* Format, ...)
	{
		va_list va;
		va_start(va, Format);
		DebugVA(0, Format, va);
		va_end(va);
	}

	void Debug(int MinimumDebugVerbosity, const char* Format, ...)
	{
		va_list va;
		va_start(va, Format);
		DebugVA(MinimumDebugVerbosity, Format, va);
		va_end(va);
	}

	void operator()(LogLevel Level, const char* Format, ...)
	{
		va_list va;
		va_start(va, Format);
		LogVA(Level, Format, va);
		va_end(va);
	}

	void LogVA(LogLevel Level, const char* Format, va_list va)
	{
#ifndef _DEBUG
		if (Level == LogLevel::Debug)
			return;
#endif

		const auto Prefix = [&] {
			switch (Level)
			{
			case LogLevel::Debug:
				return "[DEBUG] ";
			case LogLevel::Info:
				return "[INFO]  ";
			case LogLevel::Warning:
				return "[WARN]  ";
			case LogLevel::Error:
				return "[ERROR] ";
			case LogLevel::Fatal:
				return "[FATAL] ";
			default:
				assert(false);
				return "[???]   ";
			}
		}();

		char FormattedString[16 * 1024];
		const auto FormattedStringLength = vsprintf_safe(FormattedString, Format, va);

		Print(Prefix);
		
		int LastNewline = -1;
		for (int i = 0; i < FormattedStringLength; ++i)
		{
			if (FormattedString[i] != '\n' && i != FormattedStringLength - 1)
				continue;

			auto StartIndex = LastNewline + 1;
			auto Size = static_cast<size_t>(i - StartIndex);
			auto StartPtr = &FormattedString[StartIndex];
			
			auto PrevEndChar = StartPtr[Size];
			StartPtr[Size] = 0;
			DEFER([&] { FormattedString[Size] = PrevEndChar; });

			Print({ StartPtr, Size });

			if (PrevEndChar == '\n')
				PrintNewline();

			if (i == FormattedStringLength - 1)
				break;

			Print(Prefix);

			LastNewline = i;
		}
	}

private:
	// Must be null terminated.
	void Print(const StringView& String)
	{
		// Write to console.
		printf("%.*s", String.size(), String.data());

		// Write to file, if it's open.
		if (LogFile.is_open())
		{
			LogFile.write(String.data(), String.size());
		}

		// If we're compiling in debug mode and on Windows, write to debug output with
		// OutputDebugString also.
#if defined(_DEBUG) && defined(_WIN32)
		OutputDbgString(String.data());
#endif
	}

	void PrintNewline()
	{
		Print("\n");
	}

	void DebugVA(int MinimumDebugVerbosity, const char* Format, va_list va)
	{
#ifndef _DEBUG
		return;
#endif

		if (DebugVerbosity < MinimumDebugVerbosity)
			return;

		LogVA(LogLevel::Debug, Format, va);
	}

	// Defined in Log.cpp to not pollute the rest of the program with Windows.h.
	void OutputDbgString(const char*);

	MFile::RWFile LogFile;
};

extern Logger Log;