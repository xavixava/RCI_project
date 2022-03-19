//node.c

#include "node.h"

Node *new(int chave, char *address, char* port)
{
	Node *new;
	char *aux;
	new = (Node *) malloc(sizeof(Node));
	aux=strcpy(new->port,port);
	new->chave=chave;
	aux=strcpy(new->address,address); 
	new->chord = NULL;
	new->suc = NULL;
	return new;
}

int getkey(Node *n)
{
	return n->chave;
}

char *getadd(Node *n)
{
	return n->address;
}

char *getport(Node *n)
{
	return n->port;
}