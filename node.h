#ifndef NodeDef
#define NodeDef

/* Header file for the definition of the project  */
#include "geral.h"

typedef struct _Node
{	int chave;
	char *address;
	char *port;
	struct _Node *chord, *suc;
}Node;

Node *create(int chave, char *address, char* port);

#endif