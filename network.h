#ifndef NetworkOps
#define NetworkOps

/* Header file for the definition of the project  */
#include "geral.h"
#include "node.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int CreateTcpServer(char *port);
int CreateUdpServer(char *port);

int selfInform(Node *pred, Node *this);
int predInform(Node *suc, Node *old_suc);

void GenericTCPsend(Node *suc, char *message);

#endif