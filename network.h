#ifndef NetworkOps
#define NetworkOps

/* Header file for the definition of the project  */
#include "geral.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef struct _server
{	
	int UdpFd, TcpFd;
}Server;

Server *New (char *address, char *port);

int CreateTcpServer(char *port);
int CreateUdpServer(char *port);

#endif