#ifndef MALLOC_HELP_H
#define MALLOC_HELP_H

#include <stddef.h>
#include <stdint.h>

typedef struct nu_free_cell {
    int64_t              size;
    struct nu_free_cell* next;
} nu_free_cell;

int64_t nu_free_list_length();
void nu_print_free_list();
void nu_mem_print_stats();
void* hw06_malloc(size_t usize);
void hw06_free(void* addr);
void* hw06_realloc(void* prev, size_t bytes);

#endif
