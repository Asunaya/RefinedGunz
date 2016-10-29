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
		assert(!Constructed);
		new (buf) T{std::forward<Ts>(Args)...};
		Constructed = true;
	}

	void Destroy()
	{
		assert(Constructed);
		Get().~T();
		Constructed = false;
	}

	T& Get() { return reinterpret_cast<T&>(*buf); }
	const T& Get() const { return reinterpret_cast<const T&>(*buf); }
	bool IsConstructed() const { return Constructed; }
	
private:
	alignas(T) char buf[sizeof(T)];
	bool Constructed{};
};