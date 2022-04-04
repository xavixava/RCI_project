//interface.c

//AS LIGAÇÕES ENTRE OS PREDS E SUCS SÃO PERMANENTES, CORRIGIR ISSO

#include "interface.h"
#include "network.h"

#define max(A,B) ((A)>=(B)?(A):(B))

//extern Node *this=NULL, *suc=NULL, *pred=NULL, *chord=NULL; //descobrir o porquê do warning, ficaria mais fácil assim

void interface(char **args)
{
	char buffer[64], Buffer[257], *key, *address, *port, *info;
	int chave, newfd=0, maxfd = 0, counter, ring=0, TcpFd=0, UdpFd=0, seq;
	Element **ht;
	Node *this, *suc=NULL, *pred=NULL, *chord=NULL;
	struct sockaddr_in addr;
	socklen_t addrlen;
	ssize_t n;
	fd_set rfds;
	struct timeval tv;
	
	tv.tv_sec = 30;
	tv.tv_usec = 0;
		
	memset(buffer, '\0', 64); //clear buffer
	memset(Buffer, '\0', 257); //clear buffer
	 
	seq = rand() % 100;
	 
	ht = CreateHashtable();
	 
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
	fprintf(stdout, "Node created: %d %s %s\n", this->chave, this->address, this->port);//DELETE LATER
	
	suc = create(-1, NULL, NULL);
	pred = create(-1, NULL, NULL);
	chord = create(-1, NULL, NULL);
	
	addrlen=sizeof(addr);
	
	FD_ZERO(&rfds); //clear garbage for select
	while(1)
	{
		FD_SET(0,&rfds);	//select will wait for stdin
		FD_SET(TcpFd, &rfds);
		FD_SET(pred->fd,&rfds);
		FD_SET(suc->fd,&rfds);		//select will wait for tcp connection
		FD_SET(UdpFd,&rfds);	//select will wait for udp message
		FD_SET(newfd,&rfds);	//select will wait for tcp message from current connection	
		
		fprintf(stdout, "tcp: %d udp: %d newfd: %d pred: %d suc:%d\n", TcpFd, UdpFd, newfd, pred->fd, suc->fd);

		maxfd = max(0, newfd);			//Check which is the max file descriptor so select won´t have to check all fds
		maxfd = max(maxfd, UdpFd);
		maxfd = max(maxfd, TcpFd);
		maxfd = max(maxfd, pred->fd);
		maxfd = max(maxfd, suc->fd);
		
		counter = 0;
		
		counter = select(maxfd+1, &rfds, (fd_set*)NULL, (fd_set*)NULL, &tv);	
		
		if(counter==-1)
		{
		fprintf(stderr, "Select: %s\n", strerror(errno));
		exit(1);
		}
		else if (counter == 0){
			fprintf(stdout, "Please write something\n");	//timeout ocurred
			tv.tv_sec = 30;
		}
			if(TcpFd!=0 && FD_ISSET(TcpFd,&rfds)){		//New tcp connection
				FD_CLR(TcpFd,&rfds);
				
				//if((newfd=accept(TcpFd,(struct sockaddr*)&addr,&addrlen))==-1)
				if((newfd=accept(TcpFd, NULL, NULL))==-1)
				{
					fprintf(stdout, "Error in accept: ");
					fprintf(stderr, "%s\n", strerror(errno));
					exit(1);
				}; //check if it will try to connect with more than one tcp socket at once
				
				fprintf(stdout, "Accepted connection\n");
				
				FD_SET(newfd,&rfds);	//making sure select knows that now it will have to check for a new fd(newfd - tcp socket)
				
				counter--;
			}
			if(pred->fd!=0 && FD_ISSET(pred->fd,&rfds))
			{
				FD_CLR(pred->fd,&rfds);
				
				do
				{
					n = read(pred->fd, Buffer, 256);
					if(n==-1)
					{
						fprintf(stderr, "%s\n", strerror(errno));
						exit(1);
					}
					else if (n==0)
					{
						n=close(pred->fd);
						pred->fd=0;
						if(pred->chave==suc->chave)suc->fd=0;
						fprintf(stdout, "closed pred connection\n");
					}
					else
					{
						TcpRead(this, suc, pred, Buffer, buffer, pred->fd, ht);
					
						memset(Buffer, '\0', 257);
					}
				}while(*buffer!='\0');
				memset(buffer, '\0', 64);
				counter--;		
			}
			if(suc->fd!=0 && FD_ISSET(suc->fd,&rfds) && suc->fd!=pred->fd)
			{
				FD_CLR(suc->fd,&rfds);
				
				do
				{
					n = read(suc->fd, Buffer, 256);
					if(n==-1)
					{
						fprintf(stderr, "%s\n", strerror(errno));
						exit(1);
					}
					else if (n==0)
					{
						n = close(suc->fd);
						suc->fd=0;
						fprintf(stdout, "closed suc connection\n");
					}
					else{
						TcpRead(this, suc, pred, Buffer, buffer, suc->fd, ht);
					
						memset(Buffer, '\0', 257);
					}
				}while(*buffer!='\0');
				memset(buffer, '\0', 64);
				counter--;		
			} 
			if(newfd != 0 && FD_ISSET(newfd, &rfds))//something written in accepted tcp socket
			{ 
				FD_CLR(newfd,&rfds);
				
				do
				{
					n = read(newfd, Buffer, 256);
					if(n==-1)
					{
						fprintf(stderr, "%s\n", strerror(errno));
						exit(1);
					}
					else if (n==0)
					{
						n = close(newfd);
						newfd=0;
						fprintf(stdout, "closed newfd connection");
					}
					else
					{
					TcpRead(this, suc, pred, Buffer, buffer, newfd, ht);
					
					newfd=0;
					
					memset(Buffer, '\0', 257);
					}
				}while(*buffer!='\0');
				memset(buffer, '\0', 64);
				counter--;				
			}
			if(FD_ISSET(0,&rfds)) //received a command
			{
				FD_CLR(0,&rfds);
				
				if(fgets(buffer, 64, stdin)==NULL)	//ERROR 
				{
					fprintf(stdout, "Nothing read\n");
					exit(1);
				}
				
				info = handle_instructions(buffer);
				
				if ((strcmp(buffer, "new\n") == 0)||(strcmp(buffer, "n\n") == 0)) 
				{
					fprintf(stdout, "Initiating a new ring\n");
					update(pred, this->chave, this->address, this->port, 0);					
					update(suc, this->chave, this->address, this->port, 0);
					TcpFd = CreateTcpServer(this->port);
					UdpFd = CreateUdpServer(this->port);
					ring = 1;
				}
				else if((strcmp(buffer, "show\n") == 0)||(strcmp(buffer, "s\n") == 0))
				{
					fprintf(stdout, "This Node:\n-key = %d\n-address = %s\n-port = %s\n", this->chave, this->address, this->port);
					if (pred->chave!=-1)fprintf(stdout, "Predecessor Node:\n-key = %d\n-address = %s\n-port = %s\n", pred->chave, pred->address, pred->port);
					if (suc->chave!=-1)fprintf(stdout, "Successor Node:\n-key = %d\n-address = %s\n-port = %s\n", suc->chave, suc->address, suc->port);
					
					if(chord->chave==-1)fprintf(stdout, "No chord\n");
					else fprintf(stdout, "Chord to:\n-key = %d\n-address = %s\n-port = %s\n", chord->chave, chord->address, chord->port);
				}
				else if((strcmp(buffer, "pentry") == 0)||(strcmp(buffer, "p") == 0))
				{
					key = info;
					address = handle_instructions(info);
					address = handle_args(address, key);
					port = handle_instructions(address);
					port = handle_args(port, key);
					info = newline(port);
					
					if(pred->chave!=-1)fprintf(stdout, "Already in ring\n");//ring==1
					else
					{
						update(pred, atoi(key), address, port, 0);
						fprintf(stdout, "Pentry: %d %s %s\n", pred->chave, pred->address, pred->port);
				
						TcpFd = CreateTcpServer(this->port);
						UdpFd = CreateUdpServer(this->port);

						pred->fd=selfInform(pred, this);
						
						ring = 1;
					}

					
				}
				else if((strcmp(buffer, "find") == 0)||(strcmp(buffer, "f") == 0))
				{
					if (ring==1)fnd(info, this, suc, pred, seq, ht);
					else fprintf(stdout, "Not in ring!\n");
					seq = (seq + 1) % 100;
				}
				else if ((strcmp(buffer, "leave\n") == 0)||(strcmp(buffer, "l\n") == 0))
				{
					n=0;
					if(ring!=1)fprintf(stdout, "Not in ring\n");
					else
					{
						RingLeave(this, suc, pred);
						if(newfd!=0)n = close(newfd);
						newfd=0;
						if(n==-1)
						{
							fprintf(stderr, "%s\n", strerror(errno));
							exit(1);
						}
						close(TcpFd);
						if(n==-1)
						{
							fprintf(stderr, "%s\n", strerror(errno));
							exit(1);
						}
						TcpFd=0;
						close(UdpFd);
						if(n==-1)
						{
							fprintf(stderr, "%s\n", strerror(errno));
							exit(1);
						}
						UdpFd=0;
						ring=0;
					}
				}
				else if (strcmp(buffer, "m\n") == 0) fprintf(stdout, "%d %d.%s %d.%s\n", this->chave, this->chave, this->address, this->chave, this->port);
				else if ((strcmp(buffer, "exit\n") == 0)||(strcmp(buffer, "e\n") == 0))
				{
					freeNode(chord);;
					freeNode(suc);
					freeNode(pred);
					freeNode(this);
					FreeHash(ht);
					return;
				}
				else if(strcmp(buffer, "clear\n")==0) system("clear");
				else	fprintf(stdout, "Comando Desconhecido ou ainda não implementado\n");
				
				counter--;
				memset(buffer, '\0', 64);
			}
			if(FD_ISSET(UdpFd,&rfds)){ //something written in udp socket
				FD_CLR(UdpFd,&rfds);
				
				n = recvfrom(UdpFd, buffer, 64, 0, (struct sockaddr *)&addr, &addrlen);
				write(1, "received: ", 10);
				write(1, buffer, n);
				
				n = sendto(UdpFd, "ACK", 3, 0, (struct sockaddr *)&addr, addrlen);
				if(n==-1)exit(1);
				
				counter--;
				memset(buffer, '\0', 64);
			}
		tv.tv_sec = 30;
	}
	
	return;
}

char *handle_args(char *arg, char *key)//searches for the first '.' appearance in a string
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

char *handle_instructions(char *arg)//searches for the first ' ' appearance in a string
{
	char *final, *aux, *space=" ";
	
	aux = strstr(arg, space);
	
	
	if(aux==NULL && strcmp(arg, "n\n")!=0 && strcmp(arg, "new\n")!=0 && strcmp(arg, "show\n")!=0 && strcmp(arg, "s\n")!=0 && strcmp(arg, "leave\n")!=0 && strcmp(arg, "l\n")!=0 && strcmp(arg, "exit\n")!=0 && strcmp(arg, "e\n")!=0 && strcmp(arg, "clear\n")!=0 && strcmp(arg, "m\n")!=0)
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

char *newline(char *arg)//searches for the first newline appearance in a string
{
	char *aux, *nl="\n";
	aux = strstr(arg, nl);
	if(aux!=NULL)
	{
		*aux = '\0';
		return aux+1;
	}
	else return aux;
}

void TcpRead(Node *this, Node *suc, Node *pred, char *Buffer, char *buffer, int fd, Element **ht)
{
	char *aux, *info;
	
	fprintf(stdout, "received: %s\n", Buffer);
	aux = newline(Buffer);
	
	while(aux!=NULL)
	{
		
		if(*buffer!='\0')strcat(buffer, Buffer);
		else strcpy(buffer, Buffer);
	
		info = handle_instructions(buffer);
	
		if(strcmp("SELF", buffer)==0) //received SELF message 
		{
			SelfRcv(this, suc, pred, fd, info);

			fprintf(stdout, "pred: %d %s %s\n", pred->chave, pred->address, pred->port);
			fprintf(stdout, "suc: %d %s %s\n", suc->chave, suc->address, suc->port);
					
		}
		else if(strcmp("FND", buffer)==0)	FNDrecv(info, this, suc, pred);
		else if(strcmp("RSP", buffer)==0)
		{
			RSPrecv(info, this, suc, ht);
		}
		else if(strcmp("PRED", buffer)==0) //received PRED message
		{				
			PREDrcv(this, suc, pred, info);
		
			fprintf(stdout, "pred: %d %s %s\n", pred->chave, pred->address, pred->port);
			fprintf(stdout, "suc: %d %s %s\n", suc->chave, suc->address, suc->port);
		}
		Buffer = aux;
		aux = newline(Buffer);
		memset(buffer, '\0', 64);
		if(*Buffer!='\0' && aux==NULL)strcpy(buffer, Buffer);
	}	
	return;
}

void PREDrcv(Node *this, Node *suc, Node *pred, char *info) //cria um fd diferente para pred ao fazer leave sem necessidade, se tiver tempo corrigir 
{
	char *address, *port, *key;
	int n=0, chave;
	
	key = info; //transform this into a function later
	address = handle_instructions(info);
	address = handle_args(address, key);
	port = handle_instructions(address);
	port = handle_args(port, key);
	chave=atoi(key);
	key = newline(port);
					
	fprintf(stdout, "pred: %d %s %s\n", chave, address, port);
	if (pred->fd!=0 && pred->chave!=suc->chave)n=close(pred->fd);
	if(n==-1)
	{
		fprintf(stderr, "%s\n", strerror(errno));
		exit(1);
	}
	
	update(pred, chave, address, port, 0);
	if(pred->chave==suc->chave)pred->fd = suc->fd;
					
	fprintf(stdout, "pred: %d %s %s %d\n", pred->chave, pred->address, pred->port, pred->fd);
		
	if(pred->chave!=this->chave) pred->fd = selfInform(pred, this);
	else update(suc, pred->chave, pred->address, pred->port, pred->fd);
	return;
}

void SelfRcv(Node *this, Node *suc, Node *pred, int fd, char *info)
{
	Node *aux;
	char *address, *port, *key;
	int chave;
	int n=0;
	
	aux = create(-1, NULL, NULL);
	
	key = info;
	chave = atoi(key);
	address = handle_instructions(info);
	address = handle_args(address, key);
	port = handle_instructions(address);
	port = handle_args(port, key);
	key = newline(port);
	
	fprintf(stdout, "suc: %d %s %s\n", chave, address, port);
	
	update(aux, suc->chave, suc->address, suc->port, suc->fd);
	update(suc, chave, address, port, fd);
	
	if(pred->chave == this->chave){
		//sleep(1); //enable while using valgrind
		selfInform(suc, this);
		update(pred, suc->chave, suc->address, suc->port, suc->fd);
	}
	else if(aux->chave!=-1)
	{
		if(dist(aux->chave, this->chave) > dist(suc->chave, this->chave))predInform(suc, aux);
		if(aux->fd!=0 && aux->chave!=pred->chave)n = close(aux->fd);
		if(n==-1)
		{
			fprintf(stderr, "%s\n", strerror(errno));
			exit(1);
		};
	}
	
	freeNode(aux);
	
	return;
}

void FNDrecv (char *info, Node *this, Node *suc, Node *pred)	//find almost completed, lacks case with only 2 nodes and 1 node
{
	char message[64], *searchee, *seq, *key, *address, *port;
	
	searchee = info;
	seq = handle_instructions(searchee);
	key = handle_instructions(seq);
	address = handle_instructions(key);
	address = handle_args(address, key);
	port = handle_instructions(address);
	port = handle_args(port, key);
	info = newline(port);
	
	
	if(suc->chave==atoi(searchee))
	{
		sprintf(message, "RSP %s %s %d %d.%s %d.%s\n", key, seq, suc->chave, suc->chave, suc->address, suc->chave, suc->port);
	}
	else if(compareDist(atoi(searchee), this->chave, suc->chave, 0)<=0)
	{
		sprintf(message, "RSP %s %s %d %d.%s %d.%s\n", key, seq, this->chave, this->chave, this->address, this->chave, this->port);
	}
	else sprintf(message, "FND %s %s %s %s.%s %s.%s\n", searchee, seq, key, key, address, key, port);
	
	GenericTCPsend(suc, message);

	return;
	
}

void RSPrecv (char *info, Node *this, Node *suc, Element **ht)
{
	Node *aux=NULL;
	char message[64], *searchee, *seq, *key, *address, *port;
	unsigned int hashi;
	
	searchee = info;
	seq = handle_instructions(searchee);
	key = handle_instructions(seq);
	address = handle_instructions(key);
	address = handle_args(address, key);
	port = handle_instructions(address);
	port = handle_args(port, key);
	info = newline(port);
	
	hashi = hash(atoi(seq));
	aux = Retrieve_del(ht, hashi, atoi(seq));
	sprintf(message, "RSP %s %s %s %s.%s %s.%s\n", searchee, seq, key, key, address, key, port);
	if(aux==NULL){
		if(atoi(searchee) == this->chave) fprintf(stdout, "%s", message);
		else GenericTCPsend(suc, message);
	}
	else fprintf(stdout, "bentrada");
	return;
}

void fnd(char *info, Node *this, Node *suc, Node *pred, int seq, Element **ht)
{
	char message[64], *key=info;
	unsigned int hashi;
	info = newline(key);
	
	fprintf(stdout, "%s\n", key);
	
	/*if(pred->chave==this->chave)
	{
		sprintf(message, "RSP %s %d %d %d.%s %d.%s\n", key, seq, this->chave, this->chave, this->address, this->chave, this->port);
		fprintf(stdout, "%s", message);
		return;
	}*/
	
	if(suc->chave==atoi(key))
	{
		sprintf(message, "RSP %s %d %d %d.%s %d.%s\n", key, seq, suc->chave, suc->chave, suc->address, suc->chave, suc->port);
		fprintf(stdout, "%s", message);
		return;
	}
	else if (compareDist(atoi(key), pred->chave, this->chave, 0)<=0)
	{
		sprintf(message, "RSP %s %d %d %d.%s %d.%s\n", key, seq, pred->chave, pred->chave, pred->address, pred->chave, pred->port);
		fprintf(stdout, "%s", message);
		return;
	}
	else if(compareDist(atoi(key), this->chave, suc->chave, 0)<=0)
	{
		sprintf(message, "RSP %s %d %d %d.%s %d.%s\n", key, seq, this->chave, this->chave, this->address, this->chave, this->port);
		fprintf(stdout, "%s", message);
		return;
	}
	else
	{
		hashi = hash(seq);
		Insert(ht, hashi, atoi(key), NULL);
		sprintf(message, "FND %s %d %d %d.%s %d.%s\n", key, seq, this->chave, this->chave, this->address, this->chave, this->port);
	}
	
	GenericTCPsend(suc, message);

	return;
}

void RingLeave(Node *this, Node *suc, Node *pred)
{
	int n=0;
	if(pred->chave!=this->chave)//&& suc->chave!=this->chave
	{
		predInform(pred, suc);
		if(pred->chave!=suc->chave)n = close(suc->fd);
		if(n==-1)
		{
			fprintf(stderr, "%s\n", strerror(errno));
			exit(1);
		}
		n = close(pred->fd);
		if(n==-1)
		{
			fprintf(stderr, "%s\n", strerror(errno));
			exit(1);
		}
	}
	
	nodeClear(pred);
	nodeClear(suc);
	
	return;
}

/*
*	Compares distances of node a and b to this
*	Return value: 0 if dist(a, this)==dist(b, this)
*				  1 if dist(a, this)>dist(b, this)
*				 -1 if dist(a, this)<dist(b, this)
*	Extra info: if flag==0 no chords are considered, if flag==1consider chord				
*/
int compareDist(int this, int a, int b, int flag)
{
	unsigned int dista, distb;
	dista = dist(this, a);
	distb = dist(this, b);

	fprintf(stdout, "%d- %d: %d  %d: %d\n",this , a, dista, b, distb);	
	
	if(dista==distb)
	{
		fprintf(stdout, "returning 0\n");
		return 0;
	}
	else if(max(dista, distb)==dista)return 1;
	else if(max(dista, distb)==distb)return -1;
	else return -2;
}

unsigned int dist(int this, int measuree)
{
	int dist=0;
	if (this==measuree)return 0;
	else if (this<measuree){
		dist = MAX_NODES+1-(measuree);
		dist = dist + (this);
	}
	else
	{
		dist = (this)-(measuree);
	}
	return dist;
}
