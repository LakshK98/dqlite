#ifndef METRIC_HELPER
#define METRIC_HELPER

#include <stdint.h>

#include "lib/queue.h"
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

struct metric_aggregate
{
    queue head;
    uint64_t id;
    uint64_t sum;
    uint64_t avg;
    uint64_t counter;
    queue queue;
};

struct metric_node_new
{
    uint64_t id;
    uint64_t start_time;
    uint64_t duration;

    queue queue;
};

struct metric_store{
    uint64_t log_idx;
    struct timing_metric apply_commit_duration, append_duration, commit_duration;
    struct metric_node nodes [NUM_NODES];
    uint64_t write_sum;
    uint64_t write_avg;
    uint64_t counter;
    uint64_t last_log_idx;
    queue append_entry_q;
    struct metric_aggregate file_write_metric;
    struct metric_aggregate db_add_metric;
    struct metric_aggregate exec_metric;
};

uint64_t  get_cur_time(void);
void init_metric_store(struct metric_store *ms);
void record_start_time(struct timing_metric *tm);
void record_end_time(struct timing_metric *tm);
void record_write(struct metric_store *ms, uint64_t dur);
void record_start_time_new(struct metric_aggregate *ma, uint64_t id, const char *msg);
void record_end_time_new(struct metric_aggregate *ma, uint64_t id, const char *msg);
struct metric_aggregate * get_aggregate_node(queue *head,  uint64_t id);
#endif