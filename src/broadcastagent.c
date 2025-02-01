#include <pthread.h>
#include <mqueue.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdlib.h>
#include "broadcastagent.h"
#include "network.h"
#include "user.h"
#include "util.h"

#define MESSAGE_QUEUE_MAX_MSG 10
#define MODE 0644	// permissions for message queue (rwx)

static mqd_t messageQueue;
static pthread_t threadId;

static sem_t semaphore;
static bool paused = false;

static void *broadcastAgent(void *arg)
{
	while(true)
	{
		Message *message = malloc(sizeof(Message));
		if(message == NULL)
		{
			errorPrint("Not enough memory!");
			exit(EXIT_FAILURE);
			return NULL;
		}
		
		if(mq_receive(messageQueue, (char *) message, sizeof(Message), NULL) < 0)
		{
			errnoPrint("Failed to read message from message queue");
			return NULL;
		}
		
		if(sem_wait(&semaphore) < 0)
		{
			errnoPrint("Error on sem_wait while message");
			return NULL;
		}
		
		userLock();
		sendText(message);
		userUnlock();
		
		free(message);
		
		if(sem_post(&semaphore) < 0)
		{
			errnoPrint("Error on sem_post while message");
			return NULL;
		}
	}
	return arg;
}

int broadcastAgentInit()
{
	unsigned int initialValue = 1;
	int pshared = 0;	// Thread semephore
	sem_init(&semaphore, pshared, initialValue);

	struct mq_attr attr;
	attr.mq_flags = 0;		// ignored for mq_open
	attr.mq_maxmsg = MESSAGE_QUEUE_MAX_MSG;	// max # of messages on queue
	attr.mq_msgsize = sizeof(Message);	// max message size
	attr.mq_curmsgs = 0;	// ignored for mq_open
	
	messageQueue = mq_open("/broadcastAgent_mq", O_CREAT | O_EXCL | O_RDONLY, MODE, &attr);
	if(messageQueue == -1)
	{
		errnoPrint("Failed to create Message Queue!");
		return -1;
	}

	if(pthread_create(&threadId, NULL, broadcastAgent, NULL) < 0)
	{
		errnoPrint("Could not create broadcastagent thread!");
		return -1;
	}
	
	return 0;
}

void broadcastAgentCleanup()
{
	pthread_cancel(threadId);

    if (mq_close( messageQueue) < 0)
    {
        errnoPrint("Failed to close mq!");
    }

    if (mq_unlink("/broadcastAgent_mq") < 0)
    {
        errnoPrint("Failed to unlink mq!");
    }
    
    sem_destroy(&semaphore);
}

bool isPaused()
{
	return paused;
}

void broadcastPause()
{
	if(sem_wait(&semaphore) < 0)
	{
		errnoPrint("Error on sem_wait while pause");
	}
	else
	{
		paused = true;
		infoPrint("Broadcast paused!");
	}
}

void broadcastResume()
{
	if(sem_post(&semaphore) < 0)
	{
		errnoPrint("Error on sem_post while resume");
	}
	else
	{
		paused = false;
		infoPrint("Broadcast resumed!");
	}
}
