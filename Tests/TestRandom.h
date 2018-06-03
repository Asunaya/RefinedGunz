static std::mt19937 rng;

inline int RandomNumber(int Min, int InclusiveMax) {
	return std::uniform_int_distribution<>{Min, InclusiveMax}(rng);
}

inline int RandomNumber(int InclusiveMax) {
	return RandomNumber(0, InclusiveMax);
};

inline void RandomString(ArrayView<char> String)
{
	for (size_t i = 0; i < String.size() - 1; ++i)
	{
		String[i] = RandomNumber(25) + (RandomNumber(1) ? 'A' : 'a');
	}
	String.back() = 0;
}

template <typename T>
auto RandomIterator(T&& x)
{
	return x.begin() + RandomNumber(int(x.size() - 1));
}

template <typename T>
auto& RandomElement(T&& x)
{
	return *RandomIterator(x);
}

template <typename T>
void FillRandom(T& c)
{
	for (u8& byte : c)
		byte = RandomNumber(UCHAR_MAX);
}