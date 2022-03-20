
#include "network.h"
#include "geral.h"

Server *New (char *address, char *port)
{
	Server *new;
	int udp_fd, tcp_fd;
	
	new = (Server *) malloc(sizeof(Server));
	
	new -> TcpFd = CreateTcpServer(address, port);
	new -> UdpFd = CreateUdpServer(address, port);

	return new;
}

int CreateTcpServer(char *address, char *port)
{
	return 0;
}

int CreateUdpServer(char *address, char *port)
{
	return 0;
}