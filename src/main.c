#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include "connectionhandler.h"
#include "broadcastagent.h"
#include "util.h"
#define DEFAULT_PORT 8111
#define MIN_PORT 1024
#define MAX_PORT 65536

void sig_handler(int signo)
{
	switch(signo){
		case SIGINT:
			infoPrint("Ctrl+C -> server is shutting down");
            broadcastAgentCleanup();
			exit(0);
			break;
		case SIGTERM:
			infoPrint("Terminate command -> server is shutting down");
            broadcastAgentCleanup();
			exit(0);
			break;
		default:
			break;
	}
}

int main(int argc, char **argv)
{
	utilInit(argv[0]);
	infoPrint("Chat server, group 01");
	debugEnable();

    // get port of arguments, 10 -> decimal
	unsigned long port = argc >= 2 ? strtol(argv[1], NULL, 10) : DEFAULT_PORT;
	if(errno != 0)
	{
		errnoPrint("Invalid port %s", argv[1]);
		return EXIT_FAILURE;
	}

	if(port < MIN_PORT || port > MAX_PORT)
	{
		errorPrint("Port %s is not allowed!", argv[1]);
		return EXIT_FAILURE;
	}

	infoPrint("Port: %lu", port);
	
	if(broadcastAgentInit(port) < 0)
	{
		exit(EXIT_FAILURE);
	}
	signal(SIGINT,sig_handler);
    signal(SIGTERM, sig_handler);
	
	const int result = connectionHandler((in_port_t)port);

	broadcastAgentCleanup();

	return result != -1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
