#include "StdAfx.h"
#include "lexicon.h"
#include "sha1.h"

Lexicon::Lexicon(void)
{
}

Lexicon::~Lexicon(void)
{
	while(m_Keys.GetSize())
	{
		if(m_Keys[0]->m_Value) MFree(m_Keys[0]->m_Value);
		MFree(m_Keys[0]);
		m_Keys.Delete(0);
	}
}

void Lexicon::Build(const char * Data, DWORD DataLen)
{
	DWORD Pos;
	bool IsKey;

	Pos = 0;
	IsKey = true;

	ReadBlock(Data, DataLen, Pos, IsKey);
}

void Lexicon::ReadBlock(const char * Data, DWORD DataLen, DWORD & Pos, bool & IsKey)
{
	switch(Data[Pos])
	{
	case 'd':
		ReadDictionary(Data, DataLen, Pos, IsKey);
		return;

	case 'l':
		ReadList(Data, DataLen, Pos, IsKey);
		return;

	case 'i':
		ReadInteger(Data, DataLen, Pos, IsKey);
		return;
	}

	if(Data[Pos] >= '0' && Data[Pos] <= '9')
	{
		ReadString(Data, DataLen, Pos, IsKey);
		return;
	}

//	MessageBox(NULL, "Error parsing data. Unexpected token found while parsing bencoded data.", "Error", MB_OK | MB_ICONERROR);
	Pos = DataLen;

	KV * k;

	k = MAlloc(KV, 1);
	
	k->m_Key = "failure reason";

	k->m_Value = MAlloc(char, 256);
	sprintf(k->m_Value, "Corrupted Data: Unexpected Token at byte %d", Pos - 1);
	k->m_ValueLen = 256;

	m_Keys.PushBack(k);

	DEBUG_LEXICON(" Unknown Token: '%c' (0x%.2X)\n", Data[Pos], (unsigned char)Data[Pos]);

	Pos++;
}

void Lexicon::ReadDictionary(const char * Data, DWORD DataLen, DWORD & Pos, bool & IsKey)
{
	CString OldPath;
	CString OldKey;
	DWORD InfoStart;

	if(IsKey)
	{
		Pos++;

		while(Pos < DataLen && Data[Pos] != 'e')
		{
			ReadBlock(Data, DataLen, Pos, IsKey);
		}

		Pos++;
	}else{
		IsKey = true;
		
		OldPath = m_CurPath;
		OldKey = m_CurKey;
		
		m_CurPath += m_CurKey + ".";

		if(m_CurKey == "info")
		{
			InfoStart = Pos;
			ReadDictionary(Data, DataLen, Pos, IsKey);

			Sha1((BYTE *)m_InfoHash, (BYTE *)&Data[InfoStart], Pos - InfoStart);

			DEBUG_LEXICON(" InfoHash: ");
			for(int i = 0; i < 20; i++)
			{
				DEBUG_LEXICON(" %.2x", (BYTE)m_InfoHash[i]);
			}
			DEBUG_LEXICON(" \n");
		}else{
			ReadDictionary(Data, DataLen, Pos, IsKey);
		}

		m_CurPath = OldPath;
		m_CurKey = OldKey;
	}
}

void Lexicon::ReadList(const char * Data, DWORD DataLen, DWORD & Pos, bool & IsKey)
{
	int Item = 0;
	CString OldKey;
	CString OldPath;
	KV * k;

	if(IsKey)
	{
		Pos++;

		while(Pos < DataLen && Data[Pos] != 'e')
		{
			IsKey = false;

			m_CurKey.Format("%d", Item);

			ReadBlock(Data, DataLen, Pos, IsKey);

			Item++;
		}

		k = MAlloc(KV, 1);
		k->m_Key = m_CurPath + "count";
		k->m_Value = MAlloc(char, 8);
		sprintf(k->m_Value, "%d", Item);
		k->m_ValueLen = (DWORD)strlen(k->m_Value);

		m_Keys.PushBack(k);

		DEBUG_LEXICON(" %s=%s\n", k->m_Key, k->m_Value);

		Pos++;
	}else{
		IsKey = true;

		OldPath = m_CurPath;
		OldKey = m_CurKey;

		m_CurPath += m_CurKey + ".";

		ReadList(Data, DataLen, Pos, IsKey);

		m_CurPath = OldPath;
		m_CurKey = OldKey;
	}
}

void Lexicon::ReadInteger(const char * Data, DWORD DataLen, DWORD & Pos, bool & IsKey)
{
	QWORD Value = 0;
	int Count = 0;
	KV * k;

	Pos++;

	while(Pos < DataLen && Data[Pos] != 'e')
	{
		Value *= 10;
		Value += Data[Pos++] - '0';
		Count++;
	}

	Pos++;

	k = MAlloc(KV, 1);
	
	k->m_Key = m_CurPath + m_CurKey;

	k->m_Value = MAlloc(char, Count + 1);
	sprintf(k->m_Value, "%I64d", Value);
	k->m_ValueLen = Count + 1;

	m_Keys.PushBack(k);

	DEBUG_LEXICON(" %s%s=%d\n", m_CurPath, m_CurKey, Value);

	IsKey = true;
}

void Lexicon::ReadString(const char * Data, DWORD DataLen, DWORD & Pos, bool & IsKey)
{
	int Len = 0;
	char * Value;
	KV * k;

	while(Pos < DataLen && Data[Pos] != ':')
	{
		Len *= 10;
		Len += Data[Pos++] - '0';
	}

	Pos++;

	Value = MAlloc(char, Len + 1);
	memcpy(Value, &Data[Pos], Len);
	Value[Len] = 0;
	Pos += Len;

	if(IsKey)
	{
		m_CurKey = ConvertBinaryKey(Value, Len);

		MFree(Value);
	}else{
		DEBUG_LEXICON(" %s%s='%s'\n", m_CurPath, m_CurKey, Value);

		k = MAlloc(KV, 1);

		k->m_Key = m_CurPath + m_CurKey;
		k->m_Value = Value;
		k->m_ValueLen = Len;

		m_Keys.PushBack(k);
	}

	IsKey = !IsKey;
}

CString Lexicon::ToString(const char * KeyName)
{
	DWORD i;

	for(i = 0; i < m_Keys.GetSize(); i++)
	{
		if(m_Keys[i]->m_Key == KeyName)
		{
			return m_Keys[i]->m_Value;
		}
	}

	return "";
}

QWORD Lexicon::ToInteger(const char * KeyName)
{
	DWORD i;

	for(i = 0; i < m_Keys.GetSize(); i++)
	{
		if(m_Keys[i]->m_Key == KeyName)
		{
			return _atoi64(m_Keys[i]->m_Value);
		}
	}

	return 0;
}

void Lexicon::ToBinary(const char * KeyName, char * Buffer, DWORD BufLen)
{
	DWORD i;

	for(i = 0; i < m_Keys.GetSize(); i++)
	{
		if(m_Keys[i]->m_Key == KeyName)
		{
			DWORD v;
			v = m_Keys[i]->m_ValueLen;
			if(BufLen < m_Keys[i]->m_ValueLen)
			{
				ASSERT(FALSE);
			}

			memcpy(Buffer, m_Keys[i]->m_Value, (BufLen < m_Keys[i]->m_ValueLen) ? BufLen : m_Keys[i]->m_ValueLen);
			return;
		}
	}
}

DWORD Lexicon::GetKeyLen(const char * KeyName)
{
	DWORD i;

	for(i = 0; i < m_Keys.GetSize(); i++)
	{
		if(m_Keys[i]->m_Key == KeyName)
		{
			return m_Keys[i]->m_ValueLen;
		}
	}

	return 0;
}

void Lexicon::GetInfoHash(BYTE * Hash)
{
	memcpy(Hash, m_InfoHash, 20);
}

CString Lexicon::ConvertBinaryKey(const char * Key, DWORD Len)
{
	DWORD i;
	CString Result;
	char Temp[8];

	for(i = 0; i < Len; i++)
	{
		if(Key[i] < 0x20 || Key[i] > 0x7E)
		{
			sprintf(Temp, "%.2X", (unsigned char)Key[i]);
			Result += Temp;
		}else{
			Result += Key[i];
		}
	}

	return Result;
}