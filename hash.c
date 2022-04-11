#include "hash.h"

#define TABLE_SIZE 20

/*struct Element{
	void *entry;
	int key;
	struct Element *next;
};*/

/*********************************************************************************/
Element *AllocElem(){
	return (Element *) malloc(sizeof(Element));
}

/*
*	Allocates memory for the hashtable and returns a pointer to it
*/

Element **CreateHashtable(){
	Element **ht;
	int i;
	ht = (Element **) malloc(TABLE_SIZE*sizeof(Element *));
	if(ht==NULL){
		printf("Not enough memory");
		exit(1);
	}
	for (i = 0; i < TABLE_SIZE; i++){
		ht[i] = (Element *) malloc(sizeof(Element));
		if(ht[i]==NULL){
			printf("Not enough memory");
			exit(1);
		}
		ht[i]->entry=NULL;
		ht[i]->next=NULL;
		ht[i]->key=-1;
	}
	return ht;
}

/*
*	Function that deletes the Hash Table and frees the allocated space
*/

void FreeHash(Element **ht){ //FALTA dar free da informação guardada caso se dê free sem ter dado retrieve and del da info toda
	int i;
	Element *aux, *auxi;
	for (i = 0; i < TABLE_SIZE; i++){
		if(ht[i]->entry!=NULL){
			for(aux = ht[i]->next; aux!=NULL; aux = auxi){
				auxi = aux->next;
				free(aux);
			}
		}
		free(ht[i]);
	}
	free(ht);
	return;
}

/*
*	Hash function(already normalized)
*/

unsigned int hash(int key){
	return key%TABLE_SIZE;
}

/*
*	Retrieves the data of a key passed as argument
*/

void *Retrieve(Element **ht, int hash, int key){
	Element *aux;
	aux = ht[hash]->next;
	if(ht[hash]->key == key)return ht[hash]->entry;
	else while(aux!=NULL && aux->key != key)aux = aux->next;
	if(aux!=NULL)return aux->entry;
	return NULL;
}

/*
*	Retrieves the data of a key passed as argument and deletes it
*/

void *Retrieve_del(Element **ht, int hash, int key){
	Element *aux, *next;
	void *auxi;
	
	aux = ht[hash];
	while(aux->next != NULL && aux->key != key)aux = aux->next;
	auxi = aux->entry;
	if(aux->key!=key)return NULL;
	if(aux == ht[hash] && aux->next == NULL){
		aux->entry = NULL;
	}
	else if(aux->next != NULL){
		next = aux->next;
		aux->entry = next->entry;
		aux->key = next->key;
		aux->next = next->next;
		free(next);
	}
	
	return auxi;
}

/*
*	Inserts the data on the hashtable(still needs conflicts resolution)
*/

void Insert(Element **ht, int hash, int key, void *stuff){
	Element *aux, *auxi;
	
	if((ht[hash]->entry) == NULL){ 
		(ht[hash]->entry) = stuff;
		ht[hash]->next = NULL;
		ht[hash]->key = key;
	
	}
	else {
		
		auxi = AllocElem();
		aux = ht[hash];
		while(aux->next != NULL){
			aux = aux->next;
		}
		auxi->entry = stuff;
		auxi->next = NULL;
		auxi->key = key;
		aux->next = auxi;
	}
	return;
}
