#ifndef MALLOC_HELP_H
#define MALLOC_HELP_H

#include <stddef.h>
#include <stdint.h>

typedef struct memoryChunk {
    void*                memory;
} memoryChunk;

typedef struct chunk {
    int64_t     size;
} chunk;

void par_mem_print_stats();
void* par_malloc(size_t usize);
void par_free(void* addr);
void* par_realloc(void* prev, size_t bytes);

#endif
