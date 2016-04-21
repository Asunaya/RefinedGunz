#pragma once

inline void cprint(...)
{
}

template <typename T, size_t size>
inline constexpr size_t ArraySize(T(&)[size])
{
	return size;
}

static constexpr int RG_SLASH_RADIUS = 320;
static constexpr int RG_MASSIVE_RADIUS = 280;

inline uint32_t ARGB(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
	return (a << 24) | (r << 16) | (g << 8) | b;
}

inline uint32_t XRGB(uint8_t r, uint8_t g, uint8_t b)
{
	return ARGB(0xFF, r, g, b);
}

inline int RelativeWidth(int x)
{
	return float(x) / 1920 * RGetScreenWidth();
}

inline int RelativeHeight(int y)
{
	return float(y) / 1080 * RGetScreenHeight();
}

inline int RELWIDTH(int x) { return RelativeWidth(x); }
inline int RELHEIGHT(int y) { return RelativeHeight(y); }

inline unsigned long long QPC()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return li.QuadPart;
}

inline unsigned long long QPF()
{
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	return li.QuadPart;
}

template <typename T>
inline void SafeDestroy(T& obj)
{
	obj.Destroy();
}

template <typename T>
inline void SafeDestroy(T* obj)
{
	if (obj)
		obj->Destroy();
}

template <typename T>
inline void SAFE_DESTROY(T *obj) { SafeDestroy(obj); }

class EOFException : public std::runtime_error
{
public:
	EOFException(int a) : std::runtime_error("End of file was reached"), Position(a) { }
	int GetPosition() const {
		return Position;
	}
private:
	int Position;
};

namespace std
{
	template <>
	class hash<MUID> : public hash<uint64_t>
	{
	public:
		size_t operator()(const MUID &UID) const
		{
			return hash<uint64_t>::operator()(*(uint64_t *)&UID);
		}
	};
}

template <typename T>
class D3DPtr
{
public:
	T* ptr = nullptr;

	D3DPtr() : ptr(nullptr)
	{
	}

	D3DPtr(T* p) : ptr(p)
	{
	}

	~D3DPtr()
	{
		if (ptr)
			ptr->Release();
	}

	operator T*() const
	{
		return ptr;
	}

	D3DPtr* operator =(T* rhs)
	{
		ptr = rhs;
	}

	T* operator ->()
	{
		return ptr;
	}
};