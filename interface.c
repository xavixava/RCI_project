//interface.c

#include "interface.h"
#include "network.h"

#define max(A,B) ((A)>=(B)?(A):(B))

void interface(char **args)
{
	char buffer[128], *key, *address, *port, *info;
	int chave, newfd=0, maxfd = 0, counter, ring=0;
	Node *this, *suc=NULL, *pred=NULL, *chord=NULL, *aux=NULL;
	Server *fds;
	struct sockaddr_in addr;
	socklen_t addrlen;
	ssize_t n;
	fd_set rfds;
	struct timeval tv;
	
	tv.tv_sec = 30;
	tv.tv_usec = 0;
	
	memset(buffer, '\0', 128); //clear buffer
	 
	key = args[1];
	chave = atoi(key);
	if(chave<0|| chave>=MAX_NODES)  //check if the key is valid
	{
		fprintf(stdout, "Chave inválida\n");
		exit(1);
	}
	
	address = handle_args(args[2], key);
	port = handle_args(args[3], key);
	
	this = create(chave, address, port); //Saves Node info on Node struct
	fprintf(stdout, "%d %s %s\n", this->chave, this->address, this->port);//DELETE LATER
	
	addrlen=sizeof(addr);
	
	FD_ZERO(&rfds); //clear garbage for select
	while(1)
	{
		FD_SET(0,&rfds);	//select will wait for stdin
		if(ring==1){  		//only if is already in ring
		FD_SET(fds->TcpFd,&rfds);	//select will wait for tcp connection
		FD_SET(fds->UdpFd,&rfds);	//select will wait for udp message
		FD_SET(newfd,&rfds);	//select will wait for tcp message from current connection	
		
		maxfd = max(0, newfd);			//Check which is the max file descriptor so select won´t have to check all fds
		maxfd = max(maxfd, fds->UdpFd);
		maxfd = max(maxfd, fds->TcpFd);
		}
		counter = 0;
		
		counter = select(maxfd+1,&rfds, (fd_set*)NULL, (fd_set*)NULL, &tv);	
		
		if(counter < 0)		//ERROR
		{
			fprintf(stdout, "error in select!\n");
			exit(1);
		}
		else if (counter == 0){
			fprintf(stdout, "Please write something\n");	//timeout ocurred
			tv.tv_sec = 30;
		}
				
			if(ring==1)if(FD_ISSET(fds->TcpFd,&rfds)){		//New tcp connection
				FD_CLR(fds->TcpFd,&rfds);
				
				if((newfd=accept(fds->TcpFd,(struct sockaddr*)&addr,&addrlen))==-1)exit(1); //check if it will try to connect with more than one tcp socket at once
				fprintf(stdout, "Accepted connection\n");
				
				FD_SET(newfd,&rfds);	//making sure select knows that now it will have to check for a new fd(newfd - tcp socket)
				
				counter--;
				memset(buffer, '\0', 128);
			}
			if(newfd!=0) if(FD_ISSET(newfd,&rfds)){ //something written in accepted tcp socket
				FD_CLR(newfd,&rfds);
				
				n = read(newfd, buffer, 127);
				fprintf(stdout, "received: %s\n", buffer);
				
				
				info = handle_instructions(buffer);				
				if(strcmp("SELF", buffer)==0) //received SELF message 
				{
					key = info;
					address = handle_instructions(info);
					address = handle_args(address, key);
					port = handle_instructions(address);
					port = handle_args(port, key);
					port = newline(port);
					
					fprintf(stdout, "suc: %d %s %s\n", atoi(key), address, port);
					aux = suc;
					suc = create(atoi(key), address, port);
					
					if(pred == this){
						selfInform(suc, this);
						pred = suc;
					}
					else if(aux!=NULL)
					{
						if(compareDist(this, aux, suc, 0)==1)predInform(suc, aux);
						if (aux!=pred)freeNode(aux);
					}

					fprintf(stdout, "pred: %d %s %s\n", pred->chave, pred->address, pred->port);
					fprintf(stdout, "suc: %d %s %s\n", suc->chave, suc->address, suc->port);
					
				aux=NULL;
				}
				else if(strcmp("PRED", buffer)==0) //received PRED message
				{
					key = info; //transform this into a function later
					address = handle_instructions(info);
					address = handle_args(address, key);
					port = handle_instructions(address);
					port = handle_args(port, key);
					port = newline(port);
					
					fprintf(stdout, " %d %s %s\n", atoi(key), address, port);
					
					if(pred==suc||pred==NULL)pred = create(atoi(key), address, port);
					else
					{
						pred->chave = atoi(key);
						strcpy(pred->address, address);
						strcpy(pred->port, port);
					}
					
					fprintf(stdout, "pred: %d %s %s\n", pred->chave, pred->address, pred->port);
					
					if(pred->chave!=this->chave)selfInform(pred, this);
					else 
					{
						if(suc!=pred)freeNode(suc);
						suc=pred;
					}
				}
				
				
				close(newfd); //closing newfd and telling select to not check it anymore
				newfd=0;
				
				counter--;
				memset(buffer, '\0', 128);	
			}
			if(FD_ISSET(0,&rfds)){ //received a command
				FD_CLR(0,&rfds);
				
				if(fgets(buffer, 128, stdin)==NULL)	//ERROR
				{
					fprintf(stdout, "Nothing read\n");
					exit(1);
				}
				
				info = handle_instructions(buffer);
				
				if ((strcmp(buffer, "new\n") == 0)||(strcmp(buffer, "n\n") == 0)) 
				{
					fprintf(stdout, "Initiating a new ring\n");
					pred = this;
					suc = this;
					fds = New(this->address, this->port);
					ring = 1;
				}
				else if((strcmp(buffer, "show\n") == 0)||(strcmp(buffer, "s\n") == 0))
				{
					fprintf(stdout, "This Node:\n-key = %d\n-address = %s\n-port = %s\n", this->chave, this->address, this->port);
					if(pred!=NULL)fprintf(stdout, "Predecessor Node:\n-key = %d\n-address = %s\n-port = %s\n", pred->chave, pred->address, pred->port);
					if(suc!=NULL)fprintf(stdout, "Successor Node:\n-key = %d\n-address = %s\n-port = %s\n", suc->chave, suc->address, suc->port);
					
					if(chord==NULL)fprintf(stdout, "No chord\n");
					else fprintf(stdout, "Chord to:\n-key = %d\n-address = %s\n-port = %s\n", chord->chave, chord->address, chord->port);
				}
				else if((strcmp(buffer, "pentry") == 0)||(strcmp(buffer, "p") == 0))
				{
					key = info;
					address = handle_instructions(info);
					address = handle_args(address, key);
					port = handle_instructions(address);
					port = handle_args(port, key);
					port = newline(port);
					
					if(pred==NULL)pred = create(atoi(key), address, port);
					
					fprintf(stdout, "Pentry: %d %s %s\n", pred->chave, pred->address, pred->port);
		
					selfInform(pred, this);
		
					fds = New(this->address, this->port);
					ring = 1;
				}
				else if ((strcmp(buffer, "leave\n") == 0)||(strcmp(buffer, "l\n") == 0))
				{
					if(ring!=1)fprintf(stdout, "Not in ring\n");
					else
					{
						if(pred!=this && suc!=this)
						{
							predInform(pred, suc);
							freeNode(pred);
							if(pred!=suc)freeNode(suc);
						}
						pred = NULL;
						suc =NULL;
						fds = close_sockets(fds);
						if(newfd!=0)close(newfd);
						ring=0;
					}
				}
				else if ((strcmp(buffer, "exit\n") == 0)||(strcmp(buffer, "e\n") == 0))
				{
					freeNode(this);
					return;
				}
				else	fprintf(stdout, "Comando Desconhecido ou ainda não implementado\n");
				
				counter--;
				memset(buffer, '\0', 128);
			}
			if(ring==1)if(FD_ISSET(fds->UdpFd,&rfds)){ //something written in udp socket
				FD_CLR(fds->UdpFd,&rfds);
				
				n = recvfrom(fds->UdpFd, buffer, 128, 0, (struct sockaddr *)&addr, &addrlen);
				write(1, "received: ", 10);
				write(1, buffer, n);
				
				n = sendto(fds->UdpFd, "ACK", 3, 0, (struct sockaddr *)&addr, addrlen);
				if(n==-1)exit(1);
				
				counter--;
				memset(buffer, '\0', 128);
			}
		tv.tv_sec = 30;
	}
	
	return;
}

char *handle_args(char *arg, char *key)
{
	char *final, *aux, *point=".";
	
	aux = strstr(arg, point);
	final = aux+1;
	if(aux==NULL)
	{
		fprintf(stdout, "Por favor, formate devidamente os argumentos\n");
		exit(1);
	}	
	
	*aux='\0';
	
	//ADICIONAR VERIFICAÇÕES EXTRA, COMO VERIFICAR SE O ENDEREÇO E O PORTO SÃO NÚMEROS E OS SEUS TAMANHOS
	
	if(strcmp(arg, key)!=0)
	{	
		fprintf(stdout, "Por favor, use a mesma chave em todos os argumentos\n");
		exit(1);
	}
	
	return final;
}

char *handle_instructions(char *arg)
{
	char *final, *aux, *space=" ";
	
	aux = strstr(arg, space);
	
	
	if(aux==NULL && strcmp(arg, "n\n")!=0 && strcmp(arg, "new\n")!=0 && strcmp(arg, "show\n")!=0 && strcmp(arg, "s\n")!=0 && strcmp(arg, "leave\n")!=0 && strcmp(arg, "l\n")!=0 && strcmp(arg, "exit\n")!=0 && strcmp(arg, "e\n")!=0)
		{
			fprintf(stdout, "%s", arg);
			fprintf(stdout, "Por favor, formate devidamente as instruções\n");
			exit(1);
		}	
	
	if(aux!=NULL)
	{
		final = aux+1;
		*aux='\0';
	}
	
	return final;
}

char *newline(char *arg)
{
	char *aux;
	aux = strchr(arg, '\n');
	if(aux!=NULL) *aux = '\0';
	return arg;
}

/*
*	Compares distances of node a and b to this
*	Return value: 0 if dist(a, this)==dist(b, this)
*				  1 if dist(a, this)>dist(b, this)
*				 -1 if dist(a, this)<dist(b, this)
*	Extra info: if flag==0 no chords are considered, if flag==1consider chord				
*/
int compareDist(Node *this, Node *a, Node *b, int flag)
{
	int dista, distb;
	
	dista = dist(this, a);
	distb = dist(this, b);
	
	if(dista==distb)return 0;
	else if(max(dista, distb)==dista)return 1;
	else if(max(dista, distb)==distb)return -1;
	else return -2;
}

unsigned int dist(Node *this, Node *measuree)
{
	int dist=0;
	if (this->chave==measuree->chave)return 0;
	else if (this->chave>measuree->chave){
		dist = MAX_NODES+1-(this->chave);
		dist = dist + (measuree->chave);
	}
	else
	{
		dist = (measuree->chave)-(this->chave);
	}
	return dist;
}