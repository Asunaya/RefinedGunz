#pragma once

#include <limits.h>
#include "MPacketCrypter.h"
#include "MUID.h"
#include "d3dx9math.h"

inline auto MGetTimeDistance(u64 a, u64 b) { return b - a; }

class MZFileSystem;
unsigned long MGetMemoryChecksum(char *pBuffer, int nLen);
unsigned long MGetMZFileChecksum(const char* pszFileName);
void MMakeSeedKey(MPacketCrypterKey* pKey, const MUID& uidServer, const MUID& uidClient, unsigned int nTimeStamp);

template <typename T>
struct MVectorImpl
{
	T x;
	T y;
	T z;

	MVectorImpl() = default;
	MVectorImpl(T x, T y, T z) : x(x), y(y), z(z) {}
	MVectorImpl(const v3& src) : x(static_cast<T>(src.x)), y(static_cast<T>(src.y)), z(static_cast<T>(src.z)) {}

	MVectorImpl<T>& operator =(const v3& src)
	{
		this->~MVectorImpl();
		new (this) MVectorImpl<T>(src);
		return *this;
	}

	explicit operator v3() const
	{
		return{ static_cast<float>(x), static_cast<float>(y), static_cast<float>(z) };
	}
};

template <typename T>
inline MVectorImpl<T> operator *(const MVectorImpl<T>& lhs, const MVectorImpl<T>& rhs)
{
	return{ lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z };
}

template <typename T>
inline MVectorImpl<T> operator *(const MVectorImpl<T>& vec, float f)
{
	return{ static_cast<T>(vec.x * f), static_cast<T>(vec.y * f), static_cast<T>(vec.z * f) };
}

template <typename T>
inline MVectorImpl<T> operator *(float f, const MVectorImpl<T>& vec)
{
	return vec * f;
}

inline float ShortToDirElement(short x)
{
	return ((1.f / 32000.f) * x);
}

inline short DirElementToShort(float x)
{
	return (short)(32000 * x);
}

using MByteVector = MVectorImpl<i8>;
using MShortVector = MVectorImpl<i16>;

#pragma pack(push, 1)
struct PackedDirection
{
	int8_t Yaw;
	int8_t Pitch;
};
#pragma pack(pop)

PackedDirection PackDirection(const v3& src);

v3 UnpackDirection(const PackedDirection& src);