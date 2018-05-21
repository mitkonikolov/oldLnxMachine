// Author: Mitko Nikolov
// Contributor: Viviano Cantu

#include <stdint.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "par_malloc_help.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
// 64K chunk - see definition of memoryChunk in header
int64_t* masterChunk;
int init = 0;

__thread int initList = 0;
__thread void*** cacheList;
//typedef struct nu_free_cell {
//    int64_t              size;
//    struct nu_free_cell* next;
//} nu_free_cell;

/* HW11 TODO:
 *  - This should call / use your HW06 alloctor,
 *    modified to be safe and efficient for
 *    use in a threaded program.
 */
static int max_size = 32768;
static int size_to_class(int64_t size) {
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
        return index << 3;
    }
    else {
        return 1024 + ((index - 128) << 7);
    }
}





static const int64_t CHUNK_SIZE = 65536;



static
void
make_cell(int i)
{
    if(i==0) {
        printf("mapping..\n");
        void* addr = mmap(0, CHUNK_SIZE * 4, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS , -1, 0);
        masterChunk = (int64_t*) addr;
        *masterChunk = CHUNK_SIZE * 4;
    }
    else if(i==1) {
        // allocate memory for 16 entries on the list cacheList
        /*void* addr = mmap(0, 300*sizeof(cacheCategory),
                             PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
*/       
        cacheList = malloc(377*sizeof(void**));
    }
}

/**
 * transform klassSize to size in Bytes
 * @param klassSize
 * @return
 */
static
int64_t getClassChunkSize(int64_t klassSize) {
    // TODO Use that to optimize
    if(klassSize<=64) {
        return klassSize * 128;
    }
    else if(klassSize <= 128) {
        return klassSize * 32;
    }
    else if(klassSize<=1024) {
        return klassSize * 8;
    }
    else if (klassSize<=4096) {
        return klassSize * 16;
    }
    else {
        return klassSize;
    }
}

/**
 *
 */
static
void setUpCacheList(void** list, int64_t blockSize, int64_t newSize) {
    while(newSize>0) {
        if(newSize<=blockSize) {
            *list = NULL;
        }
        else {
            *list = list + blockSize / sizeof(void*);
            list = list + blockSize / sizeof(void*);
        }
        newSize = newSize - blockSize;
    }
}

void*
par_malloc(size_t usize)
{
    int64_t size = (int64_t) usize;

    // space for size
    int64_t alloc_size = size + sizeof(int64_t);

    int klass = size_to_class(alloc_size);
    int64_t klassSize = (int64_t)class_to_size(klass);
//printf("%d\n", klassSize);
    // initialize the master 64K chunk and the cache list just once
    pthread_mutex_lock(&lock);
    if(init==0 || *masterChunk <= (int64_t)usize) {
        make_cell(0); // master memory chunk 
        init=1;
    }

    if(initList==0) {
        make_cell(1); // initialize cacheList
        initList=1;
    }
    pthread_mutex_unlock(&lock); 

    // we need to allocate memory from the 64K master chunk
    if (!cacheList[klass]) {
        pthread_mutex_lock(&lock);
        // check how much memory we have in the master chunk
        int64_t newSize = getClassChunkSize(klassSize);
        if (newSize>=*masterChunk) {
            //we need to mmap more
            // FREE MASTER CHUNK
            make_cell(0);
        }
        // set the both to point to the beginning of the master chunk
        cacheList[klass] = (void**)masterChunk;

        // change the place where the master chunk points to and
        // update the value
        masterChunk = masterChunk + newSize / sizeof(void*);

        *masterChunk = (*((int64_t*)cacheList[klass]) - newSize);
        pthread_mutex_unlock(&lock);

        // set pu the cache list in respective blocks
        setUpCacheList(cacheList[klass], klassSize, newSize);

        //pthread_mutex_unlock(&lock);
    }

    //pthread_mutex_lock(&lock);

    void* temp = cacheList[klass];
    
    if(*((int*)cacheList[klass]) < 10000 && *cacheList[klass] >= 0) {
        cacheList[klass] = NULL;
    }
    else {
        // void **             void *  
        cacheList[klass] = *cacheList[klass];
    }
    *((int64_t*)temp) = klassSize;

    temp = temp + sizeof(int64_t);

    //pthread_mutex_unlock(&lock);
    return temp;

//
//    if(mem) {
//        // TODO in get getFromList add size of pointer
//        void* temp = mem;
//        cacheList[klass] = *cacheList[klass];
//        *temp = klassSize;
//        printf("before add: %p\n", temp);
//        temp=temp+sizeof(int64_t);
//        printf("after add: %p\n", temp);
//        return temp;
//    }
//    else {
//        if (klassSize > *masterChunk) {
//            //we need to mmap more
//            //TODO:
//            assert(*masterChunk < 0);
//        }
//        else {
//            int64_t newSize = getClassChunkSize(klassSize);
//            // set the both to point to the beginning of the master chunk
//            cacheList[klass] = masterChunk;
//
//            // change the place where the master chunk points to and
//            // update the value
//            masterChunk + newSize;
//            *masterChunk = *cacheList[klass] - newSize;
//
//            // set pu the cache list in respective blocks
//            setUpCacheList(cacheList[klass], klassSize, newSize);
//        }
//    }




    // space for free cell when returned to list
    /*if (alloc_size < CELL_SIZE) {
        alloc_size = CELL_SIZE;
    }

    nu_malloc_count += 1;
    nu_malloc_bytes += alloc_size;

    // TODO: Handle large allocations.
    if (alloc_size > CHUNK_SIZE) {
        void* addr = mmap(0, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        *((int64_t*)addr) = alloc_size;
        nu_malloc_chunks += 1;
        return addr + sizeof(int64_t);
    }

    nu_free_cell* cell = free_list_get_cell(alloc_size);
    if (!cell) {
        cell = make_cell();
    }

    // Return unused portion to free list.
    int64_t rest_size = cell->size - alloc_size;
    if (rest_size >= CELL_SIZE) {
        void* addr = (void*) cell;
        nu_free_cell* rest = (nu_free_cell*) (addr + alloc_size);
        rest->size = rest_size;
        nu_free_list_insert(rest);
    }

    *((int64_t*)cell) = alloc_size;
    return ((void*)cell) + sizeof(int64_t);
     */
}

void
par_free(void* addr)
{
    void** head = addr - sizeof(int64_t);
    int64_t size = *((int64_t*)head);

    //todo if size is > 32k we should unmap
    // klass index
    int klass = size_to_class(size);

    void** temp = cacheList[klass];
    cacheList[klass] = head;
    *head = temp;

/*    nu_free_cell* cell = (nu_free_cell*)(addr - sizeof(int64_t));
    int64_t size = *((int64_t*) cell);

    if (size > CHUNK_SIZE) {
        nu_free_chunks += 1;
        munmap((void*) cell, size);
    }
    else {
        cell->size = size;
        nu_free_list_insert(cell);
    }

    nu_free_count += 1;
    nu_free_bytes += size;*/
}


void*
par_realloc(void* prev, size_t bytes) {
    if(bytes==0) {
        par_free(prev);
        return par_malloc(1);
    }
    //nu_free_cell* cell = (nu_free_cell*)(prev - sizeof(int64_t));
    int64_t size = *(((int64_t*)( prev)) - sizeof(int64_t) / 8);

    //size_t size = nu_malloc_get_block_size(prev);

    if(size==bytes) {
        return prev;
    }
    else {
//printf("in size<bytes\n");
        void* p = par_malloc(bytes);
        memcpy(p, prev, bytes);
        par_free(prev);
        return p;
    }
}
