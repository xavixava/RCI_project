#ifndef NodeDef
#define NodeDef

/* Header file for the definition of the project  */
#include "geral.h"

typedef struct _Node Node;

Node *new(int chave, char *address, char* port);

int getkey(Node *n);

char *getadd(Node *n);

char *getport(Node *n);

#endif