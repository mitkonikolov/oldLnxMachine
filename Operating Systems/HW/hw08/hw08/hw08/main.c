
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

#include "int128.h"
#include "factor.h"
#include "ivec.h"

int
main(int argc, char* argv[])
{

    if (argc != 4) {
        printf("Usage:\n");
        printf("  ./main threads start count\n");
        return 1;
    }

    int threads = atoi(argv[1]);

    int128_t start = atoh(argv[2]);
    int64_t  count = atol(argv[3]);



    // FIXME: Maybe we're spawning threads in init?
    factor_init();

    // an array of threads
    pthread_t thread[threads];

    // create the threads
    for(int i=0; i<threads; i++) {
        pthread_create(&thread[i], NULL, run_jobs2, NULL);
    }



    for (int64_t ii = 0; ii < count; ++ii) {
        factor_job* job = make_job(start + ii);
        submit_job(job);
    }

    set_i_done();

    // FIXME: This should be (threads) separate threads.
    //run_jobs();

    int64_t oks = 0;


    factor_job* job;

    // FIXME: This should probably be while ((job = get_result()))
    int p=0;
    while((job=get_result())) {
        printf("got here\n");


        print_int128(job->number);

        printf(": ");

        print_ivec(job->factors);
        //printf("got here\n");
        ivec* ys = job->factors;

        int128_t prod = 1;

        for (int ii = 0; ii < ys->len; ++ii) {
            prod *= ys->data[ii];
        }

        if (prod == job->number) {
            ++oks;
        }
        else {
            fprintf(stderr, "Warning: bad factorization");
        }

        free_job(job);

    }

    printf("Factored %ld / %ld numbers.\n", oks, count);

    wake_ithread();

    for(int i=0; i<threads; i++) {
        pthread_join(thread[i], 0);
    }

    pthread_exit(NULL);

    factor_cleanup();
    // FIXME: We should have joined all spawned threads.

    return 0;
}
