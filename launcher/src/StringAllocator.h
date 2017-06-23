#pragma once

#include <vector>
#include <memory>
#include "StringView.h"
#include "ArrayView.h"

inline ArrayView<char> AllocateString(size_t Size)
{
	static std::vector<std::unique_ptr<char[]>> Strings;

	Strings.emplace_back(std::make_unique<char[]>(Size));
	return{ Strings.back().get(), Size };
}

inline StringView AllocateString(const StringView& Src)
{
	const auto Size = Src.size();
	auto String = AllocateString(Size);
	strcpy_safe(String, Src);
	return{ String.data(), String.size() };
}