
#include <stdlib.h>
#include <sys/mman.h>

#include "hw06_mem.h"

typedef struct icell {
    uint64_t size;
    struct icell* next;
}icell;

static const int64_t CHUNK_SIZE = 65536;
static const int64_t CELL_SIZE  = (int64_t)(sizeof(icell));
static const int64_t SIZE_FIELD_SIZE = (int64_t)(sizeof(uint64_t));  // entries on the stack have 8B size fields
// 2730 chunks * 24B each
//static const int64_t CHUNK_SIZE_OPT = CHUNK_SIZE % (SIZE_FIELD_SIZE+CELL_SIZE);

icell* stack=NULL;
int init=0;

void*
init_stack() {
    for(int i=0; i<100; i++) {
        void *p = mmap(0, CELL_SIZE + SIZE_FIELD_SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        icell *tmp = (icell *) p;
        tmp->size = CELL_SIZE + SIZE_FIELD_SIZE;
        tmp->next = stack;

        stack = tmp;
    }
}

void*
opt_alloc() {
    if(stack) {
        icell* res=stack;
        stack=stack->next;
        if(stack) {
            stack->size = res->size - CELL_SIZE - SIZE_FIELD_SIZE;
        }
        return ((void*)(res) + SIZE_FIELD_SIZE);
    }
    else {
        return hw06_malloc(CELL_SIZE);
    }
/*    if(stack->size > (SIZE_FIELD_SIZE+CELL_SIZE)) {
        // res points to the beginning of the memory on the stack
        icell* res = stack;
        // move the stack pointer by CELL_SIZE+SIZE_FIELD_SIZE to pop the stack
        stack = (icell*)(((void*)stack) + (CELL_SIZE+SIZE_FIELD_SIZE));
        // update the stack's size
        stack->size = (res->size) - (CELL_SIZE+SIZE_FIELD_SIZE);
        return res;
    }
    else if(stack->size == (SIZE_FIELD_SIZE+CELL_SIZE)) {
        // pop the last element from the stack
        icell* res = stack;
        // the stack is now empty
        stack=NULL;
        return res;
    }
    else {
        fprintf(stderr, "The stack has a cell < 24");
    }*/
}


void
opt_free(icell* cell) {
    uint64_t current_size;
    if(stack) {
        current_size = stack->size;
    }
    cell->next = stack;
    stack = cell;
    stack->size = current_size + CELL_SIZE + SIZE_FIELD_SIZE;
}

void* 
nu_malloc(size_t size)
{
/*    if(!stack && !init) {
        void* p=mmap(0, CHUNK_SIZE_OPT, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        stack=(icell*) p;
        stack->size=CHUNK_SIZE_OPT;
        init=true;
    }*/

    if(init==0) {
        //init_stack();
        init=1;
    }

    if(size<=CELL_SIZE) {
        return opt_alloc();
    }
    else {
        return hw06_malloc(size);
    }


    // TODO: Allocate memory using a technique optimized
    //       for sizeof(struct icell) allocations.
    // if (is_icell(size)) {
    //    return alloc_icell(...);
    // }
    // else {
    //  return hw06_malloc(size);
}

void 
nu_free(void* ptr)
{
    // TODO: Optimized deallocation.
    icell* tmp = (icell*) (ptr-SIZE_FIELD_SIZE);
    if((tmp->size) == (SIZE_FIELD_SIZE+CELL_SIZE)) {
        opt_free(tmp);
    }
    else {
        hw06_free(ptr);
    }
}
