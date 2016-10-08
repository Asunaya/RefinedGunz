#pragma once

template <typename T>
struct defer {
	defer(T& fn) : fn(fn) {}
	~defer() { execute(); }
	defer(defer&& src) : fn{ std::move(src.fn) } { src.disarm(); }

	void disarm() { fn = [] {}; }
	void execute() { fn(); }

private:
	T fn;
};

template <typename T>
auto make_defer(T& fn) {
	return defer<T>{fn};
}

#define TOKENIZE_IMPL(a, b) a##b
#define TOKENIZE(a, b) TOKENIZE_IMPL(a, b)
#define DEFER(block) const auto TOKENIZE(defer_instance, __COUNTER__){make_defer([&](){ block })};
