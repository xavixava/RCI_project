//checks.c

#include "checks.h"

void N_check(char *str)
{
	if(str==NULL)
	{
		fprintf(stdout, "Nothing read");
		exit(1);
	}
}