#include "snownet.h"

#include "snownet_monitor.h"
#include "snownet_server.h"
#include "snownet.h"
#include "atomic.h"

#include <stdlib.h>
#include <string.h>

struct snownet_monitor {
	std::atomic<int> version;
	int check_version;
	uint32_t source;
	uint32_t destination;
};

struct snownet_monitor * 
snownet_monitor_new() {
	struct snownet_monitor * ret = (struct snownet_monitor *)snownet_malloc(sizeof(*ret));
	memset(ret, 0, sizeof(*ret));
	return ret;
}

void 
snownet_monitor_delete(struct snownet_monitor *sm) {
	snownet_free(sm);
}

void 
snownet_monitor_trigger(struct snownet_monitor *sm, uint32_t source, uint32_t destination) {
	sm->source = source;
	sm->destination = destination;
	ATOM_INC(&sm->version);
}

void 
snownet_monitor_check(struct snownet_monitor *sm) {
	if (sm->version == sm->check_version) {
		if (sm->destination) {
			snownet_context_endless(sm->destination);
			int v = sm->version;
			snownet_error(NULL, "A message from [ :%08x ] to [ :%08x ] maybe in an endless loop (version = %d)", sm->source , sm->destination, v);
		}
	} else {
		sm->check_version = sm->version;
	}
}
