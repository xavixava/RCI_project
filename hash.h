#ifndef HashHeader
#define HashHeader

#include "geral.h"

//typedef struct Element Element;
typedef struct Element{
	void *entry;
	int key;
	struct Element *next;
}Element;

#include "interface.h"

Element *AllocElem();
Element **CreateHashtable();
void FreeHash(Element **ht);
unsigned int hash(int key);
void *Retrieve(Element **ht, int hash, int key);
void *Retrieve_del(Element **ht, int hash, int key);
void Insert(Element **ht, int hash, int key, void *stuff);

#endif