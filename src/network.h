#ifndef CHAT_PROTOCOL_H
#define CHAT_PROTOCOL_H
#include <stdint.h>

#define TYPE_LOGIN_REQUEST 0
#define TYPE_LOGIN_RESPONSE 1
#define TYPE_CLIENT_TO_SERVER 2
#define TYPE_SERVER_TO_CLIENT 3
#define TYPE_USER_ADDED 4
#define TYPE_USER_REMOVED 5

#define USERNAME_MAX 31
#define USERNAME_ASCII_MIN 33
#define USERNAME_ASCII_MAX 126
#define SERVERNAME_MAX 31
#define SERVER_NAME "Chatserver-group01"
#define TEXT_MAX 512

#define VERSION 0
#define LOGIN_REQUEST_MAGIC 0x0badf00d
#define LOGIN_RESPONSE_MAGIC 0xc001c001
#define LOGIN_SUCCESS 0
#define LOGIN_NAMETAKEN 1
#define LOGIN_NAMEINVALID 2
#define LOGIN_VERSIONMISMATCH 3
#define LOGIN_ERROR 255

#define REMOVED_CONNECTION_CLOSED 0
#define REMOVED_KICKED 1
#define REMOVED_ERROR 2

#define MESSAGE_OK 3

#pragma pack(1)
typedef struct LoginRequest
{
	uint32_t magic;
    	uint8_t version;
    	char name[USERNAME_MAX];
} LoginRequest;

typedef struct LoginResponse
{
    	uint32_t magic;
    	uint8_t code;
    	char serverName[SERVERNAME_MAX];
} LoginResponse;

typedef struct ClientToServer
{
	char text[TEXT_MAX];
} ClientToServer;

typedef struct ServerToClient
{
    	uint64_t timestamp;
    	char originalSender[USERNAME_MAX+1];	// +1 for null termination
   	char text[TEXT_MAX];
} ServerToClient;

typedef struct UserAdded
{
    	uint64_t timestamp;
    	char name[USERNAME_MAX];
} UserAdded;

typedef struct UserRemoved
{
    	uint64_t timestamp;
    	uint8_t code;
    	char name[USERNAME_MAX];
} UserRemoved;

typedef struct MessageHeader
{
	uint8_t type;
    	uint16_t length;
} MessageHeader;

typedef union MessageBody
{
	LoginRequest loginRequest;
    	LoginResponse loginResponse;
    	ClientToServer clientToServer;
    	ServerToClient serverToClient;
    	UserAdded userAdded;
    	UserRemoved userRemoved;
} MessageBody;

typedef struct Message
{
	MessageHeader header;
	union MessageBody body;
} Message;

int recieveHeader(int fd, Message *message);

int recieveLoginRequest(int fd, uint16_t length, Message *message);

int recieveClientToServer(int fd, uint16_t length, Message *message);

int sendMessage(int fd, Message *message);

void createLoginResponse(Message *message, uint8_t code, char *serverName);

void createUserAdded(Message *message, uint64_t timestamp, char *name);

void createUserRemoved(Message *message, uint64_t timestamp, uint8_t code, char *name);

void createServerToClient(Message *message, uint64_t timestamp, char *name, char *text);

#endif
