#pragma once

template <typename T>
struct defer {
	defer(T& fn) : fn(fn) {}
	~defer() { if (armed) execute(); }
	defer(defer&& src) : fn{ std::move(src.fn) }, armed{ src.armed } { src.disarm(); }

	void disarm() { armed = false; }
	void execute() { fn(); }

private:
	T fn;
	bool armed = true;
};

template <typename T>
auto make_defer(T& fn) {
	return defer<T>{fn};
}

#define TOKENIZE_IMPL(a, b) a##b
#define TOKENIZE(a, b) TOKENIZE_IMPL(a, b)
#define DEFER(block) const auto TOKENIZE(defer_instance, __COUNTER__){make_defer([&](){ block })};
