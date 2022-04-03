#ifndef NodeDef
#define NodeDef

/* Header file for the definition of the project  */
#include "geral.h"

typedef struct _Node
{	int chave;
	char *address;
	char *port;
	int fd;
}Node;

Node *create(int chave, char *address, char* port);
void freeNode(Node *this);
void update(Node *n, int chave, char *address, char* port, int fd);
void nodeClear(Node *n);

#endif