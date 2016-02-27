#pragma once

/**		Bittorrent Protocol에 따라 데이터 블럭을
*		Dictionary, List, Interger, String로 bencoding한다.
*
*		@author soyokaze
*		@date   2005-11-02
*/

class Lexicon
{
	struct KV
	{
		CString m_Key;
		char * m_Value;
		DWORD m_ValueLen;
	};

public:
	Lexicon(void);
	~Lexicon(void);

	void Build(const char * Data, DWORD DataLen);

	CString ToString(const char * KeyName);
	QWORD ToInteger(const char * KeyName);
	void ToBinary(const char * KeyName, char * Buffer, DWORD BufLen);

	DWORD GetKeyLen(const char * KeyName);

	void GetInfoHash(BYTE * Hash);

	static CString ConvertBinaryKey(const char * Key, DWORD Len);

protected:
	void ReadBlock(const char * Data, DWORD DataLen, DWORD & Pos, bool & IsKey);

	void ReadDictionary(const char * Data, DWORD DataLen, DWORD & Pos, bool & IsKey);
	void ReadList(const char * Data, DWORD DataLen, DWORD & Pos, bool & IsKey);
	void ReadInteger(const char * Data, DWORD DataLen, DWORD & Pos, bool & IsKey);
	void ReadString(const char * Data, DWORD DataLen, DWORD & Pos, bool & IsKey);

	List<KV *> m_Keys;
	CString m_CurPath;
	CString m_CurKey;

	char m_InfoHash[20];
};
