#pragma once

template <typename T>
class ArrayViewIterator : public std::iterator<random_access_iterator_tag, T>
{
public:
	ArrayViewIterator(T* p) : ptr(p)
	{
	}

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
	ArrayView(T* p, size_t s) : ptr(p), Size(s) { }

	template <size_t arr_size>
	ArrayView(T(&arr)[arr_size]) : ptr(arr), Size(arr_size) { }

	T& operator[](size_t Index) const
	{
		return ptr[Index];
	}

	size_t size() const
	{
		return Size;
	}

	operator ArrayView<const T>&()
	{
		return *(ArrayView<const T>*)this;
	}

	auto begin() const
	{
		return ArrayViewIterator<T>(ptr);
	}

	auto end() const
	{
		return ArrayViewIterator<T>(ptr + Size);
	}

private:
	T* ptr;
	size_t Size;
};
