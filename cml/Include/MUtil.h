#ifndef _MUTIL_H
#define _MUTIL_H


#include <string>
using std::string;


// 10진수값에 대한 집합형
#define SetBitSet(sets, item)		(sets |= (1 << item))
#define ClearBitSet(sets, item)		(sets &= ~(1 << item))
#define CheckBitSet(sets, item)		(sets & (1 << item))

/* 사용법
enum Item
{
	Foo1 = 1,
	Foo2 = 2,
	FooMax
};

unsigned long int sets = 0;
SetBitSet(sets, Foo1);
if (CheckBitSet(sets, Foo1)) extr;

*/


enum MDateType
{
	MDT_Y = 1,
	MDT_YM,
	MDT_YMD,
	MDT_YMDH,
	MDT_YMDHM,
};

const string MGetStrLocalTime( const unsigned short wYear = 0, 
							   const unsigned short wMon = 0, 
							   const unsigned short wDay = 0, 
							   const unsigned short wHour = 0, 
							   const unsigned short wMin  = 0,
							   const MDateType = MDT_YMDHM );

template <typename T1, typename T2>
T1 reinterpret(T2 val)
{
	union
	{
		T1 T1_rep;
		T2 T2_rep;
	} conversion_union;
	conversion_union.T2_rep = val;
	return conversion_union.T1_rep;
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

	ValueIterator& operator++(int)
	{
		auto temp(*this);
		++*this;
		return temp;
	}

	bool operator==(const ValueIterator& rhs) const
	{
		return it == rhs.it;
	}

	bool operator!=(const ValueIterator& rhs) const
	{
		return it == rhs.it;
	}

	auto& operator*()
	{
		return it->second;
	}

private:
	ItT it;
};

template <typename ContainerT>
class PairValueAdapter
{
public:
	PairValueAdapter(ContainerT& o) : Container(o) { }

	auto begin()
	{
		return ValueIterator<ContainerT::iterator>(Container.begin());
	}

	auto end()
	{
		return ValueIterator<ContainerT::iterator>(Container.end());
	}

	auto begin() const
	{
		return ValueIterator<ContainerT::iterator>(Container.begin());
	}

	auto end() const
	{
		return ValueIterator<ContainerT::iterator>(Container.end());
	}

private:
	ContainerT& Container;
};

template <typename T>
auto MakePairValueAdapter(T& Container)
{
	return PairValueAdapter<T>(Container);
}

template <typename T, size_t size>
inline constexpr size_t ArraySize(T(&)[size])
{
	return size;
}



#endif