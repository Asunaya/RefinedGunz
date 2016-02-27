#pragma once

class GZip
{
public:
	GZip(void);
	~GZip(void);

	// Interface
	bool Compress(const unsigned char * Data, unsigned long Length);
	bool Decompress(const unsigned char * Data, unsigned long Length);

	unsigned char * GetData(void);
	unsigned long GetLength(void);

	// gzio.cpp helpers
	int Write(const void * Data, int Len);
	int Read(void * Data, int Len);
	int Tell(void);
	bool Eof(void);

protected:
	void Init(void);

	// Input Buffer
	unsigned char * m_FileData;
	unsigned long m_FileLength;
	unsigned long m_FilePos;

	// Output Buffer
	unsigned char * m_Data;
	unsigned long m_Length;
};