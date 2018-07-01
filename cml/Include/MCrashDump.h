#pragma once
#include "GlobalTypes.h"
#include "function_view.h"
#include "ArrayView.h"

namespace MCrashDump {
void SetCallback(function_view<void(uintptr_t)> CrashCallback);
void Try(function_view<void()> Func, function_view<void(uintptr_t)> CrashCallback);
void WriteDump(uintptr_t ExceptionInfo, const char* Filename);
}