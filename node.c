//node.c

#include "node.h"

Node *create(int chave, char *address, char* port)
{
	Node *new;
	char *aux;
	
	new = (Node *) malloc(sizeof(Node));
	new->port = (char *) malloc(6);
	new->address = (char *) malloc(strlen(address)+1);
	
	new->chave=chave;
	aux=strcpy(new->address,address);
	if(aux==NULL)exit(1);
	aux=strcpy(new->port,port);
	if(aux==NULL)exit(1);
	
	return new;
}

void freeNode(Node *this)
{
	free(this->address);
	free(this->port);
	free(this);
	return;
}
