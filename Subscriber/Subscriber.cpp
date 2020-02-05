#include "../LibraryTCPConnection/TCPConnection.h"

void Subscribe(SOCKET connectSocket, char* topic);
DWORD WINAPI FunctionRECV(LPVOID lpParam);

/// <summary>
/// Function that initialize Subscribe client
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <returns>int</returns>
int __cdecl main(int argc, char **argv) 
{
	SOCKET connectSocket = INVALID_SOCKET;
	int id = 0;
	int option = 0;
	char topicLength[DEFAULT_BUFFLEN];
	memset(topicLength, 0, sizeof(int));
	char* topic;

	if (argc != 2)
	{
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}
	if (InitializeWindowsSockets() == false)
	{
		return 1;
	}

	if (ConnectSocketInitialization(&connectSocket, argv, SUBSCRIBER_PORT) == false)
	{
		WSACleanup();
		return 1;
	}

	//thread for receiving message from server
	HANDLE hRecvThread;
	hRecvThread = CreateThread(NULL, NULL, &FunctionRECV, &connectSocket, NULL, NULL);

	//menu
	while (id != 1)
	{
		printf(">>>>>>>>>>---MENU---<<<<<<<<<<\n");
		printf("1 --> SUBSCRIBE\n");
		printf("2 --> EXIT\n");
		printf("--------------------\n");
		option = _getch();

		switch (option - 48)
		{
		case 1:
			printf("Enter the length of topic message:");
			gets_s(topicLength, DEFAULT_BUFFLEN);

			if (PubSubSendVerification(topicLength, 0) == false)		//0 for length verification
			{
				continue;
			}

			topic = (char*)malloc(atoi(topicLength) + 1);				// + 1 for '\0'
			printf("Enter topic (max length of topic is: %d):", atoi(topicLength));
			gets_s(topic, atoi(topicLength) + 1);
			if (PubSubSendVerification(topic, 1) == false)
			{
				continue;
			}

			//if all verifications are ok, subsriber send topic to server
			Subscribe(connectSocket, topic);

			break;
		case 2:
			id = 1;
			printf("Exit sucessfully.\n");
			break;
		default:
			printf("Please enter a valid number (1 for subcribe or 0 for exit)\n");
			break;
		}
	}

	CloseHandle(hRecvThread);
	return 0;
}

#pragma region 1 - Subscribe function
/// <summary>
/// Function for sending topic to server
/// </summary>
/// <param name="connectSocket"></param>
/// <param name="topic"></param>
void Subscribe(SOCKET connectSocket, char* topic)
{
	int iResult = 0;
	int topicCharacterCount = GetMessageLen(topic);

	FD_SET set;
	timeval timeVal;
	FD_ZERO(&set);
	FD_SET(connectSocket, &set);
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;

	// + 4 for 4 bytes for topic length + 1 for null terminator
	char* messageToSend = (char*)malloc(topicCharacterCount + 5);
	memset(messageToSend, 0, topicCharacterCount + 5);

	*((int*)messageToSend) = topicCharacterCount + 5;					// 4 for 4 bytes
	strcpy(messageToSend + 4, topic);

	iResult = SendFunction(connectSocket, messageToSend, *((int*)messageToSend));
	if (iResult == SOCKET_ERROR)
	{
		printf("Send failed with error: %ld\n", WSAGetLastError());
		free(messageToSend);
		closesocket(connectSocket);
		WSACleanup();
	}

	free(messageToSend);
}
#pragma endregion
#pragma region 2 - FunctionRECV
/// <summary>
/// Function for receiving messages from server and display message
/// </summary>
/// <param name="lpParam"></param>
/// <returns>DWORD</returns>
DWORD WINAPI FunctionRECV(LPVOID lpParam)
{
	printf("Subscriber ready for messages from subsriber.\n");
	int iResult;
	SOCKET connectSocket = *(SOCKET*)lpParam;

	FD_SET set;
	timeval timeVal;
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;

	do
	{
		char messageSize[4];

		FD_ZERO(&set);
		FD_SET(connectSocket, &set);

		iResult = select(0, &set, NULL, NULL, &timeVal);
		if (iResult == SOCKET_ERROR)
		{
			printf("select failed: %ld\n", WSAGetLastError());
		}

		if (iResult == 0)
		{
			Sleep(1000);
			continue;
		}


		if (iResult > 0)
		{
			//receieve message length
			iResult = ReceiveFunction(connectSocket, messageSize, sizeof(int));
		}
		if (iResult <= 0)
		{
			closesocket(connectSocket);
			break;
		}


		int len = *(int*)messageSize;
		char* recvBuffer = (char*)malloc(len - 4);
		memset(recvBuffer, 0, len - 4);

		iResult = select(0, &set, NULL, NULL, &timeVal);
		if (iResult > 0)
		{
			//receive message
			iResult = ReceiveFunction(connectSocket, recvBuffer, len - 4);
			printf("MESSAGE FROM SERVER: %s\n", recvBuffer);
			printf("--------------------------------\n");
		}
		if (iResult <= 0)
		{
			closesocket(connectSocket);
			printf("Error: %ld\n", WSAGetLastError());
			free(recvBuffer);
			break;
		}

		free(recvBuffer);
	} while (1);

	return 0;
}
#pragma endregion
