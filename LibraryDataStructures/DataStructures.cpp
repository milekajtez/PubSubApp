#include "DataStructures.h"

#define DEFAULT_BUFFLEN 1024
HANDLE hSemaphoreList;

#pragma region 1 - UnPackingMessage
/// <summary>
/// Function for unpacking message from publisher
/// </summary>
/// <param name="recvBuffer"></param>
/// <param name="topic"></param>
/// <param name="message"></param>
void UnPackingMessage(char* recvBuffer, char** topic, char** message)
{
	int topicCounter = 0;
	int messageCounter = 0;
	int delemiter = 0;

	for (int i = 0; i < (int)strlen(recvBuffer); i++)
	{
		if (recvBuffer[i] == '|')
		{
			delemiter = 1;
		}
		else
		{
			if (delemiter == 0 && recvBuffer != '\0')
			{
				topicCounter++;
			}

			if (delemiter == 1 && recvBuffer != '\0')
			{
				messageCounter++;
			}
		}
	}

	*topic = (char*)malloc(topicCounter);
	*message = (char*)malloc(messageCounter);

	topicCounter = 0;
	messageCounter = 0;
	delemiter = 0;
	for (int i = 0; i < (int)strlen(recvBuffer); i++)
	{
		if (recvBuffer[i] == '|')
		{
			delemiter = 1;
		}
		else
		{
			if (delemiter == 0 && recvBuffer != '\0')
			{
				*(*topic + topicCounter) = recvBuffer[i];
				topicCounter++;
			}

			if (delemiter == 1 && recvBuffer != '\0')
			{
				*(*message + messageCounter) = recvBuffer[i];
				messageCounter++;
			}
		}
	}

	*(*topic + topicCounter) = '\0';
	*(*message + messageCounter) = '\0';

}
#pragma endregion
#pragma region 2 - GetElementFromList
/// <summary>
/// Function whitch checks if the topic in the structure (list of topics) 
/// </summary>
/// <param name="name"></param>
/// <param name="head"></param>
/// <returns></returns>
bool GetElementFromList(char* name, NodeList* head)
{
	WaitForSingleObject(hSemaphoreList, INFINITE);
	while (head != NULL && head->name != NULL)
	{
		if (strcmp(head->name, name) == 0)
		{
			//we find the name of topic in topic list
			return true;
		}
		head = head->next;
	}
	ReleaseSemaphore(hSemaphoreList, 1, NULL);
	return false;
}
#pragma endregion
#pragma region 3 - AddTopicToList
/// <summary>
/// Function for add topic to list od topics
/// </summary>
/// <param name="topic"></param>
/// <param name="topicHead"></param>
/// <returns>NodeList*</returns>
void AddTopicToList(char* topic, NodeList** topicHead) 
{
	WaitForSingleObject(hSemaphoreList, INFINITE);

	//alocation memory for topic and queue
	NodeList* newTopic = (NodeList*)malloc(sizeof(NodeList));
	newTopic->frontElement = (Queue*)malloc(sizeof(Queue));
	newTopic->rearElement = (Queue*)malloc(sizeof(Queue));

	newTopic->hSemaphore = CreateSemaphore(0, 0, 1, NULL);
	ReleaseSemaphore(newTopic->hSemaphore, 1, NULL);
	newTopic->name = topic;
	newTopic->next = NULL;
	newTopic->frontElement = NULL;
	newTopic->rearElement = NULL;

	if (*topicHead == NULL)
	{
		*topicHead = newTopic;
	}
	else
	{
		NodeList* currentT = *topicHead;
		while (currentT->next != NULL)
		{
			currentT = currentT->next;
		}

		currentT->next = newTopic;
	}

	ReleaseSemaphore(hSemaphoreList, 1, NULL);
}
#pragma endregion
#pragma region 4 - AddMessageToQueue
/// <summary>
/// Function for add new mesage to all queues with valid socket
/// </summary>
/// <param name="topic"></param>
/// <param name="message"></param>
/// <param name="topicHead"></param>
/// <param name="subHead"></param>
void AddMessageToQueue(char* topic, char* message, NodeList** topicHead, NodeList** subHead) 
{
	WaitForSingleObject((*topicHead)->hSemaphore, INFINITE);

	NodeList* currentT = *topicHead;
	while (currentT != NULL) 
	{
		if (strcmp(currentT->name, topic) == 0) 
		{
			break;
		}
		currentT = currentT->next;
	}

	if (currentT != NULL) 
	{
		Queue* currentQ = currentT->frontElement;
		while (currentQ != NULL) 
		{
			NodeList* currentS = *subHead;
			while (currentS != NULL) 
			{
				if (strcmp(currentQ->data, currentS->name) == 0) 
				{
					break;
				}
				currentS = currentS->next;
			}

			if (currentS != NULL) 
			{
				Queue* newMessage = (Queue*)malloc(sizeof(Queue));
				newMessage->data = message;
				newMessage->next = NULL;

				if (currentS->frontElement == NULL) 
				{
					currentS->frontElement = newMessage;
					currentS->rearElement = newMessage;
				}
				else 
				{
					currentS->rearElement->next = newMessage;
					currentS->rearElement = newMessage;

					if (currentS->frontElement->next == NULL) 
					{
						currentS->frontElement->next = newMessage;
					}
				}
			}

			currentQ = currentQ->next;
		}
	}

	ReleaseSemaphore((*topicHead)->hSemaphore, 1, NULL);
}
#pragma endregion
#pragma region 5 - AddSocketToList
/// <summary>
/// Function for add new socket
/// </summary>
/// <param name="topic"></param>
/// <param name="socketString"></param>
/// <param name="topicHead"></param>
/// <param name="subHead"></param>
/// <returns>NodeList*</returns>
void AddSocketToList(char* topic, char* socketString, NodeList** topicHead, NodeList** subHead) 
{
	WaitForSingleObject(hSemaphoreList, INFINITE);

	//alocatation memory for socket and queue
	NodeList* newSocket = (NodeList*)malloc(sizeof(NodeList));
	newSocket->frontElement = (Queue*)malloc(sizeof(Queue));
	newSocket->rearElement = (Queue*)malloc(sizeof(Queue));

	newSocket->hSemaphore = CreateSemaphore(0, 0, 1, NULL);
	ReleaseSemaphore(newSocket->hSemaphore, 1, NULL);
	newSocket->name = socketString;
	newSocket->next = NULL;
	newSocket->frontElement = NULL;
	newSocket->rearElement = NULL;


	if (*subHead == NULL)
	{
		*subHead = newSocket;
	}
	else 
	{
		NodeList* currentS = *subHead;
		while (currentS->next != NULL) 
		{
			currentS = currentS->next;
		}
		currentS->next = newSocket;
	}

	//check if the topic exists in the list of topics
	NodeList* currentT = *topicHead;
	while (currentT != NULL)
	{
		if (strcmp(currentT->name, topic) == 0)
		{
			break;
		}
		currentT = currentT->next;
	}

	if (currentT != NULL)
	{
		Queue* newSocket = (Queue*)malloc(sizeof(Queue));
		newSocket->data = socketString;
		newSocket->next = NULL;

		if (currentT->frontElement == NULL)
		{
			currentT->frontElement = newSocket;
			currentT->rearElement = newSocket;
		}
		else
		{
			currentT->rearElement->next = newSocket;
			currentT->rearElement = newSocket;

			if (currentT->frontElement->next == NULL)
			{
				currentT->frontElement->next = newSocket;
			}
		}
	}

	ReleaseSemaphore(hSemaphoreList, 1, NULL);
}
#pragma endregion
#pragma region 6 - AddSocketToQueue
/// <summary>
/// Function for add new socket to all queues with valid topic
/// </summary>
/// <param name="topic"></param>
/// <param name="socketString"></param>
/// <param name="topicHead"></param>
void AddSocketToQueue(char* topic, char* socketString, NodeList** topicHead) 
{
	WaitForSingleObject((*topicHead)->hSemaphore, INFINITE);
	NodeList* currentT = *topicHead;

	while (currentT != NULL)
	{
		if (strcmp(currentT->name, topic) == 0)
		{
			break;
		}
		currentT = currentT->next;
	}

	if (currentT != NULL)
	{
		Queue* currentHelpS = currentT->frontElement;
		while (currentHelpS->data != NULL)
		{
			if (strcmp(currentHelpS->data, socketString) == 0)
			{
				printf("Subscriber je vrec pretplacen na uneti topic.\n");
				return;
			}
		}

		Queue* newSocket = (Queue*)malloc(sizeof(Queue));
		newSocket->data = socketString;
		newSocket->next = NULL;

		if (currentT->frontElement == NULL)
		{
			currentT->frontElement = newSocket;
			currentT->rearElement = newSocket;
		}
		else
		{
			currentT->rearElement->next = newSocket;
			currentT->rearElement = newSocket;

			if (currentT->frontElement->next == NULL)
			{
				currentT->frontElement->next = newSocket;
			}
		}
	}

	ReleaseSemaphore((*topicHead)->hSemaphore, 1, NULL);
}
#pragma endregion
#pragma region 7 - GetSocketFromList
/// <summary>
/// Function for seet socket from list of sockets
/// </summary>
/// <param name="socketName"></param>
/// <param name="subHead"></param>
/// <returns>NodeList*</returns>
NodeList* GetSocketFromList(char* socketName, NodeList* subHead)
{
	if (subHead != NULL) 
	{
		WaitForSingleObject(subHead->hSemaphore, INFINITE);

		//first find socket in subHead
		NodeList* currentSUB = subHead;

		while (currentSUB != NULL)
		{
			if (strcmp(currentSUB->name, socketName) == 0)
			{
				break;		//find socket
			}
			else
			{
				currentSUB = currentSUB->next;
			}
		}

		ReleaseSemaphore(subHead->hSemaphore, 1, NULL);
		return currentSUB;
	}
	
	return NULL;
}
#pragma endregion
#pragma region 8 - RemoveMessageFromQueue
/// <summary>
/// Function for delete message from queue of messages
/// </summary>
/// <param name="currentSocket"></param>
void RemoveMessageFromQueue(NodeList** currentSocket)
{
	WaitForSingleObject((*currentSocket)->hSemaphore, INFINITE);

	if ((*currentSocket)->frontElement != NULL)
	{
		Queue* elementToDelete;
		elementToDelete = (*currentSocket)->frontElement;
		if ((*currentSocket)->frontElement == (*currentSocket)->rearElement)
		{
			(*currentSocket)->frontElement = NULL;
			(*currentSocket)->rearElement = NULL;
		}
		else
		{
			(*currentSocket)->frontElement = (*currentSocket)->frontElement->next;
		}
		free(elementToDelete);
	}
	ReleaseSemaphore((*currentSocket)->hSemaphore, 1, NULL);
}
#pragma endregion
#pragma region 9 - CleanSUBStructure
/// <summary>
/// Function for cleaning sockets from subscriber structure when subscriber shutdown
/// </summary>
/// <param name="stringSocket"></param>
/// <param name="subHead"></param>
void CleanSUBStructure(char* stringSocket, NodeList** subHead)
{
	WaitForSingleObject(hSemaphoreList, INFINITE);
	NodeList* currentS = *subHead;
	if (*subHead != NULL) {

		if (strcmp(currentS->name, stringSocket) == 0)
		{
			*subHead = currentS->next;
			currentS = NULL;
			ReleaseSemaphore(hSemaphoreList, 1, NULL);
		}
		else
		{
			while (currentS->next != NULL)
			{
				if (strcmp(currentS->next->name, stringSocket) == 0)
					break;
				currentS = currentS->next;
			}

			NodeList* deleteS = currentS->next;
			currentS->next = currentS->next->next;
			deleteS = NULL;
		}
	}

	ReleaseSemaphore(hSemaphoreList, 1, NULL);	
}
#pragma endregion
