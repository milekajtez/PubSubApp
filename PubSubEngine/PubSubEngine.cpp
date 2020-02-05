#include "../LibraryTCPConnection/TCPConnection.h"
#include "../LibraryDataStructures/DataStructures.h"

DWORD WINAPI FunctionRecvPUB(LPVOID lpParam);
DWORD WINAPI AcceptSUB(LPVOID lpParam);
DWORD WINAPI FunctionRecvSUB(LPVOID lpParam);
DWORD WINAPI FunctionSendSUB(LPVOID lpParam);

NodeList* topicHead = NULL;
NodeList* subHead = NULL;

/// <summary>
/// function that initialize the sockets and calls all service operations
/// </summary>
/// <returns>int</returns>
int main(void)
{
	int iResult;
	int definitionSocketIndicator;
	SOCKET acceptedSocketPUB = INVALID_SOCKET;						//for receive messages from publisher
	SOCKET listenSocketPUB = INVALID_SOCKET;						//for listening publisher
	SOCKET listenSocketSUB = INVALID_SOCKET;						//for listening subscriber
	HANDLE hRecvThread;
	HANDLE hRecvSUBThread;

	//define a socket
	definitionSocketIndicator = ListenSocketDefiniton(&listenSocketPUB, PUBLISHER_PORT);
	if (definitionSocketIndicator == 1)
	{
		printf("Initialization socket failed.\n");
		return 1;
	}
	else
	{
		printf("PubSubEngine initialized.Waiting publishers...\n");
	}

	definitionSocketIndicator = ListenSocketDefiniton(&listenSocketSUB, SUBSCRIBER_PORT);
	if (definitionSocketIndicator == 1)
	{
		printf("Initialization socket failed.\n");
		return 1;
	}
	else
	{
		printf("PubSubEngine initialized.Waiting subscribers...\n");
	}

	//initialization structures
	topicHead = NULL;
	subHead = NULL;

	//make thread for reciving messages from subscriber
	hRecvSUBThread = CreateThread(NULL, NULL, &AcceptSUB, (SOCKET*)&listenSocketSUB, NULL, NULL);

	int counter = 0;
	do
	{
		if (SocketNonBlockingMode(&acceptedSocketPUB, &listenSocketPUB) == true)
		{
			printf("Publisher accepted.\n");
		}
		else
		{
			return 1;
		}

		//make thread for reciving messages from publisher
		hRecvThread = CreateThread(NULL, NULL, &FunctionRecvPUB, &acceptedSocketPUB, NULL, NULL);

	} while (1);


	// shutdown accepted socket
	iResult = ShutdownFunction(&acceptedSocketPUB);
	if (iResult == 1)
	{
		return iResult;
	}

	//close thread's handle
	CloseHandle(hRecvThread);
	CloseHandle(hRecvSUBThread);

	//close all sockets
	closesocket(acceptedSocketPUB);
	closesocket(listenSocketPUB);
	closesocket(listenSocketSUB);

	WSACleanup();
	return 0;
}

#pragma region 1 - FunctionRecvPUB
/// <summary>
/// Function for receiving data from publisher
/// </summary>
/// <param name="lpParam"></param>
/// <returns>DWORD</returns>
DWORD WINAPI FunctionRecvPUB(LPVOID lpParam)
{
	printf("PubSubEngine ready for reciving messages from publisher.\n");

	SOCKET acceptedSocket = *(SOCKET*)lpParam;
	int iResult = 0;

	FD_SET set;
	timeval timeVal;
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;

	do
	{
		char messageSize[4];

		FD_ZERO(&set);
		FD_SET(acceptedSocket, &set);

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
			//receive message length
			iResult = ReceiveFunction(acceptedSocket, messageSize, sizeof(int));
		}
		if (iResult <= 0)
		{
			closesocket(acceptedSocket);
			break;
		}


		int len = *(int*)messageSize;
		char* recvBuffer = (char*)malloc(len - 4);

		iResult = select(0, &set, NULL, NULL, &timeVal);
		if (iResult > 0)
		{
			iResult = ReceiveFunction(acceptedSocket, recvBuffer, len - 4);

			char* topic = NULL;
			char* message = NULL;
			UnPackingMessage(recvBuffer, &topic, &message);		//unpacking message

			printf("TOPIC: %s\n", topic);
			printf("MESSAGE: %s\n", message);
			printf("--------------------------------\n");

			// input data in structures
			if (GetElementFromList(topic, topicHead) == false)
			{
				//add new topic
				AddTopicToList(topic, &topicHead);

				int a = 0;
			}
			else
			{
				if (subHead != NULL)
				{
					//add message to queue
					AddMessageToQueue(topic, message, &topicHead, &subHead);
					int c = 0;
				}
			}
		}
		if (iResult <= 0)
		{
			closesocket(acceptedSocket);
			printf("Error: %ld\n", WSAGetLastError());
			free(recvBuffer);
			break;
		}

	} while (1);

	return 0;
}
#pragma endregion
#pragma region 2 - AcceptSUB
/// <summary>
/// Function for reciving topics from subscriber
/// </summary>
/// <param name="lpParam"></param>
/// <returns></returns>
DWORD WINAPI AcceptSUB(LPVOID lpParam)
{
	SOCKET listenSocketSUB = *(SOCKET*)lpParam;
	SOCKET acceptedSocketSUB = INVALID_SOCKET;						//subscriber
	HANDLE hStoreSUB;

	do
	{
		if (SocketNonBlockingMode(&acceptedSocketSUB, &listenSocketSUB) == true)
		{
			printf("Subscriber accepted.\n");
		}
		else
		{
			return 1;
		}

		//make thread for input data in strucuture 2
		hStoreSUB = CreateThread(NULL, NULL, &FunctionRecvSUB, (void*)&acceptedSocketSUB, NULL, NULL);
		if (hStoreSUB == 0)
		{
			break;
		}

	} while (1);

	CloseHandle(hStoreSUB);
	return 0;
}
#pragma endregion
#pragma region 3 - FunctionRecvSUB
/// <summary>
/// Function for reciving topics from subscriber
/// </summary>
/// <param name="lpParam"></param>
/// <returns></returns>
DWORD WINAPI FunctionRecvSUB(LPVOID lpParam)
{
	printf("PubSubEngine ready for reciving messages from subscriber.\n");

	SOCKET acceptedSocketSUB = *(SOCKET*)lpParam;
	int iResult;
	HANDLE hSendToSUB = NULL;

	FD_SET set;
	timeval timeVal;
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;

	do
	{
		char messageSize[4];

		FD_ZERO(&set);
		FD_SET(acceptedSocketSUB, &set);

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
			//receiving message length
			iResult = ReceiveFunction(acceptedSocketSUB, messageSize, sizeof(int));
		}
		if (iResult <= 0)
		{
			char stringSocket[DEFAULT_BUFFLEN];
			_itoa(acceptedSocketSUB, stringSocket, 10);
			printf("Subcriber number %s is shutdown.\n", stringSocket);

			//cleaning sutrcture when subscriber shutdown
			CleanSUBStructure(stringSocket, &subHead);

			//it stops sending message to subscriber it has shut down
			CloseHandle(hSendToSUB);
			closesocket(acceptedSocketSUB);
			break;
		}


		int len = *(int*)messageSize;
		char* recvBuffer = (char*)malloc(len - 4);

		iResult = select(0, &set, NULL, NULL, &timeVal);
		if (iResult > 0)
		{
			iResult = ReceiveFunction(acceptedSocketSUB, recvBuffer, len - 4);


			char* topicSUB = recvBuffer;
			char* socketString = (char*)malloc(sizeof(char)* DEFAULT_BUFFLEN);

			_itoa(acceptedSocketSUB, socketString, 10);
			printf("TOPIC: '%s' from subscriber number %s\n", topicSUB, socketString);
			printf("--------------------------------\n");


			if (GetElementFromList(socketString, subHead) == false)
			{
				AddSocketToList(topicSUB, socketString, &topicHead, &subHead);
				int d = 0;

				//make thread for sending data to subscriber
				hSendToSUB = CreateThread(NULL, NULL, &FunctionSendSUB, (void*)&acceptedSocketSUB, NULL, NULL);

			}
			else
			{
				if (topicHead != NULL)
				{
					AddSocketToQueue(topicSUB, socketString, &topicHead);
					int y = 0;

				}
			}
		}

		if (iResult <= 0)
		{
			char stringSocket[DEFAULT_BUFFLEN];
			_itoa(acceptedSocketSUB, stringSocket, 10);
			printf("Subcriber number %s is shutdown.\n", stringSocket);

			//cleaning sutrcture when subscriber shutdown
			CleanSUBStructure(stringSocket, &subHead);

			//it stops sending message to subscriber it has shut down
			CloseHandle(hSendToSUB);
			closesocket(acceptedSocketSUB);
			free(recvBuffer);
			break;
		}

	} while (1);

	return 0;
}
#pragma endregion
#pragma region 4 - FunctionSendSUB
/// <summary>
/// Function for send message to subscriber and delete sended message
/// </summary>
/// <param name="lpParam"></param>
/// <returns>DWORD</returns>
DWORD WINAPI FunctionSendSUB(LPVOID lpParam)
{
	SOCKET acceptedSocketSUB = *(SOCKET*)lpParam;
	int iResult = NULL;

	char* socketString = (char*)malloc(sizeof(char)* DEFAULT_BUFFLEN);
	_itoa(acceptedSocketSUB, socketString, 10);

	char* messageSUB = (char*)malloc(sizeof(char)* DEFAULT_BUFFLEN);
	char* sendBuffer = (char*)malloc(sizeof(char)* DEFAULT_BUFFLEN + 4);		//+4 for message length


	//uzimam socket-sa kog skidam poruke iz queue-a
	NodeList* currentSocket = GetSocketFromList(socketString, subHead);

	if (currentSocket != NULL)
	{
		do
		{
			printf("Brisnje treba da radi...%s\n", _itoa(acceptedSocketSUB, socketString, 10));

			if (currentSocket->frontElement != NULL)
			{
				*((int*)sendBuffer) = (int)strlen(messageSUB);
				strcpy(sendBuffer + 4, currentSocket->frontElement->data);

				iResult = SendFunction(acceptedSocketSUB, sendBuffer, *((int*)sendBuffer));
				if (iResult == SOCKET_ERROR)
				{
					printf("Send failed with error: %d\n", WSAGetLastError());
					closesocket(acceptedSocketSUB);
					break;
				}

				RemoveMessageFromQueue(&currentSocket);
			}
			else
			{
				Sleep(5000);
			}

			currentSocket = GetSocketFromList(socketString, subHead);
			if (currentSocket == NULL)
			{
				break;
			}

		} while (1);
	}


	free(messageSUB);
	free(sendBuffer);
	return 0;
}
#pragma endregion
