
#include "network.h"
#include "geral.h"

Server *New (char *address, char *port)
{
	Server *new;
	
	new = (Server *) malloc(sizeof(Server));
	
	new -> TcpFd = CreateTcpServer(port);
	new -> UdpFd = CreateUdpServer(port);

	return new;
}

int CreateTcpServer(char *port)
{
	int fd;
	unsigned int porto = atoi(port);
	ssize_t n;
	struct sockaddr_in server_addr;

	fd = socket(AF_INET, SOCK_STREAM, 0); //tcp_socket
	if(fd==-1)exit(1);
	
	memset(&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(porto);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	n = bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(n==-1)exit(1);

	if(listen(fd, 5)==-1)exit(1);
	fprintf(stdout, "listening\n");
	
	return fd;
}

int CreateUdpServer(char *port)
{
	int fd;
	ssize_t n;
	unsigned int porto = atoi(port);
	struct sockaddr_in server_addr;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0); //socket udp
	if(fd==-1)exit(1);
	
	memset(&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(porto);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	n = bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(n==-1)exit(1);
	
	fprintf(stdout, "udp socket binded\n");
	
	return fd;
}