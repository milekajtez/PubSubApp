#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <conio.h>

struct Queue {
	char* data;
	Queue* next;
};

struct NodeList {
	char* name;
	NodeList* next;
	HANDLE hSemaphore;
	Queue *frontElement;
	Queue *rearElement;
};

void UnPackingMessage(char* recvBuffer, char** topic, char** message);
bool GetElementFromList(char* name, NodeList* head);
void AddTopicToList(char* topic, NodeList** topicHead);
void AddMessageToQueue(char* topic, char* message, NodeList** topicHead, NodeList** subHead);
void AddSocketToList(char* topic, char* socketString, NodeList** topicHead, NodeList** subHead);
void AddSocketToQueue(char* topic, char* socketString, NodeList** topicHead);
NodeList* GetSocketFromList(char* socketName, NodeList* subHead);
void RemoveMessageFromQueue(NodeList** currentSocket);
void CleanSUBStructure(char* stringSocket, NodeList** subHead);
