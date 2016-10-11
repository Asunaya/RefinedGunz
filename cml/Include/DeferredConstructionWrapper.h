#pragma once

template <typename T>
class DeferredConstructionWrapper
{
public:
	~DeferredConstructionWrapper()
	{
		if (Constructed)
			Get().~T();
	}

	template <typename... Ts>
	void Construct(Ts&&... Args)
	{
		new (buf) T{std::forward<Ts>(Args)...};
		Constructed = true;
	}

	T& Get()
	{
		return reinterpret_cast<T&>(*buf);
	}
	
private:
	alignas(T) char buf[sizeof(T)];
	bool Constructed{};
};