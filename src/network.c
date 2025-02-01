#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include "network.h"
#include "util.h"

int recieveHeader(int fd, Message *message)
{
	ssize_t bytes = recv(fd, &message->header, sizeof(MessageHeader), 0);
	if(bytes <= 0)
	{
		return (int)bytes;
	}
	message->header.length = ntohs(message->header.length);
	return (int)bytes;
}

int recieveLoginRequest(int fd, uint16_t length, Message *message)
{
	ssize_t bytes = recv(fd, &message->body.loginRequest, length, 0);
	if(bytes <= 0)
	{
		// connection lost
		return (int)bytes;
	}
	message->body.loginRequest.magic = ntohl(message->body.loginRequest.magic);
	return (int)bytes;
}

int recieveClientToServer(int fd, uint16_t length, Message *message)
{
	ssize_t bytes = recv(fd, &message->body.clientToServer, length, 0);
	if(bytes <= 0)
	{
		// connection lost
		return (int)bytes;
	}
	return (int)bytes;		
}

static void hostToNetwork(Message *message)
{
	message->header.length = htons(message->header.length);
	switch(message->header.type)
	{
		case TYPE_LOGIN_RESPONSE:
			message->body.loginResponse.magic = htonl(message->body.loginResponse.magic);
			break;
		case TYPE_SERVER_TO_CLIENT:
			message->body.serverToClient.timestamp = hton64u(message->body.serverToClient.timestamp);
			break;
		case TYPE_USER_ADDED:
			message->body.userAdded.timestamp = hton64u(message->body.userAdded.timestamp);
			break;
		case TYPE_USER_REMOVED:
			message->body.userRemoved.timestamp = hton64u(message->body.userRemoved.timestamp);
			break;
		default:
			// never reached!
			break;
	}
}

int sendMessage(int fd, Message *message)
{
	size_t length = sizeof(MessageHeader) + message->header.length;

	hostToNetwork(message);
	
	ssize_t bytes = send(fd, message, length, MSG_NOSIGNAL);
	if(bytes < 0)
	{
		errnoPrint("Failed to send message");
		hostToNetwork(message);
		return -1;	
	}
	
	hostToNetwork(message);

	return (int)bytes;
}

void createLoginResponse(Message *loginResponse, uint8_t code, char *serverName)
{
	loginResponse->header.type = TYPE_LOGIN_RESPONSE;
	loginResponse->header.length = sizeof(loginResponse->body.loginResponse.magic) + sizeof(code) + strlen(serverName);
	loginResponse->body.loginResponse.magic = LOGIN_RESPONSE_MAGIC;
	loginResponse->body.loginResponse.code = code;
	strcpy(loginResponse->body.loginResponse.serverName, serverName);
}

void createUserAdded(Message *userAdded, uint64_t timestamp, char *name)
{
	userAdded->header.type = TYPE_USER_ADDED;
	userAdded->header.length = sizeof(timestamp) + strlen(name);
	userAdded->body.userAdded.timestamp = timestamp;
	strcpy(userAdded->body.userAdded.name, name);
}

void createUserRemoved(Message *userRemoved, uint64_t timestamp, uint8_t code, char *name)
{
	userRemoved->header.type = TYPE_USER_REMOVED;
	userRemoved->header.length = sizeof(timestamp) + sizeof(code) + strlen(name);
	userRemoved->body.userRemoved.timestamp = timestamp;
	userRemoved->body.userRemoved.code = code;
	strcpy(userRemoved->body.userRemoved.name, name);
}

void createServerToClient(Message *serverToClient, uint64_t timestamp, char *name, char *text)
{
	serverToClient->header.type = TYPE_SERVER_TO_CLIENT;
	serverToClient->header.length = sizeof(timestamp) + USERNAME_MAX + 1 + strlen(text);
	serverToClient->body.serverToClient.timestamp = timestamp;
	strncpy(serverToClient->body.serverToClient.originalSender, name, USERNAME_MAX + 1);
	strcpy(serverToClient->body.serverToClient.text, text);
}
