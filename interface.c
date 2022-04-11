//interface.c

//AS LIGAÇÕES ENTRE OS PREDS E SUCS SÃO PERMANENTES, CORRIGIR ISSO

#include "interface.h"

#define max(A,B) ((A)>=(B)?(A):(B))

//extern Node *this=NULL, *suc=NULL, *pred=NULL, *chord=NULL; //descobrir o porquê do warning, ficaria mais fácil assim

void interface(char **args)
{
	char buffer[64], Buffer[257], *key, *address, *port, *info, message[32];
	int chave, newfd=0, maxfd = 0, counter, ring=0, TcpFd=0, UdpFd=0, seq, bent=0, ack=0;
	Element **ht;
	int i=0;
	Node *this, *suc=NULL, *pred=NULL, *chord=NULL, *aux=NULL;
	struct sockaddr_in addr;
	Save *aux_addr=NULL;
	socklen_t addrlen;
	ssize_t n;
	fd_set rfds;
	struct timeval tv;
	
	tv.tv_sec = 300;
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
	
	address = args[2];
	i = verifyAddr(address);
	if(i==0)
	{
		fprintf(stdout, "Endereço inválido. Exiting...\n");
		exit(1);
	}
	
	port = args[3];
	i = verifyPort(port);
	if(i==0)
	{
		fprintf(stdout, "Porto inválido. Exiting...\n");
		exit(1);
	}
	
	memset(message, '\0', 32);
	
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
		if(ack>1)FD_SET(ack, &rfds);
		
		fprintf(stdout, "tcp: %d udp: %d newfd: %d pred: %d suc:%d ack:%d\n", TcpFd, UdpFd, newfd, pred->fd, suc->fd, ack);

		maxfd = max(0, newfd);			//Check which is the max file descriptor so select won´t have to check all fds
		maxfd = max(maxfd, UdpFd);
		maxfd = max(maxfd, TcpFd);
		maxfd = max(maxfd, pred->fd);
		maxfd = max(maxfd, suc->fd);
		if(ack>1)maxfd = max(maxfd, ack);
		
		counter = 0;
		
		counter = select(maxfd+1, &rfds, (fd_set*)NULL, (fd_set*)NULL, &tv);	
		
		if(counter==-1)
		{
		fprintf(stderr, "Select: %s\n", strerror(errno));
		exit(1);
		}
		else if (counter == 0){
			if (ack>=1)
			{
				if(message[0]!='\0')
				{
					fprintf(stdout, "did not receive ACK, sending via tcp\n");
					GenericTCPsend(suc, message);
					memset(message, '\0', 32);
					}
				else fprintf(stdout, "did not receive ACK, please try another node\n");
				ack = 0;
			}
			else if(bent==1)fprintf(stdout, "did not receive bent, please try another node\n");
			else fprintf(stdout, "Please write something\n");	//timeout ocurred
			tv.tv_sec = 300;
			bent=0;
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
						if(n==-1)
						{
							fprintf(stderr, "%s\n", strerror(errno));
							exit(1);
						}
						update(pred, -1, NULL, NULL, 0);
						if(pred->chave==suc->chave) update(suc, -1, NULL, NULL, 0);
						fprintf(stdout, "\tClosed pred connection! Exiting ring...\n");
						ring=0;
					}
					else
					{
						aux_addr = TcpRead(this, suc, pred, Buffer, buffer, pred->fd, ht, chord, message, &ack);
					
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
						if(n==-1)
						{
							fprintf(stderr, "%s\n", strerror(errno));
							exit(1);
						}
						suc->fd=0;
						update(suc, -1, NULL, NULL, 0);
						fprintf(stdout, "\tClosed suc connection! Exiting ring...\n");
						ring=0;
					}
					else{
						aux_addr = TcpRead(this, suc, pred, Buffer, buffer, suc->fd, ht, chord, message, &ack);
					
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
						if(n==-1)
						{
							fprintf(stderr, "%s\n", strerror(errno));
							exit(1);
						}
						newfd=0;
						fprintf(stdout, "closed newfd connection");
					}
					else
					{
					aux_addr = TcpRead(this, suc, pred, Buffer, buffer, newfd, ht, chord, message, &ack);
					
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
					update(pred, this->chave, this->address, this->port, 0);					
					update(suc, this->chave, this->address, this->port, 0);
					TcpFd = CreateTcpServer(this->port);
					UdpFd = CreateUdpServer(this->port);
					fprintf(stdout, "Initiated a new ring\n");
					ring = 1;
				}
				else if((strcmp(buffer, "show\n") == 0)||(strcmp(buffer, "s\n") == 0))
				{
					fprintf(stdout, "\n\tThis Node:\n-key = %d\n-address = %s\n-port = %s\n", this->chave, this->address, this->port);
					if (pred->chave!=-1)fprintf(stdout, "\tPredecessor Node:\n-key = %d\n-address = %s\n-port = %s\n", pred->chave, pred->address, pred->port);
					if (suc->chave!=-1)fprintf(stdout, "\tSuccessor Node:\n-key = %d\n-address = %s\n-port = %s\n", suc->chave, suc->address, suc->port);
					
					if(chord->chave==-1)fprintf(stdout, "\tNo chord\n");
					else fprintf(stdout, "\tChord to:\n-key = %d\n-address = %s\n-port = %s\n\n", chord->chave, chord->address, chord->port);
				}
				else if((strcmp(buffer, "pentry") == 0)||(strcmp(buffer, "p") == 0))
				{
					key = info;
					address = handle_instructions(info);
					
					port = handle_instructions(address);
					
					if (port!=NULL)info = newline(port);
					
					if (address!=NULL)i = verifyAddr(address);
					if(i==0) fprintf(stdout, "Mensagem malformatada(Endereço mal formatado ou inválido)\n");
					else
					{
						i = verifyPort(port);
						if(i==0) fprintf(stdout, "Mensagem malformatada(Porto mal formatado ou inválido)\n");
						{
							

							if(pred->chave!=-1 || suc->chave!=-1)fprintf(stdout, "\tAlready in ring\n");//ring==1
							else
							{
								update(pred, atoi(key), address, port, 0);
								//fprintf(stdout, "Pentry: %d %s %s\n", pred->chave, pred->address, pred->port);
			
								if(TcpFd==0)TcpFd = CreateTcpServer(this->port);
								if(UdpFd==0)UdpFd = CreateUdpServer(this->port);
								
								fprintf(stdout, "Sockets created\n");
								
								pred->fd=selfInform(pred, this);
					
								ring = 1;
							}
						}
					}
					
				}
				else if((strcmp(buffer, "bentry") == 0)||(strcmp(buffer, "b") == 0))
				{
					key = info;
					address = handle_instructions(info);
					if (address!=NULL)port = handle_instructions(address);
					if (port!=NULL)info = newline(port);

					if(pred->chave!=-1 || suc->chave!=-1)fprintf(stdout, "\tAlready in ring\n");//ring==1
					else
					{
						if(address == NULL && port ==NULL)fprintf(stdout, "Instrução mal formatada(Endereço ou porto mal formatado)");
						else
						{
							aux=create(atoi(key), address, port);
						
							//fprintf(stdout, "Bentry: %d %s %s\n", aux->chave, aux->address, aux->port);
				
							sprintf(message, "EFND %d", this->chave);
						
							tv.tv_sec = 1;
							
							if(UdpFd!=0)
							{
								n = close(UdpFd);
								if(n==-1)
								{
									fprintf(stderr, "%s\n", strerror(errno));
									exit(1);
								}
							}
							UdpFd = GenericUDPsend(aux, message); //atenção, ele pode aqui ficar bloqueado à espera do ACK, pois ainda não foram criados outros sockets
							
							memset(message, '\0', 32);
							
							bent=1;
							ack=1;
						
							freeNode(aux);
						}
					}
				}
				else if((strcmp(buffer, "find") == 0)||(strcmp(buffer, "f") == 0))
				{
					if (ring==1)fnd(info, this, suc, pred, seq, ht, NULL, chord, message, &ack);
					else fprintf(stdout, "\tNot in ring!\n");
					seq = (seq + 1) % 100;
				}
				else if ((strcmp(buffer, "leave\n") == 0)||(strcmp(buffer, "l\n") == 0))
				{
					n=0;
					if(pred->chave==-1 && suc->chave==-1)fprintf(stdout, "\tNot in ring\n");
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
				else if (strcmp(buffer, "m\n") == 0) fprintf(stdout, "%d %s %s\n", this->chave, this->address, this->port);
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
				else if(strcmp(buffer, "h\n")==0)
				{
					fprintf(stdout, "\n========================================================================================================================\n");
					fprintf(stdout, "new(n) -> creates new ring  show(s) -> shows info on node and also on predecessor node and successor and chord(if any)\n");
					fprintf(stdout, "pentry(p) -> joins a ring (requires knowledge on who is predecessor)  leave(l) -> leave ring(self-explanatory)\n");
					fprintf(stdout, "bentry(b) -> joins a ring (needs info on 1 node)    find(f) -> finds the position of a certain index in ring\n");
					fprintf(stdout, "chord(c) -> creates a chord to a node in ring   dchord(d) -> deletes existing chord\n");
					fprintf(stdout, "exit(e) -> exit application   clear -> clear terminal window\n");
					fprintf(stdout, "========================================================================================================================\n\n");
					}
				else if(strcmp(buffer, "chord")==0 || strcmp(buffer, "c")==0)
				{
					key = info;
                    address = handle_instructions(info);
					port = handle_instructions(address);
					i=verifyAddr(address);
					if(i) i=verifyPort(port);
					if(i)
					{
						
						info = newline(port);
						update(chord, atoi(key), address, port, 0);
					}
				}
				else if(strcmp(buffer, "dchord")==0 || strcmp(buffer, "d")==0) update(chord, -1, NULL, NULL, 0);
				else fprintf(stdout, "\tComando Desconhecido ou ainda não implementado\n\tPress h for help\n");
			
				counter--;
				memset(buffer, '\0', 64);
			}
			if(aux_addr!=NULL)
			{
				/*fprintf(stdout, "check fam, %d %d\n", addr.sin_family, aux_addr->addr.sin_family);
				if(addr.sin_port==aux_addr->addr.sin_port)fprintf(stdout,"check port");
				fprintf(stdout, "port, %u %u\n", addr.sin_port, aux_addr->addr.sin_port);
				fprintf(stdout, "check addr, %u %u\n", addr.sin_addr.s_addr, aux_addr->addr.sin_addr.s_addr);*/
				addr=aux_addr->addr;
				/*if(addr.sin_family!=AF_INET)fprintf(stdout, "ERROR!!!\n");
				//addr.sin_family=AF_INET;*/
				n = sendto(UdpFd, aux_addr->message, strlen(aux_addr->message), 0, (struct sockaddr *)&addr, addrlen);
				if(n==-1)
				{
					fprintf(stderr, "%s\n", strerror(errno));
					exit(1);
				}
				ack = 1;
			}
			if(FD_ISSET(ack,&rfds) && ack > 1)
			{
				n = recvfrom(ack, Buffer, 64, 0, (struct sockaddr *)&addr, &addrlen);
				if(strcmp(Buffer, "ack")==0)
				{
					n=close(ack);
					if(n==-1)
					{
						fprintf(stderr, "%s\n", strerror(errno));
						exit(1);
					} 
					ack = 0;
				}
			}
			if(FD_ISSET(UdpFd,&rfds) && UdpFd!=0){ //something written in udp socket
				FD_CLR(UdpFd,&rfds);
				
				n = recvfrom(UdpFd, Buffer, 256, 0, (struct sockaddr *)&addr, &addrlen);
				if(n==-1)
				{
					fprintf(stderr, "%s\n", strerror(errno));
					exit(1);
				}
				
				fprintf(stdout, "\tReceived: %s", Buffer);
				
				fprintf(stdout, " via udp\n");
				
				if(strcmp(Buffer, "ACK")==0) ack=0;
				else 
				{
					n = sendto(UdpFd, "ACK", 3, 0, (struct sockaddr *)&addr, addrlen); 
					if(n==-1)
					{
						fprintf(stderr, "%s\n", strerror(errno));
						exit(1);
					}
					
					info = handle_instructions(Buffer);
					
					if(strcmp(Buffer, "EFND")==0)
					{ 
						aux_addr = (Save *) malloc(sizeof(Save));
						aux_addr->addr=addr;
						aux_addr = fnd(info, this, suc, pred, seq, ht, aux_addr, chord, message, &ack);
						seq = (seq + 1) % 100;
						if(aux_addr!=NULL)
						{
							n = sendto(UdpFd, aux_addr->message, strlen(aux_addr->message), 0, (struct sockaddr *)&addr, addrlen);
							if(n==-1)
							{
								fprintf(stderr, "%s\n", strerror(errno));
								exit(1);
							}
							ack = 1;
						}
					}
					else if(strcmp(Buffer, "EPRED")==0)
					{
						bent = 0;
						
						key = info;
						address = handle_instructions(info);
						port = handle_instructions(address);
						info = newline(port);
						
						update(pred, atoi(key), address, port, 0);
						//fprintf(stdout, "Pentry: %d %s %s\n", pred->chave, pred->address, pred->port);
						
						n = close(UdpFd);
						if(n==-1)
						{
							fprintf(stderr, "%s\n", strerror(errno));
							exit(1);
						}
						
						if(TcpFd == 0)TcpFd = CreateTcpServer(this->port);
						UdpFd = CreateUdpServer(this->port);

						fprintf(stdout, "Sockets created\n");

						pred->fd=selfInform(pred, this);
						
						ring = 1;
					}
					else if(strcmp(Buffer, "FND")==0) FNDrecv(info, this, suc, pred, chord,  message, &ack);
					else if(strcmp(Buffer, "RSP")==0)
					{
						aux_addr = RSPrecv(info, this, suc, ht, chord, message, &ack);
						if(aux_addr!=NULL)
						{
							n = sendto(UdpFd, aux_addr->message, strlen(aux_addr->message), 0, (struct sockaddr *)&addr, addrlen);
							if(n==-1)
							{
								fprintf(stderr, "%s\n", strerror(errno));
								exit(1);
							}
							ack = 1;
						}
					}
				}
				
				counter--;
				memset(Buffer, '\0', 257);
			}
		if(bent==0&&ack==0)tv.tv_sec = 300;
		else tv.tv_sec = 1;
		if(aux_addr!=NULL)
		{
			free(aux_addr->message);
			free(aux_addr);
			aux_addr=NULL;
		}
	}
	
	return;
}

char *handle_instructions(char *arg)//searches for the first ' ' appearance in a string
{
	char *final=NULL, *aux, *space=" ";
	
	aux = strstr(arg, space);
	
	
	if(aux==NULL && strcmp(arg, "n\n")!=0 && strcmp(arg, "new\n")!=0 && strcmp(arg, "show\n")!=0 && strcmp(arg, "s\n")!=0 && strcmp(arg, "leave\n")!=0 && strcmp(arg, "l\n")!=0 && strcmp(arg, "exit\n")!=0 && strcmp(arg, "e\n")!=0 && strcmp(arg, "clear\n")!=0 && strcmp(arg, "m\n")!=0 && strcmp(arg, "h\n")!=0)
		{
			fprintf(stdout, "%s", arg);
			fprintf(stdout, "\t***Mensagem Mal formatada***\n");
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

void *TcpRead(Node *this, Node *suc, Node *pred, char *Buffer, char *buffer, int fd, Element **ht, Node *chord, char *m, int *ack)
{
	char *aux, *info;
	void *fun_aux=NULL;
	
	fprintf(stdout, "\n\tReceived: %s\n", Buffer);
	aux = newline(Buffer);
	
	while(aux!=NULL)
	{
		
		if(*buffer!='\0')strcat(buffer, Buffer);
		else strcpy(buffer, Buffer);
	
		info = handle_instructions(buffer);
		
		fprintf(stdout, "buffer: %s in tcpread", buffer);
	
		if(strcmp("SELF", buffer)==0) SelfRcv(this, suc, pred, fd, info); //Received SELF message
		else if(strcmp("FND", buffer)==0) FNDrecv(info, this, suc, pred, chord, m, ack);
		else if(strcmp("RSP", buffer)==0) fun_aux = RSPrecv(info, this, suc, ht, chord, m, ack);
		else if(strcmp("PRED", buffer)==0) PREDrcv(this, suc, pred, info); //Received PRED message
		
		Buffer = aux;
		aux = newline(Buffer);
		memset(buffer, '\0', 64);
		if(*Buffer!='\0' && aux==NULL)strcpy(buffer, Buffer);
	}	
	return fun_aux;
}

void PREDrcv(Node *this, Node *suc, Node *pred, char *info) //cria um fd diferente para pred ao fazer leave sem necessidade, se tiver tempo corrigir 
{
	char *address, *port, *key;
	int n=0, chave;
	
	key = info; //transform this into a function later
	address = handle_instructions(info);
	port = handle_instructions(address);
	chave=atoi(key);
	key = newline(port);
	n = verifyAddr(address);
	if(n==0)
	{
		fprintf(stdout, "Mensagem malformatada(Endereço mal formatado ou inválido)\n");
		return;
	}
	n = verifyPort(port);
	if(n==0)
	{
		fprintf(stdout, "Mensagem malformatada(Porto mal formatado ou inválido)\n");
		return;
	}
					
	
	if (pred->fd!=0 && pred->chave!=suc->chave)n=close(pred->fd);
	if(n==-1)
	{
		fprintf(stderr, "%s\n", strerror(errno));
		exit(1);
	}
	
	update(pred, chave, address, port, 0);
	if(pred->chave==suc->chave)pred->fd = suc->fd;
					
	fprintf(stdout, "\tUpdated pred: %d %s %s %d\n", pred->chave, pred->address, pred->port, pred->fd);
		
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
	
	key = info;
	chave = atoi(key);
	address = handle_instructions(info);
	port = handle_instructions(address);
	key = newline(port);
	n = verifyAddr(address);
	if(n==0)
	{
		fprintf(stdout, "Mensagem malformatada(Endereço mal formatado ou inválido)\n");
		return;
	}
	n = verifyPort(port);
	if(n==0)
	{
		fprintf(stdout, "Mensagem malformatada(Porto mal formatado ou inválido)\n");
		return;
	}
	
	if(chave == this->chave || suc->chave == chave) return;
	
	aux = create(-1, NULL, NULL);
	
	update(aux, suc->chave, suc->address, suc->port, suc->fd);
	
	if(pred->chave == this->chave){
		update(suc, chave, address, port, fd);
		selfInform(suc, this);
		update(pred, suc->chave, suc->address, suc->port, suc->fd);
	}
	else if(suc->chave!=-1)
	{
		if(dist(chave, pred->chave) < dist(this->chave, pred->chave) && chave!=pred->chave)
		{
			fprintf(stdout, "Nó %d está a tentar entrar numa posição errada!\n", chave);
			n = close(fd);
			if(n==-1)
			{
				fprintf(stderr, "%s\n", strerror(errno));
				exit(1);
			}
			freeNode(aux);
			return;
		}
		
		update(suc, chave, address, port, fd);
		
		if(dist(aux->chave, this->chave) > dist(suc->chave, this->chave))predInform(suc, aux);
		if(aux->fd!=0 && aux->chave!=pred->chave)n = close(aux->fd);
		if(n==-1)
		{
			fprintf(stderr, "%s\n", strerror(errno));
			exit(1);
		}
	}
	else 
	{
		update(suc, chave, address, port, fd);
	}
	
	fprintf(stdout, "\tUpdated suc: %d %s %s\n", chave, address, port);
	
	freeNode(aux);
	
	return;
}

void FNDrecv (char *info, Node *this, Node *suc, Node *pred, Node *chord, char *m, int *ack)	//find almost completed, lacks case with only 2 nodes and 1 node
{
	char message[64], *searchee, *seq, *key, *address, *port;
	int n = 0;
	
	fprintf(stdout, "meio mal\n");
	
	searchee = info;
	seq = handle_instructions(searchee);
	key = handle_instructions(seq);
	address = handle_instructions(key);
	port = handle_instructions(address);
	info = newline(port);
	n = verifyAddr(address);
	if(n==0)
	{
		fprintf(stdout, "Mensagem malformatada(Endereço mal formatado ou inválido)\n");
		return;
	}
	n = verifyPort(port);
	if(n==0)
	{
		fprintf(stdout, "Mensagem malformatada(Porto mal formatado ou inválido)\n");
		return;
	}
	
	
	if(suc->chave==atoi(searchee))
	{
		sprintf(message, "RSP %s %s %d %s %s\n", key, seq, suc->chave, suc->address, suc->port);
	}
	else if(compareDist(atoi(searchee), this->chave, suc->chave, 0)<=0)
	{
		sprintf(message, "RSP %s %s %d %s %s\n", key, seq, this->chave, this->address, this->port);
	}
	else sprintf(message, "FND %s %s %s %s %s\n", searchee, seq, key, address, port);
	
	if(chord->chave>-1 && dist(atoi(searchee), suc->chave) > dist(atoi(searchee), chord->chave) && *m=='\0')
	{
		*ack = GenericUDPsend(chord, message);
		strcpy(m, message);
	}
	else GenericTCPsend(suc, message);

	return;
	
}

void *RSPrecv (char *info, Node *this, Node *suc, Element **ht, Node *chord, char *m, int *ack)
{
	Save *aux;
	char message[64], *searchee, *seq, *key, *address, *port, *msg;
	unsigned int hashi, n = 0;
	
	searchee = info;
	seq = handle_instructions(searchee);
	key = handle_instructions(seq);
	address = handle_instructions(key);
	port = handle_instructions(address);
	info = newline(port);
	n = verifyAddr(address);
	if(n==0)
	{
		fprintf(stdout, "Mensagem malformatada(Endereço mal formatado ou inválido)\n");
		return NULL;
	}
	n = verifyPort(port);
	if(n==0)
	{
		fprintf(stdout, "Mensagem malformatada(Porto mal formatado ou inválido)\n");
		return NULL;
	}
	
	
	hashi = hash(atoi(seq));
	aux = Retrieve_del(ht, hashi, atoi(seq));
	
	if(aux==NULL){
		sprintf(message, "RSP %s %s %s %s %s\n", searchee, seq, key, address, port);
		if(atoi(searchee) == this->chave) fprintf(stdout, "%s", message);
		else {
			if(chord->chave>-1 && dist(atoi(searchee), suc->chave) > dist(atoi(searchee), chord->chave) && *m=='\0')
			{
				*ack=GenericUDPsend(chord, message);
				strcpy(m, message);
				
			}
			else GenericTCPsend(suc, message);
		}
	}
	else 
	{
		sprintf(message, "EPRED %s %s %s\n", key, address, port);
		msg = (char *) malloc (strlen(message)+1);
		strcpy(msg, message);
		aux->message=msg;
	}
	return aux;
}


void *fnd(char *info, Node *this, Node *suc, Node *pred, int seq, Element **ht, Save *addr, Node *chord, char *m, int *ack)
{
	char message[64], *key=info, *msg;
	unsigned int hashi;
	
	info = newline(key);
	
	fprintf(stdout, "%s\n", key);
	
	if(suc->chave==atoi(key))
	{
		if (addr==NULL)
		{
			sprintf(message, "RSP %d %s %s\n", suc->chave, suc->address, suc->port);
			fprintf(stdout, "%s", message);
		}
		else
		{
			sprintf(message, "EPRED %d %s %s", suc->chave, suc->address, suc->port);
			msg = (char *) malloc (strlen(message)+1);
			strcpy(msg, message);
			addr->message=msg;
		}
		return addr;
	}
	else if (compareDist(atoi(key), pred->chave, this->chave, 0)<=0)
	{
		if (addr==NULL)
		{
			sprintf(message, "RSP %s %d %d %s %s\n", key, seq, pred->chave, pred->address, pred->port);
			fprintf(stdout, "%s", message);
		}
		else
		{
			sprintf(message, "EPRED %d %s %s", pred->chave, pred->address, pred->port);
			msg = (char *) malloc (strlen(message)+1);
			strcpy(msg, message);
			addr->message=msg;
		}
		return addr;
	}
	else if(compareDist(atoi(key), this->chave, suc->chave, 0)<=0)
	{
		if (addr==NULL)
		{
			sprintf(message, "RSP %s %d %d %s %s\n", key, seq, this->chave, this->address, this->port);
			fprintf(stdout, "%s", message);
		}
		else
		{
			sprintf(message, "EPRED %d %s %s", this->chave, this->address, this->port);
			msg = (char *) malloc (strlen(message)+1);
			strcpy(msg, message);
			addr->message=msg;
		}
		return addr;
	}
	else
	{
		hashi = hash(seq);
		Insert(ht, hashi, seq, addr);
		sprintf(message, "FND %s %d %d %s %s\n", key, seq, this->chave, this->address, this->port);
	}
	
	if(chord->chave>-1 && dist(atoi(key), suc->chave) > dist(atoi(key), chord->chave) && m[0]=='\0') 
	{
		*ack=GenericUDPsend(chord, message);
		strcpy(m, message);
	}
	else GenericTCPsend(suc, message);

	return NULL;
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

int verifyPort(char *port)
{
	unsigned int porto, n=0;
	char auxi[24];

	n = sscanf(port, "%u%s", &porto, auxi);

	if (porto>65000 || n != 1)
	{
		fprintf(stdout, "Porto inválido\n");
		return 0;
	}
	
	return 1;
}

int verifyAddr(char *addr)
{
	unsigned int b[4], n=0;
	char auxi[24];

	n = sscanf(addr, "%u.%u.%u.%u%s", &b[0], &b[1], &b[2], &b[3], auxi);
	if(n!=4 || b[0]>255 || b[1]>255 || b[2]>255 || b[3]>255) return 0;
	
	return 1;
}

/*
*	Compares distances of node a and b to this
*	Return value: 0 if dist(a, this)==dist(b, this)
*				  1 if dist(a, this)>dist(b, this)
*				 -1 if dist(a, this)<dist(b, this)
*	Extra info: if flag==0 no chords are considered, if flag==1 consider chord				
*/
int compareDist(int this, int a, int b, int flag)
{
	unsigned int dista, distb;
	dista = dist(this, a);
	distb = dist(this, b);
	
	if(dista==distb) return 0;
	else if(max(dista, distb)==dista)return 1;
	else if(max(dista, distb)==distb)return -1;
	else return -2;
}

unsigned int dist(int this, int measuree)
{
	int dist=0;
	if (this==measuree)return 0;
	else if (this<measuree){
		dist = MAX_NODES-(measuree);
		dist = dist + (this);
	}
	else
	{
		dist = (this)-(measuree);
	}
	
	return dist;
}
