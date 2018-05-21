#ifndef QUEUE_H
#define QUEUE_H

typedef struct qnode {
    void*         data;
    struct qnode* prev;
    struct qnode* next;
} qnode;

typedef struct queue {
    qnode* head;
    qnode* tail;
    pthread_mutex_t lock;
    pthread_cond_t  condv;
    int done;
} queue;

void set_doneq(queue* qq);
queue* make_queue();
void free_queue(queue* qq);
void wake_up(queue* qq);
void queue_put(queue* qq, void* msg);
void* queue_get(queue* qq);

#endif
