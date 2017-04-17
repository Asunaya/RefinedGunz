#pragma once

#include <map>
#include <vector>
#include "GlobalTypes.h"

/// MAIET Unique ID
struct MUID{
	u32	High{};
	u32	Low{};

	MUID() = default;
	MUID(unsigned long int h, unsigned long int l){
		High = h;
		Low = l;
	}

	void SetZero(){
		High = Low = 0;
	}
	void SetInvalid(){
		SetZero();
	}

	MUID Increase(unsigned long int nSize=1){
		if(Low+nSize>UINT_MAX){
			_ASSERT(High<UINT_MAX);
			Low = nSize-(UINT_MAX-Low);
			High++;
		}
		else{
			Low+=nSize;
		}
		return *this;
	}

	bool IsInvalid(){
		if(High==Low && Low==0) return true;
		return false;
	}
	bool IsValid(){
		if(High==Low && Low==0) return false;
		return true;
	}

	inline friend bool operator > (const MUID& a, const MUID& b){
		if(a.High>b.High) return true;
		if(a.High==b.High){
			if(a.Low>b.Low) return true;
		}
		return false;
	}
	inline friend bool operator >= (const MUID& a, const MUID& b){
		if(a.High>b.High) return true;
		if(a.High==b.High){
			if(a.Low>=b.Low) return true;
		}
		return false;
	}
	inline friend bool operator < (const MUID& a, const MUID& b){
		if(a.High<b.High) return true;
		if(a.High==b.High){
			if(a.Low<b.Low) return true;
		}
		return false;
	}
	inline friend bool operator <= (const MUID& a, const MUID& b){
		if(a.High<b.High) return true;
		if(a.High==b.High){
			if(a.Low<=b.Low) return true;
		}
		return false;
	}

	inline MUID& operator=(int v){
		High = 0;
		Low = v;
		return *this;
	}
	inline MUID& operator=(const MUID& a){
		High = a.High;
		Low = a.Low;
		return *this;
	}
	inline friend bool operator==(const MUID& a, const MUID& b){
		if(a.High==b.High){
			if(a.Low==b.Low) return true;
		}
		return false;
	}
	inline friend bool operator!=(const MUID& a, const MUID& b){
		if(a.Low!=b.Low) return true;
		if(a.High!=b.High) return true;
		return false;
	}
	inline friend MUID& operator++(MUID& a){
		a.Increase();
		return a;
	}

	static MUID Invalid();
};

struct MUIDRANGE{
	MUID Start;
	MUID End;

	bool IsEmpty(){
		return Start == End;
	}
	void Empty(){
		SetZero();
	}
	void SetZero(){
		Start.SetZero();
		End.SetZero();
	}
};

#define MAKEMUID(_high, _low)	MUID(_high, _low)

class MUIDRefMap final : protected std::map<MUID, void*> {
	MUID m_CurrentMUID;
public:
	MUIDRefMap();
	~MUIDRefMap();

	MUID Generate(void* pRef);

	void* GetRef(const MUID& uid);

	void* Remove(const MUID& uid);

	MUIDRANGE Reserve(int nSize);

	MUIDRANGE GetReservedCount();
};

template <typename T>
class MUIDRefCache : public std::map<MUID, T*> {
public:
	void Insert(const MUID& uid, T* pRef)
	{
#ifdef _DEBUG
		if (GetRef(uid)) {
			_ASSERT(0);
			OutputDebugString("MUIDRefCache DUPLICATED Data. \n");
		}
#endif
		insert(value_type(uid, pRef));
	}
	T* GetRef(const MUID& uid)
	{
		iterator i = find(uid);
		if (i == end()) return nullptr;
		return i->second;
	}
	T* Remove(const MUID& uid)
	{
		iterator i = find(uid);
		if (i == end()) return nullptr;
		auto pRef = i->second;
		erase(i);
		return pRef;
	}
};

using MMatchObjectMap = MUIDRefCache<class MMatchObject>;

namespace std
{
	template <>
	class hash<MUID> : public hash<uint64_t>
	{
	public:
		size_t operator()(const MUID &UID) const
		{
			static_assert(sizeof(u64) == sizeof(MUID), "MUID is not 64 bits wide");

			u64 uid64;
			memcpy(&uid64, &UID, sizeof(UID));
			return hash<uint64_t>::operator()(uid64);
		}
	};
}
