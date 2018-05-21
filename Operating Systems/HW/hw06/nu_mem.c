#include <stdint.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>

#include "nu_mem.h"

static const int64_t CHUNK_SIZE = 65536;
static int64_t CELL_SIZE = 24;

// You should update these counters on memory allocation / deallocation events.
// These counters should only go up, and should provide totals for the entire
// execution of the program.
static int64_t nu_malloc_count  = 0; // How many times has malloc returned a block.     ready
static int64_t nu_malloc_bytes  = 0; // How many bytes have been allocated total        ready
static int64_t nu_free_count    = 0; // How many times has free recovered a block.      ready
static int64_t nu_free_bytes    = 0; // How many bytes have been recovered total.       ready (not incl free_chunks())
static int64_t nu_malloc_chunks = 0; // How many chunks have been mmapped?              ready
static int64_t nu_free_chunks   = 0; // How many chunks have been munmapped?            ready

static int64_t nu_curr_free_blocks = 0; // The number of free blocks currently present.


typedef struct Cell{
    uint64_t size;
    struct Cell* next;
    struct Cell* prev;
}Cell;

Cell* free_list=NULL;

///
/// \return the number of {@code Cells} available on the free list
int64_t
nu_free_list_length()
{

    // TODO: This should return how many blocks of already allocated memory
    // TODO: you have available for future malloc requests.

    return nu_curr_free_blocks;
}

/// Prints the statistics of the program
void
nu_mem_print_stats()
{
    fprintf(stderr, "\n== nu_mem stats ==\n");
    fprintf(stderr, "malloc count: %lld\n", nu_malloc_count);
    fprintf(stderr, "malloc bytes: %lld\n", nu_malloc_bytes);
    fprintf(stderr, "free count: %lld\n", nu_free_count);
    fprintf(stderr, "free bytes: %lld\n", nu_free_bytes);
    fprintf(stderr, "malloc chunks: %lld\n", nu_malloc_chunks);
    fprintf(stderr, "free chunks: %lld\n", nu_free_chunks);
    fprintf(stderr, "free list length: %lld\n", nu_free_list_length());
}

void
free_chunks() {
    while(nu_curr_free_blocks>4) {
        // get a pointer to the current head
        Cell* temp = free_list;
        // set the head to the element after it
        free_list = free_list->next;
        munmap(temp, temp->size);
        nu_curr_free_blocks--;
        nu_free_chunks++;
    }
}

/// Combines free cells that are next to each other in memory.
/// \param newCell an address that was just added and should be checked
/// for possible combinations with adjacent cells.
void coalesce(Cell* newCell) {
    // coalesce newCell with the one after it
    if(newCell->next!=NULL &&
       ((&(*(newCell->next))) == (((void*)&(*newCell)) + newCell->size))) {
        Cell* tmp = newCell->next;

        //fix links
        newCell->next = tmp->next;
        if(tmp->next!=NULL) {
            Cell* tmp2=tmp->next;
            tmp2->prev = newCell;
        }

        // update size
        printf("newCell's size is %lld\n", newCell->size);
        printf("next's size is %lld\n", tmp->size);

        newCell->size=newCell->size+tmp->size;

        nu_curr_free_blocks--;
    }

    // coalesce newCell's prev with newCell
    Cell* tmpNext = newCell->next;
    Cell* tmpPrev = newCell->prev;

    if(tmpPrev!=NULL &&
       (&(*newCell) == (((void*)&(*tmpPrev)) + tmpPrev->size))) {

        // update the links
        tmpPrev->next=tmpNext;
        if(tmpNext!=NULL) {
            tmpNext->prev = tmpPrev;
        }

        // update the size

        printf("newCell's size is %lld\n", newCell->size);
        printf("prev's size is %lld\n", tmpPrev->size);
        tmpPrev->size=tmpPrev->size + newCell->size;

        nu_curr_free_blocks--;
    }
}


/// Adds the given cell back to the free list.
///
/// @param newCell a pointer to the very beginning of the memory
/// chunk that needs to be added back to the free list
void
add_to_free_list(Cell* newCell) {

    // newCell is going to be the new head of the free list
    if(free_list==NULL || (&(*free_list) > &(*newCell))) {
        if(free_list!=NULL) {
            free_list->prev=newCell;
        }
        newCell->next=free_list;
        newCell->prev=NULL;
        free_list=newCell;
        nu_curr_free_blocks++;      // successfully added a new chunk of memory
        coalesce(newCell);

        return ;
    }

    // p will be used to traverse the free list
    Cell* p = free_list;

    // traverse until EOL or until next element's address is bigger
    // than newCell's
    while((p->next!=0) && (&(*(p->next)) < &(*newCell))) {
        p=p->next;
    }

    // insert newCell at its correct place
    if(p->next!=NULL) {
        Cell *tmp = p->next;
        p->next = newCell;
        tmp->prev = newCell;
        newCell->next = tmp;
        newCell->prev = p;
    }
    else {
        newCell->next=NULL;
        p->next=newCell;
        newCell->prev=p;
    }
    nu_curr_free_blocks++;      // successfully added a new chunk of memory

    // check if newCell has introduced opportunities for coalescing
    coalesce(newCell);

    if(nu_free_list_length()>4) {
        //free_chunks();
    }
}



void
add64K() {
    // allocate 64K
    void* addr = mmap(NULL, CHUNK_SIZE, PROT_READ|PROT_WRITE,
                      MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    Cell* cellp2 = (Cell*) addr;
    nu_malloc_chunks++;     // mmap was called

    cellp2->size=CHUNK_SIZE;
    // add the 64K to the list
    if(!free_list) {
        cellp2->next=NULL;
        cellp2->prev=NULL;
        free_list=cellp2;
    }
    else {
        add_to_free_list(cellp2);
    }
}





/// Looks for a memory {@code Cell} on the free list that is at least as big
/// as the required amount {@param realSize} and if found, removes it from
/// the list.
///
/// @param realSize the total amount of memory that is needed
///
/// @return a {@code Cell} {@code pointer} to the very beginning of the
/// suitable chunk of memory or NULL if such is not found.
Cell*
find_memory(size_t realSize) {
    if(!free_list) {
        add64K();
    }

    Cell* p=free_list;

    // use p to traverse the whole list
    while(p) {
        Cell* tmpNext=p->next;
        printf("p is not NULL\n");
        printf("p's size is %lld\n", p->size);
        printf("real size is %lld\n", (uint64_t)realSize);

        //printf("free list's curr size is %lld\n", p->size);
        // a chunk big enough was found and needs to be removed from the list

        if((p->size < realSize) ||
                (((p->size) - (uint64_t)realSize) <= CELL_SIZE)) {
            //printf("\n\n\ngive me more memory\n\n\n");
            //add64K();
            free_list=NULL;
            return find_memory(realSize);
        }


        if((p->size < realSize) && !tmpNext) {
            break;
        }
        if(((p->size - realSize)<CELL_SIZE) && !tmpNext) {
            break;
        }

        if(p->size >= realSize) {

            Cell* tempNext = p->next;
            Cell* tempPrev = p->prev;
            // size is bigger than necessary
            if(p->size > realSize) {
                // create a new cell for the extra memory
                Cell* tmp;
                tmp = (((void*)&(*p)) + realSize);
                // set links out and size of the cell for the extra memory
                tmp->next=p->next;
                printf("extra-memory's size is %ld\n", tmp->size);
                printf("curr memory's size is %ld\n", p->size);
                tmp->prev = p->prev;
                tmp->size=p->size - realSize;

                // set links into the cell for the extra memory
                if(tempNext!=NULL) {
                    tempNext->prev = tmp;
                }
                if(tempPrev!=NULL) {
                    tempPrev->next = tmp;
                }
                else { // the remaining cell is the new head of the list
                    free_list=tmp;
                }

                // update p's size and it is ready to be returned
                p->size=realSize;

                return p;
            }
            else {

                // p has fitted and is the head, therefore tempNext becomes
                // the new head
                if (!tempPrev) {
                    if (tempNext != NULL) {
                        tempNext->prev = NULL;
                    }
                    free_list = tempNext;

                    // update p's size and it is ready to be returned
                    p->size = realSize;

                    // a block of memory has been removed from the free list
                    nu_curr_free_blocks--;

                    return p;
                } // p has fitted and is not the head
                else {
                    tempPrev->next = p->next;
                    if (tempNext != NULL) {
                        tempNext->prev = p->prev;
                    }

                    // update p's size and it is ready to be returned
                    p->size = realSize;

                    // a block of memory has been removed from the free list
                    nu_curr_free_blocks--;

                    return p;
                }
            }
        }

        p=p->next;
    }

    return NULL;
}




void*
nu_malloc(size_t usize)
{
    // TODO: Make this allocate memory.
    //
    // Allocate small blocks of memory by allocating 64k chunks
    // and then satisfying multiple requests from that.
    //
    // Allocate large blocks (>= 64k) of memory directly with
    // mmap.

    // realSize is the size of the memory including the size block
    int realSize = usize + 8;
    // the size needed is greater than 64K
    if(realSize>=CHUNK_SIZE) {
        void* memp = mmap(NULL, realSize, PROT_READ|PROT_WRITE,
                          MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        nu_malloc_chunks++;     // mmap was called

        //printf("\n\nREAL SIZE is %lld\n\n", realSize);

        // set the size and change the pointer to point to the data
        // segment from the memory
        Cell* node = (Cell*) memp;
        node->size = realSize;

        nu_malloc_count++;      // a block will be returned
        nu_malloc_bytes = nu_malloc_bytes + realSize;     // same as above
        return (void*)(&(node->next));
    }

    // if realSize is < CELL_SIZE make it as big as cell size
    if(realSize<CELL_SIZE) {
        realSize=CELL_SIZE;
    }

    // size is less than 64k check if there is enough memory on the free list
    // and get it
    Cell* cellp=find_memory(realSize);

    // increase the counters
    nu_malloc_count++;
    nu_malloc_bytes=nu_malloc_bytes+realSize;


    return (void*)(&(cellp->next));

}


/// Frees the memory associated witht the given {@param addr} {@code pointer}.
///
/// \param addr the pointer whose memory should be cleaned
void
nu_free(void* addr) 
{
    // TODO: Make this free memory.
    //
    // Free small blocks by saving them for reuse.
    //   - Stick together adjacent small blocks into bigger blocks.
    //   - Advanced: If too many full chunks have been freed (> 4 maybe?)
    //     return some of them with munmap.
    // Free large blocks with munmap.
    addr-=8;
    Cell* mem = (Cell*)addr;

    nu_free_count++;
    nu_free_bytes+=mem->size;

    if((mem->size >= CHUNK_SIZE) || nu_free_list_length()>4) {
        nu_free_chunks++;
        munmap(mem, mem->size);
    }
    else {
        add_to_free_list(mem);
    }


}

