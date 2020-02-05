#pragma once
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#include <conio.h>
#include <string.h>

#define DEFAULT_BUFFLEN 1024
#define PUBLISHER_PORT "27016"
#define SUBSCRIBER_PORT "27017"

bool InitializeWindowsSockets();
int ListenSocketDefiniton(SOCKET *listenSocket, PCSTR port);
bool SocketNonBlockingMode(SOCKET *acceptedSocket, SOCKET *listenSocket);
bool ConnectSocketInitialization(SOCKET *connectSocket, char **argv, PCSTR port);
int ReceiveFunction(SOCKET recvSocket, char* recvBuffer, int len);
int SendFunction(SOCKET sendSocket, char* sendBuffer, int len);
bool PubSubSendVerification(char* message, int verificationMode);
int GetMessageLen(char* message);
int ShutdownFunction(SOCKET* acceptedSocket);