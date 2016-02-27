/*
	WaveFile.h
	----------

	Programming by Chojoongpil
	This Routine Handles Waveform File(s).
*/
#ifndef __WAVEFILE_HEADER__
#define __WAVEFILE_HEADER__

class Package;

class RSMemWaveFile {
private:
	BYTE *m_pImageData;	// 실제 Waveform Data
	DWORD m_dwImageLen;	// 실제 Waveform Data Length

	BOOL m_bResource;	
public:
	RSMemWaveFile();
	virtual ~RSMemWaveFile();

	// Methods
	BOOL Open( const char *szFileName );
	BOOL Open( UINT uID, HMODULE hMod );
	BOOL Open( Package *pPackage, int nIndex );
	BOOL Open( Package *pPackage, char* Name );

	void Close();
	
	// Inline Methods
	// IsValid : Whether valid or invalid?
	BOOL IsValid() const { return (m_pImageData?TRUE:FALSE); }
	
	// Play : Instant Playback of Waveform Data
	BOOL Play( BOOL bAsync = TRUE, BOOL bLooped = FALSE ){
		if( !IsValid() ){
#ifdef _DEBUG
			OutputDebugString("DsWave : Play, Data is not ready.\n");
#endif
			return FALSE;
		}
		return ::PlaySound( (LPSTR) m_pImageData, NULL, SND_MEMORY|SND_NODEFAULT|(bAsync?SND_ASYNC:SND_SYNC)|(bLooped?(SND_LOOP|SND_ASYNC):0) );
	}
	BOOL GetFormat( WAVEFORMATEX& wfFormat );
	DWORD GetData( BYTE*& pWaveData, DWORD dwMaxLen );
	//DWORD GetDataLen(){ return m_dwImageLen; }
	DWORD GetSize(){ return m_dwImageLen; }
};

/**
	Streaming을 위한 Wave File Class
	--------------------------------
*/
class RSMWaveFile
{
public:
    RSMWaveFile();
    virtual ~RSMWaveFile();
    BOOL Open(LPSTR pszFilename);
    BOOL Cue();
    UINT Read(BYTE * pbDest, UINT cbSize);
	BYTE GetSilenceData();

	// Inline Functions
    UINT GetNumBytesRemaining(){ return (m_nDataSize - m_nBytesPlayed); }
    UINT GetAvgDataRate(){ return (m_nAvgDataRate); }
    UINT GetDataSize(){ return (m_nDataSize); }
    UINT GetNumBytesPlayed(){ return (m_nBytesPlayed); }
    UINT GetDuration(){ return (m_nDuration); }
    
    WAVEFORMATEX * m_pwfmt;
protected:
    HMMIO m_hmmio;
    MMRESULT m_mmr;
    MMCKINFO m_mmckiRiff;
    MMCKINFO m_mmckiFmt;
    MMCKINFO m_mmckiData;
    UINT m_nDuration;           // duration of sound in msec
    UINT m_nBlockAlign;         // wave data block alignment spec
    UINT m_nAvgDataRate;        // average wave data rate
    UINT m_nDataSize;           // size of data chunk
    UINT m_nBytesPlayed;        // offset into data chunk
};



// (SDK root)\samples\C++\Common\Include\dsutil.h

//-----------------------------------------------------------------------------
// Typing macros 
//-----------------------------------------------------------------------------
#define WAVEFILE_READ   1
#define WAVEFILE_WRITE  2

class CWaveFile
{
public:
    WAVEFORMATEX* m_pwfx;        // Pointer to WAVEFORMATEX structure
    HMMIO         m_hmmio;       // MM I/O handle for the WAVE
    MMCKINFO      m_ck;          // Multimedia RIFF chunk
    MMCKINFO      m_ckRiff;      // Use in opening a WAVE file
    DWORD         m_dwSize;      // The size of the wave file
    MMIOINFO      m_mmioinfoOut;
    DWORD         m_dwFlags;
    //BOOL          m_bIsReadingFromMemory;
    BYTE*         m_pbData;
    BYTE*         m_pbDataCur;
    ULONG         m_ulDataSize;
    CHAR*         m_pResourceBuffer;

protected:
    HRESULT ReadMMIO();
    HRESULT WriteMMIO( WAVEFORMATEX *pwfxDest );

public:
    CWaveFile();
    ~CWaveFile();

    HRESULT Open( LPTSTR strFileName, WAVEFORMATEX* pwfx=NULL, DWORD dwFlags=WAVEFILE_READ );
	//HRESULT OpenFromMemory( BYTE* pbData, ULONG ulDataSize, WAVEFORMATEX* pwfx, DWORD dwFlags=WAVEFILE_READ );
    HRESULT Close();

	BOOL IsValid() const { return (m_hmmio?TRUE:FALSE); }

    HRESULT Read( BYTE* pBuffer, DWORD dwSizeToRead, DWORD* pdwSizeRead );
    HRESULT Write( UINT nSizeToWrite, BYTE* pbData, UINT* pnSizeWrote );

    DWORD   GetSize();
    HRESULT ResetFile();
    WAVEFORMATEX* GetFormat() { return m_pwfx; };
};


#endif