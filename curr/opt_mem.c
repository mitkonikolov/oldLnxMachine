#include <stdlib.h>
#include <sys/mman.h>

#include "hw06_mem.h"

typedef struct icell {
    uint64_t size;
    struct icell* next;
}icell;

static const int64_t CHUNK_SIZE = 1048576;
static const int64_t CELL_SIZE  = (int64_t)(sizeof(icell));
// entries on the stack have 8B size fields
static const int64_t SIZE_FIELD_SIZE = (int64_t)(sizeof(uint64_t));

icell* stack=NULL;


/// Allocates memory by popping from the stack or if the stack is NULL,
/// by using malloc from hw06.
/// \return a \code void* to a chunk of memory of size CELL_SIZE
void*
opt_alloc() {
    // the stack is not empty
    if(stack) {
        icell* res=stack;
        stack=stack->next;
        if(stack) {
            stack->size = res->size - CELL_SIZE - SIZE_FIELD_SIZE;
        }
        // return a pointer to the data segment of the cell
        return ((void*)(res) + SIZE_FIELD_SIZE);
    }
    else { // the stack is empty
        return hw06_malloc(CELL_SIZE);
    }
}


/// Frees the chunk of memory to which the pointer \param cell points.
/// \param cell a pointer to the chunk of memory that needs to be freed
void
opt_free(icell* cell) {
    uint64_t current_size;
    // if the stack is different from 0, its new size will be a result
    // of the sum of its previous size, CELL_SIZE, and SIZE_FIELD_SIZE
    if(stack) {
        current_size = stack->size;
    }
    // push the cell on the stack and update the pointer to its new head
    cell->next = stack;
    stack = cell;
    stack->size = current_size + CELL_SIZE + SIZE_FIELD_SIZE;
}


/// Allocates as much memory as requested in \param size including enough
/// for a size field. For \param size <= CELL_SIZE, it uses optimized method.
/// \param size the amount of memory requested by the user
/// \return a \code void* to a chunk of memory of size at least \param size
void* 
nu_malloc(size_t size)
{
    if(size<=CELL_SIZE) {
        return opt_alloc();
    }
    else {
        return hw06_malloc(size);
    }
}


/// Frees the chunk of memory provided by the user through \param ptr.
/// \param ptr the beginning of a chunk of memory that needs to be freed
void 
nu_free(void* ptr)
{
    icell* tmp = (icell*) (ptr-SIZE_FIELD_SIZE);
    if((tmp->size) == (SIZE_FIELD_SIZE+CELL_SIZE)) {
        opt_free(tmp);
    }
    else {
        hw06_free(ptr);
    }
}
