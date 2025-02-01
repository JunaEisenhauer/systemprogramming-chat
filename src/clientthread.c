#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>
#include <time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include "clientthread.h"
#include "user.h"
#include "broadcastagent.h"
#include "util.h"
#include "network.h"

#define CMD_PAUSE "/pause"
#define CMD_RESUME "/resume"
#define CMD_KICK "/kick "

#define MESSAGE_PERMISSION_DENIED "Permission denied!"
#define MESSAGE_INVALID_CMD "Invalid command!"
#define MESSAGE_CHAT_PAUSED "Chat paused."
#define MESSAGE_CHAT_RESUMED "Chat resumed."
#define MESSAGE_ALREADY_PAUSED "Chat already paused!"
#define MESSAGE_NOT_PAUSED "Chat is not paused!"
#define MESSAGE_QUEUE_FULL "Discarded your message, because the chat is paused and the send queue is full!"
#define MESSAGE_USER_NOT_FOUND "User '%s' does not exist!"
#define MESSAGE_USER_IS_SELF "You can not kick yourself!"
#define MESSAGE_KICK "'%s' kicked from the server."

static mqd_t mq;

bool isAdmin(User *user)
{
	if(strcmp(user->name, "Admin") == 0)
	{
		return true;
	}
	return false;
}

int handleCommand(User *user, char *text)
{
	int client_sock = user->client_sock;
	uint64_t timestamp = time(NULL);
	
	// handle command
	if(strcasecmp(text, CMD_PAUSE) == 0)
	{
		if(isAdmin(user))
		{
			if(isPaused())
			{
				// already paused
				Message *alreadyPausedMessage = malloc(sizeof(Message));
				if(alreadyPausedMessage == NULL)
				{
					errorPrint("Not enough memory!");
					exit(EXIT_FAILURE);
					return REMOVED_ERROR;
				}
				createServerToClient(alreadyPausedMessage, timestamp, "\0", MESSAGE_ALREADY_PAUSED);
					
				sendMessage(client_sock, alreadyPausedMessage);
					
				free(alreadyPausedMessage);
			}
			else
			{
				// pause broadcastagent
				broadcastPause();
					
				Message *pauseMessage = malloc(sizeof(Message));
				if(pauseMessage == NULL)
				{
					errorPrint("Not enough memory!");
					exit(EXIT_FAILURE);
					return REMOVED_ERROR;
				}
				createServerToClient(pauseMessage, timestamp, "\0", MESSAGE_CHAT_PAUSED);
					
				sendText(pauseMessage);
					
				free(pauseMessage);
			}
		}
		else
		{
			// permission denied
			Message *permissionMessage = malloc(sizeof(Message));
			if(permissionMessage == NULL)
			{
				errorPrint("Not enough memory!");
				   exit(EXIT_FAILURE);
				return REMOVED_ERROR;
			}
			createServerToClient(permissionMessage, timestamp, "\0", MESSAGE_PERMISSION_DENIED);
				
			sendMessage(client_sock, permissionMessage);
				
			free(permissionMessage);
		}
	}
	else if(strcasecmp(text, CMD_RESUME) == 0)
	{
		if(isAdmin(user))
		{
			if(!isPaused())
			{
				// already paused
				Message *notPausedMessage = malloc(sizeof(Message));
				if(notPausedMessage == NULL)
				{
					errorPrint("Not enough memory!");
					exit(EXIT_FAILURE);
					return REMOVED_ERROR;
				}
				createServerToClient(notPausedMessage, timestamp, "\0", MESSAGE_NOT_PAUSED);
					
				sendMessage(client_sock, notPausedMessage);
					
				free(notPausedMessage);
			}
			else
			{
				// resume broadcastagent
				broadcastResume();
					
				Message *resumeMessage = malloc(sizeof(Message));
				if(resumeMessage == NULL)
				{
					errorPrint("Not enough memory!");
					exit(EXIT_FAILURE);
					return REMOVED_ERROR;
				}
				createServerToClient(resumeMessage, timestamp, "\0", MESSAGE_CHAT_RESUMED);
					
				sendText(resumeMessage);
					
				free(resumeMessage);
			}
		}
		else
		{
			// permission denied
			Message *permissionMessage = malloc(sizeof(Message));
			if(permissionMessage == NULL)
			{
				errorPrint("Not enough memory!");
				exit(EXIT_FAILURE);
				return REMOVED_ERROR;
			}
			createServerToClient(permissionMessage, timestamp, "\0", MESSAGE_PERMISSION_DENIED);
				
			sendMessage(client_sock, permissionMessage);
				
			free(permissionMessage);
		}
	}
	else if(strncasecmp(text, CMD_KICK, strlen(CMD_KICK)) == 0)
	{
		if(isAdmin(user))
		{
			int cmdLength = strlen(CMD_KICK);
			int nameLength = strlen(text) - cmdLength;
			char kickedUsername[USERNAME_MAX+1];
			strncpy(kickedUsername, text + cmdLength, nameLength);
			kickedUsername[nameLength] = '\0';
			
			User* kickedUser = getUser(kickedUsername);
			
			if(kickedUser == user)
			{
				// user is self
				Message *userIsSelfMessage = malloc(sizeof(Message));
				if(userIsSelfMessage == NULL)
				{
					errorPrint("Not enough memory!");
					exit(EXIT_FAILURE);
					return REMOVED_ERROR;
				}
				
				createServerToClient(userIsSelfMessage, timestamp, "\0", MESSAGE_USER_IS_SELF);
					
				sendMessage(client_sock, userIsSelfMessage);
					
				free(userIsSelfMessage);
			}
			else
			{
				if(kickedUser == NULL)
				{
					// user not found
					Message *userNotFoundMessage = malloc(sizeof(Message));
					if(userNotFoundMessage == NULL)
					{
						errorPrint("Not enough memory!");
						exit(EXIT_FAILURE);
						return REMOVED_ERROR;
					}
					char msg[TEXT_MAX];
					sprintf(msg, MESSAGE_USER_NOT_FOUND, kickedUsername);
					createServerToClient(userNotFoundMessage, timestamp, "\0", msg);
						
					sendMessage(client_sock, userNotFoundMessage);
						
					free(userNotFoundMessage);
				}
				else
				{
					// kick user
					
					pthread_cancel(kickedUser->threadId);
					debugPrint("Client thread stopping.");
					close(kickedUser->client_sock);
					infoPrint("Connection closed by server");
					
					infoPrint("User '%s' removed", kickedUser->name);
					
					userLock();
					sendUserRemoved(kickedUser, REMOVED_KICKED);
					removeUser(kickedUser);
					userUnlock();
				}
			}
		}
		else
		{
			// permission denied
			Message *permissionMessage = malloc(sizeof(Message));
			if(permissionMessage == NULL)
			{
				errorPrint("Not enough memory!");
				   exit(EXIT_FAILURE);
				return REMOVED_ERROR;
			}
			createServerToClient(permissionMessage, timestamp, "\0", MESSAGE_PERMISSION_DENIED);
				
			sendMessage(client_sock, permissionMessage);
				
			free(permissionMessage);
		}
	}
	else
	{
		// command not found
		Message *invalidCmdMessage = malloc(sizeof(Message));
		if(invalidCmdMessage == NULL)
		{
			errorPrint("Not enough memory!");
			exit(EXIT_FAILURE);
			return REMOVED_ERROR;
		}
		createServerToClient(invalidCmdMessage, timestamp, "\0", MESSAGE_INVALID_CMD);
			
		sendMessage(client_sock, invalidCmdMessage);
			
		free(invalidCmdMessage);
	}
	
	return MESSAGE_OK;
}

int handleText(User *user, char *text)
{
	int client_sock = user->client_sock;
	uint64_t timestamp = time(NULL);
	
	// handle text
	Message *serverToClient = malloc(sizeof(Message));
	if(serverToClient == NULL)
	{
		errorPrint("Not enough memory!");
        exit(EXIT_FAILURE);
		return REMOVED_ERROR;
	}
	createServerToClient(serverToClient, timestamp, user->name, text);
	
	// Add message to message queue
	struct timespec timeout = { 
		.tv_sec = 1, 
		.tv_nsec = 0
	};
	if (mq_timedsend(mq, (const char *) serverToClient, sizeof(Message), 0, &timeout) < 0)
	{
		if (errno == ETIMEDOUT) {
			errorPrint("Message queue full!");
			Message *mqFullMessage = malloc(sizeof(Message));
			if(mqFullMessage == NULL)
			{
				errorPrint("Not enough memory!");
			    exit(EXIT_FAILURE);
				return REMOVED_ERROR;
			}
			createServerToClient(mqFullMessage, timestamp, "\0", MESSAGE_QUEUE_FULL);
			
			sendMessage(client_sock, mqFullMessage);
			
			free(mqFullMessage);
			
			
		} else {
			errnoPrint("Failed to enqueue message!");
		}
	}
	
	free(serverToClient);
	
	return MESSAGE_OK;
}

int handleMessage(User *user)
{
	int client_sock = user->client_sock;

	// recieve clientToServer message
	Message *clientToServer = malloc(sizeof(Message));
	if(clientToServer == NULL)
	{
		errorPrint("Not enough memory!");
		exit(EXIT_FAILURE);
		return REMOVED_ERROR;
	}
	int bytesHeader = recieveHeader(client_sock, clientToServer);
	if(bytesHeader < 0)
	{
		errnoPrint("Connection error");
		return REMOVED_ERROR;
	}
	if(bytesHeader == 0)
	{
		infoPrint("Connection closed by client");
		return REMOVED_CONNECTION_CLOSED;
	}

	if(clientToServer->header.type != TYPE_CLIENT_TO_SERVER)
	{
		errorPrint("Client sent wrong message type");
		return REMOVED_ERROR;
	}
	
	if(clientToServer->header.length <= 0)
	{
		errorPrint("Client has sent too short message");
		return REMOVED_ERROR;
	}

	if(clientToServer->header.length > TEXT_MAX)
	{
		errorPrint("Client has sent too long message");
		return REMOVED_ERROR;
	}

	int bytesC2S = recieveClientToServer(client_sock, clientToServer->header.length, clientToServer);
	if(bytesC2S < 0)
	{
		errnoPrint("Connection error");
		return REMOVED_ERROR;
	}
	if(bytesC2S == 0)
	{
		infoPrint("Connection closed by client");
		return REMOVED_CONNECTION_CLOSED;
	}

	char text[TEXT_MAX];
	strncpy(text, clientToServer->body.clientToServer.text, clientToServer->header.length);
	text[clientToServer->header.length] = '\0';
	free(clientToServer);


	// handle text
	if (text[0] == '/')
	{
		handleCommand(user, text);
	}
	else
	{
		handleText(user, text);
	}
	
	return MESSAGE_OK;
}

uint8_t checkLogin(Message *message, char *name)
{
	// check package
	if(message->header.length < sizeof(message->body.loginRequest.magic) + sizeof(message->body.loginRequest.version))
	{
		errorPrint("Client has sent too short message");
		return LOGIN_ERROR;
	}
	
	if(message->header.length > sizeof(message->body.loginRequest.magic) + sizeof(message->body.loginRequest.version) + USERNAME_MAX)
	{
		errorPrint("Client has sent too long message");
		return LOGIN_ERROR;
	}

	if(message->body.loginRequest.magic != LOGIN_REQUEST_MAGIC)
	{
		errorPrint("Client has sent wrong magic number");
		return LOGIN_ERROR;
	}

	if(message->body.loginRequest.version != VERSION)
	{
		errorPrint("Client has other version");
		return LOGIN_VERSIONMISMATCH;
	}

	// check name
	int nameLength = message->header.length - sizeof(message->body.loginRequest.magic) - sizeof(message->body.loginRequest.version);
	strncpy(name, message->body.loginRequest.name, nameLength);
	name[nameLength] = '\0';

	if(getUser(name) != NULL)
	{
		errorPrint("Name is already taken");
		return LOGIN_NAMETAKEN;	
	}
    
	unsigned int i;
	for(i = 0; i < strlen(name); i++)
	{
		if(name[i] < USERNAME_ASCII_MIN || name[i] > USERNAME_ASCII_MAX || name[i] == '\'' || name[i] == '\"' || name[i] == '`')
		{
			errorPrint("Name is invalid");
			return LOGIN_NAMEINVALID;
		}
	}

	return LOGIN_SUCCESS;
}

User* handleLogin(int client_sock)
{
	User *user;

	// recieve loginRequest message
	Message *loginRequest = malloc(sizeof(Message));
	if(loginRequest == NULL)
	{
		errorPrint("Not enough memory!");
		exit(EXIT_FAILURE);
		return NULL;
	}

	int bytesHeader = recieveHeader(client_sock, loginRequest);
	if(bytesHeader < 0)
	{
		errnoPrint("Connection error");
		return NULL;
	}
	if(bytesHeader == 0)
	{
		infoPrint("Connection closed by client");
		return NULL;
	}

	if(loginRequest->header.type != TYPE_LOGIN_REQUEST)
	{
		errorPrint("Client sent wrong message type");
		return NULL;
	}

	int bytesLR = recieveLoginRequest(client_sock, loginRequest->header.length, loginRequest);
	if(bytesLR < 0)
	{
		errnoPrint("Connection error");
		return NULL;
	}
	if(bytesLR == 0)
	{
		infoPrint("Connection closed by client");
		return NULL;
	}
	

	userLock();

	char name[USERNAME_MAX+1];
	uint8_t loginStatus = checkLogin(loginRequest, name);

	free(loginRequest);

	// send loginResponse
	if(loginStatus != LOGIN_ERROR)
	{
		Message *loginResponse = malloc(sizeof(Message));
		if(loginResponse == NULL)
		{
			errorPrint("Not enough memory!");
			userUnlock();
			exit(EXIT_FAILURE);
			return NULL;
		}
		createLoginResponse(loginResponse, loginStatus, SERVER_NAME);
		sendMessage(client_sock, loginResponse);
		free(loginResponse);
	}

	if(loginStatus != LOGIN_SUCCESS)
	{
		errorPrint("Login requests failed with code %d", loginStatus);
		userUnlock();
		return NULL;
	}

	user = addUser(client_sock, name);
	
	sendUserAdded(user);
	
	userUnlock();

	infoPrint("User '%s' added", user->name);

	return user;
}

int handleClient(int client_sock)
{
	User *user = handleLogin(client_sock);

	if(user == NULL)
	{
		return -1;
	}
	
	int disconnectCause = REMOVED_ERROR;
	// endless loop for recieving messages
	for(;;)
	{
		disconnectCause = handleMessage(user);
		if(disconnectCause != MESSAGE_OK)
		{
			break;
		}
	}

	infoPrint("User '%s' removed", user->name);
	userLock();
	sendUserRemoved(user, disconnectCause);
	removeUser(user);
	userUnlock();
	return 0;
}

void *clientthread(void *arg)
{
	debugPrint("Client thread started.");
	
	int *client_sock = (int *)arg;
	
	// open message queue for write only
	mq = mq_open("/broadcastAgent_mq", O_WRONLY);
	if(mq < 0)
	{
		errnoPrint("Failed to open Message Queue!");
		return NULL;
	}
	
	handleClient(*client_sock);

	debugPrint("Client thread stopping.");
	
	return NULL;	
}
