#pragma once

#include <type_traits>

template <size_t wanted_index, size_t cur_index, typename T, typename... rest>
struct get_template_argument_impl
{
	using type = std::conditional_t<wanted_index == cur_index,
		T,
		typename get_template_argument_impl<wanted_index, cur_index + 1, rest...>::type>;
};

template <size_t wanted_index, size_t cur_index, typename T>
struct get_template_argument_impl<wanted_index, cur_index, T>
{
	using type = T;
	static_assert(wanted_index == cur_index, "Template does not have that many arguments");
};

template <typename T, size_t index>
struct get_template_argument;

template <template <typename...> class template_, typename... arguments, size_t index>
struct get_template_argument<template_<arguments...>, index>
{
	using type = typename get_template_argument_impl<index, 0, arguments...>::type;
};

template <typename T, size_t index>
using get_template_argument_t = typename get_template_argument<T, index>::type;