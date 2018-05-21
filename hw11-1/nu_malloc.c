
#include <stdio.h>
#include <pthread.h>


#include "xmalloc.h"
#include "nu_malloc_help.h"


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
/* HW11 TODO:
 *  - This should call / use your HW06 alloctor,
 *    modified to be thread-safe.
 */

void*
xmalloc(size_t bytes)
{

    //fprintf(stderr, "TODO: Call HW06 allocator in nu_malloc.c\n");
    void* p;
    pthread_mutex_lock(&lock);
    p = hw06_malloc(bytes);
    pthread_mutex_unlock(&lock);
    return p;

}

void
xfree(void* ptr)
{
    //fprintf(stderr, "TODO: Call HW06 allocator in nu_malloc.c\n");
    pthread_mutex_lock(&lock);
    hw06_free(ptr);
    pthread_mutex_unlock(&lock);
}

void*
xrealloc(void* prev, size_t bytes)
{
    // fprintf(stderr, "TODO: Implement realloc with HW06 allocator in nu_malloc.c\n")
    void* p;
    pthread_mutex_lock(&lock);
    p = hw06_realloc(prev, bytes);
    pthread_mutex_unlock(&lock);
    return p;
}

//static void
