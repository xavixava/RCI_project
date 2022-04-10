#ifndef InterfaceHeader
#define InterfaceHeader

/* Header file for the definition of the project interface */
#include "geral.h"
#include "node.h"
#include "hash.h"
#include "network.h"

#define MAX_NODES 32

typedef struct Save
{	
	char *message;
	struct sockaddr_in addr;
}Save;

//struct Save Save;

void interface(char **args);
char *handle_args(char *arg, char *key);
char *handle_instructions(char *arg);
char *newline(char *arg);

void *TcpRead(Node *this, Node *suc, Node *pred, char *Buffer, char *buffer, int fd, Element **ht);
void SelfRcv(Node *this, Node *suc, Node *pred, int fd, char *info);
void PREDrcv(Node *this, Node *suc, Node *pred, char *info);
void* RSPrecv (char *info, Node *this, Node *suc, Element **ht);
void FNDrecv (char *info, Node *this, Node *suc, Node *pred);
void* fnd(char *info, Node *this, Node *suc, Node *pred, int seq, Element **ht, struct Save *addr);
void RingLeave(Node *this, Node *suc, Node *pred);

int verifyPort(char *port);
int verifyAddr(char *addr);

int compareDist(int this, int a, int b, int flag);
unsigned int dist(int this, int measuree);

#endif