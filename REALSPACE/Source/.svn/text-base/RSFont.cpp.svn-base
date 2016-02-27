//#include "FontMaker.h"
#include "RSFont.h"
#include "RSD3D.h"

static WORD mask_r=0xf800,mask_g=0x07e0,mask_b=0x001f;

RSFont::RSFont()
{
	mFontArray=NULL;
}

RSFont::~RSFont()
{
	Destroy();
}

RSFont::Destroy()
{
	if(mFontArray)
	{
		for(int i=0;i<mFontCount;i++)
		{
			if(mFontArray[i].data)
				delete []mFontArray[i].data;
		}
		delete []mFontArray;
		mFontArray=NULL;
	}
	return TRUE;
}
/*
void RSFont::SetPixelFormat(RSPIXELFORMAT pixelformat)
{
	m_PixelFormat=pixelformat;
	switch(pixelformat)
	{
	case RS_PIXELFORMAT_555 :
		{
			mask_r=0x7c00;
			mask_g=0x03e0;
			mask_b=0x001f;
		}break;
	case RS_PIXELFORMAT_565 :
		{
			mask_r=0xf800;
			mask_g=0x07e0;
			mask_b=0x001f;
		}break;
	}
}
*/

int RSFont::GetByteSize(int Width)
{
	switch(mBPP)
	{
	case 1:return (((Width)+7)/8);break;
	case 4:return (((Width)+1)/2);break;
	case 8:return Width;break;	
	}
	return -1;
}

BOOL RSFont::New(int nFontCount,int BPP)
{
	Destroy();
	mFontArray=new RSFONTDATA[nFontCount];
	for(int i=0;i<nFontCount;i++)
	{
		mFontArray[i].data=NULL;
	}
	mFontCount=nFontCount;
	mBPP=BPP;
	return TRUE;
}

void RSFont::SetRSFONTDATA(int ind,RSFONTDATA* pfd)
{
	if(mFontArray[ind].data)
	{
		delete [](mFontArray[ind].data);
		mFontArray[ind].data=NULL;
	}
	if(pfd->data)
	{
		mFontArray[ind].Width=pfd->Width;
		int BYTESIZE=GetByteSize(pfd->Width)*mHeight;
		if(BYTESIZE)
		{
			mFontArray[ind].data=new BYTE[BYTESIZE];
			memcpy(mFontArray[ind].data,pfd->data,BYTESIZE);
		}
	}
	else
		mFontArray[ind].Width=0;
}

BOOL RSFont::Save(const char* filename)
{
	FILE *file=NULL;
	RSFONTDATAHEADER mHeader={RSFONTDATAHEADER_HEADERID,RSFONTDATAHEADER_BUILD,0};
	mHeader.nFontCount=mFontCount;
	mHeader.nBitsPerPixel=mBPP;

	file=fopen(filename,"wb+");
	if(!file) return FALSE;
	fwrite(&mHeader,sizeof(mHeader),1,file);
	fwrite(&mHeight,sizeof(mHeight),1,file);
	for(int i=0;i<mFontCount;i++)
	{
		fwrite(&(mFontArray[i].Width),sizeof(int),1,file);
		if(mFontArray[i].Width)
			fwrite(mFontArray[i].data,GetByteSize(mFontArray[i].Width),mHeight,file);
	}
	fclose(file);
	return TRUE;
}

BOOL RSFont::Open(const char* filename)
{
	FILE *file=NULL;
	RSFONTDATAHEADER mHeader;
	file=fopen(filename,"rb");
	if(!file) return FALSE;
	fread(&mHeader,sizeof(mHeader),1,file);
	New(mHeader.nFontCount,mHeader.nBitsPerPixel);
	fread(&mHeight,sizeof(mHeight),1,file);
	for(int i=0;i<mFontCount;i++)
	{
		fread(&(mFontArray[i].Width),sizeof(int),1,file);
		if(mFontArray[i].Width)
		{
			if(mFontArray[i].data) delete [](mFontArray[i].data);
			int BYTESIZE=GetByteSize(mFontArray[i].Width)*mHeight;
			mFontArray[i].data=new BYTE[BYTESIZE];
			fread(mFontArray[i].data,GetByteSize(mFontArray[i].Width),mHeight,file);
		}
	}
	fclose(file);
	return TRUE;
}

void RSFont::Draw(int i,void *pscreen)
{
	int j,k,l,ii;
	RSFONTDATA *fd=GetPRSFONTDATA(i);
	if(fd)
	{
		int nByte=GetByteSize(fd->Width);
		if(fd->Width)
		{
			if(RSPixelFormat==RS_PIXELFORMAT_X888)
			{
				DWORD *screen=(DWORD*)pscreen;
				switch(mBPP)
				{
					case 8:
					{
						BYTE *pB;
						DWORD *pS;
						DWORD mask_r=0xff0000,mask_g=0xff00,mask_b=0xff;

						DWORD ColorR=mColor&mask_r;
						DWORD ColorG=mColor&mask_g;
						DWORD ColorB=mColor&mask_b;

						for(j=0;j<GetHeight();j++)
						{
							pB=&fd->data[j*nByte];
							pS=(DWORD*)pscreen;

							for(k=0;k<fd->Width;k++)
							{
								i=*pB;
								ii=16-i;
								if(i)
								{
									DWORD c=*pS;
									DWORD out=
										(((c&mask_r)*ii+ColorR*i)>>4)&mask_r|
										(((c&mask_g)*ii+ColorG*i)>>4)&mask_g|
										(((c&mask_b)*ii+ColorB*i)>>4)&mask_b
										;
										
									*pS=out;
								}
								pB++;
								pS++;
							}
							screen+=m_lPitch>>2;
						}
					}break;
					case 1:
					{
						for(j=0;j<GetHeight();j++)
						{
							for(k=0;k<nByte;k++)
							{
								for(l=0;l<8;l++)
								{
									if(fd->data[j*nByte+k]&(1<<(7-l)))
										*(screen+(k*8+l))=mColor;
								}
							}
							screen+=m_lPitch>>2;
						}
					}
					break;
				}
			}
			else		// 16 bit mode
			{
				WORD *screen=(WORD*)pscreen;
				WORD color=RGB322RGB16(mColor);
				switch(mBPP)
				{
					case 8:
					{
						BYTE *pB;
						WORD *pS;

						DWORD ColorR=color&mask_r;
						DWORD ColorG=color&mask_g;
						DWORD ColorB=color&mask_b;

						for(j=0;j<GetHeight();j++)
						{
							pB=&fd->data[j*nByte];
							pS=(WORD*)screen;

							for(k=0;k<fd->Width;k++)
							{
								i=*pB;
								ii=16-i;
								if(i)
								{
									DWORD c=*pS;
									DWORD out=
										(((c&mask_r)*ii+ColorR*i)>>4)&mask_r|
										(((c&mask_g)*ii+ColorG*i)>>4)&mask_g|
										(((c&mask_b)*ii+ColorB*i)>>4)&mask_b
										;
										
									*pS=(WORD)out;
								}
								pB++;
								pS++;
							}
							screen+=m_lPitch>>1;
						}
					}break;
					case 1:
					for(j=0;j<GetHeight();j++)
					{
						for(k=0;k<nByte;k++)
						{
							for(l=0;l<8;l++)
							{
								if(fd->data[j*nByte+k]&(1<<(7-l)))
									*(screen+(k*8+l))=color;
							}
						}
						screen+=m_lPitch>>1;
					}
					break;
				}
			}
		}
	}
}

void RSFont::Draw(int i,void *pscreen,int nSkipX,int nLenX,int nSkipY,int nLenY)
{
	int j,k,l,m,n,ii;

	if(nSkipX < 0 || nSkipY < 0 || nLenX < 0 || nLenY < 0) return;

	RSFONTDATA *fd=GetPRSFONTDATA(i);
	if(fd)
	{
		int nByte=GetByteSize(fd->Width);
		if(fd->Width)
		{
			if(RSPixelFormat==RS_PIXELFORMAT_X888)
			{
				DWORD *screen=(DWORD*)pscreen;
				switch(mBPP)
				{
				case 8:
					{
						BYTE *pB;
						DWORD *pS;
						DWORD mask_r=0xff0000,mask_g=0xff00,mask_b=0xff;

						DWORD ColorR=mColor&mask_r;
						DWORD ColorG=mColor&mask_g;
						DWORD ColorB=mColor&mask_b;

						screen += (m_lPitch>>2)*nSkipY;

						for(j=nSkipY;j<nLenY;j++)
						{
							pB=&fd->data[j*nByte];
							pS=screen;
							for(k=nSkipX;k<nLenX;k++)
							{
								i=*pB;
								ii=16-i;
								if(i)
								{
									DWORD c=*pS;
									DWORD out=
										(((c&mask_r)*ii+ColorR*i)>>4)&mask_r|
										(((c&mask_g)*ii+ColorG*i)>>4)&mask_g|
										(((c&mask_b)*ii+ColorB*i)>>4)&mask_b
										;
									*pS=out;
								}
								pB++;
								pS++;
							}
							screen+=m_lPitch>>2;
						}
					}
					break;
				case 1:
					if(nLenX == -1) nLenX = fd->Width;

					for(j=nSkipY;j<nLenY;j++)
					{
						m = nSkipX;
						n = 0;

						for(k=0;k<nByte;k++)
						{
							for(l=0;l<8;l++)
							{
								n++;
								m--;
								if(m < 0)
									if(fd->data[j*nByte+k]&(1<<(7-l))){
										if(n<=nLenX) *(screen+((m_lPitch>>2)*j)+(k*8+l))=mColor;
									}
							}
						}
					}
					break;
				}
			}
			else					// 16 bit mode
			{
				WORD *screen=(WORD*)pscreen;
				WORD color=RGB322RGB16(mColor);
				switch(mBPP)
				{
				case 8:
					{
						BYTE *pB;
						WORD *pS;

						DWORD ColorR=color&mask_r;
						DWORD ColorG=color&mask_g;
						DWORD ColorB=color&mask_b;

						screen += (m_lPitch>>1)*nSkipY;

						for(j=nSkipY;j<nLenY;j++)
						{
							pB=&fd->data[j*nByte];
							pS=screen;
							for(k=nSkipX;k<nLenX;k++)
							{
								i=*pB;
								ii=16-i;
								if(i)
								{
									DWORD c=*pS;
									DWORD out=
										(((c&mask_r)*ii+ColorR*i)>>4)&mask_r|
										(((c&mask_g)*ii+ColorG*i)>>4)&mask_g|
										(((c&mask_b)*ii+ColorB*i)>>4)&mask_b
										;
									*pS=(WORD)out;
								}
								pB++;
								pS++;
							}
							screen+=m_lPitch>>1;
						}
					}
					break;
				case 1:
					if(nLenX == -1) nLenX = fd->Width;

					for(j=nSkipY;j<nLenY;j++)
					{
						m = nSkipX;
						n = 0;

						for(k=0;k<nByte;k++)
						{
							for(l=0;l<8;l++)
							{
								n++;
								m--;
								if(m < 0)
									if(fd->data[j*nByte+k]&(1<<(7-l))){
										if(n<=nLenX) *(screen+((m_lPitch>>1)*j)+(k*8+l))=color;
									}
							}
						}
					}
					break;
				}
			}
		}
	}
}