
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <semaphore.h>

#include "job.h"
#include "queue.h"
#include "factor.h"

static queue* iqueue;
static queue* oqueue;

static int worker_count = 0;
static pid_t workers[64];
static pid_t printer;

//static sem_t done_flag;

void
work_off_jobs()
{
    while (1) {
        job jj = queue_get(iqueue);

        // there is no more work on the iqueue and you can break
        if (jj.number < 0) {
            break;
        }

        factor(jj.number, &(jj.count), &(jj.factors[0]));

        queue_put(oqueue, jj);
    }
    unmap_queue(iqueue);
    unmap_queue(oqueue);
}

void
print_results(int64_t count)
{
    int64_t oks = 0;

    for (int64_t ii = 0; ii < count; ++ii) {
        job res = get_result();

        printf("%ld: ", res.number);
        int64_t prod = 1;
        for (int64_t jj = 0; jj < res.count; ++jj) {
            int64_t xx = res.factors[jj];
            prod *= xx;
            printf("%ld ", xx);
        }
        printf("\n");

        if (prod == res.number) {
            ++oks;
        }
        else {
            fprintf(stderr, "Warning: bad factorization");
        }
    }

    //int q=sem_post(&done_flag);
//printf("semposted in print with res %d\n",q);
    printf("Factored %ld / %ld numbers.\n", oks, count);
}

void
factor_wait_done()
{
    waitpid(printer, 0, 0);
}

void
factor_init(int num_procs, int64_t count)
{
    if (iqueue == 0) iqueue = make_queue();
    if (oqueue == 0) oqueue = make_queue();

    //sem_init(&done_flag, 0, 0);

    int cpid=1;
    worker_count=num_procs;

    // num_procs processes are created
    for(int i=0; cpid && (i<num_procs); i++) {
	cpid = fork();
	if(cpid) {
		workers[i] = cpid;
	}
    }

    // parent
    if(cpid) {
        // create printing process
        cpid=fork();
        printer=cpid;
        // printing child
        if(!cpid) {
            print_results(count);
	    exit(0);
        }
    }
    else { // other children workers
        work_off_jobs();
	exit(0);
    }
}

void
factor_cleanup()
{
//printf("entered cleanup\n");
    job done = make_job(-1);

    // we want each thread to see this job and thus finish
    for (int ii = 0; ii < worker_count; ++ii) {
        submit_job(done);
    }

    // FIXME: Make sure all the workers are done.
    // I will need to join them somewhere
    int status;
    for(int i=0; i<worker_count; i++) {
        waitpid(workers[i], &status, 0);
    }
    
    // wait for the printer to finish
    waitpid(printer, &status, 0);

    free_queue(iqueue);
    iqueue = 0;
    free_queue(oqueue);
    oqueue = 0;
}

void
submit_job(job jj)
{
    queue_put(iqueue, jj);
}

job
get_result()
{
    return queue_get(oqueue);
}

static
int64_t
isqrt(int64_t xx)
{
    double yy = ceil(sqrt((double)xx));
    return (int64_t) yy;
}

void
factor(int64_t xx, int64_t* size, int64_t* ys)
{
    int jj = 0;

    while (xx % 2 == 0) {
        ys[jj++] = 2;
        xx /= 2;
    }

    for (int64_t ii = 3; ii <= isqrt(xx); ii += 2) {
        int64_t x1 = xx / ii;
        if (x1 * ii == xx) {
            ys[jj++] = ii;
            xx = x1;
            ii = 1;
        }
    }

    ys[jj++] = xx;
    *size = jj;
}

