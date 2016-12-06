#pragma once

struct rplane
{
	float a, b, c, d;

	rplane() {}
	rplane(float a, float b, float c, float d) : a{ a }, b{ b }, c{ c }, d{ d } {}

	operator float* () { return reinterpret_cast<float*>(this); }
	operator const float* () const { return reinterpret_cast<const float*>(this); }

	rplane operator +() const { return *this; }
	rplane operator -() const { return{ -a, -b, -c, -d }; }
};