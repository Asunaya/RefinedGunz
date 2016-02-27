#ifndef _CONSOLECMDS_H
#define _CONSOLECMDS_H

#include "MConsoleFrame.h"

void InitConsoleCmds();
void HelpCmd(const int argc, char **const argv);
void ClearCmd(const int argc, char **const argv);
void ClientCmd(const int argc, char **const argv);

void ConnectCmd(const int argc, char **const argv);
void DisconnectCmd(const int argc, char **const argv);
void SendCmd(const int argc, char **const argv);

// Socket Test
bool SocketConnectEvent(void* pCallbackContext, SOCKET sock);
bool SocketDisconnectEvent(void* pCallbackContext, SOCKET sock);
void SocketErrorEvent(void* pCallbackContext, SOCKET sock, SOCKET_ERRER_EVENT ErrorEvent, int &ErrorCode);
#endif