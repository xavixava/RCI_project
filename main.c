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
	struct sigaction act;

	memset(&act,0,sizeof act);
	act.sa_handler=SIG_IGN;
	if(sigaction(SIGPIPE,&act,NULL)==-1)exit(1);//error
	
	if(argc!=4){
		fprintf(stdout, "Número de argumentos errado\n");
		exit(1);
	}
	
	system("clear");
	fprintf(stdout, "\n\t\tWelcome to Marta Bezerra & Xavier Antunes network\t\n");
	fprintf(stdout, "\t\t                (press h for help)                 \t\n");
	
	interface(argv);
	
	return 0;
}