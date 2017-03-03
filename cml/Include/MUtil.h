#pragma once

#include <string>
#include <memory>
#include "TMP.h"
#include "GlobalTypes.h"

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

struct D3DDeleter
{
	void operator()(IUnknown* ptr) const
	{
		ptr->Release();
	}
};

template <typename T>
using D3DPtr = std::unique_ptr<T, D3DDeleter>;

template <typename T>
inline void SafeRelease(D3DPtr<T>& ptr)
{
	ptr = nullptr;
}

#define SAFE_RELEASE(p)      { SafeRelease(p); }

#define EXPAND_VECTOR(v) v[0], v[1], v[2]

#define SetBitSet(sets, item)		(sets |= (1 << item))
#define ClearBitSet(sets, item)		(sets &= ~(1 << item))
#define CheckBitSet(sets, item)		(sets & (1 << item))

#ifdef _MSC_VER
#define WARN_UNUSED_RESULT _Check_return_
#else
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#endif

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

enum MDateType
{
	MDT_Y = 1,
	MDT_YM,
	MDT_YMD,
	MDT_YMDH,
	MDT_YMDHM,
};

const std::string MGetStrLocalTime( const unsigned short wYear = 0, 
							   const unsigned short wMon = 0, 
							   const unsigned short wDay = 0, 
							   const unsigned short wHour = 0, 
							   const unsigned short wMin  = 0,
							   const MDateType = MDT_YMDHM );

template <typename T1, typename T2>
T1 reinterpret(const T2& val)
{
	T1 T1_rep;
	memcpy(&T1_rep, &val, min(sizeof(T1), sizeof(T2)));
	return T1_rep;
}

template <typename T1>
void reinterpret(void*) = delete;

template <typename T>
struct Range
{
	decltype(auto) begin() { return its.first; }
	decltype(auto) end() { return its.second; }
	decltype(auto) begin() const { return its.first; }
	decltype(auto) end() const { return its.second; }

	std::pair<T, T> its;
};

template <typename T>
auto MakeRange(T&& begin, T&& end) { return Range<std::remove_reference_t<T>>{ {begin, end} }; }

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

inline std::pair<bool, int> StringToInt(const char* String, int Radix = 10)
{
	char *endptr = nullptr;

	int IntVal = strtol(String, &endptr, Radix);

	if (endptr != String + strlen(String))
		return{ false, -1 };

	return{ true, IntVal };
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
	using StoredType = get_template_argument_t<T, 0>;
public:
	~WriteProxy() { ptr = T{ temp }; }

	operator StoredType**() && { return &temp; }

private:
	WriteProxy(T& ptr) : ptr(ptr), temp(ptr.get()) {}
	WriteProxy(const WriteProxy&) = delete;
	WriteProxy(WriteProxy&&) = default;
	WriteProxy& operator =(const WriteProxy&) = delete;
	WriteProxy& operator =(WriteProxy&&) = delete;

	template <typename... U>
	friend WriteProxy<std::unique_ptr<U...>> MakeWriteProxy(std::unique_ptr<U...>&);

	T& ptr;
	StoredType* temp{};
};

template <typename... T>
WriteProxy<std::unique_ptr<T...>> MakeWriteProxy(std::unique_ptr<T...>& ptr) {
	return WriteProxy<std::unique_ptr<T...>>(ptr);
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
	Container.erase(std::remove_if(Container.begin(), Container.end(), std::forward<PredicateType>(Predicate)),
		Container.end());
}
