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
        void* addr = mmap(0, CHUNK_SIZE * 4, PROT_READ | PROT_WRITE, 
        MAP_PRIVATE | MAP_ANONYMOUS , -1, 0);

        masterChunk = (int64_t*) addr;
        *masterChunk = CHUNK_SIZE * 4;
    }
    else if(i==1) {
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
    if(klassSize<=24) {
        return klassSize * 256;
    } 
    else if(klassSize<=64) {
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
}

void
par_free(void* addr)
{
    void** head = addr - sizeof(int64_t);
    int64_t size = *((int64_t*)head);

    // klass index
    int klass = size_to_class(size);

    void** temp = cacheList[klass];
    cacheList[klass] = head;
    *head = temp;
}


void*
par_realloc(void* prev, size_t bytes) {
    if(bytes==0) {
        par_free(prev);
        return par_malloc(1);
    }
    int64_t size = *(((int64_t*)( prev)) - sizeof(int64_t) / 8);

    if(size==bytes) {
        return prev;
    }
    else {
        void* p = par_malloc(bytes);
        memcpy(p, prev, bytes);
        par_free(prev);
        return p;
    }
}
