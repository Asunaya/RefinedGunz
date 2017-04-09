#pragma once

#include <cstring>
#include <cassert>
#include <string>

class StringView
{
public:
	using CharType = char;

	StringView() : ptr{ "" }, sz{ 0 } {}

	StringView(const CharType* ptr) : ptr{ ptr } {
		sz = strlen(ptr);
	}

	StringView(const CharType* ptr, size_t sz)
		: ptr{ ptr }, sz{ sz } {}

	StringView(const std::string& str)
		: ptr{ str.c_str() }, sz{ str.size() } {}

	bool operator==(const StringView& rhs) const {
		return size() == rhs.size() && !memcmp(ptr, rhs.ptr, size());
	}

	const CharType& operator[](size_t i) const {
		assert(i < sz);
		return ptr[i];
	}

	const CharType* data() const { return ptr; }
	size_t size() const { return sz; }
	bool empty() const { return sz == 0; }

	const CharType* begin() const { return ptr; }
	const CharType* end() const { return ptr + size(); }

	std::string str() const { return{ data(), size() }; }

	StringView substr(size_t pos, size_t count = npos) const {
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

	size_t find(const StringView& needle) const {
		for (size_t i = 0; i < size(); ++i)
		{
			if (substr(i, needle.size()) == needle)
			{
				return i;
			}
		}

		return npos;
	}

	size_t find_last_of(const StringView& needle, size_t pos = npos) const {
		if (pos == npos)
			pos = size() - 1;

		for (size_t i = pos; i != npos; --i)
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
		return find_last_of(StringView{ &c, 1 }, pos);
	}

	static constexpr size_t npos = static_cast<size_t>(-1);

private:
	const char* ptr;
	size_t sz;
};

// Case-insensitive functions
inline bool iequals(const StringView& lhs, const StringView& rhs) {
	if (lhs.size() != rhs.size())
		return false;

	return _strnicmp(lhs.data(), rhs.data(), lhs.size()) == 0;
}

inline size_t ifind(const StringView& haystack, const StringView& needle) {
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

inline bool icontains(const StringView& haystack, const StringView& needle) {
	return ifind(haystack, needle) != haystack.npos;
}

inline char* strcpy_safe(char *Dest, size_t DestSize, const StringView& Source)
{
	size_t CopySize = Source.size();
	if (CopySize + 1 > DestSize)
	{
		CopySize = DestSize - 1;
	}

	memcpy(Dest, Source.data(), CopySize);
	Dest[CopySize] = 0;

	return Dest + CopySize;
}

template <size_t DestSize>
char* strcpy_safe(char(&Dest)[DestSize], const StringView& Source) {
	return strcpy_safe(Dest, DestSize, Source);
}