/*
 * mm_alloc.c
 *
 * Stub implementations of the mm_* routines. Remove this comment and provide
 * a summary of your allocator's design here.
 */

#include "mm_alloc.h"

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
/* Your final implementation should comment out this macro. */
void *zee = NULL;
struct nca {
  size_t size;
  bool free;
  struct nca* next;
  struct nca* prev;
};
#define BAR_SIZE sizeof(struct nca)
struct nca* nca2 (struct nca *prev, size_t size){
	
	struct nca *newNca;
	newNca = sbrk(0); 
	sbrk(size+BAR_SIZE); 
	if (prev){	
		prev->next=newNca;
		newNca->prev=prev;
	}
	newNca->size = size;
	newNca->next = NULL;
	newNca->free = false;

	return newNca;	
}
struct block* splitBlock(struct nca* nca1, size_t size){
	
	if (nca1->size>=1+size+BAR_SIZE){  
		struct nca * zwide = (struct nca *)((char*)nca1+BAR_SIZE+size); 
		zwide->size = nca1->size-(size+BAR_SIZE); 
		zwide->free=true;
		zwide->next=nca1->next;	
		zwide->prev=nca1;
		if (nca1->next){
			nca1->next->prev=zwide;
		}
		nca1->size=size;
		nca1->next = zwide;
	}
	return nca1;	
}

struct nca* Free(struct nca *prev, size_t size){	
	struct nca *thisnca = zee;	

	while (true){	
		if (!thisnca){	
			return nca2(prev ,size);
		}
		if (thisnca&&thisnca->free && (thisnca->size)>=size){	
			thisnca=splitBlock(thisnca,size);
			thisnca->free=false;
			return thisnca;	
		}
		prev = thisnca; 
		thisnca = thisnca->next;	
	}
	return 0;	
}
void merge(struct nca *thisnca){	
	if (thisnca->next){	
		if (thisnca->next->free){	
			thisnca->size+=thisnca->next->size+BAR_SIZE;	
			if (thisnca->next->next){	
				thisnca->next->next->prev=thisnca;
				thisnca->next=thisnca->next->next;
			}
		}
	}
	if (thisnca->prev){
		if (thisnca->prev->free){	
			merge(thisnca->prev);
		}
	}
}


void* mm_malloc(size_t size)	
{
	struct nca* newNca;
	if (zee){	
		struct nca* finalnca=zee;
		newNca=Free(finalnca, size);
	}else{	
                newNca=nca2(NULL,size);
		zee=newNca;
	}
  return (void*) ((long) newNca+BAR_SIZE);

}
