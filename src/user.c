#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "user.h"
#include "clientthread.h"
#include "util.h"

static pthread_mutex_t mutexLock = PTHREAD_MUTEX_INITIALIZER;
static User *userFront = NULL;
static User *userBack = NULL;

void userLock()
{
	pthread_mutex_lock(&mutexLock);
}

void userUnlock()
{
	pthread_mutex_unlock(&mutexLock);
}

User* addUser(int client_sock, char *name)
{
	User *newUser = malloc(sizeof(User));
	if(newUser == NULL)
	{
		errorPrint("Not enough memory!");
		exit(EXIT_FAILURE);
		return NULL;
	}

	// List empty
	if(userFront == NULL && userBack == NULL)
	{
		userFront = newUser;
		newUser->prev = NULL;
	}
	else
	{
		newUser->prev = userBack;
		userBack->next = newUser;
	}
	 
	userBack = newUser;

	newUser->next = NULL; 		
	newUser->client_sock = client_sock;
	strncpy(newUser->name, name, USERNAME_MAX+1);
	newUser->threadId = pthread_self();

	return newUser;	
}

void removeUser(User *user)
{
	if (user == userBack && user == userFront) 	// The user is the only user in the list
    	{
        	userBack = NULL;
        	userFront = NULL;
    	} 
    	else if (user == userBack) 	// The user is the last user in the list
    	{
        	userBack = user->prev;
        	user->prev->next = NULL;
    	} 
    	else if (user == userFront) 	// The user is the first user in the list
    	{
        	userFront = user->next;
        	user->next->prev = NULL;
    	} 
    	else 
    	{
        	user->prev->next = user->next;
       	 	user->next->prev = user->prev;
    	}

	free(user);
}

void sendUserAdded(User *userToAdd)
{
	uint64_t timestamp = time(NULL);	
	
	User *user = userFront;
	while (user != NULL)
	{
        // add new user to others
		Message *userAdded = malloc(sizeof(Message));
		if(userAdded == NULL)
		{
			errorPrint("Not enough memory!");
			exit(EXIT_FAILURE);
			break;
		}
		createUserAdded(userAdded, timestamp, userToAdd->name);
		sendMessage(user->client_sock, userAdded);
		free(userAdded);

        // add the other users to new user
		if (userToAdd != user)
		{
			Message *userAddedOther = malloc(sizeof(Message));
			if(userAddedOther == NULL)
			{
				errorPrint("Not enough memory!");
				exit(EXIT_FAILURE);
				break;
			}
			createUserAdded(userAddedOther, (uint64_t)0, user->name);
			sendMessage(userToAdd->client_sock, userAddedOther);
			free(userAddedOther);
		}

		user = user->next;
	}
}

void sendUserRemoved(User *userToRemove, uint8_t code)
{
	uint64_t timestamp = time(NULL);	

	User *user = userFront;
	while (user != NULL)
	{
		if(user == userToRemove)
		{
			user = user->next;
			continue;
		}
		
		// remove user to others
		Message *userRemoved = malloc(sizeof(Message));
		if(userRemoved == NULL)
		{
			errorPrint("Not enough memory!");
			exit(EXIT_FAILURE);
			break;
		}
		createUserRemoved(userRemoved, timestamp, code, userToRemove->name);
		sendMessage(user->client_sock, userRemoved);
		free(userRemoved);

		user = user->next;
	}
}

void sendText(Message* serverToClient)
{
	User *user = userFront;
	while (user != NULL)
	{
		sendMessage(user->client_sock, serverToClient);
		user = user->next;
	}
}

User* getUser(char *name)
{
	User *target = NULL;

	User *user = userFront;
	while (user != NULL)
	{
		if(strcmp(name, user->name) == 0)
		{
			target = user;
			break;
		}

		user = user->next;
	}

	return target;
}

