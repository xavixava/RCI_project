//node.c

#include "node.h"

/*
 *	allocates space for new node 
*/

Node *create(int chave, char *address, char* port)
{
	Node *new;
	char *aux;
	
	new = (Node *) malloc(sizeof(Node));
	new->port = (char *) malloc(6);
	new->address = (char *) malloc(33);
	
	new->chave=chave;
	memset(new->address, '\0', 33);
	if(address != NULL)
	{
		aux=strcpy(new->address,address);
		if(aux==NULL)exit(1);
	}
	
	memset(new->port, '\0', 6);
	if(port != NULL) 
	{
		aux=strcpy(new->port, port);
		if(aux==NULL)exit(1);
	}
	
	new->fd = 0;
	
	return new;
}

void freeNode(Node *this)
{
	free(this->address);
	free(this->port);
	free(this);
	return;
}

void update(Node *n, int chave, char *address, char* port, int fd)
{
	n->chave=chave;
	memset(n->address, '\0', 33);
	memset(n->port, '\0', 6);
	if(address != NULL)	strcpy(n->address, address);
	if(port != NULL) strcpy(n->port, port);
	n->fd=fd;
	return;
}

void nodeClear(Node *n)
{
	n->chave=-1;
	memset(n->address, '\0', 33);
	memset(n->port, '\0', 6);
	n->fd=0;
	return;
}