#pragma once

class Util
{
public:
	static void CheckPath(CString & Path);
	static CString ReadString(HANDLE hFile);
	static bool ReadHttpFile(const char * Url, BYTE *& Data, DWORD & Len);
	static CString StrError(DWORD ErrorNum);
	static CString HttpEncode(const BYTE * Data, DWORD Len);
	static CString BinaryEncode(const BYTE * Data, DWORD Len);
	static void ErrorMessage(const char * Format, ...);
	static void WarningMessage(const char * Format, ...);
	static DWORD Round(float f);
	static COLORREF FadeColor(float f, COLORREF c1, COLORREF c2);
	static CString FormatBps(DWORD bps);
	static CString FormatBytes(QWORD b);
	static CString FormatTime(QWORD t);
	static bool CreateDirectory(const char * Path);
	static bool BrowsePath(const char * Title, CString & Path);
	static CString HttpDecode(const char * Str);
	static float GetOSVersion(void);
};
