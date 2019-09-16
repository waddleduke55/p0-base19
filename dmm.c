#include <stdio.h>  // needed for size_t etc.
#include <unistd.h> // needed for sbrk etc.
#include <sys/mman.h> // needed for mmap
#include <assert.h> // needed for asserts
#include "dmm.h"

/* 
 * The lab handout and code guide you to a solution with a single free list containing all free
 * blocks (and only the blocks that are free) sorted by starting address.  Every block (allocated
 * or free) has a header (type metadata_t) with list pointers, but no footers are defined.
 * That solution is "simple" but inefficient.  You can improve it using the concepts from the
 * reading.
 */

/* 
 *size_t is the return type of the sizeof operator.   size_t type is large enough to represent
 * the size of the largest possible object (equivalently, the maximum virtual address).
 */

typedef struct metadata {
  size_t size;
  struct metadata* next;
  struct metadata* prev;
} metadata_t;

/* Head of the freelist: pointer to the header of the first free block. */
static metadata_t* freelist = NULL;

void* dmalloc(size_t numbytes) {

  if(freelist == NULL) {
    if(!dmalloc_init())
      return NULL;
  }

  assert(numbytes > 0);

  /* your code here */
  //curr is a copy of the freelist header ptr. Used to manipulate freelist
  metadata_t* curr = freelist;
  //have a copy to freelist so that you can free blocks that aren't at the start
  metadata_t* block= freelist;
  //actual number of bytes
  size_t anumbytes = ALIGN(numbytes);

    //iterate through the file

    while(curr!=NULL){
        if (curr->size >= (anumbytes+METADATA_T_ALIGNED)){ //curr block is big enough to allocate memory for desired space
            //Splitting the block
            //if its at the start (i.e. the prev ptr points to the freelist header)
            if(curr->prev == NULL){
                //move the freelist ptr to the next block
                freelist= (metadata_t*) (((void*) curr) + METADATA_T_ALIGNED + anumbytes);
                freelist->size = curr->size - METADATA_T_ALIGNED - anumbytes; //adjust the metadata in freelist to be a smaller size since a block has been taken out
                freelist->next = curr->next;
            }

            //if the block is in the middle somewhere in the freelist
            else{
                //move block to where curr is in middle of list
                block=curr;
                //move block to point to the space after allocation of space
                block= (((void*) curr) + METADATA_T_ALIGNED + anumbytes);
                //change the size of this "free" block to account for change
                block->size=curr->size-METADATA_T_ALIGNED-anumbytes;
                //move curr's previous block to point to the free block
                curr->prev->next = block;
                //move block to curr's previous next
                block->prev=curr->prev;
                //curr's next is where block is, so block's next is curr's next next
                block->next=curr->next->next;
            }
            //set curr's next and prev to NULL to show it's not free
            curr->next = NULL;
            curr->prev = NULL;
            //set curr's size to the size needed to allocate
            curr->size=anumbytes;
            return ((void*) curr)+METADATA_T_ALIGNED; //return curr's spot but without the header (useless to user)

        }
        return NULL;

        //*****TALK TO TUTOR ABOUT OTHER POSSIBLE CASES???
    }



}

void dfree(void* ptr) {
  /* your code here */
}

/*
 * Allocate heap_region slab with a suitable syscall. Treat it as one large free block on freelist.
 */
bool dmalloc_init() {

  size_t max_bytes = ALIGN(MAX_HEAP_SIZE);

  //  freelist = (metadata_t*) sbrk(max_bytes);
  freelist = (metadata_t*)
     mmap(NULL, max_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (freelist == (void *)-1)
    return false;
  freelist->next = NULL;
  freelist->prev = NULL;
  freelist->size = max_bytes-METADATA_T_ALIGNED;
  return true;
}


/* for debugging; can be turned off through -NDEBUG flag*/
void print_freelist() {
  metadata_t *freelist_head = freelist;
  while(freelist_head != NULL) {
    DEBUG("\tFreelist Size:%zd, Head:%p, Prev:%p, Next:%p\t",
	  freelist_head->size,
	  freelist_head,
	  freelist_head->prev,
	  freelist_head->next);
    freelist_head = freelist_head->next;
  }
  DEBUG("\n");
}
