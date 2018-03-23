#pragma once

#include <cstdint>

namespace MProcess
{

// Starts a process, returns a handle to the process on success or 0 on error.
uintptr_t Start(const char* File, const char * const * argv = nullptr);

struct AwaitResult {
	bool TimedOut;
	int ExitCode;
};

constexpr u32 Infinite = 0xFFFFFFFF;

// Waits for a process to terminate.
AwaitResult Await(uintptr_t Handle, u32 Timeout = Infinite);

// Terminates a process.
bool Terminate(uintptr_t Handle, u32 ExitCode);

}