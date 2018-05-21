#include <stdint.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>
#include "nu_mem.h"
#include <math.h>

void coallesce();
typedef struct Cell {
    int64_t size;
    struct Cell* next;
    struct Cell* prev;
} Cell;

Cell* free_list=NULL;


static const int64_t CHUNK_SIZE = 65536;
static char* free_array = NULL; // array pointer for current space in array

// You should update these counters on memory allocation / deallocation events.
// These counters should only go up, and should provide totals for the entire
// execution of the program.
static int64_t nu_malloc_count  = 0; // How many times has malloc returned a block.
static int64_t nu_malloc_bytes  = 0; // How many bytes have been allocated total
static int64_t nu_free_count    = 0; // How many times has free recovered a block.
static int64_t nu_free_bytes    = 0; // How many bytes have been recovered total.
static int64_t nu_malloc_chunks = 0; // How many chunks have been mmapped?
static int64_t nu_free_chunks   = 0; // How many chunks have been munmapped?

int64_t nu_free_list_length()
{
    // TODO: This should return how many blocks of already allocated memory
    //   you have available for future malloc requests.
    return 4;
}

void nu_mem_print_stats()
{
    fprintf(stderr, "\n== nu_mem stats ==\n");
    fprintf(stderr, "malloc count: %ld\n", nu_malloc_count);
    fprintf(stderr, "malloc bytes: %ld\n", nu_malloc_bytes);
    fprintf(stderr, "free count: %ld\n", nu_free_count);
    fprintf(stderr, "free bytes: %ld\n", nu_free_bytes);
    fprintf(stderr, "malloc chunks: %ld\n", nu_malloc_chunks);
    fprintf(stderr, "free chunks: %ld\n", nu_free_chunks);
    fprintf(stderr, "free list length: %ld\n", nu_free_list_length());
}

// return NULL if unavailbale, else returns location in free list
Cell* findLocation(int input) {
    if (free_list == NULL){
	return NULL;
    }
    else {
	Cell* currentcell = free_list;
	while (currentcell->next != NULL){
	    if (currentcell->size >= input){
		return currentcell;
	    }
	    else {
		currentcell = currentcell->next;
	    }
	}
	// because this will only happen if you do not return address
	return NULL;
    }
}

void* nu_malloc(size_t usize) {
    // TODO: Make this allocate memory.
    //
    // Allocate small blocks of memory by allocating 64k chunks
    // and then satisfying multiple requests from that.
    //
    // Allocate large blocks (>= 64k) of memory directly with
    // mmap.
    void* pointa;
    Cell* inside;
    int metadata = sizeof(int64_t);// + sizeof(inside) + sizeof(inside);
    if (usize >= 65536) {
	pointa = mmap(0, usize+metadata, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

	inside = pointa;
	inside->size = usize;
	return pointa;
    }
    else {
	Cell *gotoblock = findLocation(usize+metadata);
	if (gotoblock == NULL) {
	    // create entire block of memory
	    Cell* wholeblock;
	    pointa = mmap(0, 65536, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	    // create used block
	    wholeblock = pointa;
	    wholeblock->size = 65536 - metadata;
	    // get freespace block
	    void* traversepointer; // do this so you can increment by correct amount based on pointer size
	    traversepointer = wholeblock+metadata+usize;
	    inside = traversepointer;
	    inside->size = wholeblock->size - (usize + metadata) ;
	    // assign freespace block to free list
	    if (free_list == NULL) {
		free_list = inside;
	    }
	    else {
		inside->next = free_list;
		free_list->prev = inside;
		free_list = inside;
	    }
	    return pointa;
	}
	else {
	    // case where you find the Cell you need in free list
	    Cell* newfree;
            void* traversepointer;
	    
	    traversepointer = gotoblock + usize + metadata;
	    newfree = traversepointer;
	    newfree->size = gotoblock->size - (usize+metadata);
	    // reassign pointers of Cells on free list
	    Cell* prevblock = gotoblock->prev;
	    if (prevblock !=  NULL) {
		prevblock->next = newfree;
	    }
	    Cell* nextblock = gotoblock->next;
	    if (nextblock != NULL) {
		nextblock->prev = newfree;
	    }
	    newfree->prev = prevblock;
	    newfree->next = nextblock;
	    return gotoblock;
	}
    }
    coallesce();
}

void nu_free(void* addr) 
{
    // TODO: Make this free memory.
    //
    // Free small blocks by saving them for reuse.
    //   - Stick together adjacent small blocks into bigger blocks.
    //   - Advanced: If too many full chunks have been freed (> 4 maybe?)
    //     return some of them with munmap.
    // Free large blocks with munmap.
    //
    Cell* cellpointer;
    cellpointer = addr;
    int metadata = sizeof(int64_t) + sizeof(cellpointer) + sizeof(cellpointer);
    
    if (cellpointer->size >= 65536) {
	munmap(addr, cellpointer->size+metadata);
    }
    else {
	addr = addr-metadata;
	Cell* newcell;
	newcell = addr;

	if (free_list == NULL){
	    newcell = free_list;
	}
	else {
	    newcell->next = free_list;
	    free_list->prev = newcell;
	    free_list = newcell;
	}
    coallesce();
    }
}

void coallesce() {
    Cell* currstaticnode = free_list;
    int metadata = sizeof(int64_t) + sizeof(currstaticnode) + sizeof(currstaticnode);
    // int metadata = sizeof(cell);
    while (currstaticnode != NULL) {
	int found = 0;
	void* traverser;
	traverser = currstaticnode + metadata + currstaticnode->size;
	Cell* currtraversenode = currstaticnode->next;
	while (currtraversenode != NULL && found == 0){
	    if (traverser == currtraversenode){
		found = 1;
		currstaticnode->size = currstaticnode->size + metadata + currtraversenode->size;
		Cell* nextcell = currtraversenode->next;
		currstaticnode->next = nextcell;
		if (nextcell != NULL){
		    nextcell->prev = currstaticnode;
		}
	    }
	    else {
		currtraversenode = currtraversenode->next;
	    }
    	}
	currstaticnode = currstaticnode->next;
    }
}
