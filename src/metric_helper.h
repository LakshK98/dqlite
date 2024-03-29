#ifndef METRIC_HELPER
#define METRIC_HELPER

#include <stdint.h>

#define LOG_METRIC " LOG_METRIC "

#define NUM_NODES 2

struct timing_metric {
    uint64_t counter;
    uint64_t start_time;
    uint64_t avg;
    uint64_t sum;
    uint64_t duration;
};

struct metric_node
{
    uint64_t node_id;
    uint64_t log_idx;
    uint64_t prev_log_idx;

    struct timing_metric replication_duration;
};

struct metric_store{
    uint64_t log_idx;
    struct timing_metric apply_commit_duration;
    struct metric_node nodes [NUM_NODES];
};

uint64_t  get_cur_time(void);
void init_metric_store(struct metric_store *ms);
void record_start_time(struct timing_metric *tm);
void record_end_time(struct timing_metric *tm);

#endif