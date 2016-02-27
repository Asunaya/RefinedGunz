#include "gzip.h"
#include "zlib.h"
#include "memory.h"

gzFile gzopen(GZip * gz, const char * Mode);

GZip::GZip(void)
{
	m_Data = 0;
	m_Length = 0;
}

GZip::~GZip(void)
{
	if(m_Data) delete [] m_Data;
}

bool GZip::Compress(const unsigned char * Data, unsigned long Length)
{
	gzFile File;

	Init();

	File = gzopen(this, "wb");

	gzwrite(File, Data, Length);

	gzclose(File);

	return true;
}

//#include "windows.h"
//#include "stdarg.h"
//#include "stdio.h"
//
//void Trace(char * Format, ...)
//{
//	char Buffer[4096];
//
//	va_list Args;
//
//	va_start(Args, Format);
//	vsprintf(Buffer, Format, Args);
//	va_end(Args);
//
//	OutputDebugString(Buffer);
//}

bool GZip::Decompress(const unsigned char * Data, unsigned long Length)
{
	gzFile File;
	int Result;
	unsigned char * Temp;
	char Buffer[1024];

	Init();

	m_Data = new unsigned char [0];

	m_FileData = (unsigned char *)Data;
	m_FileLength = Length;

	File = gzopen(this, "rb");

	do
	{
		Result = gzread(File, Buffer, 1024);

		if(Result == 0) break;
		if(Result == -1)
		{
			gzclose(File);
			return false;
		}

		Temp = new unsigned char [m_Length + Result];
		memcpy(Temp, m_Data, m_Length);
		memcpy(&Temp[m_Length], Buffer, Result);
		delete [] m_Data;
		m_Data = Temp;
		m_Length += Result;
	}while(1);

	gzclose(File);

	return true;
}

unsigned char * GZip::GetData(void)
{
	return m_Data;
}

unsigned long GZip::GetLength(void)
{
	return m_Length;
}

void GZip::Init(void)
{
	if(m_Data)
	{
		delete [] m_Data;
		m_Data = 0;
		m_Length = 0;
	}

	m_FilePos = 0;
}

int GZip::Write(const void * Data, int Len)
{
	unsigned char * Temp;

	if(!m_Data) m_Data = new unsigned char [0];

	Temp = new unsigned char [m_Length + Len];

	memcpy(Temp, m_Data, m_Length);
	memcpy(&Temp[m_Length], Data, Len);

	delete [] m_Data;
	m_Data = Temp;
	m_Length += Len;

	return Len;
}

int GZip::Read(void * Data, int Len)
{
	if(m_FilePos + Len > m_FileLength)
	{
		Len = m_FileLength - m_FilePos;
	}

	memcpy(Data, &m_FileData[m_FilePos], Len);
	m_FilePos += Len;

	return Len;
}

int GZip::Tell(void)
{
	return m_FilePos;
}

bool GZip::Eof(void)
{
	if(m_FilePos >= m_FileLength) return true;
	return false;
}