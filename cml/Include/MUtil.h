#pragma once

#include <string>
#include <memory>
#include "TMP.h"

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

template <typename T>
class D3DPtr;

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

inline uint32_t ARGB(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
	return (a << 24) | (r << 16) | (g << 8) | b;
}

inline uint32_t RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return ARGB(a, r, g, b);
}

inline uint32_t XRGB(uint8_t r, uint8_t g, uint8_t b)
{
	return ARGB(0xFF, r, g, b);
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
	union
	{
		T1 T1_rep;
		T2 T2_rep;
	} conversion_union;
	conversion_union.T2_rep = val;
	return conversion_union.T1_rep;
}

template <typename T1>
T1 reinterpret(void* val)
{
	T1 x;
	memcpy(&x, val, sizeof(T1));
	return x;
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

template <typename ContainerT>
class PairValueAdapter
{
public:
	PairValueAdapter(ContainerT& o) : Container(o) { }

	auto begin() {
		return ValueIterator<typename ContainerT::iterator>(Container.begin()); }
	auto end() {
		return ValueIterator<typename ContainerT::iterator>(Container.end()); }
	auto begin() const {
		return ValueIterator<typename ContainerT::iterator>(Container.begin()); } 
	auto end() const {
		return ValueIterator<typename ContainerT::iterator>(Container.end()); }

private:
	ContainerT& Container;
};

template <typename T>
auto MakePairValueAdapter(T& Container) { return PairValueAdapter<T>(Container); }

template <typename T, size_t size>
inline constexpr size_t ArraySize(T(&)[size]) { return size; }

inline std::pair<bool, int> StringToInt(const char* String, int Radix = 10)
{
	char *endptr = nullptr;

	int IntVal = strtol(String, &endptr, Radix);

	if (endptr != String + strlen(String))
		return{ false, -1 };

	return{ true, IntVal };
}

template <typename T>
class D3DPtr
{
public:
	D3DPtr() {}
	D3DPtr(T* p) : ptr(p) {}
	D3DPtr(D3DPtr&& src) { Move(std::move(src)); }

	D3DPtr<T>& operator=(const D3DPtr&) = delete;
	D3DPtr<T>& operator=(D3DPtr&& src)
	{
		Move(std::move(src));
		return *this;
	}
	D3DPtr<T>& operator=(nullptr_t)
	{
		Release();
		ptr = nullptr;
		return *this;
	}

	~D3DPtr() { Release(); }

	operator T*() const { return ptr; }
	T* operator ->() const { return ptr; }

	auto* get() { return ptr; }
	auto* get() const { return ptr; }

private:
	void Move(D3DPtr&& src)
	{
		Release();
		ptr = src.ptr;
		src.ptr = nullptr;
	}

	void Release()
	{
		if (ptr)
			ptr->Release();
	}

	T* ptr{};
};

template <typename T>
class WriteProxy
{
	using StoredType = get_template_argument_t<T, 0>;
public:
	WriteProxy(T& ptr) : ptr(ptr), temp(ptr.get()) {}
	~WriteProxy() { ptr = temp; }

	operator StoredType**() { return &temp; }

private:
	T& ptr;
	StoredType* temp;
};

template <typename T>
auto MakeWriteProxy(D3DPtr<T>& ptr) { return WriteProxy<D3DPtr<T>>{ptr}; }

template <typename... T>
auto MakeWriteProxy(std::unique_ptr<T...>& ptr) { return WriteProxy<std::unique_ptr<T...>>(ptr); }