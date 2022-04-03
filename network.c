
#include "network.h"
#include "geral.h"

/*Server *close_sockets (Server *closee)
{
	close(closee->TcpFd);
	close(closee->UdpFd);
	free(closee);
	return NULL;
}*/

int CreateTcpServer(char *port)
{
	int fd, enable=1;
	unsigned int porto = atoi(port);
	ssize_t n;
	struct sockaddr_in server_addr;

	fd = socket(AF_INET, SOCK_STREAM, 0); //tcp_socket
	if(fd==-1)exit(1);
	
	memset(&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(porto);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) ==-1)
	{
		fprintf(stderr, "Reuse:%s\n", strerror(errno));
		exit(1);
	};
	
	n = bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(n==-1)
	{
		fprintf(stderr, "Tcp bind:%s\n", strerror(errno));
		exit(1);
	}

	if(listen(fd, 5)==-1)
	{
		fprintf(stderr, "tcp listen: %s\n", strerror(errno));
		exit(1);
	};
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
	if(n==-1)
	{
		fprintf(stderr, "%s\n", strerror(errno));
		exit(1);
	};
	
	fprintf(stdout, "udp socket binded\n");
	
	return fd;
}

int selfInform(Node *pred, Node *this)
{
	int fd, errcode;
	ssize_t n;
	struct addrinfo hints, *res;
	char message[64];
	
	memset(message, '\0', sizeof(message));
	
	sprintf(message, "SELF %d %d.%s %d.%s\n", this->chave, this->chave, this->address, this->chave, this-> port);
	
	fprintf(stdout, "sending: %s", message);
	fprintf(stdout, "to: %d %s %s\n", pred->chave, pred->address, pred->port);
	
	if(pred->fd!=0)fd=pred->fd;
	else
	{
		fd=socket(AF_INET,SOCK_STREAM, 0);//TCP socket
		if(fd==-1) 
		{
			fprintf(stderr, "Socket: %s\n", strerror(errno));
			exit(1);
		}
	
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_INET;//IPv4
		hints.ai_socktype = SOCK_STREAM;//TCP socket
	
		errcode = getaddrinfo(pred->address, pred->port, &hints, &res);
		if (errcode!=0)
		{
			fprintf(stderr, "Addrinfo: %s\n", gai_strerror(errcode));
			exit(1);
		};
	
		n = connect(fd, res->ai_addr, res->ai_addrlen);
		if(n==-1)
		{
			fprintf(stderr, "Connect: %s\n", strerror(errno));
			exit(1);
		}
		freeaddrinfo(res);
	}
	n = write(fd, message, strlen(message));
	if(n==-1)
	{
		fprintf(stderr, "%s\n", strerror(errno));
		exit(1);
	};
		
	//close(fd);

	return fd;
}

int predInform(Node *suc, Node *old_suc)
{
	int fd, errcode;
	ssize_t n;
	struct addrinfo hints, *res;
	char message[64];
	
	memset(message, '\0', sizeof(message));
	
	sprintf(message, "PRED %d %d.%s %d.%s\n", suc->chave, suc->chave, suc->address, suc->chave, suc-> port);
	
	fprintf(stdout, "sending: %s", message);
	fprintf(stdout, "to: %d %s %s through %d\n",  old_suc->chave, old_suc->address, old_suc->port, old_suc->fd);
	
	if(old_suc->fd!=0)fd=old_suc->fd;
	else
	{
		fd=socket(AF_INET,SOCK_STREAM, 0);//TCP socket
		if(fd==-1) 
		{
			fprintf(stderr, "%s\n", strerror(errno));
			exit(1);
		}
	
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_INET;//IPv4
		hints.ai_socktype = SOCK_STREAM;//TCP socket
	
		errcode = getaddrinfo(old_suc->address, old_suc->port, &hints, &res);
		if (errcode!=0)	
		{
			fprintf(stderr, "%s\n", gai_strerror(errcode));
			exit(1);
		}
	
		n = connect(fd, res->ai_addr, res->ai_addrlen);
		if(n==-1) 
		{
			fprintf(stderr, "%s\n", strerror(errno));
			exit(1);
		}
		freeaddrinfo(res);
	}
	fprintf(stdout, "%d\n", fd);
	n = write(fd, message, strlen(message));
	if(n==-1)
	{
		fprintf(stderr, "%s\n", strerror(errno));
		exit(1);
	}
	
	fprintf(stdout, "sent\n");
	
	//close(fd);

	return fd;
}

void GenericTCPsend(Node *suc, char *message)
{
	int fd, errcode;
	ssize_t n;
	struct addrinfo hints, *res;
	
	fprintf(stdout, "sending: %s", message);
	fprintf(stdout, "to: %d %s %s\n", suc->chave, suc->address, suc->port);
	
	if(suc->fd!=0)(fd=suc->fd);
	else
	{
		fd=socket(AF_INET,SOCK_STREAM, 0);//TCP socket
		if(fd==-1) 
		{
			fprintf(stderr, "%s\n", strerror(errno));
			exit(1);
		}
	
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_INET;//IPv4
		hints.ai_socktype = SOCK_STREAM;//TCP socket
	
		errcode = getaddrinfo(suc->address, suc->port, &hints, &res);
		if (errcode!=0)	
		{
			fprintf(stderr, "%s\n", gai_strerror(errcode));
			exit(1);
		}
	
		n = connect(fd, res->ai_addr, res->ai_addrlen);
		if(n==-1) 
		{
			fprintf(stderr, "%s\n", strerror(errno));
			exit(1);
		}
		freeaddrinfo(res);
	}
	n = write(fd, message, strlen(message));
	if(n==-1)
	{
		fprintf(stderr, "%s\n", strerror(errno));
		exit(1);
	}
	
	fprintf(stdout, "sent\n");
	
	//close(fd);

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