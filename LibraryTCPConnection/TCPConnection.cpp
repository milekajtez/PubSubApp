#include "TCPConnection.h"

#pragma region 1 - InitializeSocket
/// <summary>
/// Function for initialization windows socket
/// </summary>
/// <returns>boolean</returns>
bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}
#pragma endregion
#pragma region 2 - ListenSocketDefiniton
/// <summary>
/// Function for initialize lsitenSocket to listen on port
/// </summary>
/// <param name="listenSocket"></param>
/// <param name="port"></param>
/// <returns>int</returns>
int ListenSocketDefiniton(SOCKET *listenSocket, PCSTR port)
{
	int iResult;
	if (InitializeWindowsSockets() == false)
	{
		return 1;
	}

	addrinfo *resultingAddress = NULL;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;				// IPv4 address
	hints.ai_socktype = SOCK_STREAM;		// Provide reliable data streaming
	hints.ai_protocol = IPPROTO_TCP;		// Use TCP protocol
	hints.ai_flags = AI_PASSIVE;			// 

	iResult = getaddrinfo(NULL, port, &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	*listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		return 1;
	}

	iResult = bind(*listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(*listenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(resultingAddress);

	iResult = listen(*listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(*listenSocket);
		WSACleanup();
		return 1;
	}
	return 0;
}
#pragma endregion
#pragma region 3 - SocketNonBlockingMode
/// <summary>
/// Function for set acceptedSocket to nonblocking mode
/// </summary>
/// <param name="acceptedSocket"></param>
/// <param name="listenSocket"></param>
/// <returns>boolean</returns>
bool SocketNonBlockingMode(SOCKET *acceptedSocket, SOCKET *listenSocket)
{
	int iResult;

	*acceptedSocket = accept(*listenSocket, NULL, NULL);
	if (*acceptedSocket == INVALID_SOCKET)
	{
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(*listenSocket);
		WSACleanup();
		return false;
	}

	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(*acceptedSocket, FIONBIO, &nonBlockingMode);
	if (iResult == SOCKET_ERROR)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		return false;
	}

	return true;
}
#pragma endregion
#pragma region 4 - ConnectSocketInitialization
/// <summary>
/// Function for initialize connectSocket + noneBlockMode
/// </summary>
/// <param name="connectSocket"></param>
/// <param name="argv"></param>
/// <param name="port"></param>
/// <returns>boolean</returns>
bool ConnectSocketInitialization(SOCKET *connectSocket, char **argv, PCSTR port)
{
	int iResult;
	*connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*connectSocket == INVALID_SOCKET)
	{
		printf("Socket failed with error: %ld\n", WSAGetLastError());
		return false;
	}

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
	serverAddress.sin_port = htons(atoi(port));
	if (connect(*connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(*connectSocket);
		return false;
	}

	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(*connectSocket, FIONBIO, &nonBlockingMode);
	if (iResult == SOCKET_ERROR)
	{
		printf("Socket faield with error: %ld\n", WSAGetLastError());
		return false;
	}

	return true;
}
#pragma endregion
#pragma region 5 - ReceiveFunction
/// <summary>
/// Function for receiving data
/// </summary>
/// <param name="recvSocket"></param>
/// <param name="recvBuffer"></param>
/// <param name="len"></param>
/// <returns></returns>
int ReceiveFunction(SOCKET recvSocket, char* recvBuffer, int len)
{
	int iResult;
	int recvCounter = 0;

	while (recvCounter < len) 
	{
		FD_SET set;
		timeval timeVal;
		timeVal.tv_sec = 0;
		timeVal.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(recvSocket, &set);

		iResult = select(0, &set, NULL, NULL, &timeVal);
		if (iResult == SOCKET_ERROR)
		{
			printf("select failed in ReceiveFunction: %ld\n", WSAGetLastError());
		}
		else if (iResult == 0)
		{
			Sleep(1000);
		}
		else if (iResult > 0)
		{
			iResult = recv(recvSocket, recvBuffer, len - recvCounter, 0);
			recvCounter += iResult;
			if (iResult <= 0) 
			{
				break;
			}
		}
	}

	return recvCounter;		//mozda treba i recvCounter
}
#pragma endregion
#pragma region 6 - SendFunction
/// <summary>
/// Function forsending data
/// </summary>
/// <param name="sendSocket"></param>
/// <param name="sendBuffer"></param>
/// <param name="len"></param>
/// <returns></returns>
int SendFunction(SOCKET sendSocket, char* sendBuffer, int len)
{
	int iResult;
	int sendCounter = 0;

	FD_SET set;
	timeval timeVal;
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;

	while (sendCounter < len) 
	{
		FD_ZERO(&set);
		FD_SET(sendSocket, &set);

		iResult = select(0, NULL, &set, NULL, &timeVal);
		if (iResult == SOCKET_ERROR)
		{
			printf("select failed in SendFunction: %ld\n", WSAGetLastError());
		}
		else if (iResult == 0) 
		{
			Sleep(1000);
		}
		else if (iResult > 0) 
		{
			iResult = send(sendSocket, sendBuffer + sendCounter, len - sendCounter, 0);
			sendCounter += iResult;
		}
	}

	return iResult;
}
#pragma endregion
#pragma region 7 - PubSubSendVerification
/// <summary>
/// Function for verification clients inputs
/// </summary>
/// <param name="message"></param>
/// <param name="verificationMode">
/// 0 - verification for topic length
/// 1 - verification for topic and message input
/// </param>
/// <returns>boolean</returns>
bool PubSubSendVerification(char* message, int verificationMode)
{
	if (verificationMode == 0)
	{
		if (atoi(message) == 0 || atoi(message) < 0)
		{
			printf("The length of topic must be number greater than 0.\n");
			return false;
		}
	}
	else if (verificationMode == 1)
	{
		if (strcmp(message, "") == 0)
		{
			printf("Topic and message must not be empty string.\n");
			return false;
		}

		int onlySpaceTab = 0;
		for (int i = 0; i < (int)strlen(message); i++)
		{
			if (strcmp(message, " ") == 0 || strcmp(message, "\t") == 0)
			{
				onlySpaceTab++;
			}
		}
		if (onlySpaceTab == strlen(message))
		{
			printf("Message witch you input have only spaces and tabs.This type of message is not valid.\n");
			return false;
		}
	}

	return true;
}
#pragma endregion
#pragma region 8 - GetMessageLen
/// <summary>
/// Function that calculates the length of message
/// </summary>
/// <param name="message"></param>
/// <returns>int</returns>
int GetMessageLen(char* message)
{
	int messageCharacterCount = 0;

	for (int i = 0; i < (int)strlen(message); i++)
	{
		if (message[i] != '\0') 
		{
			messageCharacterCount++;
		}
		else 
		{
			break;
		}
	}

	return messageCharacterCount;
}
#pragma endregion
#pragma region 9 - ShutdownFunction
/// <summary>
/// Function for shutdown acceoted socket
/// </summary>
/// <param name="acceptedSocket"></param>
/// <returns>int</returns>
int ShutdownFunction(SOCKET* acceptedSocket)
{
	int iResult;
	iResult = shutdown(*acceptedSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(*acceptedSocket);
		WSACleanup();
		return 1;
	}

	return 0;
}
#pragma endregion
