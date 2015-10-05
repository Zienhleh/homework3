/*
 * mm_alloc.c
 *
 * Stub implementations of the mm_* routines. Remove this comment and provide
 * a summary of your allocator's design here.
 */


#include "mm_alloc.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

/* Thread safety has not been implemented */

#define ALIGN_SIZE 5
pthread_mutex_t threadlock=PTHREAD_MUTEX_INITIALIZER;

void* mm_malloc(size_t size)
{
  if(size%ALIGN_SIZE>0){   //Convert to multiple of ALIGN_SIZE for alignment of pointers. It has been set to 4 because int size is smallest.
    size=(ALIGN_SIZE-size%ALIGN_SIZE)+size;
  }
  
  if(head==NULL){ 
    head=sbrk(BLOCK_SIZE);
    if(head==(void *)-1){
      return NULL;     // the head failed to be allocated
    }
    head->size=length;
    head->next=NULL;
    head->prev=NULL;
    head->free=0;
    head->ptr=sbrk(length);
    if(head->ptr==(void *)-1){
      return NULL;     //Could not allocate space for data
    }
    tail=head;
    return head->ptr;
  }

  // then Check if there is a open block big enough
  s_block_ptr block = fit_block(head,length);
  if(block!=NULL){
    return block->ptr;
  }

  //Last option. Extend heap

  block=extend_heap(tail,length);
  
  if(block!=NULL){
    tail=block;
    return block->ptr;
  }

  return NULL; //We couldn't allocate memory
  
  
}

void* mm_realloc(void* ptr, size_t length)
{
  if(ptr==NULL){  //You gave NULL? We will just allocate space
    return mm_malloc(length);
  }
  
  s_block_ptr oldblock= get_block(ptr); //Find old block. we need size of it
  if(oldblock->size>length){  //Size you want is too small
    return NULL;
  }
  void* newptr= mm_malloc(length);        //Get a new block of 'size'
  //s_block_ptr newblock=get_block(newptr);
  memcpy(newptr,ptr,oldblock->size);
  
  mm_free(ptr);

  return newptr;
}

/*valid pointer:
		- attaining block address, then mark free, but uf the the one before was free we go back and block and fuse, but in the instance where we at the last memory we
                  free the memory. else if there arent any free block we go back to norm state. 
*/
void mm_free(void* ptr)
{
  s_block_ptr temp = get_block(ptr);
  if(temp!=NULL){
    memset(temp->ptr,0,temp->size);
    temp->free=1;
  }else{
    return;  //the block implemented was not found
  }

  //fusion(temp);  //Fuse block to the left then recurse for right
  
  s_block_ptr curr = head;

  while(curr!=NULL){
    if(curr->free){
      fusion(curr);
    }
    curr=curr->next;
  }
  
  if(tail->free){ //Last block is free,lets move 'break'
    if(tail->prev==NULL){ //we are at the head
      sbrk(-1*(tail->size+BLOCK_SIZE));
      //brk(tail);
      tail=NULL;
      head=NULL;
    }else{
      if((void *)tail<=tail->ptr){  //Normal order
	s_block_ptr temp=tail->prev;
	temp->next=NULL;
	sbrk(-1*(tail->size+BLOCK_SIZE));
	//brk(tail);
	tail=temp;
	}
    }
  }
}
  
s_block_ptr fit_block(s_block_ptr headcopy,size_t size){
  s_block_ptr avail=NULL;
  while(headcopy!=NULL){
    if(headcopy->free && headcopy->size>=size){ //Find a free block that bigger or equal to the space we need
      if(avail==NULL){
	avail=headcopy;
      }else if(avail->size>headcopy->size){     //We want to split biggest block first to avoid leaving small unusable spaces. Note Minsize
	avail=headcopy;                         // of block will be equal to ALIGN_SIZE
      }
    }
    headcopy=headcopy->next;
  }
  if(avail==NULL){ // Can't find suitable block
    return NULL;
  }

  if(avail->size==size){ //We found a block of the same size
    avail->free=0;
    return avail;
  }

  //the smallest block that is bigger than the required size
  // We need to split it

  split_block(avail,size);
  avail->free=0;
  return avail;
}
    
s_block_ptr extend_heap (s_block_ptr last , size_t s){ //Simply adds a block to the end
  s_block_ptr newblock=sbrk(BLOCK_SIZE);
  if(newblock==(void *)-1){
    return NULL;
  }
  last->next=newblock;
  newblock->prev=last;
  last=newblock;

  newblock->size=s;
  newblock->free=0;
  newblock->ptr=sbrk(s);
  if(newblock->ptr==(void *)-1){
    return NULL;
  }
  newblock->next=NULL;
  return newblock;
}

void split_block(s_block_ptr b, size_t s){ //Spliting blocks
  if(b==NULL){
    return;
  }
  
  if(b->size>s){ //It's possible to reuse the second part
    s_block_ptr newblock=sbrk(BLOCK_SIZE); //We initalise the new pointer_data at the top of the heap. This increases complexity but saves space
    newblock->next=b->next;                // otherwise to keep the (pointer_data|data) structure we need to remaining space to be more than 40.
    newblock->prev=b;
    b->next=newblock;
    
    newblock->size=b->size-s;
    newblock->free=1;
    newblock->ptr=b->ptr+b->size;
    b->size=s;
    if(newblock->next!=NULL)
      newblock->next->prev=newblock;
    else
      tail=newblock;
  }else{
    b->size=s;
  }
}

void print(s_block_ptr headcopy){  //Simple print that lists all the items in the linked list. Just pass head to it.
  while(headcopy!=NULL){           // we could just access head as it is a global variable. We would still need to create a temp to move.
    printf("Info add:%p\nBlock add:%p\nSize:%i\nFree:%i\n",headcopy,headcopy->ptr,
	   (int)headcopy->size,
	   headcopy->free);
    headcopy=headcopy->next;
  }
}

s_block_ptr get_block (void *z){
  s_block_ptr temp=head; //Copying the head
  while(temp!=NULL){
    if(temp->ptr==d){
      return temp;
    }
    temp=temp->next;
  }
  return NULL; //Not found
}

s_block_ptr fusion(s_block_ptr d){ //Block fusion
  if(d==NULL){
    return NULL;
  }
  
  if(d->next!=NULL && d->next->free){   //We always free from the right we then recurse by passing the previous block
     s_block_ptr temp=d->next;
      d->size+=temp->size;
      temp->size=0; //We took the data. This is done so we can try to reclaim the block head memory
      if((void *)temp<=temp->ptr){ //Data is behind block as intended
	d->next=temp->next;
	d->size+=BLOCK_SIZE;
	if(temp->next!=NULL){
	  temp->next->prev=d;
	}else{
	  tail=d;
	}
      }else{             
	temp->ptr=temp;
      }
  }
  if(d->prev!=NULL && d->prev->free){
    fusion(d->prev);
    d->ptr=d->prev->ptr;
  }
  return d;
}

void lock_mutex(){
  if(pthread_mutex_lock(&threadlock)){
    exit(-1); 
  }
}

void unlock_mutex(){
  if(pthread_mutex_unlock(&threadlock)){
    exit(-1); 
  }
}
