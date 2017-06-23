#pragma once

#include "GlobalTypes.h"
#include "StringView.h"
#include <cctype>

inline size_t HashFNV(const void* Memory, size_t Length)
{
#if defined(_WIN64)
	static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
	const size_t FNV_offset_basis = 14695981039346656037ULL;
	const size_t FNV_prime = 1099511628211ULL;

#else /* defined(_WIN64) */
	static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
	const size_t FNV_offset_basis = 2166136261U;
	const size_t FNV_prime = 16777619U;
#endif /* defined(_WIN64) */

	size_t ret = FNV_offset_basis;
	for (size_t Next = 0; Next < Length; ++Next)
	{	// fold in another byte
		ret ^= static_cast<size_t>(reinterpret_cast<const unsigned char*>(Memory)[Next]);
		ret *= FNV_prime;
	}
	return ret;
}

inline size_t HashFNVCaseInsensitive(const void* Memory, size_t Length)
{
#if defined(_WIN64)
	static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
	const size_t FNV_offset_basis = 14695981039346656037ULL;
	const size_t FNV_prime = 1099511628211ULL;

#else /* defined(_WIN64) */
	static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
	const size_t FNV_offset_basis = 2166136261U;
	const size_t FNV_prime = 16777619U;
#endif /* defined(_WIN64) */

	size_t ret = FNV_offset_basis;
	for (size_t Next = 0; Next < Length; ++Next)
	{	// fold in another byte
		auto c = reinterpret_cast<const u8*>(Memory)[Next];
		if (isalpha(c))
			c = tolower(c);
		ret ^= static_cast<size_t>(c);
		ret *= FNV_prime;
	}
	return ret;
}

inline size_t HashFNVPath(const void* Memory, size_t Length)
{
#if defined(_WIN64)
	static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
	const size_t FNV_offset_basis = 14695981039346656037ULL;
	const size_t FNV_prime = 1099511628211ULL;

#else /* defined(_WIN64) */
	static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
	const size_t FNV_offset_basis = 2166136261U;
	const size_t FNV_prime = 16777619U;
#endif /* defined(_WIN64) */

	size_t ret = FNV_offset_basis;
	for (size_t Next = 0; Next < Length; ++Next)
	{	// fold in another byte
		auto c = reinterpret_cast<const u8*>(Memory)[Next];
		if (isalpha(c))
			c = tolower(c);
		if (c == '\\')
			c = '/';
		ret ^= static_cast<size_t>(c);
		ret *= FNV_prime;
	}
	return ret;
}

struct StringHasher {
	size_t operator()(const StringView& str) const {
		return HashFNV(str.data(), str.size());
	}
};

struct CaseInsensitiveStringHasher {
	size_t operator()(const StringView& str) const {
		return HashFNVCaseInsensitive(str.data(), str.size());
	}
};

struct PathHasher {
	size_t operator()(const StringView& str) const {
		return HashFNVPath(str.data(), str.size());
	}
};

struct PathComparer {
	bool operator()(const StringView& lhs, const StringView& rhs) const {
		if (lhs.size() != rhs.size())
			return false;

		for (size_t i = 0; i < lhs.size(); ++i)
		{
			auto Trans = [&](auto c) {
				if (isalpha(c))
					c = tolower(c);
				if (c == '\\')
					c = '/';
				return c;
			};

			if (Trans(lhs[i]) != Trans(rhs[i]))
				return false;
		}

		return true;
	}
};