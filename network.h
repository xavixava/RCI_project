#ifndef NetworkOps
#define NetworkOps

/* Header file for the definition of the project  */
#include "geral.h"

typedef struct _server
{	
	int UdpFd, TcpFd;
}Server;

Server *New (char *address, char *port);

int CreateTcpServer(char *address, char *port);
int CreateUdpServer(char *address, char *port);

#endif