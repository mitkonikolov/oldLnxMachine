
#include "job.h"

job
make_job(int64_t nn)
{
    job jj;
    jj.number = nn;
    jj.count  = 0; // what am I going to use count for?
    return jj;
}

