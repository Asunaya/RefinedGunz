#pragma once

#ifdef _WIN32
#include "targetver.h"

#define POINTER_64 __ptr64

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#else
#include <type_traits>
#include "GlobalTypes.h"
constexpr bool TRUE = true;
constexpr bool FALSE = false;
using BYTE = u8;
using WORD = u16;
using DWORD = u32;
using QWORD = u64;
using LPBYTE = BYTE*;
using LPWORD = WORD*;
using LPDWORD = DWORD*;
using LPQWORD = QWORD*;
using DWORD_PTR = std::conditional_t<sizeof(void*) == 4, DWORD, QWORD>;
#endif

#ifndef _MSC_VER
#define _FILE_OFFSET_BITS 64
#ifndef __cpp_lib_nonmember_container_access
#include <cstddef>
namespace std {
template <typename T, std::size_t N>
constexpr std::size_t size(T (&)[N]) { return N; }
template <typename T>
constexpr auto size(T&& x) -> decltype(x.size()) { return x.size(); }
}
#endif
#endif

#include <stdio.h>
#include <list>
#include <map>

#include "MTime.h"
#include "SafeString.h"

#include <algorithm>
using std::min;
using std::max;