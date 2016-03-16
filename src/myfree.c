#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>
#include"mymalloc.h"

typedef struct block_header{
  unsigned int size : 29,
               zero : 2,
               alloc : 1;
} block_header;

static char* base_heap = NULL;
unsigned int HEAP_TOTAL_SIZE = 0; /*proportional to the 1st call of mymalloc*/

void* mymalloc(size_t size){

  unsigned int HEADER_BLK_SIZE = sizeof(block_header); /*size header*/
  unsigned int ALIGNED_PAYLOAD_SIZE = ((((((unsigned int)size)-1)>>2)<<2)+4); /*payload rounded for alignement*/
  unsigned int BLK_SIZE = (HEADER_BLK_SIZE) + (ALIGNED_PAYLOAD_SIZE); /*total block size (header + rounded payload)*/

    /*1st call of mymalloc : initialize the heap and allocate the 1st block*/

    if(!base_heap){

      /*heap initialize*/
      HEAP_TOTAL_SIZE = ((BLK_SIZE)*1024);
      base_heap = (char*)sbrk(HEAP_TOTAL_SIZE);
      char* end_heap = (char*)sbrk(0);

      /*if sbrk does not work*/
      if(base_heap == (char*)-1 || end_heap == (char*)-1) {
        return NULL;
      }

      /*otherwise allocate the 1st block*/
      block_header* first_blk = (block_header*) base_heap;
      first_blk->size = BLK_SIZE;
      first_blk->zero = 2;
      first_blk->alloc = 1;
      /*block_header* second_blk = (block_header*)(base_heap+1);
      second_blk->size = HEAP_TOTAL_SIZE - BLK_SIZE;
      second_blk->zero = 2;
      second_blk->alloc = 0;*/

      /*adress 1st block*/
      return (void*)( ((char*)first_blk) + (HEADER_BLK_SIZE) );
    }

    /*second call of mymalloc : find a block in the heap and allocate*/
    else if (HEAP_TOTAL_SIZE < BLK_SIZE) {
      return NULL;
    }

    else{
      block_header* ptr = (block_header*)base_heap;
      block_header* best = NULL;
      unsigned int i = 0;

      /*travel the heap, starting point is the base_heap*/
      while(i<HEAP_TOTAL_SIZE){
        /*1st case : if the block is allocate we go to the next one*/
        if(ptr->alloc == 1 && (ptr->size != 0)){
          i += (ptr->size);
          ptr = (block_header*)((char*)ptr + ptr->size);
        }

        /*second case : if the block is free we allocate one and it was not allocate before*/
        else if (ptr->alloc == 0 && (ptr->size == 0)){
          i += (BLK_SIZE);
          ptr->size = BLK_SIZE;
          ptr->zero = 2;
          ptr->alloc = 1;
          return (void*)( ((char*)ptr) + (HEADER_BLK_SIZE) );
        }

        /*third case : if the block is free, was allocate before, but is not big enough, jump to the next one*/
        else if (ptr->alloc == 0 && ptr->size< BLK_SIZE){
          i = i + (ptr->size);
          ptr = (block_header*)((char*)ptr + ptr->size);
        }

        /*fourth case : if the block is free, was allocate before and is big enough, we allocate one*/
        else if (ptr->alloc == 0 && ptr->size>=BLK_SIZE){

	  if(ptr->size==BLK_SIZE){
	    return (void*)( ((char*)ptr) + (HEADER_BLK_SIZE) );
	  }

	  else if (best==NULL || (ptr->size) < (best->size)){
	    best = ptr;
	    i = i+ (ptr->size);
	  }
	  else {
	    i = i + (ptr->size);
	    ptr = (block_header*)((char*)ptr + ptr->size);
	  }
	}

        /*fifth case : if it did not find one*/
        else{
          return NULL;
        }

      }
      if(best==NULL){
	return NULL;
      }
      else {
	best->size = BLK_SIZE;
	best->alloc = 1;
	return (block_header*)((char*)best + best->size);
      }
    }
}
unsigned int myfree(void* ptr) {
  
    unsigned int HEADER_BLK_SIZE = (unsigned int)(sizeof(block_header));
    block_header* blk = (block_header*)(ptr-(HEADER_BLK_SIZE));
    if ((char*)ptr == base_heap+(HEADER_BLK_SIZE)){
      block_header* next = (block_header*)(base_heap+(blk->size));
      if (next->alloc==0 && blk->size==0){
	blk->alloc = 0;
        blk->size = HEAP_TOTAL_SIZE;
      }
      else if (next->alloc == 0){
	blk->alloc = 0;
	blk->size += next->size;
      }
      else
	blk->alloc = 0;
    }
    else {
      char* prev = base_heap;
      block_header* blk1 = (block_header*)prev;
      char* cur = prev + (blk1->size);
      block_header* blk2 = (block_header*)cur;
      while (cur != (char*)(blk)){
	prev += blk1->size;
	blk1 += blk1->size;
        cur += blk2->size;
	blk2 += blk2->size;
      }
      block_header* a = (block_header*)prev;
      block_header* b = (block_header*)cur;
      if (a->alloc == 0) {
	if (((block_header*)((char*)blk + (blk->size)))->alloc == 0) {
	  a->size += blk->size + ((block_header*)((char*)blk + (blk->size)))->size;
	}
	else {
          a->size += blk->size;
        }
      }
      else {
	if (((block_header*)((char*)blk + (blk->size)))->alloc == 0) {
	  blk->size += ((block_header*)((char*)blk + (blk->size)))->size;
          blk->alloc = 0;
        }
        else {
	  blk->alloc = 0;
        }
      }
    }
    return ((block_header*)base_heap)->size;
}


void* mycalloc(size_t size){
    /*block memory by mymalloc*/
    void* blk = mymalloc(size);
    char* ptr = (char*)blk;
    unsigned int i;
    unsigned int ALIGNED_PAYLOAD_SIZE = ((((((unsigned int)size)-1)>>2)<<2)+4);

    /*Travel the block memory by ptr and we initialize every bits to 0*/
    for(i=0 ; i<ALIGNED_PAYLOAD_SIZE ; i++){
        (*(ptr+i)) = 0;
    }

    /*return the blk and every value set to O*/

    return blk;
}

void main(){
  int* ii = (int*)mymalloc(16);
  printf("ii : %p\n", ii);
  printf("free : %d\n",myfree(ii));
  printf("%p\n", mymalloc(12));
  printf("free : %d\n", ii);
  printf("%p\n", mymalloc(20));
  int* i = (int*) mycalloc(16);
  printf("%p\n", i); 
  printf("%d\n", *i); /*valeur est zero donc mycalloc ok*/
}
