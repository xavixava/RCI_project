/*
*
*	Computer Networks and the internet project by: Xavier Antunes & Marta Bezerra
*	Base de dados em anel-de-cordas
*
*
*/

#include "interface.h"

int main(int argc, char**argv)
{
	if(argc!=4){
		fprintf(stdout, "Número de argumentos errado\n");
		exit(1);
	}
	
	interface(argv);
	
	return 0;
}