#include "StdAfx.h"
#include "util.h"
#include "gzip.h"

void Util::CheckPath(CString & Path)
{
	Path.TrimRight();

	if(Path.GetLength() == 0)
	{
		ASSERT(FALSE);
		return;
	}

	if(Path.Right(1) != '\\')
	{
		Path += '\\';
	}
}

CString Util::ReadString(HANDLE hFile)
{
	CString Result;
	char c;
	DWORD BytesRead;

	while(1)
	{
		if(ReadFile(hFile, &c, 1, &BytesRead, NULL) == 0)
		{
			return Result;
		}

		if(c == 0) break;
		Result += c;
	}

	return Result;
}

// You must delete the pointer returned in Data
bool Util::ReadHttpFile(const char * Url, BYTE *& Data, DWORD & Len)
{
	TRACE("Reading Http File: '%s'\n", Url);

	CInternetSession Session;
	CHttpFile * File;
	DWORD Result;
	char Buffer[256];
	BYTE * Temp1;
	BYTE * Temp2;
//	char HttpHeader[] = "Accept-Encoding: gzip";
	char HttpHeader[] = "";
	GZip gz;

	Len = 0;

	try
	{
		File = (CHttpFile *)Session.OpenURL(Url, 1, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, HttpHeader, sizeof(HttpHeader) - 1);
	}
	catch(CException * e)
	{
		Data = MAlloc(BYTE, 1024);
		e->GetErrorMessage((char *)Data, 1024, 0);
		e->Delete();
		strtok((char *)Data, "\r\n");
		TRACE("Exception: %s\n", Data);
		return false;
	}

	Temp1 = MAlloc(BYTE, 0);

	while(1)
	{
		Result = File->Read(Buffer, sizeof(Buffer));

		if(Result == 0)
		{
			break;
		}

		Temp2 = MAlloc(BYTE, Len + Result);
		memcpy(Temp2, Temp1, Len);
		memcpy(&Temp2[Len], Buffer, Result);
		MFree(Temp1);
		Temp1 = Temp2;
		Len += Result;
	}

	File->Close();
	delete File;

	Data = Temp1;

	if(Len > 2)
	{
		if(Data[0] == 0x1F && Data[1] == 0x8B)	// Is Data GZipped?
		{
			TRACE("Decompressing Http Data...\n");

			if(gz.Decompress(Data, Len))
			{
				MFree(Data);
				Data = MAlloc(BYTE, gz.GetLength());
				Len = gz.GetLength();
				memcpy(Data, gz.GetData(), Len);
			}else{
				TRACE("Decompression Failed.\n");
				MFree(Data);
				Data = MAlloc(BYTE, 256);
				strcpy((char *)Data, "GZip Decompression Failed");
				return false;
			}

		}
	}

	return true;
}

CString Util::StrError(DWORD ErrorNum)
{
	CString Result;

	LPVOID lpMsgBuf;
	
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, ErrorNum, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);

	strtok((LPSTR)lpMsgBuf, "\r\n");

	Result = (LPSTR)lpMsgBuf;

	LocalFree(lpMsgBuf);

	return Result;
}

CString Util::HttpEncode(const BYTE * Data, DWORD Len)
{
	CString Result;
	char Buffer[8];
	DWORD i;

	for(i = 0; i < Len; i++)
	{
		sprintf(Buffer, "%%%.02X", Data[i]);
		Result += Buffer;
	}

	return Result;
}

CString Util::BinaryEncode(const BYTE * Data, DWORD Len)
{
	CString Result;
	char Buffer[8];
	DWORD i;

	for(i = 0; i < Len; i++)
	{
		sprintf(Buffer, "%.02X", Data[i]);
		Result += Buffer;
	}

	return Result;
}

// Provided 4K of extra storage space for data passed
void Util::ErrorMessage(const char * Format, ...)
{
	char * eMessage;
	va_list Args;
	
	eMessage = MAlloc(char, (DWORD)strlen(Format) + 4096);

	va_start(Args, Format);
	vsprintf(eMessage, Format, Args);
	va_end(Args);

	MessageBox(*AfxGetMainWnd(), eMessage, "Error", MB_OK | MB_ICONERROR);

	MFree(eMessage);
}

// Provided 4K of extra storage space for data passed
void Util::WarningMessage(const char * Format, ...)
{
	char * eMessage;
	va_list Args;
	
	eMessage = MAlloc(char, (DWORD)strlen(Format) + 4096);

	va_start(Args, Format);
	vsprintf(eMessage, Format, Args);
	va_end(Args);

	MessageBox(*AfxGetMainWnd(), eMessage, "Warning", MB_OK | MB_ICONWARNING);

	MFree(eMessage);
}

DWORD Util::Round(float f)
{
	float o;
	DWORD r;

	r = (DWORD)f;
	o = f - r;

	if(o > 0.5f)
	{
		r += 1;
	}

	return r;
}

COLORREF Util::FadeColor(float f, COLORREF c1, COLORREF c2)
{
	BYTE r, g, b;

	if(f < 0.0f) f = 0.0f;
	if(f > 1.0f) f = 1.0f;

	r = (BYTE)((1 - f) * GetRValue(c1) + f * GetRValue(c2));
	g = (BYTE)((1 - f) * GetGValue(c1) + f * GetGValue(c2));
	b = (BYTE)((1 - f) * GetBValue(c1) + f * GetBValue(c2));

	return RGB(r, g, b);
}

CString Util::FormatBps(DWORD bps)
{
	CString Result;

	if(bps >= 1073741824)
	{
		Result.Format("%.2f GB/s", bps / 1024.0f / 1024.0f / 1024.0f);
	}
	else if(bps >= 1048576)
	{
		Result.Format("%.2f MB/s", bps / 1024.0f / 1024.0f);
	}
	else if(bps >= 1024)
	{
		Result.Format("%.2f KB/s", bps / 1024.0f);
	}
	else
	{
		Result.Format("%d Bs/s", bps);
	}

	return Result;
}

CString Util::FormatBytes(QWORD b)
{
	CString Result;

	if(b >= 1099511627776)
	{
		Result.Format("%.2f TBytes", b / 1024.0f / 1024.0f / 1024.0f / 1024.0f);
	}
	else if(b >= 1073741824)
	{
		Result.Format("%.2f GBytes", b / 1024.0f / 1024.0f / 1024.0f);
	}
	else if(b >= 1048576)
	{
		Result.Format("%.2f MBytes", b / 1024.0f / 1024.0f);
	}
	else if(b >= 1024)
	{
		Result.Format("%.2f KBytes", b / 1024.0f);
	}
	else
	{
		Result.Format("%d Bytes", b);
	}

	return Result;
}

CString Util::FormatTime(QWORD t)
{
	CString Result;
	CString Temp;

	bool Days = false;
	bool Hours = false;
	bool Minutes = false;

	if(t >= 60 * 60 * 24)
	{
		if(((t / 60) / 60) / 24 == 1)
		{
			Temp.Format("1 day ");
		}else{
			Temp.Format("%d days ", ((t / 60) / 60) / 24);
		}
		Result += Temp;

		Days = true;
	}

	if(t >= 60 * 60)
	{
		if(((t / 60) / 60) % 24 == 1)
		{
			Temp.Format("1 hour ");
		}else{
			Temp.Format("%d hours ", ((t / 60) / 60) % 24);
		}
		Result += Temp;

		Hours = true;
	}

	if(!Days)
	{
		if(t >= 60)
		{
			if((t / 60) % 60 == 1)
			{
				Temp.Format("1 minute ");
			}else{
				Temp.Format("%d minutes ", (t / 60) % 60);
			}
			Result += Temp;

			Minutes = true;
		}
	}

	if(!Minutes && !Hours && !Days)
	{
		if(t % 60 == 1)
		{
			Temp.Format("1 second", t % 60);
		}else{
			Temp.Format("%d seconds", t % 60);
		}
		Result += Temp;
	}
	
	return Result;
}

bool Util::CreateDirectory(const char * Path)
{
	char Drive[_MAX_DRIVE];
	char Dir[_MAX_DIR];
	char FName[_MAX_FNAME];
	char Ext[_MAX_EXT];
	CString Folder;
	char * Temp;

	_splitpath(Path, Drive, Dir, FName, Ext);


	Temp = strtok(Dir, "\\");

	Folder = Drive;
	Folder += "\\";

	while(Temp)
	{
		Folder += Temp;
		Folder += "\\";

		::CreateDirectory(Folder, NULL);

		Temp = strtok(NULL, "\\");
	}

	if(!::CreateDirectory(Path, NULL))
	{
		if(GetLastError() == 183) return true;	// Cannot create a file when that file already exists.
		return false;
	}

	return true;
}

bool Util::BrowsePath(const char * Title, CString & Path)
{
	BROWSEINFO bi;
	ITEMIDLIST * iid;
	LPMALLOC m;
	char FName[MAX_PATH];

	ZeroMemory(&bi, sizeof(bi));

	bi.hwndOwner = NULL;	// Fixme:?
	bi.lpszTitle = Title;
//	bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
	bi.ulFlags = BIF_NEWDIALOGSTYLE;

	iid = SHBrowseForFolder(&bi);

	if(iid == NULL)
	{
		return false;
	}

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if(SHGetPathFromIDList(iid, FName) == FALSE) return false;

	Path = FName;

	SHGetMalloc(&m);
	m->Free(iid);
	m->Release();

	CoUninitialize();

	return true;
}

CString Util::HttpDecode(const char * Str)
{
	char Hex[3];
	DWORD i;
	CString Result;
	DWORD c;

	for(i = 0; i < (DWORD)strlen(Str); i++)
	{
		switch(Str[i])
		{
		case '%':

			if(i + 2 >= strlen(Str)) break;

			Hex[0] = Str[i + 1];
			Hex[1] = Str[i + 2];
			Hex[2] = 0;

			sscanf(Hex, "%x", &c);
			Result += (char)c;

			i += 2;

			break;
		default:
			Result += Str[i];
		}
	}

	return Result;
}

float Util::GetOSVersion(void)
{
	float Result;

	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&vi);

	Result = vi.dwMajorVersion + (vi.dwMinorVersion / 10.0f);

	return Result;
}
