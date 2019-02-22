#include "snownet.h"
#include "snownet_harbor.h"
#include "snownet_server.h"
#include "snownet_mq.h"
#include "snownet_handle.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

static struct snownet_context * REMOTE = 0;
static unsigned int HARBOR = ~0;

static inline int
invalid_type(int type) {
	return type != PTYPE_SYSTEM && type != PTYPE_HARBOR;
}

void 
snownet_harbor_send(struct remote_message *rmsg, uint32_t source, int session) {
	assert(invalid_type(rmsg->type) && REMOTE);
	snownet_context_send(REMOTE, rmsg, sizeof(*rmsg) , source, PTYPE_SYSTEM , session);
}

int 
snownet_harbor_message_isremote(uint32_t handle) {
	assert(HARBOR != ~0);
	int h = (handle & ~HANDLE_MASK);
	return h != HARBOR && h !=0;
}

void
snownet_harbor_init(int harbor) {
	HARBOR = (unsigned int)harbor << HANDLE_REMOTE_SHIFT;
}

void
snownet_harbor_start(void *ctx) {
	// the HARBOR must be reserved to ensure the pointer is valid.
	// It will be released at last by calling snownet_harbor_exit
	snownet_context_reserve((struct snownet_context *)ctx);
	REMOTE = (struct snownet_context *)ctx;
}

void
snownet_harbor_exit() {
	struct snownet_context * ctx = REMOTE;
	REMOTE= NULL;
	if (ctx) {
		snownet_context_release(ctx);
	}
}
