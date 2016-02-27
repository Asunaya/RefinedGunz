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



#endif