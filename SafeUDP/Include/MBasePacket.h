#ifndef BASEPACKET_H
#define BASEPACKET_H

/////////////////////////////////////////////////////////////
//	BasePacket.h
//								 Programmed by Kim Young-Ho 
//								    LastUpdate : 2000/07/20
/////////////////////////////////////////////////////////////


#define SAFEUDP_FLAG_SAFE_PACKET		1
#define SAFEUDP_FLAG_CONTROL_PACKET	1 << 1
#define SAFEUDP_FLAG_ACK_PACKET		1 << 2
#define SAFEUDP_FLAG_LIGHT_PACKET	1 << 3	// NetNode 없이 송수신 가능


#pragma pack(1)


struct MBasePacket {
	BYTE	nFlags;

	MBasePacket()					{ nFlags = 0; }
	~MBasePacket()					{ }
	BOOL GetFlag(BYTE nTFlag)		{ return (nFlags & nTFlag); }
	void SetFlag(BYTE nTFlag)		{ nFlags |= nTFlag; }
	void ResetFlags(BYTE nTFlag)	{ nFlags &= (0xffffffff ^ nTFlag); }
};

struct MACKPacket : MBasePacket {
	BYTE	nSafeIndex;

	MACKPacket()		{ SetFlag(SAFEUDP_FLAG_ACK_PACKET); }
	~MACKPacket()		{ }
};

struct MNormalPacket : MBasePacket {
	WORD	wMsg;
};

struct MSafePacket : MBasePacket {
	WORD	wMsg;
	BYTE	nSafeIndex;		// Using for SafePacket, Ignore on NormalPacket

	MSafePacket()		{ SetFlag(SAFEUDP_FLAG_SAFE_PACKET); }
	~MSafePacket()		{ }
};

struct MLightPacket : MBasePacket {
	WORD	wMsg;

	MLightPacket()		{ SetFlag(SAFEUDP_FLAG_LIGHT_PACKET); }
	~MLightPacket()		{ }
};

struct MControlPacket : MSafePacket {
public:
	enum CONTROL {
		CONTROL_SYN,
		CONTROL_SYN_RCVD,
		CONTROL_FIN,
		CONTROL_FIN_RCVD,
		CONTROL_ACK
	};

	CONTROL	nControl;

	MControlPacket()	{ SetFlag(SAFEUDP_FLAG_CONTROL_PACKET); wMsg = 0; }
	~MControlPacket()	{ }
};


#pragma pack()


#endif