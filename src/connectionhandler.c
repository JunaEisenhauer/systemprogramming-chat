#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "connectionhandler.h"
#include "clientthread.h"
#include "util.h"


static int createPassiveSocket(in_port_t port)
{
	int fd = -1;
	const int option = 1;
	// set the maximum queued clients	
	const int backlog = 1;
	
	// create IPv4 TCP socket
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd < 0)
	{
		errnoPrint("Could not create socket!");
		return -1;		
	}

	// set options -> free port on stop
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));	
	
	
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);


	// bind socket to address 
	if(bind(fd, (const struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		errnoPrint("Could not bind socket to address!");
		return -1;	
	}

	// mark socket to accept connections
	if(listen(fd, backlog) < 0)
	{
		errnoPrint("Could not mark socket to accept connections!");
		return -1;
	}

	return fd;
}

int connectionHandler(in_port_t port)
{
	const int fd = createPassiveSocket(port);
	if(fd == -1)
	{
		errnoPrint("Unable to create server socket");
		return -1;
	}
	
	infoPrint("Successfully created server socket!");
	
	// endless loop for accepting connections
	for(;;)
	{
		// accept a new incoming connection
		const int client_sock = accept(fd, NULL, NULL);		// accept is blocking
		if(client_sock < 0)
		{
			errnoPrint("Could not accept client connection!");
			continue;			
		}	
		
		// create user thread
		pthread_t thread;
		if(pthread_create(&thread, NULL, clientthread, (void *)&client_sock) < 0)
		{
			errnoPrint("Could not create client thread!");
			continue;
		}

		infoPrint("Client successfully connected!");
	}
	
	close(fd);
		
	return 0;	//never reached
}
