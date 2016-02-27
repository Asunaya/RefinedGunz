#pragma once

#ifdef _DEBUG
 #pragma comment(lib, "gzipd.lib")
#else
 #pragma comment(lib, "gzip.lib")
#endif

/**		gzip library를 이용한 압축 클래스
*
*		@author soyokaze
*		@date   2005-11-02
*/

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