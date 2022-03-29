#ifndef InterfaceHeader
#define InterfaceHeader

/* Header file for the definition of the project interface */
#include "geral.h"
#include "node.h"


#define MAX_NODES 32

void interface(char **args);
char *handle_args(char *arg, char *key);
char *handle_instructions(char *arg);
char *newline(char *arg);

void SelfRcv(Node *this, Node **suc, Node **pred, char *info);
void PREDrcv(Node *this, Node **suc, Node **pred, char *info);
void RingLeave(Node *this, Node **suc, Node **pred);


int compareDist(Node *this, Node *a, Node *b, int flag);
unsigned int dist(Node *this, Node *measuree);

#endif