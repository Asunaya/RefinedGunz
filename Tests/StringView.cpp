#include <string>
#include <cstring>
#include "StringView.h"
#include "TestAssert.h"

void TestStringView()
{
	std::string str = "Hello world!";
	auto wsstr = "    " + str + "  ";
	StringView a = wsstr;
	auto b = trim(a);
	TestAssert(b == "Hello world!");
	TestAssert(b == StringView("Hello world!"));
	TestAssert(b.front() == 'H' && b.back() == '!');
	for (size_t i = 0; i < str.size(); ++i)
		TestAssert(str[i] == b[i]);
	TestAssert(starts_with(b, "Hello") && ends_with(b, "world!"));
	TestAssert(b.find("ld") == 9);
	TestAssert(ifind(b, "Ld") == 9);
	TestAssert(ifind(a, b) == 4);
	TestAssert(icontains(b, "Ld"));
	TestAssert(icontains(a, b));
	TestAssert(iequals(b, "HeLlO wOrLd!"));
	TestAssert(b.find_first_of("o") == 4);
	TestAssert(b.find_first_of('o') == 4);
	TestAssert(b.find_last_of("roqw") == 8);
	TestAssert(b.find_first_not_of("lelHw o") == 8);
	TestAssert(b.find_last_not_of("abocdefrld!") == 6);
	TestAssert(b.find_first_of("dw", 8) == 10);
	TestAssert(b.find_last_of("do", 8) == 7);
	TestAssert(b.find_last_of('o', 8) == 7);
	TestAssert(b.find_first_not_of("rw", 8) == 9);
	TestAssert(b.find_last_not_of("og", 8) == 6);
	auto c = b, d = b;
	c.remove_suffix(7);
	TestAssert(c == "Hello");
	d.remove_prefix(6);
	TestAssert(d == "world!");
	TestAssert(b.substr(0, 5) == c && b.substr(6) == d);
}