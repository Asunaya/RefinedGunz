#ifndef MUID_H
#define MUID_H

#include <map>
#include <vector>
#include <crtdbg.h>

using namespace std;

/// MAIET Unique ID
struct MUID{
	unsigned long int	High;	///< High 4 Byte
	unsigned long int	Low;	///< High 4 Byte

	MUID(void) { SetZero(); }
	MUID(unsigned long int h, unsigned long int l){
		High = h;
		Low = l;
	}

	/// MUID 0으로 초기화
	void SetZero(void){
		High = Low = 0;
	}
	void SetInvalid(void){
		SetZero();
	}

	/*
	/// MUID 1만큼 증가
	void Increase(void){
		if(Low==UINT_MAX){
			_ASSERT(High<UINT_MAX);
			High++;
			Low = 0;
		}
		else{
			Low++;
		}
	}
	/*/

	/// MUID를 nSize만큼 증가
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

	/// MUID가 유효한지 검사, 0:0이면 Invalid
	bool IsInvalid(void){
		// 0이면 Invalid
		if(High==Low && Low==0) return true;
		return false;
	}
	/// MUID가 유효한지 검사, 0:0이면 Invalid
	bool IsValid(void){
		// 0이면 Invalid
		if(High==Low && Low==0) return false;
		return true;
	}

	/// 크기 비교를 위한 오퍼레이터 오버로딩
	inline friend bool operator > (const MUID& a, const MUID& b){
		if(a.High>b.High) return true;
		if(a.High==b.High){
			if(a.Low>b.Low) return true;
		}
		return false;
	}
	/// 크기 비교를 위한 오퍼레이터 오버로딩
	inline friend bool operator >= (const MUID& a, const MUID& b){
		if(a.High>b.High) return true;
		if(a.High==b.High){
			if(a.Low>=b.Low) return true;
		}
		return false;
	}
	/// 크기 비교를 위한 오퍼레이터 오버로딩
	inline friend bool operator < (const MUID& a, const MUID& b){
		if(a.High<b.High) return true;
		if(a.High==b.High){
			if(a.Low<b.Low) return true;
		}
		return false;
	}
	/// 크기 비교를 위한 오퍼레이터 오버로딩
	inline friend bool operator <= (const MUID& a, const MUID& b){
		if(a.High<b.High) return true;
		if(a.High==b.High){
			if(a.Low<=b.Low) return true;
		}
		return false;
	}

	/// 대입 오퍼레이터 오버로딩
	inline MUID& operator=(int v){
		High = 0;
		Low = v;
		return *this;
	}
	/// 대입 오퍼레이터 오버로딩
	inline MUID& operator=(const MUID& a){
		High = a.High;
		Low = a.Low;
		return *this;
	}
	/// 비교 오퍼레이터 오버로딩
	inline friend bool operator==(const MUID& a, const MUID& b){
		if(a.High==b.High){
			if(a.Low==b.Low) return true;
		}
		return false;
	}
	/// 비교 오퍼레이터 오버로딩
	inline friend bool operator!=(const MUID& a, const MUID& b){
		if(a.Low!=b.Low) return true;
		if(a.High!=b.High) return true;
		return false;
	}
	/// 가산 오퍼레이터 오버로딩
	inline friend MUID& operator++(MUID& a){
		a.Increase();
		return a;
	}

	/// Invalid한 UID 얻기
	static MUID Invalid(void);
};

/// MUID 영역
struct MUIDRANGE{
	MUID	Start;
	MUID	End;

	bool IsEmpty(void){
		return (Start==End);
	}
	void Empty(void){
		SetZero();
	}
	void SetZero(void){
		Start.SetZero();
		End.SetZero();
	}
};

/// int 두개값으로 MUID 생성
#define MAKEMUID(_high, _low)	MUID(_high, _low)


/// MUID Reference Map
/// - 순차적으로 1씩 증가하는 ID를 얻어낼 수 있는 MUID 집합
class MUIDRefMap : protected map<MUID, void*>{
	MUID	m_CurrentMUID;		///< 현재 발급된 MUID
public:
	MUIDRefMap(void);
	virtual ~MUIDRefMap(void);

	/// 레퍼런스를 MUID하나에 할당한다.
	/// @param pRef	레퍼런스 포인터
	/// @return		할당된 MUID
	MUID Generate(void* pRef);

	/// MUID를 통해 레퍼런스 포인터를 얻어낸다.
	/// @param uid	MUID
	/// @return		레퍼런스 포인터, MUID가 존재하지 않으면 NULL을 리턴
	void* GetRef(MUID& uid);

	/// 등록된 MUID 삭제.
	/// @param uid	MUID
	/// @return		등록되었던 레퍼런스 포인터, MUID가 존재하지 않으면 NULL을 리턴
	void* Remove(MUID& uid);

	/// nSize만큼 확보
	MUIDRANGE Reserve(int nSize);

	/// 지금까지 발급된 UID 구간
	MUIDRANGE GetReservedCount(void);
};

/*
/// MUID Reference Array
/// - 비교적 작은 용량의 MUID를 생성해내기 위한 MUID 집합
class MUIDRefArray : protected vector<void*>{
	MUID	m_CurrentMUID;		///< 현재 MUID
public:
	MUIDRefArray(void);
	virtual ~MUIDRefArray(void);

	/// 레퍼런스를 MUID하나에 할당한다.
	/// @param pRef	레퍼런스 포인터
	/// @return		할당된 MUID
	MUID Generate(void* pRef);
	/// MUID를 통해 레퍼런스 포인터를 얻어낸다.
	/// @param uid	MUID
	/// @return		레퍼런스 포인터, MUID가 존재하지 않으면 NULL을 리턴
	void* GetRef(MUID& uid);
};
*/

/// MUID Reference Map Cache
/// - 전체 UID의 검색 부하를 줄이기 위한 캐쉬 클래스
template <typename T>
class MUIDRefCache : public map<MUID, T*>{
public:
	/// 레퍼런스를 MUID하나에 할당한다.
	/// @param pRef	레퍼런스 포인터
	/// @return		할당된 MUID
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
	/// MUID를 통해 레퍼런스 포인터를 얻어낸다.
	/// @param uid	MUID
	/// @return		레퍼런스 포인터, MUID가 존재하지 않으면 NULL을 리턴
	T* GetRef(const MUID& uid)
	{
		iterator i = find(uid);
		if (i == end()) return nullptr;
		return i->second;
	}
	/// 등록된 MUID 삭제.
	/// @param uid	MUID
	/// @return		등록되었던 레퍼런스 포인터, MUID가 존재하지 않으면 NULL을 리턴
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

#endif