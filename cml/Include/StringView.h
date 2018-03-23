#pragma once

#include <cstring>
#include <cassert>
#include <string>
#include <cwchar>
#include <cctype>
#include <cwctype>
#include <utility>
#include "ArrayView.h"

namespace detail {
inline size_t len(const char* str) { return strlen(str); }
inline size_t len(const wchar_t* str) { return wcslen(str); }
}

template <typename CharType>
class BasicStringView
{
	using rev_it_t = std::reverse_iterator<const CharType*>;
public:
	using size_type = size_t;
	using iterator = const CharType*;

	BasicStringView() : ptr{ "" }, sz{ 0 } {}

	BasicStringView(const CharType* ptr) : ptr{ ptr } {
		sz = detail::len(ptr);
	}

	BasicStringView(const CharType* ptr, size_t sz)
		: ptr{ ptr }, sz{ sz } {}

	BasicStringView(const std::basic_string<CharType>& str)
		: ptr{ str.c_str() }, sz{ str.size() } {}

	bool operator==(const BasicStringView& rhs) const {
		return size() == rhs.size() && !memcmp(ptr, rhs.ptr, size() * sizeof(CharType));
	}

	bool operator!=(const BasicStringView& rhs) const {
		return !(*this == rhs);
	}

	const CharType& operator[](size_t i) const {
		assert(i < sz);
		return ptr[i];
	}

	const CharType* data() const { return ptr; }
	size_t size() const { return sz; }
	bool empty() const { return sz == 0; }

	iterator begin() const { return ptr; }
	iterator end() const { return ptr + size(); }

	rev_it_t rbegin() const { return rev_it_t{ end() }; }
	rev_it_t rend() const { return rev_it_t{ begin() }; }

	std::basic_string<CharType> str() const { return{ data(), size() }; }

	BasicStringView substr(size_t pos, size_t count = npos) const {
		if (count == npos)
			count = size() - pos;

		return{ data() + pos, count };
	}

	const CharType& front() const {
		assert(!empty());
		return data()[0];
	}

	const CharType& back() const {
		assert(!empty());
		return data()[size()];
	}

	size_t find(const BasicStringView& needle) const {
		for (size_t i = 0; i < size(); ++i)
		{
			if (substr(i, needle.size()) == needle)
			{
				return i;
			}
		}

		return npos;
	}

	size_t find_first_of(const BasicStringView& needle, size_t pos = npos) const {
		if (pos == npos)
			pos = 0;

		for (size_t i = pos, end = size(); i < end; ++i)
		{
			for (auto&& c : needle)
			{
				if (data()[i] == c)
				{
					return i;
				}
			}
		}

		return npos;
	}

	size_t find_first_of(CharType c, size_t pos = npos) const {
		return find_first_of(BasicStringView{ &c, 1 }, pos);
	}

	size_t find_last_of(const BasicStringView& needle, size_t pos = npos) const {
		if (pos == npos)
			pos = size() - 1;

		for (size_t i = pos, end = size(); i < end; --i)
		{
			for (auto&& c : needle)
			{
				if (data()[i] == c)
				{
					return i;
				}
			}
		}

		return npos;
	}

	size_t find_last_of(CharType c, size_t pos = npos) const {
		return find_last_of(BasicStringView{ &c, 1 }, pos);
	}

	static constexpr size_t npos = static_cast<size_t>(-1);

private:
	const CharType* ptr;
	size_t sz;
};

using StringView = BasicStringView<char>;
using WStringView = BasicStringView<wchar_t>;

// Case-insensitive functions
inline bool iequals(const StringView& lhs, const StringView& rhs) {
	if (lhs.size() != rhs.size())
		return false;
	
	for (size_t i = 0, end = lhs.size(); i < end; ++i)
	{
		if (tolower(lhs[i]) != tolower(rhs[i]))
		{
			return false;
		}
	}

	return true;
}

inline bool iequals(const WStringView& lhs, const WStringView& rhs) {
	if (lhs.size() != rhs.size())
		return false;

	for (size_t i = 0, end = lhs.size(); i < end; ++i)
	{
		if (towlower(lhs[i]) != towlower(rhs[i]))
		{
			return false;
		}
	}

	return true;
}

template <typename CharType>
inline size_t ifind(const BasicStringView<CharType>& haystack, const BasicStringView<CharType>& needle) {
	if (haystack.size() < needle.size())
		return haystack.npos;

	for (size_t i = 0; i < haystack.size(); ++i)
	{
		if (iequals(haystack.substr(i, needle.size()), needle))
		{
			return i;
		}
	}

	return haystack.npos;
}

inline size_t ifind(const StringView& haystack, const StringView& needle) {
	return ifind<char>(haystack, needle);
}

inline size_t ifind(const WStringView& haystack, const WStringView& needle) {
	return ifind<wchar_t>(haystack, needle);
}

template <typename CharType>
inline bool icontains(const BasicStringView<CharType>& haystack, const BasicStringView<CharType>& needle) {
	return ifind(haystack, needle) != haystack.npos;
}

inline bool icontains(const StringView& haystack, const StringView& needle) {
	return icontains<char>(haystack, needle);
}

inline bool icontains(const WStringView& haystack, const WStringView& needle) {
	return icontains<wchar_t>(haystack, needle);
}

template <typename CharType>
CharType* strcpy_safe(CharType *Dest, size_t DestSize, const BasicStringView<CharType>& Source)
{
	size_t CopySize = Source.size();
	if (CopySize + 1 > DestSize)
	{
		CopySize = DestSize - 1;
	}

	memcpy(Dest, Source.data(), CopySize * sizeof(CharType));
	Dest[CopySize] = 0;

	return Dest + CopySize;
}

template <typename CharType, size_t DestSize>
CharType* strcpy_safe(CharType(&Dest)[DestSize], const BasicStringView<CharType>& Source) {
	return strcpy_safe(Dest, DestSize, Source);
}

template <typename CharType>
CharType* strcpy_safe(ArrayView<CharType>& Dest, const BasicStringView<CharType>& Source) {
	return strcpy_safe(Dest.data(), Dest.size(), Source);
}

template <typename CharType>
CharType* strcat_safe(CharType *Dest, size_t DestSize, const BasicStringView<CharType>& Source)
{
	auto DestLen = strlen(Dest);
	auto SourceLen = Source.size();

	auto CharsToCopy = SourceLen;
	if (DestLen + CharsToCopy + 1 > DestSize && DestSize > 0)
	{
		CharsToCopy = DestSize - 1 - DestLen;
	}

	memcpy(Dest + DestLen, Source.data(), CharsToCopy * sizeof(CharType));

	Dest[DestLen + CharsToCopy] = 0;

	return Dest + DestLen + CharsToCopy;
}

template <typename CharType, size_t DestSize>
CharType* strcat_safe(CharType(&Dest)[DestSize], const BasicStringView<CharType>& Source) {
	return strcat_safe(Dest, DestSize, Source);
}

template <typename CharType>
CharType* strcat_safe(ArrayView<CharType>& Dest, const BasicStringView<CharType>& Source) {
	return strcat_safe(Dest.data(), Dest.size(), Source);
}