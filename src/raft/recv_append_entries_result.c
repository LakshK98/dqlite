#include "recv_append_entries_result.h"
#include "../tracing.h"
#include "assert.h"
#include "configuration.h"
#include "recv.h"
#include "replication.h"

#include "../metric_helper.h"

int recvAppendEntriesResult(struct raft *r,
			    const raft_id id,
			    const char *address,
			    const struct raft_append_entries_result *result)
{
	int match;
	const struct raft_server *server;
	int rv;


	tracef(LOG_METRIC "recvAppendEntriesResult");

	struct metric_store *ms = (struct metric_store *) r->leader_state.reserved[0];
	struct metric_aggregate *aggr = get_aggregate_node(&ms->append_entry_q, (uint64_t) id);
	record_end_time_new(aggr,  (uint64_t)result->last_log_index, "replication_duration");


	// for(i = 0; i<NUM_NODES; i++)
	// {
	// 	if(ms->nodes[i].node_id  == (uint64_t) id)
	// 	{

	// 		if(ms->nodes[i].log_idx != (uint64_t)result->last_log_index ||  ms->nodes[i].prev_log_idx == (uint64_t)result->last_log_index)
	// 		{
	// 			tracef(LOG_METRIC "recvAppendEntriesResult index does not match %lu \n", ms->nodes[i].log_idx);
	// 		}
	// 		else
	// 		{	
	// 			ms->nodes[i].prev_log_idx = ms->nodes[i].log_idx;
	// 			record_end_time(&ms->nodes[i].replication_duration);
	// 			tracef(LOG_METRIC "replication_duration avg time : %lu duration : %lu  counter: %lu id: %lu\n", ms->nodes[i].replication_duration.avg, ms->nodes[i].replication_duration.duration, ms->nodes[i].replication_duration.counter, ms->nodes[i].node_id );
	// 		}
	// 		break;
	// 	}
	// }

	assert(r != NULL);
	assert(id > 0);
	assert(address != NULL);
	assert(result != NULL);

	tracef(
	    "self:%llu from:%llu@%s last_log_index:%llu rejected:%llu "
	    "term:%llu",
	    r->id, id, address, result->last_log_index, result->rejected,
	    result->term);

	if (r->state != RAFT_LEADER) {
		tracef("local server is not leader -> ignore");
		return 0;
	}

	rv = recvEnsureMatchingTerms(r, result->term, &match);
	if (rv != 0) {
		return rv;
	}

	if (match < 0) {
		tracef("local term is higher -> ignore ");
		return 0;
	}

	/* If we have stepped down, abort here.
	 *
	 * From Figure 3.1:
	 *
	 *   [Rules for Servers] All Servers: If RPC request or response
	 * contains term T > currentTerm: set currentTerm = T, convert to
	 * follower.
	 */
	if (match > 0) {
		assert(r->state == RAFT_FOLLOWER);
		return 0;
	}

	assert(result->term == r->current_term);

	/* Ignore responses from servers that have been removed */
	server = configurationGet(&r->configuration, id);
	if (server == NULL) {
		tracef("unknown server -> ignore");
		return 0;
	}

	/* Update the progress of this server, possibly sending further entries.
	 */
	rv = replicationUpdate(r, server, result);
	if (rv != 0) {
		return rv;
	}

	return 0;
}

#undef tracef
