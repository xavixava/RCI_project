//interface.c

#include "interface.h"
#include "node.h"
//#include "checks.h"

void interface(char **args)
{
	char buffer[128], *key, *address, *port, *info;
	int chave;
	Node *this;
	memset(buffer, '\0', 128);
	 
	key = args[1];
	chave = atoi(key);
	if(chave<0|| chave>=MAX_NODES)
	{
		fprintf(stdout, "Chave inválida\n");
		exit(1);
	}
	
	address = handle_args(args[2], key);
	port = handle_args(args[3], key);
	
	if(fgets(buffer, 128, stdin)==NULL)
	{
		fprintf(stdout, "Nothing read\n");
		exit(1);
	}
	
	info = handle_instructions(buffer);
	
	if ((strcmp(buffer, "new\n") == 0)||(strcmp(buffer, "n\n") == 0)) 
	{
		fprintf(stdout, "Initiating a new ring\n");
		this = new(chave, address, port);
		fprintf(stdout, "%d %s %s\n", this->chave, this->address, this->port);
		
	} else if(strcmp(buffer, "pentry") == 0)
	{
		fprintf(stdout, "pentry\n");
	}else	fprintf(stdout, "Comando Desconhecido ou ainda não implementado\n");
	
	return;
}

char *handle_args(char *arg, char *key)
{
	char *final, *aux, *point=".";
	
	aux = strstr(arg, point);
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
		fprintf(stdout, "Por favor, use a mesma chave em todos os argumentos\n");
		exit(1);
	}
	
	return final;
}

char *handle_instructions(char *arg)
{
	char *final, *aux, *space=" ";
	
	aux = strstr(arg, space);
	
	
	if(aux==NULL && strcmp(arg, "n\n")!=0)
		if(strcmp(arg, "new\n")!=0)
		{
			fprintf(stdout, "%s", arg);
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