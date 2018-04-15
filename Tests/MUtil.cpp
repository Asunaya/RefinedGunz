#include "TestAssert.h"
#include "MUtil.h"
#include "SafeString.h"
#include <climits>
#include <string>
#include <algorithm>

template <typename T, typename N>
bool WithinRange(N n)
{
	return mixed_sign::gt_eq(n, std::numeric_limits<T>::lowest()) &&
		mixed_sign::lt_eq(n, std::numeric_limits<T>::max());
}

template <int Radix, typename... Ts>
struct TestForTypes
{
	template <typename T, typename N>
	static void For1(N Num)
	{
		bool ShouldSucceed = WithinRange<T>(Num);
		char NumString[256];
		itoa_safe(Num, NumString, Radix == 0 ? 10 : Radix);
		auto ConvertedNum = StringToInt<T, Radix>(NumString);
		TestAssert(ConvertedNum.has_value() == ShouldSucceed);
		if (ConvertedNum.has_value())
		{
			TestAssert(ConvertedNum.value() == Num);
		}
		if (Radix == 8 || Radix == 16)
		{
			bool Negative = Num < 0;
			auto Src = NumString + Negative;
			auto Sign = Negative ? "-" : "";
			auto Prefix = Radix == 8 ? "0o" : "0x";
			char NumString2[256];
			sprintf_safe(NumString2, "%s%s%s", Sign, Prefix, Src);
			ConvertedNum = StringToInt<T, Radix>(NumString2);
			TestAssert(ConvertedNum.has_value() == ShouldSucceed);
			if (ConvertedNum.has_value())
			{
				TestAssert(ConvertedNum.value() == Num);
			}
		}
	}

	template <typename T>
	static void f(T Num)
	{
		auto Zeroes = {(For1<Ts>(Num), 0)...};
		(void)Zeroes;
	}
};

template <int Radix, typename T>
void TestStringToInt(T Num)
{
	TestForTypes<Radix, i8, u8, i16, u16, i32, u32, i64, u64>::f(Num);
}

template <int Radix, typename... Args>
void TestAllForRadix(Args... args)
{
	auto Zeroes = {(TestStringToInt<Radix>(args), 0)...};
	(void)Zeroes;
}

template <typename... Args>
void TestAll(Args... args)
{
	TestAllForRadix<0>(args...);
	TestAllForRadix<8>(args...);
	TestAllForRadix<10>(args...);
	TestAllForRadix<16>(args...);
	TestAllForRadix<32>(args...);
}

void TestMUtil()
{
	TestAssert(mixed_sign::lt(-2, 4u));

	TestAll(-1, 0, 1, 2,
		SHRT_MIN, SHRT_MAX, USHRT_MAX,
		INT_MIN, INT_MAX, UINT_MAX, 2147483648, -2147483649ll,
		LONG_MIN, LONG_MAX, LONG_MAX,
		LLONG_MIN, LLONG_MAX, ULLONG_MAX);

	auto x = StringToInt<i32>("2147483648");
	TestAssert(!x);
	x = StringToInt<i32, 0, true>("2147483648");
	TestAssert(x && *x == INT_MIN);
	x = StringToInt<i32, 0, true>("-2147483649");
	TestAssert(x && *x == INT_MAX);
	x = StringToInt("  42   ");
	TestAssert(x && *x == 42);
}