#ifndef InterfaceHeader
#define InterfaceHeader

/* Header file for the definition of the project inetrface */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void interface(char **args);
char *handle_args(char *arg, char *key);
char *handle_instructions(char *arg);

#endif