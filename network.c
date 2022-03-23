
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

void predEntry(Node *pred, Node *this)
{
	int fd, errcode;
	ssize_t n;
	struct addrinfo hints, *res;
	char buffer[4], message[64];
	
	buffer[4]='\0';
	memset(message, '\0', sizeof(message));
	
	sprintf(message, "SELF %d %d.%s %d.%s\n", this->chave, this->chave, this->address, this->chave, this-> port);
	
	fprintf(stdout, "%s", message);
	
	fd=socket(AF_INET,SOCK_STREAM, 0);//TCP socket
	if(fd==-1)	exit(1);
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;//IPv4
	hints.ai_socktype = SOCK_STREAM;//TCP socket
	
	fprintf(stdout, "pred: %d %s %s\n", pred->chave, pred->address, pred->port);

	errcode = getaddrinfo(pred->address, pred->port, &hints, &res);
	if (errcode!=0)exit(1);
	
	fprintf(stdout, "connecting\n");
	
	n = connect(fd, res->ai_addr, res->ai_addrlen);
	if(n==-1) exit(1);
	
	fprintf(stdout, "connected\n");
	
	n = write(fd, message, strlen(message));
	if(n==-1)exit(1);
	
	fprintf(stdout, "sent\n");
	
	n = read(fd, buffer, 128);
	if(n==-1)exit(1);
	write(1, "echo: ", 6);
	write(1, buffer, n);
	
	freeaddrinfo(res);
	close(fd);

	return;
}


	/*Connect by udp
	
	int fd, errcode;
	ssize_t n;
	socklen_t addrlen;
	struct addrinfo hints, *res;
	struct sockaddr_in addr;
	char buffer[4], message[64];
	
	
	buffer[4]='\0';
	memset(message, '\0', sizeof(message));
	
	sprintf(message, "SELF %d %d.%s %d.%s\n", this->chave, this->chave, this->address, this->chave, this-> port);
	
	fprintf(stdout, "%s", message);
	
	fd = socket(AF_INET, SOCK_DGRAM, 0); //socket udp
	if(fd==-1)exit(1);
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; //IPv4
	hints.ai_socktype = SOCK_DGRAM; //UDP socket
	
	fprintf(stdout, "pred: %d %s %s\n", pred->chave, pred->address, pred->port);

	errcode = getaddrinfo(pred->address, pred->port, &hints, &res);
	if (errcode!=0)exit(1);
	
	n = sendto(fd, message, strlen(message), 0, res->ai_addr, res->ai_addrlen);
	if(n==-1)exit(1);

	addrlen = sizeof(addr);
	n = recvfrom(fd, buffer, 48, 0, (struct sockaddr *)&addr, &addrlen);
	write(1, "echo: ", 6);
	write(1, buffer, n);*/