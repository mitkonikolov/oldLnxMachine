#ifndef MALLOC_HELP_H
#define MALLOC_HELP_H

#include <stddef.h>
#include <stdint.h>
/*
typedef struct cacheCategory {
    void*                mem;
    cacheCategory*       next;
} cacheCategory;*/

typedef struct memoryChunk {
    void*                memory;
} memoryChunk;

typedef struct chunk {
    int64_t     size;
} chunk;

/*int64_t par_free_list_length();
void par_print_free_list();*/
void par_mem_print_stats();
void* par_malloc(size_t usize);
void par_free(void* addr);
void* par_realloc(void* prev, size_t bytes);

#endif
