#pragma once
#include <functional>
#include "GlobalTypes.h"
#include "ArrayView.h"
#include "function_view.h"

namespace MCrashDump {
void SetCallback(std::function<void(uintptr_t)> CrashCallback);
void Try(function_view<void()> Func, function_view<void(uintptr_t)> CrashCallback);
void WriteDump(uintptr_t ExceptionInfo, const char* Filename);
}
