#pragma once

#include <string>
#include <memory>
#include <algorithm>
#include <intrin.h>
#include "TMP.h"
#include "GlobalTypes.h"
#include "StringView.h"
#include "optional.h"

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif

#ifdef SAFE_RELEASE
#undef SAFE_RELEASE
#endif

template <typename T>
inline void SafeRelease(T& ptr)
{
	if (!ptr)
		return;
	ptr->Release();
	ptr = nullptr;
}

struct D3DDeleter {
	void operator()(struct IUnknown* ptr) const;
};

template <typename T>
using D3DPtr = std::unique_ptr<T, D3DDeleter>;

template <typename T>
inline void SafeRelease(D3DPtr<T>& ptr)
{
	ptr = nullptr;
}

using WIN_DWORD_PTR = std::conditional_t<sizeof(void*) == 4, unsigned long, unsigned long long>;

#define SAFE_RELEASE(p)      { SafeRelease(p); }

#define EXPAND_VECTOR(v) v[0], v[1], v[2]

#define SetBitSet(sets, item)		(sets |= (1 << item))
#define ClearBitSet(sets, item)		(sets &= ~(1 << item))
#define CheckBitSet(sets, item)		(sets & (1 << item))

#ifdef _MSC_VER
#define WARN_UNUSED_RESULT _Check_return_
#define STDCALL __stdcall
#else
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#define STDCALL __attribute__((stdcall))
#endif

#define TOKENIZE_IMPL(a, b) a##b
#define TOKENIZE(a, b) TOKENIZE_IMPL(a, b)

inline constexpr uint32_t ARGB(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
	return (a << 24) | (r << 16) | (g << 8) | b;
}

inline constexpr uint32_t RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return ARGB(a, r, g, b);
}

inline constexpr uint32_t XRGB(uint8_t r, uint8_t g, uint8_t b)
{
	return ARGB(0xFF, r, g, b);
}

inline constexpr uint32_t XRGB(uint8_t value)
{
	return XRGB(value, value, value);
}

inline constexpr uint32_t RGBAF(float r, float g, float b, float a)
{
	return RGBA(u8(r * 255), u8(g * 255), u8(b * 255), u8(a * 255));
}

inline constexpr uint32_t ARGBF(float r, float g, float b, float a)
{
	return RGBA(u8(r * 255), u8(g * 255), u8(b * 255), u8(a * 255));
}

enum MDateType
{
	MDT_Y = 1,
	MDT_YM,
	MDT_YMD,
	MDT_YMDH,
	MDT_YMDHM,
};

std::string MGetStrLocalTime(unsigned short wYear = 0,
	unsigned short wMon = 0,
	unsigned short wDay = 0,
	unsigned short wHour = 0,
	unsigned short wMin = 0,
	MDateType = MDT_YMDHM);

namespace detail{
template <typename T, typename = void>
struct is_container : std::false_type {};

template <typename T>
struct is_container<T, decltype(std::begin(std::declval<T>()), std::end(std::declval<T>()), void())> : std::true_type {};
}

template <typename T>
struct Range
{
	T first;
	T second;

	Range() = default;

	Range(const T& first, const T& second) : first{ first }, second{ second } {}

	template <typename U, typename = std::enable_if_t<detail::is_container<U>::value>>
	Range(U&& Container) : first{ Container.begin() }, second{ Container.end() } {}

	auto begin() { return first; }
	auto end() { return second; }
	auto begin() const { return first; }
	auto end() const { return second; }
};

template <typename T>
auto MakeRange(const T& begin, const T& end) {
	return Range<T>{ begin, end };
}

template <typename T>
auto MakeRange(T&& x) -> std::enable_if_t<detail::is_container<T>::value, Range<decltype(std::begin(x))>> {
	return Range<decltype(std::begin(x))>{ x };
}

template <typename T>
auto MakeRange(const std::pair<T, T>& x) {
	return Range<T>{ x.first, x.second };
}

template <template <typename...> class itT, typename T>
auto MakeAdapter(T& Container) {
	return MakeRange(
		itT<decltype(Container.begin())>{ Container.begin() },
		itT<decltype(Container.end())>{ Container.end() });
}

template <typename ItT>
class ValueIterator
{
public:
	ValueIterator(ItT i) : it(i) { }

	ValueIterator& operator++()
	{
		++it;
		return *this;
	}

	ValueIterator operator++(int)
	{
		auto temp(*this);
		++*this;
		return temp;
	}

	bool operator==(const ValueIterator& rhs) const { return it == rhs.it; }
	bool operator!=(const ValueIterator& rhs) const { return it != rhs.it; }
	auto& operator*() { return it->second; }
	auto& operator->() { return this->operator*(); }

private:
	ItT it;
};

// Returns an adapter whose iterators returns .second of the pair that
// iterators of the original container return, i.e. the values of a map.
template <typename T>
auto MakePairValueAdapter(T& Container) { return MakeAdapter<ValueIterator>(Container); }

namespace detail
{
inline bool isdecdigit(char c) { return c >= '0' && c <= '9'; }
inline char tolower(char c) { return char(u8(u8(c) | 0x20)); }

// Returns the positive value of the digit, or -1 on error.
template <int Radix>
int GetDigit(char c)
{
	if (isdecdigit(c))
	{
		const auto Digit = c - '0';
		if (Digit > Radix - 1)
			return -1;
		return Digit;
	}

	if (Radix <= 10)
		return -1;

	c = toupper(c);

	constexpr auto LastValidDigitInBase = 'A' + (Radix - 10);
	if (c < 'A' || c > LastValidDigitInBase)
		return -1;

	return c - 'A' + 10;
}
}

template <typename T>
auto Reverse(T&& Container)
{
	return MakeRange(std::rbegin(Container), std::rend(Container));
}

template <typename DestType, int Radix = 10>
optional<DestType> StringToInt(StringView Str)
{
	static_assert(Radix > 0 && Radix <= 36, "Invalid radix");

	// Discard whitespace in the beginning
	{
		size_t LastWhitespacePos = 0;

		while (true)
		{
			if (LastWhitespacePos >= Str.size())
			{
				// The string is either empty (if LastWhitespacePos == 0), or
				// non-empty and entirely composed of whitespace.
				return nullopt;
			}

			if (Str[LastWhitespacePos] != ' ')
				break;

			++LastWhitespacePos;
		}

		Str = Str.substr(LastWhitespacePos);
	}

	DestType Accumulator = 0;
	DestType Coefficient = 1;

	bool IsNegative = Str[0] == '-';
	if (IsNegative)
		Str = Str.substr(1);

	for (auto DigitChar : Reverse(Str))
	{
		auto Digit = detail::GetDigit<Radix>(DigitChar);
		if (Digit == -1)
			return nullopt;

		Accumulator += Digit * Coefficient;
		Coefficient *= Radix;
	}

	if (IsNegative)
		Accumulator = 0 - Accumulator;

	return Accumulator;
}

// WriteProxy
// A class that acts as a pointer-to-pointer wrapper for smart pointers for passing to functions
// that expect to "return" a pointer by writing to a pointer-to-pointer argument.
// Should only ever be instantiated by MakeWriteProxy as a temporary for a argument.
// The smart pointer will then have the returned value (if set) after the full expression it appears in.
//
// Example:
// void foo(int**); std::unique_ptr<int> ptr; foo(MakeWriteProxy(ptr));
template <typename T>
class WriteProxy
{
	using StoredType = tmp::get_template_argument_t<T, 0>;
public:
	~WriteProxy() { ptr = T{ temp }; }

	operator StoredType**() && { return &temp; }

private:
	WriteProxy(T& ptr) : ptr(ptr), temp(ptr.get()) {}
	WriteProxy(const WriteProxy&) = delete;
	WriteProxy& operator=(const WriteProxy&) = delete;

	template <typename... U>
	friend WriteProxy<std::unique_ptr<U...>> MakeWriteProxy(std::unique_ptr<U...>&);

	T& ptr;
	StoredType* temp{};
};

template <typename... T>
WriteProxy<std::unique_ptr<T...>> MakeWriteProxy(std::unique_ptr<T...>& ptr) {
	return{ ptr };
}

// Converts an rvalue to a mutable lvalue
template <typename T>
T& unmove(T&& x) { return x; }

// Returns value rounded up towards the nearest power of two
inline u32 NextPowerOfTwo(u32 value)
{
	unsigned long rightmost_bit;
	if (!_BitScanReverse(&rightmost_bit, value))
		return 2;
	auto rightmost_bit_value = 1 << rightmost_bit;
	if ((value ^ rightmost_bit_value) != 0)
		rightmost_bit_value <<= 1;
	return rightmost_bit_value;
}

template <typename ContainerType, typename ValueType>
void erase_remove(ContainerType&& Container, ValueType&& Value) {
	Container.erase(std::remove(Container.begin(), Container.end(), Value), Container.end());
}

template <typename ContainerType, typename PredicateType>
void erase_remove_if(ContainerType&& Container, PredicateType&& Predicate) {
	Container.erase(std::remove_if(Container.begin(), Container.end(),
		std::forward<PredicateType>(Predicate)),
		Container.end());
}

struct CFileCloser {
	void operator()(FILE* ptr) const {
		if (ptr)
			fclose(ptr);
	}
};

using CFilePtr = std::unique_ptr<FILE, CFileCloser>;

template <typename DerivedType, typename ValueType, typename CategoryType = std::random_access_iterator_tag>
struct IteratorBase
{
	bool operator!=(const IteratorBase& rhs) const { return !(Derived() == rhs.Derived()); }

	IteratorBase& operator++(int) {
		auto temp = *this;
		++Derived();
		return temp;
	}

	using difference_type = ptrdiff_t;
	using value_type = ValueType;
	using pointer = value_type*;
	using reference = value_type&;
	using iterator_category = CategoryType;

private:
	auto& Derived() { return static_cast<DerivedType&>(*this); }
	auto& Derived() const { return static_cast<const DerivedType&>(*this); }
};