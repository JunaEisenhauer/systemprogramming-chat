#ifndef USER_H
#include "network.h"
#define USER_H


typedef struct User
{
	struct User *prev;
	struct User *next;
	int client_sock;		//socket for client
	char name[USERNAME_MAX+1];	// +1 for null termination
	pthread_t threadId;
} User;

void userLock();

void userUnlock();

User* addUser(int client_sock, char *name);

void removeUser(User *user);

void sendUserAdded(User *userToAdd);

void sendUserRemoved(User *userToRemove, uint8_t code);

void sendText(Message* serverToClient);

User* getUser(char *name);

#endif
