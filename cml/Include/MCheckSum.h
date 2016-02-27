#ifndef MCHECKSUM_H
#define MCHECKSUM_H

#include <list>
#include <crtdbg.h>
//#include "RSDebug.h"

using namespace std;


// _CHECKSUM_LEVEL==0				:	Not Log CheckSum
// _CHECKSUM_LEVEL==1				:	Log CheckSum, But Not Log Sub CheckSum
// _CHECKSUM_LEVEL==2 or Default	:	Log All CheckSum


enum MCheckSumType{
	MCST_INT = 0,
	MCST_FLOAT = 1,
};

struct MSUBCHECKSUM{
	char			szName[256];
	int				nCheckSum;
	MCheckSumType	nType;
};

// 4Byte Value Convert
#ifndef _DEBUG
#define _INTVALUE(_n)		(*((int*)(&_n)))
#else
template <class T> int _INTVALUE(T& n){ _ASSERT(sizeof(n)==4); return(*((int*)(&n))); }
#endif


class MCheckSum : public list<MSUBCHECKSUM*>{
public:
	int		m_nCurCheckSum;		// ÇöÀç Ã½¼¶°ª
public:
	MCheckSum(void);
	virtual ~MCheckSum(void);
	void Initialize(void);
};

// ÃÑ ¹é¾÷µÇ´Â Ã½¼¶ °¹¼ö ¼³Á¤
void SetCheckSumBackUpCount(int i);
int GetCheckSumBackUpCount(void);

// CheckSum Enable Setting
void EnableCheckSum(bool bEnable);
bool IsEnableCheckSum(void);

class MCheckSums{
	MCheckSum*			m_pCurCheckSum;		// ÇöÀç Ã½¼¶
	list<MCheckSum*>	m_CheckSumBackUps;	// Ã½¼¶ ¹é¾÷

public:
	MCheckSums(void);
	virtual ~MCheckSums(void);

	void ClearCheckSum(void);
	void InitCheckSum(void);
	void AddCheckSum(int nCheckSum, char* szName, MCheckSumType t);
	int GetCheckSum(void);
	void LogSubCheckSum(const char* szCheckSumName, MCheckSum* pCheckSum, int nCheckSum);
	void LogSubCheckSum(const char* szCheckSumName);
};

// Example)
//	class A{
//		DECLARE_CHECKSUM(A)
//	};

#if (!defined(_CHECKSUM_LEVEL) || (_CHECKSUM_LEVEL==2))

	// CheckSum Level 2
	/////////////////////////////////////////////////////////////////////////////////////
	//#pragma message( "CheckSum Level 2" )

	#define DECLARE_CHECKSUM(_CheckSumName)												\
	protected:																			\
		MCheckSums			m_CheckSums;												\
																						\
		void InitCheckSum(void){														\
			if(IsEnableCheckSum()==false) return;										\
			m_CheckSums.InitCheckSum();													\
		}																				\
		void AddCheckSum(int nCheckSum, char* szName, MCheckSumType t){					\
			if(IsEnableCheckSum()==false) return;										\
			m_CheckSums.AddCheckSum(nCheckSum, szName, t);								\
		}																				\
		void AddCheckSum(int nCheckSum, char* szName=NULL){								\
			if(IsEnableCheckSum()==false) return;										\
			AddCheckSum(nCheckSum, szName, MCST_INT);									\
		}																				\
		void AddCheckSum(float fCheckSum, char* szName=NULL){							\
			if(IsEnableCheckSum()==false) return;										\
			AddCheckSum(_INTVALUE(fCheckSum), szName, MCST_FLOAT);						\
		}																				\
	public:																				\
		int GetCheckSum(void){															\
			if(IsEnableCheckSum()==false) return 0;										\
			return m_CheckSums.GetCheckSum();											\
		}																				\
		void LogSubCheckSum(void){														\
			if(IsEnableCheckSum()==false) return;										\
			m_CheckSums.LogSubCheckSum(#_CheckSumName);									\
		}																				\



	#define DECLARE_LOGCHECKSUM()														\
	public:																				\
		virtual void LogCheckSum(void){ LogSubCheckSum(); }								\


	#define _INITCHECKSUM()				InitCheckSum()
	#define _ADDCHECKSUM(_nCheckSum)	AddCheckSum((_nCheckSum), #_nCheckSum)


#elif (_CHECKSUM_LEVEL == 1)

	// CheckSum Level 1
	/////////////////////////////////////////////////////////////////////////////////////
	//#pragma message( "CheckSum Level 1" )

	#define DECLARE_CHECKSUM(_CheckSumName)												\
	protected:																			\
		int		m_nCheckSum;															\
		void InitCheckSum(void){														\
			m_nCheckSum = 0;															\
		}																				\
		void AddCheckSum(int nCheckSum){												\
			if(IsEnableCheckSum()==false) return;										\
			nCheckSum += nCheckSum;														\
		}																				\
		void AddCheckSum(float fCheckSum){												\
			if(IsEnableCheckSum()==false) return;										\
			AddCheckSum(_INTVALUE(fCheckSum));											\
		}																				\
	public:																				\
		int GetCheckSum(void){															\
			if(IsEnableCheckSum()==false) return 0;										\
			return m_nCheckSum;															\
		}																				\
		void LogSubCheckSum(void){														\
			if(IsEnableCheckSum()==false) return;										\
			rslog(#_CheckSumName" = %d\n", m_nCheckSum);								\
		}																				\


	#define DECLARE_LOGCHECKSUM()														\
	public:																				\
		virtual void LogCheckSum(void){ LogSubCheckSum(); }								\


	#define _INITCHECKSUM()	InitCheckSum()
	#define _ADDCHECKSUM(_nCheckSum)	AddCheckSum((_nCheckSum), #_nCheckSum)

#else

	// CheckSum Level 0
	/////////////////////////////////////////////////////////////////////////////////////
	//#pragma message( "CheckSum Level 0" )

	#define DECLARE_CHECKSUM(_CheckSumName)												\
	public:																				\
		int GetCheckSum(void){ return 0; }												\
		void LogSubCheckSum(void){}														

	#define DECLARE_LOGCHECKSUM()														\
	public:																				\
		virtual void LogCheckSum(void){}

	#define _INITCHECKSUM()							;
	#define _ADDCHECKSUM(_nCheckSum)				;

#endif



#endif
