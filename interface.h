#ifndef InterfaceHeader
#define InterfaceHeader

/* Header file for the definition of the project interface */
#include "geral.h"
#include "node.h"


#define MAX_NODES 32

void interface(char **args);
char *handle_args(char *arg, char *key);
char *handle_instructions(char *arg);

#endif