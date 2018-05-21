#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#include "queue.h"

void
set_doneq(queue* qq)
{
    qq->done=1;
}
queue* 
make_queue()
{
    queue* qq = malloc(sizeof(queue));
    qq->head = 0;
    qq->tail = 0;
    pthread_mutex_init(&(qq->lock), 0);
    pthread_cond_init(&(qq->condv), 0);
    qq->done = 0;

    return qq;
}

void wake_up(queue* qq) {
    pthread_mutex_lock(&(qq->lock));
    pthread_cond_broadcast(&(qq->condv));
    pthread_mutex_unlock(&(qq->lock));
}

void
free_queue(queue* qq)
{

    assert(qq->head == 0 && qq->tail == 0);
    free(qq);
}

void 
queue_put(queue* qq, void* msg)
{
    pthread_mutex_lock(&(qq->lock));

    qnode* node = malloc(sizeof(qnode));
    node->data = msg;
    node->prev = 0;
    node->next = 0;
    
    node->next = qq->head;
    qq->head = node;

    if (node->next) {
        node->next->prev = node;
    } 
    else {
        qq->tail = node;
    }

    pthread_cond_broadcast(&(qq->condv));

    pthread_mutex_unlock(&(qq->lock));
}

void* 
queue_get(queue* qq)
{
    pthread_mutex_lock(&(qq->lock));
    void *msg = NULL;
    while(!(qq->tail)) {
        if((qq->done)) {
            break;
        }
        else {
            pthread_cond_wait(&(qq->condv), &(qq->lock));
        }
    }
    if(qq->tail) {
        qnode *node = qq->tail;

        if (node->prev) {
            qq->tail = node->prev;
            node->prev->next = 0;
        } else {
            qq->head = 0;
            qq->tail = 0;
        }

        msg = node->data;
        free(node);

    }
    pthread_mutex_unlock(&(qq->lock));
    return msg;

}


