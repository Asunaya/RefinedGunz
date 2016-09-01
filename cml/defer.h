#pragma once

template <typename T>
struct defer {
	defer(T& fn) : fn(fn) {}
	~defer() { fn(); }
	defer(defer&& src) { fn = std::move(src.fn); src.fn = []() {}; }

private:
	T& fn;
};

auto make_defer = [&](auto& fn) { return defer<decltype(fn)>{fn}; };
#define TOKENIZE_IMPL(a, b) a##b
#define TOKENIZE(a, b) TOKENIZE_IMPL(a, b)
#define DEFER(block) auto TOKENIZE(defer_instance, __COUNTER__){make_defer([&](){ block })};