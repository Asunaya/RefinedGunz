#ifndef _ZFILE_H
#define _ZFILE_H

#include <stdio.h>
#include "../sdk/zlib/include/zlib.h"

#define BUFFER_SIZE	1024


// TODO : 입력/출력을 stream 으로 일반화 하여 
//			CML & MZFileSystem 의 MZFile을 대체하면 좋을듯.

// 한개의 파일이 단순히 압축되어있는 파일
class ZFile {

	FILE			*m_pFile;
	bool			m_bWrite;	// 쓰고있는 중인지..
	z_stream		m_Stream;
	unsigned char	m_Buffer[BUFFER_SIZE];

public:
	ZFile();
	virtual ~ZFile();

	bool Open(const char *szFileName,bool bWrite = false);

	int Read(void *pBuffer,int nByte);
	int Write(void *pBuffer,int nByte);

	template<typename T>
	int Read(T &obj)
	{
		return Read(&obj, sizeof(T)) / sizeof(T);
	}
	template<typename T, size_t size>
	int Read(T(&obj)[size])
	{
		int ItemsRead = 0;
		for (int i = 0; i < size; i++)
		{
			ItemsRead += Read(obj[i]);
		}
		return ItemsRead;
	}

	bool Close();
};



ZFile *zfopen(const char *szFileName,bool bWrite = false);
int zfread(void *pBuffer,int nItemSize,int nItemCount,ZFile *pFile);
int zfwrite(void *pBuffer,int nItemSize,int nItemCount,ZFile *pFile);
bool zfclose(ZFile *pFile);

#endif // of _ZREPLAYFILE