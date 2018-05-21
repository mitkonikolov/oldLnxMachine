#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>

#include "queue.h"

// TODO: Make this an interprocess queue.

queue*
make_queue()
{
    int pages = 1 + sizeof(queue) / 4096;
    queue* qq = mmap(NULL, (size_t) (pages * 4096), PROT_EXEC|PROT_WRITE|PROT_READ, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
            // malloc(pages * 4096); // FIXME: Queue should be shared. use mmap

    //qq->qii=malloc(sizeof(unsigned int));
    //qq->qii = 0;
    //qq->qjj = 0;
    //qq->jobs = malloc(QUEUE_SIZE * sizeof(job));
    // either empty or full must always be greater than 0
    sem_init(&(qq->empty), 1, QUEUE_SIZE); // how much space is available
    sem_init(&(qq->full), 1, 0); // how much is written

    return qq;
}

void
unmap_queue(queue* qq) {
    munmap(qq, 4096 * (1 + (sizeof(queue) / 4096)));
}

void
free_queue(queue* qq)
{
    assert(qq->qii == qq->qjj);
    //free(qq);
    unmap_queue(qq);
}

void
queue_put(queue* qq, job msg)
{
    sem_wait(&(qq->empty));
    unsigned int ii = atomic_fetch_add(&(qq->qii), 1); // the place where I can add data
    qq->jobs[ii % QUEUE_SIZE] = msg;
    sem_post(&(qq->full));
}

job
queue_get(queue* qq)
{
    sem_wait(&(qq->full));
    unsigned int jj = atomic_fetch_add(&(qq->qjj), 1); // this should also probably update the queue somehow
    sem_post(&(qq->empty));
    return qq->jobs[jj % QUEUE_SIZE]; // return the data from the end of the queue
}

/*unsigned int
fetch_add(unsigned int * ij, int n) {
    sem_wait(&lock);
    unsigned int temp = *ij;
    if(n>=0) {
        *ij = *ij + n;
    }
    *ij++;
    sem_post(&lock);
    return temp;
}*/

