
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <pthread.h>

#include "queue.h"
#include "factor.h"

// FIXME: Shared mutable data
static queue* iqueue;
static queue* oqueue;

pthread_mutex_t lock2;

void
wake_ithread() {
    wake_up(iqueue);
}

void
wake_othread() {
    wake_up(oqueue);
}

void set_i_done() {
    set_doneq(iqueue);
}

void set_o_done() {
    set_doneq(oqueue);
}

void 
factor_init()
{
    pthread_mutex_init(&lock2, 0);
    if (iqueue == 0) iqueue = make_queue();
    if (oqueue == 0) oqueue = make_queue();
}

void
factor_cleanup()
{
    free_queue(iqueue);
    printf("iq is done");
    iqueue = 0;
    free_queue(oqueue);
    printf("oq is done");
    oqueue = 0;
}

factor_job* 
make_job(int128_t nn)
{
    factor_job* job = malloc(sizeof(factor_job));
    job->number  = nn;
    job->factors = 0;
    return job;
}

void 
free_job(factor_job* job)
{
    if (job->factors) {
        free_ivec(job->factors);
    }
    free(job);
}

void 
submit_job(factor_job* job)
{
    queue_put(iqueue, job);
}

factor_job* 
get_result()
{
    return queue_get(oqueue);
}

static
int128_t
isqrt(int128_t xx)
{
    double yy = ceil(sqrt((double)xx));
    return (int128_t) yy;
}

ivec*
factor(int128_t xx)
{
    ivec* ys = make_ivec();

    while (xx % 2 == 0) {
        ivec_push(ys, 2);
        xx /= 2;
    }

    for (int128_t ii = 3; ii <= isqrt(xx); ii += 2) {
        int128_t x1 = xx / ii;
        if (x1 * ii == xx) {
            ivec_push(ys, ii);
            xx = x1;
            ii = 1;
        }
    }

    ivec_push(ys, xx);

    return ys;
}

void 
run_jobs()
{
    //pthread_mutex_lock(&lock2);
    // FIXME: This should happen in its own thread.
    factor_job* job;

    // FIXME: We should block on an empty queue waiting for more work.
    //        We can still use null job as the "end" signal.

    while ((job = queue_get(iqueue))) {
        job->factors = factor(job->number);
        queue_put(oqueue, job);
    }
    queue_put(oqueue, job);
    //pthread_mutex_unlock(&lock2);
}

void*
run_jobs2(void* ptr) {
    run_jobs();
    set_o_done();
    wake_othread();
    return 0;
}