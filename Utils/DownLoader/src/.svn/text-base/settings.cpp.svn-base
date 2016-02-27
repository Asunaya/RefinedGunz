#include "StdAfx.h"
#include "settings.h"

List<Settings::KV *> Settings::m_Keys;
const TCHAR* strRegPath = _T("Software\\MAIET\\Torrent");

void Settings::InitBool(const char * Var, bool Default)
{
	HKEY hKey;
	char Buffer[32];
	DWORD BufLen;
	KV * k;
	DWORD Type;

	RegCreateKey(HKEY_LOCAL_MACHINE, strRegPath, &hKey);

	k = MAlloc(KV, 1);

	k->Key = Var;
	k->Min = 0;
	k->Max = 1;
	m_Keys.PushBack(k);

	Type = REG_SZ;
	BufLen = sizeof(Buffer);
	if(RegQueryValueEx(hKey, Var, 0, &Type, (BYTE *)Buffer, &BufLen) == ERROR_SUCCESS)
	{
		if(stricmp(Buffer, "true") == 0)
		{
			DEBUG_SETTINGS(" %s = true\n", Var);
			k->Value = 1;
		}else{
			DEBUG_SETTINGS(" %s = false\n", Var);
			k->Value = 0;
		}
	}else{
		DEBUG_SETTINGS(" Init: %s = %s\n", Var, Default ? "true" : "false");
		SetBool(Var, Default);
	}


	RegCloseKey(hKey);
}

void Settings::InitDWord(const char * Var, DWORD Default, DWORD Min, DWORD Max)
{
	HKEY hKey;
	DWORD Result;
	DWORD Type;
	DWORD BufLen;
	KV * k;

	RegCreateKey(HKEY_LOCAL_MACHINE, strRegPath, &hKey);

	k = MAlloc(KV, 1);
	k->Key = Var;
	k->Min = Min;
	k->Max = Max;
	m_Keys.PushBack(k);

	Type = REG_DWORD;
	BufLen = sizeof(Result);
	if(RegQueryValueEx(hKey, Var, 0, &Type, (BYTE *)&Result, &BufLen) == ERROR_SUCCESS)
	{
		if(Result < Min) Result = Min;
		if(Result > Max) Result = Max;
		DEBUG_SETTINGS(" %s = %d\n", Var, Result);
		k->Value = Result;
	}else{
		DEBUG_SETTINGS(" Init: %s = %d\n", Var, Default);
		SetDWord(Var, Default);
	}

	RegCloseKey(hKey);
}

void Settings::InitString(const char * Var, const char * Default)
{
	HKEY hKey;
	char Buffer[256];
	DWORD BufLen;
	DWORD Type;
	KV * k;

	RegCreateKey(HKEY_LOCAL_MACHINE, strRegPath, &hKey);

	k = MAlloc(KV, 1);
	k->Key = Var;
	m_Keys.PushBack(k);

	Type = REG_SZ;
	BufLen = sizeof(Buffer);
	if(RegQueryValueEx(hKey, Var, 0, &Type, (BYTE *)Buffer, &BufLen) == ERROR_SUCCESS)
	{
		DEBUG_SETTINGS(" %s = '%s'\n", Var, Buffer);
		k->Str = Buffer;
	}else{
		DEBUG_SETTINGS(" Init: %s = '%s'\n", Var, Default);
		SetString(Var, Default);
	}

	RegCloseKey(hKey);
}

bool Settings::GetBool(const char * Var)
{
	DWORD i;

	for(i = 0; i < (DWORD)m_Keys.GetSize(); i++)
	{
		if(m_Keys[i]->Key.CompareNoCase(Var) == 0)
		{
			if(m_Keys[i]->Value)
			{
				return true;
			}else{
				return false;
			}
		}
	}

	return false;
}

DWORD Settings::GetDWord(const char * Var)
{
	DWORD i;

	for(i = 0; i < (DWORD)m_Keys.GetSize(); i++)
	{
		if(m_Keys[i]->Key.CompareNoCase(Var) == 0)
		{
			return m_Keys[i]->Value;
		}
	}

	return 0;
}

CString Settings::GetString(const char * Var)
{
	DWORD i;

	for(i = 0; i < (DWORD)m_Keys.GetSize(); i++)
	{
		if(m_Keys[i]->Key.CompareNoCase(Var) == 0)
		{
			return m_Keys[i]->Str;
		}
	}

	return "";
}

void Settings::SetBool(const char * Var, bool Value)
{
	HKEY hKey;
	DWORD i;

	RegCreateKey(HKEY_LOCAL_MACHINE, strRegPath, &hKey);

	if(Value)
	{
		RegSetValueEx(hKey, Var, 0, REG_SZ, (BYTE *)"true", 5);
	}else{
		RegSetValueEx(hKey, Var, 0, REG_SZ, (BYTE *)"false", 5);
	}

	RegCloseKey(hKey);

	DEBUG_SETTINGS(" Setting Changed: %s = '%s'\n", Var, Value ? "true" : "false");

	for(i = 0; i < (DWORD)m_Keys.GetSize(); i++)
	{
		if(m_Keys[i]->Key.CompareNoCase(Var) == 0)
		{
			m_Keys[i]->Value = Value ? 1 : 0;
			break;
		}
	}
}

void Settings::SetDWord(const char * Var, DWORD Value)
{
	HKEY hKey;
	DWORD i;

	RegCreateKey(HKEY_LOCAL_MACHINE, strRegPath, &hKey);

	RegSetValueEx(hKey, Var, 0, REG_DWORD, (BYTE *)&Value, sizeof(Value));

	RegCloseKey(hKey);

	DEBUG_SETTINGS(" Setting Changed: %s = %d\n", Var, Value);

	for(i = 0; i < (DWORD)m_Keys.GetSize(); i++)
	{
		if(m_Keys[i]->Key.CompareNoCase(Var) == 0)
		{
			m_Keys[i]->Value = Value;
			break;
		}
	}
}

void Settings::SetString(const char * Var, const char * Value)
{
	HKEY hKey;
	DWORD i;

	RegCreateKey(HKEY_LOCAL_MACHINE, strRegPath, &hKey);

	RegSetValueEx(hKey, Var, 0, REG_SZ, (BYTE *)Value, (DWORD)strlen(Value) + 1);

	RegCloseKey(hKey);

	DEBUG_SETTINGS(" Setting Changed: %s = '%s'\n", Var, Value);

	for(i = 0; i < (DWORD)m_Keys.GetSize(); i++)
	{
		if(m_Keys[i]->Key.CompareNoCase(Var) == 0)
		{
			m_Keys[i]->Str = Value;
			break;
		}
	}
}

void Settings::Cleanup(void)
{
	while(m_Keys.GetSize())
	{
		MFree(m_Keys[0]);
		m_Keys.Delete(0);
	}
}