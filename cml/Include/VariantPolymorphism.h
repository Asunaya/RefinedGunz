#pragma once

#define POLYVAR_METHOD(func) template <typename... Ts> \
auto func(Ts&&... args) { visit(var, [&](auto& x) { x.func(std::forward<Ts>(args)...); }); } \
template <typename T, typename... Ts> \
auto func##Static(Ts&&... args) { var.get_ref<T>().func(std::forward<Ts>(args)...); }

struct D3D9Tag {};
struct VulkanTag {};