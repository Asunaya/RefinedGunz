#ifndef MAILSLOTMSG_H

#define MAILSLOTMSG_H





#pragma pack(1)





typedef struct {

	unsigned int	dwType;

	char			pBuffer[1];			// Sender+'\0', Receiver+'\0'

} RMSG_GENERIC, *PRMSG_GENERIC;



typedef struct {

public:

	PRMSG_GENERIC pMsg;

	DWORD dwMsgSize;

} BUFFERED_RMSG;





#define MSMSG_KNOCK			0

typedef struct {

	unsigned int	dwType;				// message type (MSMSG_KNOCK)

	char			pBuffer[1];

} RMSG_KNOCK, *PRMSG_KNOCK;





#pragma pack()





#endif