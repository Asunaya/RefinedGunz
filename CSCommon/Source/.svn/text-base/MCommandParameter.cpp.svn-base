#include "stdafx.h"
#include "MCommandParameter.h"
#include <string.h>

bool MCommandParamConditionMinMax::Check(MCommandParameter* pCP)
{
	switch (pCP->GetType())
	{
	case MPT_INT:
		{
			int nValue;
			pCP->GetValue(&nValue);
			if ((nValue < m_nMin) || (nValue > m_nMax)) return false;
		}
		break;
	case MPT_UINT:
		{
			unsigned int nValue;
			pCP->GetValue(&nValue);
			if ((nValue < (unsigned int)m_nMin) || (nValue > (unsigned int)m_nMax)) return false;
		}
		break;
	case MPT_CHAR:
		{
			char nValue;
			pCP->GetValue(&nValue);
			if ((nValue < (char)m_nMin) || (nValue > (char)m_nMax)) return false;
		}
		break;
	case MPT_UCHAR:
		{
			unsigned char nValue;
			pCP->GetValue(&nValue);
			if ((nValue < (unsigned char)m_nMin) || (nValue > (unsigned char)m_nMax)) return false;
		}
		break;
	case MPT_SHORT:
		{
			short nValue;
			pCP->GetValue(&nValue);
			if ((nValue < (short)m_nMin) || (nValue > (short)m_nMax)) return false;
		}
		break;
	case MPT_USHORT:
		{
			unsigned short nValue;
			pCP->GetValue(&nValue);
			if ((nValue < (unsigned short)m_nMin) || (nValue > (unsigned short)m_nMax)) return false;
		}
		break;
	default:
		{
			_ASSERT(0); // 없는 제약조건이다.
		}
	};

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
MCommandParameterDesc::MCommandParameterDesc(MCommandParameterType nType, char* szDescription)
{
	m_nType = nType;
	strcpy(m_szDescription, szDescription);
}

void MCommandParameterDesc::InitializeConditions()
{
	for(int i=0; i<(int)m_Conditions.size(); i++){
		delete m_Conditions[i];
	}
	m_Conditions.clear();
}

MCommandParameterDesc::~MCommandParameterDesc(void)
{
	InitializeConditions();
}

void MCommandParameterDesc::AddCondition(MCommandParamCondition* pCondition)
{
	m_Conditions.push_back(pCondition);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
MCommandParameterInt::MCommandParameterInt(void)
 : MCommandParameter(MPT_INT)
{
	m_Value = 0;
}
MCommandParameterInt::MCommandParameterInt(int Value)
 : MCommandParameter(MPT_INT)
{
	m_Value = Value;
}
MCommandParameter* MCommandParameterInt::Clone(void)
{
	return (new MCommandParameterInt(m_Value));
}
void MCommandParameterInt::GetValue(void* p)
{
	*(int*)p = m_Value;
}
int MCommandParameterInt::GetData(char* pData, int nSize)
{
	int nValueSize = sizeof(m_Value);
	if(nValueSize>nSize) return 0;
	memcpy(pData, &m_Value, nValueSize);
	return nValueSize;
}
int MCommandParameterInt::SetData(char* pData)
{
	int nValueSize = sizeof(m_Value);
	memcpy(&m_Value, pData, nValueSize);
	return nValueSize;
}

MCommandParameterUInt::MCommandParameterUInt(void)
: MCommandParameter(MPT_UINT)
{
	m_Value = 0;
}
MCommandParameterUInt::MCommandParameterUInt(unsigned int Value)
: MCommandParameter(MPT_UINT)
{
	m_Value = Value;
}
MCommandParameter* MCommandParameterUInt::Clone(void)
{
	return (new MCommandParameterUInt(m_Value));
}
void MCommandParameterUInt::GetValue(void* p)
{
	*(unsigned int*)p = m_Value;
}
int MCommandParameterUInt::GetData(char* pData, int nSize)
{
	int nValueSize = sizeof(m_Value);
	if(nValueSize>nSize) return 0;
	memcpy(pData, &m_Value, nValueSize);
	return nValueSize;
}
int MCommandParameterUInt::SetData(char* pData)
{
	int nValueSize = sizeof(m_Value);
	memcpy(&m_Value, pData, nValueSize);
	return nValueSize;
}


MCommandParameterFloat::MCommandParameterFloat(void)
 : MCommandParameter(MPT_FLOAT)
{
	m_Value = 0;
}
MCommandParameterFloat::MCommandParameterFloat(float Value)
 : MCommandParameter(MPT_FLOAT)
{
	m_Value = Value;
}
MCommandParameter* MCommandParameterFloat::Clone(void)
{
	return (new MCommandParameterFloat(m_Value));
}
void MCommandParameterFloat::GetValue(void* p)
{
	*(float*)p = m_Value;
}
int MCommandParameterFloat::GetData(char* pData, int nSize)
{
	int nValueSize = sizeof(m_Value);
	if(nValueSize>nSize) return 0;
	memcpy(pData, &m_Value, nValueSize);
	return nValueSize;
}
int MCommandParameterFloat::SetData(char* pData)
{
	int nValueSize = sizeof(m_Value);
	memcpy(&m_Value, pData, nValueSize);
	return nValueSize;
}


MCommandParameterString::MCommandParameterString(void)
 : MCommandParameter(MPT_STR)
{
	m_Value = 0;
}
MCommandParameterString::MCommandParameterString(const char* Value)
 : MCommandParameter(MPT_STR)
{
	int nLen = (int)strlen(Value)+2;

	if (nLen > (USHRT_MAX-2))
	{
		m_Value = 0;
		return;
	}

	m_Value = new char[strlen(Value)+2];
	strcpy(m_Value, Value);
}
MCommandParameterString::~MCommandParameterString(void)
{
	if(m_Value!=NULL){
		delete m_Value;
		m_Value=NULL;
	}
}
MCommandParameter* MCommandParameterString::Clone(void)
{
	return (new MCommandParameterString(m_Value));
}
void MCommandParameterString::GetValue(void* p)
{
	strcpy((char*)p, m_Value);
}
int MCommandParameterString::GetData(char* pData, int nSize)
{
	if(m_Value==NULL) 
	{
		unsigned short nEmptySize = 0;
		memcpy( pData, &nEmptySize, sizeof(nEmptySize) );
		return sizeof(nEmptySize);
	}

	unsigned short nValueSize = (unsigned short)strlen(m_Value)+2;
	if((int)nValueSize+(int)sizeof(nValueSize)>nSize) return 0;

	memcpy(pData, &nValueSize, sizeof(nValueSize));
	memcpy(pData+sizeof(nValueSize), m_Value, nValueSize);

	return nValueSize+sizeof(nValueSize);
}
int MCommandParameterString::SetData(char* pData)
{
	if(m_Value!=NULL) 
	{
		delete[] m_Value;
		m_Value = 0;
	}

	unsigned short nValueSize = 0;
	memcpy(&nValueSize, pData, sizeof(nValueSize));

	if( (nValueSize > (USHRT_MAX-2)) || (0 == nValueSize) )
	{
		ASSERT( 0 && "비정상 길이의 문자." );
		return sizeof(nValueSize);
	}

	m_Value = new char[nValueSize];

	memcpy(m_Value, pData+sizeof(nValueSize), nValueSize);
	return nValueSize+sizeof(nValueSize);
}

int MCommandParameterString::GetSize()
{
	if(m_Value==NULL) return 0;
	return ((int)strlen(m_Value)+2 + sizeof(unsigned short));
}

MCommandParameterVector::MCommandParameterVector(void)
 : MCommandParameter(MPT_VECTOR)
{
	m_fX = m_fY = m_fZ = 0;
}
MCommandParameterVector::MCommandParameterVector(float x ,float y, float z)
 : MCommandParameter(MPT_VECTOR)
{
	m_fX = x;
	m_fY = y;
	m_fZ = z;
}
MCommandParameterVector::~MCommandParameterVector(void)
{
}
MCommandParameter* MCommandParameterVector::Clone(void)
{
	return (new MCommandParameterVector(m_fX, m_fY, m_fZ));
}
void MCommandParameterVector::GetValue(void* p)
{
	((float*)p)[0] = m_fX;
	((float*)p)[1] = m_fY;
	((float*)p)[2] = m_fZ;
}
int MCommandParameterVector::GetData(char* pData, int nSize)
{
	int nValueSize = sizeof(m_fX) * 3;
	if(nValueSize>nSize) return 0;
	float v[3] = {m_fX, m_fY, m_fZ};
	memcpy(pData, v, nValueSize);
	return nValueSize;
}
int MCommandParameterVector::SetData(char* pData)
{
	int nValueSize = sizeof(m_fX) * 3;
	float v[3];
	memcpy(v, pData, nValueSize);
	m_fX = v[0];
	m_fY = v[1];
	m_fZ = v[2];
	return nValueSize;
}

MCommandParameter* MCommandParameterBool::Clone(void)
{
	return (new MCommandParameterBool(m_Value));
}
void MCommandParameterBool::GetValue(void* p)
{
	*(bool*)p = m_Value;
}
int MCommandParameterBool::GetData(char* pData, int nSize)
{
	int nValueSize = sizeof(m_Value);
	if(nValueSize>nSize) return 0;
	memcpy(pData, &m_Value, nValueSize);
	return nValueSize;
}
int MCommandParameterBool::SetData(char* pData)
{
	int nValueSize = sizeof(m_Value);
	memcpy(&m_Value, pData, nValueSize);
	return nValueSize;
}
void *MCommandParameterBool::GetPointer(void)
{
	return &m_Value;
}

MCommandParameterUID::MCommandParameterUID(void)
 : MCommandParameter(MPT_UID)
{
}
MCommandParameterUID::MCommandParameterUID(const MUID& uid)
 : MCommandParameter(MPT_UID)
{
	m_Value = uid;
}
MCommandParameterUID::~MCommandParameterUID(void)
{
}
MCommandParameterUID* MCommandParameterUID::Clone(void)
{
	return (new MCommandParameterUID(m_Value));
}
void MCommandParameterUID::GetValue(void* p)
{
	*(MUID*)p = m_Value;
}
int MCommandParameterUID::GetData(char* pData, int nSize)
{
	int nValueSize = sizeof(m_Value);
	if(nValueSize>nSize) return 0;
	memcpy(pData, &m_Value, nValueSize);
	return nValueSize;
}
int MCommandParameterUID::SetData(char* pData)
{
	int nValueSize = sizeof(m_Value);
	memcpy(&m_Value, pData, nValueSize);
	return nValueSize;
}

MCommandParameterBlob::MCommandParameterBlob(void)
: MCommandParameter(MPT_BLOB)
{
	m_Value = 0;
	m_nSize = 0;
}
MCommandParameterBlob::MCommandParameterBlob(const void* Value, int nSize)
: MCommandParameter(MPT_BLOB)
{
	if (nSize > MAX_BLOB_SIZE)
	{
		m_Value = NULL;
		m_nSize = 0;
		return;
	}

	m_Value = new unsigned char[nSize];
	memcpy(m_Value, Value, nSize);
	m_nSize = nSize;
}
MCommandParameterBlob::~MCommandParameterBlob(void)
{
	if(m_Value!=NULL){
		delete[] m_Value;
		m_Value = NULL;
	}
}

MCommandParameterBlob* MCommandParameterBlob::Clone(void)
{
	return new MCommandParameterBlob(m_Value, m_nSize);
}
void MCommandParameterBlob::GetValue(void* p)
{
	memcpy(p, m_Value, m_nSize);
}
int MCommandParameterBlob::GetData(char* pData, int nSize)
{
	if(m_Value==NULL) return 0;
	if(m_nSize+(int)sizeof(m_nSize)>nSize) return 0;

	memcpy(pData, &m_nSize, sizeof(m_nSize));
	memcpy(pData+sizeof(m_nSize), m_Value, m_nSize);

	return m_nSize+sizeof(m_nSize);
}
int MCommandParameterBlob::SetData(char* pData)
{
	if(m_Value!=NULL) delete[] m_Value;

	memcpy(&m_nSize, pData, sizeof(m_nSize));
	if (m_nSize > MAX_BLOB_SIZE)
	{
		m_Value = NULL;
		m_nSize = 0;
		return sizeof(m_nSize);
	}

	m_Value = new char[m_nSize];

	memcpy(m_Value, pData+sizeof(m_nSize), m_nSize);
	return m_nSize+sizeof(m_nSize);
}

int MCommandParameterBlob::GetSize()
{
	return (m_nSize+sizeof(m_nSize));
}

///////////////////////////////////////////////////////////////////////////////
MCommandParameterChar::MCommandParameterChar(void)
 : MCommandParameter(MPT_CHAR)
{
	m_Value = 0;
}
MCommandParameterChar::MCommandParameterChar(char Value)
 : MCommandParameter(MPT_CHAR)
{
	m_Value = Value;
}

MCommandParameter* MCommandParameterChar::Clone(void)
{
	return (new MCommandParameterChar(m_Value));
}

void MCommandParameterChar::GetValue(void* p)
{
	*(char*)p = m_Value;
}
int MCommandParameterChar::GetData(char* pData, int nSize)
{
	int nValueSize = sizeof(m_Value);
	if(nValueSize>nSize) return 0;
	memcpy(pData, &m_Value, nValueSize);
	return nValueSize;
}
int MCommandParameterChar::SetData(char* pData)
{
	int nValueSize = sizeof(m_Value);
	memcpy(&m_Value, pData, nValueSize);
	return nValueSize;
}

///////////////////////////////////////////////////////////////////////////////
MCommandParameterUChar::MCommandParameterUChar(void)
 : MCommandParameter(MPT_UCHAR)
{
	m_Value = 0;
}
MCommandParameterUChar::MCommandParameterUChar(unsigned char Value)
 : MCommandParameter(MPT_UCHAR)
{
	m_Value = Value;
}

MCommandParameter* MCommandParameterUChar::Clone(void)
{
	return (new MCommandParameterUChar(m_Value));
}

void MCommandParameterUChar::GetValue(void* p)
{
	*(unsigned char*)p = m_Value;
}
int MCommandParameterUChar::GetData(char* pData, int nSize)
{
	int nValueSize = sizeof(m_Value);
	if(nValueSize>nSize) return 0;
	memcpy(pData, &m_Value, nValueSize);
	return nValueSize;
}
int MCommandParameterUChar::SetData(char* pData)
{
	int nValueSize = sizeof(m_Value);
	memcpy(&m_Value, pData, nValueSize);
	return nValueSize;
}

///////////////////////////////////////////////////////////////////////////////
MCommandParameterShort::MCommandParameterShort(void)
 : MCommandParameter(MPT_SHORT)
{
	m_Value = 0;
}
MCommandParameterShort::MCommandParameterShort(short Value)
 : MCommandParameter(MPT_SHORT)
{
	m_Value = Value;
}

MCommandParameter* MCommandParameterShort::Clone(void)
{
	return (new MCommandParameterShort(m_Value));
}

void MCommandParameterShort::GetValue(void* p)
{
	*(short*)p = m_Value;
}
int MCommandParameterShort::GetData(char* pData, int nSize)
{
	int nValueSize = sizeof(m_Value);
	if(nValueSize>nSize) return 0;
	memcpy(pData, &m_Value, nValueSize);
	return nValueSize;
}
int MCommandParameterShort::SetData(char* pData)
{
	int nValueSize = sizeof(m_Value);
	memcpy(&m_Value, pData, nValueSize);
	return nValueSize;
}

///////////////////////////////////////////////////////////////////////////////
MCommandParameterUShort::MCommandParameterUShort(void)
 : MCommandParameter(MPT_USHORT)
{
	m_Value = 0;
}
MCommandParameterUShort::MCommandParameterUShort(unsigned short Value)
 : MCommandParameter(MPT_USHORT)
{
	m_Value = Value;
}

MCommandParameter* MCommandParameterUShort::Clone(void)
{
	return (new MCommandParameterUShort(m_Value));
}

void MCommandParameterUShort::GetValue(void* p)
{
	*(short*)p = m_Value;
}
int MCommandParameterUShort::GetData(char* pData, int nSize)
{
	int nValueSize = sizeof(m_Value);
	if(nValueSize>nSize) return 0;
	memcpy(pData, &m_Value, nValueSize);
	return nValueSize;
}
int MCommandParameterUShort::SetData(char* pData)
{
	int nValueSize = sizeof(m_Value);
	memcpy(&m_Value, pData, nValueSize);
	return nValueSize;
}

///////////////////////////////////////////////////////////////////////////////
MCommandParameterInt64::MCommandParameterInt64(void)
 : MCommandParameter(MPT_INT64)
{
	m_Value = 0;
}
MCommandParameterInt64::MCommandParameterInt64(int64 Value)
 : MCommandParameter(MPT_INT64)
{
	m_Value = Value;
}

MCommandParameter* MCommandParameterInt64::Clone(void)
{
	return (new MCommandParameterInt64(m_Value));
}

void MCommandParameterInt64::GetValue(void* p)
{
	*(int64*)p = m_Value;
}
int MCommandParameterInt64::GetData(char* pData, int nSize)
{
	int nValueSize = sizeof(m_Value);
	if(nValueSize>nSize) return 0;
	memcpy(pData, &m_Value, nValueSize);
	return nValueSize;
}
int MCommandParameterInt64::SetData(char* pData)
{
	int nValueSize = sizeof(m_Value);
	memcpy(&m_Value, pData, nValueSize);
	return nValueSize;
}

///////////////////////////////////////////////////////////////////////////////
MCommandParameterUInt64::MCommandParameterUInt64(void)
 : MCommandParameter(MPT_UINT64)
{
	m_Value = 0;
}
MCommandParameterUInt64::MCommandParameterUInt64(uint64 Value)
 : MCommandParameter(MPT_UINT64)
{
	m_Value = Value;
}

MCommandParameter* MCommandParameterUInt64::Clone(void)
{
	return (new MCommandParameterUInt64(m_Value));
}

void MCommandParameterUInt64::GetValue(void* p)
{
	*(uint64*)p = m_Value;
}
int MCommandParameterUInt64::GetData(char* pData, int nSize)
{
	int nValueSize = sizeof(m_Value);
	if(nValueSize>nSize) return 0;
	memcpy(pData, &m_Value, nValueSize);
	return nValueSize;
}
int MCommandParameterUInt64::SetData(char* pData)
{
	int nValueSize = sizeof(m_Value);
	memcpy(&m_Value, pData, nValueSize);
	return nValueSize;
}



/////////////////////////////////////////////////////////////////////////////////
MCommandParameterShortVector::MCommandParameterShortVector(void)
 : MCommandParameter(MPT_SVECTOR)
{
	m_nX = m_nY = m_nZ = 0;
}
MCommandParameterShortVector::MCommandParameterShortVector(short x ,short y, short z)
 : MCommandParameter(MPT_SVECTOR)
{
	m_nX = x;
	m_nY = y;
	m_nZ = z;
}

MCommandParameterShortVector::MCommandParameterShortVector(float x ,float y, float z)
: MCommandParameter(MPT_SVECTOR)
{
	m_nX = (short)floorf(x + 0.5f);
	m_nY = (short)floorf(y + 0.5f);
	m_nZ = (short)floorf(z + 0.5f);
}

MCommandParameterShortVector::~MCommandParameterShortVector(void)
{
}
MCommandParameter* MCommandParameterShortVector::Clone(void)
{
	return (new MCommandParameterShortVector(m_nX, m_nY, m_nZ));
}
void MCommandParameterShortVector::GetValue(void* p)
{
	((short*)p)[0] = m_nX;
	((short*)p)[1] = m_nY;
	((short*)p)[2] = m_nZ;
}
int MCommandParameterShortVector::GetData(char* pData, int nSize)
{
	int nValueSize = sizeof(m_nX) * 3;
	if(nValueSize>nSize) return 0;
	short v[3] = {m_nX, m_nY, m_nZ};
	memcpy(pData, v, nValueSize);
	return nValueSize;
}
int MCommandParameterShortVector::SetData(char* pData)
{
	int nValueSize = sizeof(m_nX) * 3;
	short v[3];
	memcpy(v, pData, nValueSize);
	m_nX = v[0];
	m_nY = v[1];
	m_nZ = v[2];
	return nValueSize;
}
