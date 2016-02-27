#pragma once

/**		레지스트리 설정
*
*		@author soyokaze
*		@date   2005-11-02
*/

class Settings
{
	struct KV
	{
		CString Key;
		DWORD Value;
		CString Str;
		DWORD Min;
		DWORD Max;
	};
public:
	static void InitBool(const char * Var, bool Default);
	static void InitDWord(const char * Var, DWORD Default, DWORD Min, DWORD Max);
	static void InitString(const char * Var, const char * Default);

	static bool GetBool(const char * Var);
	static DWORD GetDWord(const char * Var);
	static CString GetString(const char * Var);

	static void SetBool(const char * Var, bool Value);
	static void SetDWord(const char * Var, DWORD Value);
	static void SetString(const char * Var, const char * Value);

	static void Cleanup(void);

protected:
	static List<KV *> m_Keys;
};
