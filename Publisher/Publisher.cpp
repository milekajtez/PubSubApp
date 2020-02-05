#include "..//LibraryTCPConnection/TCPConnection.h"

void Publish(SOCKET connectSocket, char* topic, char* message);

/// <summary>
/// Function that initialize Publish client
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
	char* topic = NULL;
	char message[DEFAULT_BUFFLEN];


	if (argc != 2)
	{
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}
	if (InitializeWindowsSockets() == false)
	{
		return 1;
	}

	if (ConnectSocketInitialization(&connectSocket, argv, PUBLISHER_PORT) == false)
	{
		WSACleanup();
		return 1;
	}


	while (id != 1)
	{
		printf(">>>>>>>>>>---MENU---<<<<<<<<<<\n");
		printf("1 --> SEND MESSAGE\n");
		printf("2 --> EXIT\n");
		printf("--------------------\n");
		option = _getch();

		switch (option - 48)
		{
		case 1:
			printf("Enter the length of topic message:");
			gets_s(topicLength, DEFAULT_BUFFLEN);

			if (PubSubSendVerification(topicLength, 0) == false)
			{
				continue;
			}
			
			topic = (char*)malloc(atoi(topicLength) + 1);	// + 1 for '\0'
			printf("Enter topic (max length of topic is: %d):", atoi(topicLength));
			gets_s(topic, atoi(topicLength) + 1);
			if (PubSubSendVerification(topic, 1) == false) 
			{
				continue;
			}

			printf("Enter message:");
			memset(message, 0, DEFAULT_BUFFLEN);
			gets_s(message, DEFAULT_BUFFLEN);
			if (PubSubSendVerification(message, 1) == false) 
			{
				continue;
			}

			//if all verifications are ok, publisher send topic and message to server
			Publish(connectSocket, topic, message);

			break;
		case 2:
			id = 1;
			printf("Exit sucessfully.\n");
			break;
		default:
			printf("Please enter a valid number (1 for send message or 0 for exit)\n");
			break;
		}
		printf("\n\n");

	}

	if(topic != NULL)
		free(topic);

	closesocket(connectSocket);
	WSACleanup();
	return 0;
}

#pragma region 1 - Publish function
/// <summary>
/// Function for publish message on server
/// </summary>
/// <param name="connectSocket"></param>
/// <param name="topic"></param>
/// <param name="message"></param>
void Publish(SOCKET connectSocket, char* topic, char* message)
{
	int iResult = 0;
	int messageCharacterCount = GetMessageLen(message);

	FD_SET set;						//zakomentarisacu ove inicjalizacije za select..jer u funkciji od send imam isto to
	timeval timeVal;				//pa cu da vidim da li radi
	FD_ZERO(&set);
	FD_SET(connectSocket, &set);
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;

	char* textAboutTopic = (char*)malloc(messageCharacterCount);

	/*
		Format of messageToSend variable:
			4 bytes for message length +
			bytes for topic +
			messageCharacterCount bytes for message +
			2 btyes for null terminators
	*/

    int len = 6 + (int)strlen(topic) + messageCharacterCount;
	char* messageToSend = (char*)malloc(len);
	memset(messageToSend, 0, len);

	*(int*)messageToSend = len;
	strcpy(messageToSend + 4, topic);
	strcpy(messageToSend + 4 + (int)strlen(topic), "|");
	strcpy(messageToSend + 5 + (int)strlen(topic), message);

	iResult = SendFunction(connectSocket, messageToSend, len);
	if (iResult == SOCKET_ERROR)
	{
		printf("Send failed with error: %ld\n", WSAGetLastError());
		free(textAboutTopic);
		free(messageToSend);
		closesocket(connectSocket);
		WSACleanup();
	}

	free(textAboutTopic);
	free(messageToSend);
}
#pragma endregion