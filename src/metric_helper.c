#include "metric_helper.h"

#include <time.h>

uint64_t  get_cur_time(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return  (uint64_t)t.tv_sec * 1000000000L + (uint64_t) t.tv_nsec;
}

void init_metric_store(struct metric_store *ms)
{
    ms->log_idx = 0;
    ms->apply_commit_duration = (struct timing_metric) {0};

    unsigned i;
    for(i=0; i<NUM_NODES; i++)
    {
        ms->nodes[i] = (struct metric_node) {0};
    }
}

void record_start_time(struct timing_metric *tm)
{
    tm->start_time = get_cur_time();
}

void record_end_time(struct timing_metric *tm)
{
    tm->counter++;
    if(tm->counter < 5)
        return;
    uint64_t cur_time = get_cur_time();
    tm->duration = cur_time - tm->start_time;
    tm->sum += tm->duration;
    tm->avg = tm->sum /tm->counter;
}