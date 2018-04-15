#include "TestAssert.h"
#include "SafeString.h"
#include "StringView.h"

template <typename T>
static bool CheckRet(T* Str, int Ret)
{
	return Ret == strlen_generic(Str);
}

template <typename T>
static bool CheckRet(T* Str, T* Ret)
{
	return Ret == Str + strlen_generic(Str);
}

static bool Overflowed = false;

auto Check = [](auto&& a, auto&& b, auto&& c, auto&& d) {
	TestAssert(equals(a, b));
	TestAssert(CheckRet(a, c));
	TestAssert(Overflowed == d);
	Overflowed = false;
};

void TestSafeString()
{
	char Str[128];

	SafeStringOnOverflowFunc = [] { Overflowed = true; };

	{
		auto End = strcpy_literal(Str, "Hello world!");
		Check(Str, "Hello world!", End, false);
	}

	{
		auto End = strcpy_trunc(Str, "Hello world!");
		Check(Str, "Hello world!", End, false);
	}

	{
		auto End = strcpy_safe(Str, "Hello world!");
		Check(Str, "Hello world!", End, false);
	}

	{
		auto End = strncpy_safe(Str, "Hello world!", 11);
		Check(Str, "Hello world", End, false);
	}

	{
		strcpy_safe(Str, "Hello ");
		auto End = strcat_safe(Str, "world!");
		Check(Str, "Hello world!", End, false);
	}

	{
		strcpy_safe(Str, "Hello ");
		auto End = strncat_safe(Str, "world!", 5);
		Check(Str, "Hello world", End, false);
	}

	{
		auto Sz = sprintf_safe(Str, "%s %d", "Hello world!", 123);
		Check(Str, "Hello world! 123", Sz, false);
	}

	char SmallStr[16];
	StringView Sixteen = "0123456789123456";

	{
		auto End = strcpy_trunc(SmallStr, Sixteen);
		Check(SmallStr, Sixteen.substr(0, 15), End, false);
	}

	{
		auto End = strcpy_safe(SmallStr, Sixteen);
		Check(SmallStr, Sixteen.substr(0, 15), End, true);
	}

	{
		auto End = strncpy_safe(SmallStr, Sixteen, 15);
		Check(SmallStr, Sixteen.substr(0, 15), End, false);
	}

	{
		auto End = strncpy_safe(SmallStr, Sixteen, 16);
		Check(SmallStr, Sixteen.substr(0, 15), End, true);
	}

	{
		strcpy_trunc(SmallStr, Sixteen);
		SmallStr[14] = 0;
		auto End = strcat_safe(SmallStr, "abcd");
		Check(SmallStr, "01234567891234a", End, true);
	}

	{
		strcpy_trunc(SmallStr, Sixteen);
		SmallStr[14] = 0;
		auto End = strncat_safe(SmallStr, "abcd", 3);
		Check(SmallStr, "01234567891234a", End, true);
	}

	{
		auto Sz = sprintf_safe(SmallStr, "%s %d", "Hello world!", 123);
		Check(SmallStr, "Hello world! 12", Sz, true);
	}
}