#pragma once

#include <utility>

template <typename T>
class ArrayView
{
public:
	ArrayView() = default;

	ArrayView(T* p, size_t s) : ptr(p), Size(s) {}

	template <size_t arr_size>
	ArrayView(T(&arr)[arr_size]) : ptr(arr), Size(arr_size) {}

	auto& operator[](size_t Index)       { return ptr[Index]; }
	const auto& operator[](size_t Index) const { return ptr[Index]; }

	size_t size() const { return Size; }

	auto begin() const { return ptr; }
	auto end() const { return ptr + Size; }

	auto* data() { return ptr; }
	const auto* data() const { return ptr; }

private:
	T* ptr{};
	size_t Size{};
};