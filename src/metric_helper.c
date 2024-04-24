#include "metric_helper.h"
#include "raft/heap.h"

#include "tracing.h"

#include <time.h>
#include <stdio.h>


uint64_t  get_cur_time(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return  (uint64_t)t.tv_sec * 1000000000L + (uint64_t) t.tv_nsec;
}

void init_metric_store(struct metric_store *ms)
{
    ms->log_idx = 0;

    init_aggr_node(&ms->file_write_metric);
    init_aggr_node(&ms->db_add_metric);
    init_aggr_node(&ms->exec_metric);

    queue_init(&ms->append_entry_q);
}

void init_aggr_node(struct metric_aggregate *ma)
{
    *ma = (struct metric_aggregate) {0};
    queue_init(&ma->head);
}

struct metric_aggregate* new_aggr_node(void)
{
    struct metric_aggregate *ma = RaftHeapMalloc(sizeof (struct metric_aggregate));
    *ma = (struct metric_aggregate) {0};
    queue_init(&ma->head);
    return ma;
}

void free_aggr_node(struct metric_aggregate *ma)
{
    RaftHeapFree(ma);
}

void record_write(struct metric_store *ms, uint64_t dur)
{
    ms->counter++;

    if(ms->counter < 5)
        return;

    ms->write_sum += dur;
    ms->write_avg = ms->write_sum /ms->counter;

}

void set_start_time(struct metric_node_new *mnn)
{
    mnn->start_time = get_cur_time();
}

void set_duration(struct metric_node_new *mnn)
{
    uint64_t cur_time = get_cur_time();
    mnn->duration = cur_time - mnn->start_time;
}

void record_start_time(struct metric_aggregate *ma, uint64_t id, const char* msg)
{
    // if((int64_t)id >= 0)
    //     return;
    queue *queu_itr;
	struct metric_node_new *mnn = NULL;
    bool found = false;
	QUEUE_FOREACH(queu_itr, &ma->head)
	{
		mnn = QUEUE_DATA(queu_itr, struct metric_node_new, queue);

		if (mnn->id == (uint64_t)id) {
            found = true;
			set_start_time(mnn);
			break;
		}
	}
    if(!found)
    {
        mnn = RaftHeapMalloc(sizeof (struct metric_node_new));
        *mnn = (struct metric_node_new) {0};
        mnn->id = id;
        queue_init(&mnn->queue);

        set_start_time(mnn);

        queue_insert_tail( &ma->head, &mnn->queue);

    }

    tracef(LOG_METRIC "%s", msg);
}


void record_end_time(struct metric_aggregate *ma, uint64_t id, const char *msg)
{
    // if((int64_t)id >= 0)
    //     return;
    queue *head;
	struct metric_node_new *metric_node_itr = NULL;
	QUEUE_FOREACH(head, &ma->head)
	{
		metric_node_itr = QUEUE_DATA(head, struct metric_node_new, queue);

        if (metric_node_itr->id == (uint64_t)id) {
            set_duration(metric_node_itr);
            ma->counter++;
            ma->sum += metric_node_itr->duration;
            ma->avg = ma->sum/ma->counter;
            tracef(LOG_METRIC "%s  node_id: %lu  avg time : %lu duration : %lu counter: %lu\n",msg,  id, ma->avg,  metric_node_itr->duration, ma->counter);
            break;
        }
    }
    if(head != &ma->head)
    {
        queue_remove(head);
    	RaftHeapFree(metric_node_itr);

    }

}

struct metric_aggregate * get_aggregate_node(queue *head,  uint64_t id)
{
    queue *q_itr;
	struct metric_aggregate *ma = NULL;

	QUEUE_FOREACH(q_itr, head)
	{
        ma = QUEUE_DATA(q_itr, struct metric_aggregate, queue);

        if (ma->id == (uint64_t)id) {

            return ma;
        }
    }

    ma = RaftHeapMalloc(sizeof (struct metric_aggregate));
    *ma = (struct metric_aggregate) {0};
    ma->id = id;
    queue_init(&ma->queue);
    queue_init(&ma->head);

    queue_insert_tail( head, &ma->queue);

    return ma;

}