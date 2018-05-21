

#include <stdio.h>


#include "par_malloc_help.h"
#include "xmalloc.h"

/* HW11 TODO:
 *  - This should call / use your HW06 alloctor,
 *    modified to be safe and efficient for
 *    use in a threaded program.
 
static int max_size = 32768;
static int class_index(size_t size) {
    if (size <= 1024) {
        return ((int) size + 7) >> 3;
    }
    else {
        return ((int) size + 127 + (120 << 7)) >> 7;
    }
}
//static int num_of_classes = (max_size + 127 + (120 << 7)) >> 7;;
//static size_t class_to_size[377];
static size_t class_to_size(int index) {
    if (index <= 128) {
        return index >> 3;
    }
    else {
        return 1024 + ((index - 128) >> 7);
    }
}*/

void*
xmalloc(size_t bytes)
{
    return par_malloc(bytes);
}

void
xfree(void* ptr)
{
 //   par_free(ptr);
}

void*
xrealloc(void* prev, size_t bytes)
{
    return par_realloc(prev, bytes);
}

