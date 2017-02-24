#pragma once

#include <utility>

template <typename T>
class ArrayViewIterator : public std::iterator<random_access_iterator_tag, T>
{
public:
	ArrayViewIterator(T* p) : ptr(p) {}

	bool operator !=(const ArrayViewIterator<T>& rhs) const
	{
		return ptr != rhs.ptr;
	}

	auto& operator ++()
	{
		ptr++;
		return *this;
	}

	T& operator*()
	{
		return *ptr;
	}

private:
	T* ptr;
};

template <typename T>
class ArrayView
{
public:
	ArrayView() = default;

	ArrayView(T* p, size_t s) : ptr(p), Size(s) {}

	template <size_t arr_size>
	ArrayView(T(&arr)[arr_size]) : ptr(arr), Size(arr_size) {}

	auto& operator[](size_t Index)       { return ptr[Index]; }
	auto& operator[](size_t Index) const { return ptr[Index]; }

	size_t size() const { return Size; }

	auto begin() const { return ArrayViewIterator<T>(ptr); }
	auto end() const { return ArrayViewIterator<T>(ptr + Size); }

private:
	T* ptr{};
	size_t Size{};
};
