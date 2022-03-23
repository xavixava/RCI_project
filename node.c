//node.c

#include "node.h"

Node *create(int chave, char *address, char* port)
{
	Node *new;
	char *aux;
	
	new = (Node *) malloc(sizeof(Node));
	new->port = (char *) malloc(strlen(port)+1);
	new->address = (char *) malloc(strlen(address)+1);
	
	new->chave=chave;
	aux=strcpy(new->address,address);
	if(aux==NULL)exit(1);
	aux=strcpy(new->port,port);
	if(aux==NULL)exit(1);
	
	return new;
}
