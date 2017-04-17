#pragma once

struct in_place_t {};

template <typename T>
class optional
{
public:
	optional() = default;

	template <typename... ArgsType>
	optional(in_place_t, ArgsType&&... Args) {
		emplace(std::forward<ArgsType>(Args)...);
	}

	~optional()
	{
		if (has_value())
			reset();
	}

	template <typename... ArgsType>
	T& emplace(ArgsType&&... Args)
	{
		assert(!Constructed);
		new (buf) T{std::forward<ArgsType>(Args)...};
		Constructed = true;
		return value();
	}

	void reset()
	{
		assert(Constructed);
		value().~T();
		Constructed = false;
	}

	T& value() & { return reinterpret_cast<T&>(*buf); }
	const T& value() const & { return reinterpret_cast<const T&>(*buf); }
	T&& value() && { return reinterpret_cast<T&>(*buf); }
	const T& value() const && { return reinterpret_cast<const T&>(*buf); }

	bool has_value() const { return Constructed; }
	
private:
	alignas(T) char buf[sizeof(T)];
	bool Constructed{};
};