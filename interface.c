//interface.c

#include "interface.h"
//#include "checks.h"

void interface(char **args)
{
	char buffer[128], *key, *address, *port, *info;
	memset(buffer, '\0', 128);
	 
	key = args[1];
	address = handle_args(args[2], key);
	port = handle_args(args[3], key);
	
	if(fgets(buffer, 128, stdin)==NULL)
	{
		fprintf(stdout, "Nothing read\n");
		exit(1);
	}
	
	info = handle_instructions(buffer);
	
	if (strcmp(buffer, "new\n") == 0) 
	{
		fprintf(stdout, "Initiating a new ring\n");
	} else if(strcmp(buffer, "pentry\n") == 0)
	{
		fprintf(stdout, "pentry\n");
	}else	fprintf(stdout, "Comando Desconhecido ou ainda não implementado\n");
	
	return;
}

char *handle_args(char *arg, char *key)
{
	char *final, *aux;
	
	aux = strrchr(arg, '.');
	final = aux+1;
	if(aux==NULL)
	{
		fprintf(stdout, "Por favor, formate devidamente os argumentos\n");
		exit(1);
	}	
	
	*aux='\0';
	
	//ADICIONAR VERIFICAÇÕES EXTRA, COMO VERIFICAR SE O ENDEREÇO E O PORTO SÃO NÚMEROS E OS SEUS TAMANHOS
	
	if(strcmp(arg, key)!=0)
	{
		fprintf(stdout, "Por favor, formate devidamente os argumentos\n");
		exit(1);
	}
	fprintf(stdout, "%s\n", final);
	return final;
}

char *handle_instructions(char *arg)
{
	char *final, *aux;
	
	aux = strrchr(arg, ' ');
	
	if(aux==NULL && strcmp(arg, "new\n")!=0)
	{
		fprintf(stdout, "Por favor, formate devidamente as instruções\n");
		exit(1);
	}	
	
	if(aux!=NULL)
	{
		final = aux+1;
		*aux='\0';
	}
	
	return final;
}