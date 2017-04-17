#pragma once

#include <type_traits>
#include <limits>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <algorithm>
#include "GlobalTypes.h"

// Use the lowest of u8, u16, and u32 that support the size
template <typename T, size_t N>
using RingBufferCursorType =
std::conditional_t<N <= (std::numeric_limits<u8>::max)() + 1u, u8,
	std::conditional_t<N <= (std::numeric_limits<u16>::max)() + 1u, u16,
	u32>>;

template <typename T, size_t N>
struct RingBuffer;

template <typename T, size_t N>
struct RingIterator
{
	RingBuffer<T, N>& Ring;
	size_t Offset;

	bool operator==(const RingIterator& rhs) const { return Offset == rhs.Offset; }
	bool operator!=(const RingIterator& rhs) const { return !(*this == rhs); }

	T& operator*();
	const T& operator*() const;

	RingIterator& operator++() {
		++Offset;
		return *this;
	}

	RingIterator& operator++(int) {
		auto temp = *this;
		++*this;
		return temp;
	}
};

template <typename T, size_t N>
struct RingBuffer
{
	RingBuffer() = default;
	~RingBuffer()
	{
		for (auto&& x : *this)
			x.~T();
	}

	using CursorType = RingBufferCursorType<T, N>;

	T& operator[](size_t i) { return *get(i); }
	const T& operator[](size_t i) const { return *get(i); }

	template <typename... ArgsType>
	void emplace_front(ArgsType&&... Args) {
		Cursor = (Cursor - 1) % N;
		new (get(0)) T{ std::forward<ArgsType>(Args)... };
		Size = (std::min)(Size + 1, N);
	}

	template <typename... ArgsType>
	void emplace_back(ArgsType&&... Args) {
		new (get(0)) T{ std::forward<ArgsType>(Args)... };
		Cursor = (Cursor + 1) % N;
		Size = (std::min)(Size + 1, N);
	}

	auto begin() { return RingIterator<T, N>{ *this, 0 }; }
	auto end() { return RingIterator<T, N>{ *this, Size }; }
	auto begin() const { return RingIterator<T, N>{ *this, 0 }; }
	auto end() const { return RingIterator<T, N>{ *this, Size }; }

	size_t size() const { return Size; }

private:
	const T* get(size_t i) const {
		i = (Cursor + i) % N;
		return reinterpret_cast<const T*>(buf + sizeof(T) * i);
	}

	T* get(size_t i) {
		auto* ptr = static_cast<const RingBuffer*>(this)->get(i);
		return const_cast<T*>(ptr);
	}

	alignas(T) char buf[sizeof(T) * N];
	CursorType Cursor{};
	size_t Size{};
};

template <typename T, size_t N>
T& RingIterator<T, N>::operator*() { return Ring[Offset]; }
template <typename T, size_t N>
const T& RingIterator<T, N>::operator*() const { return Ring[Offset]; }