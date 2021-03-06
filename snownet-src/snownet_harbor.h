#ifndef SNOWNET_HARBOR_H
#define SNOWNET_HARBOR_H

//TODO ����
#include <stdint.h>
#include <stdlib.h>

#define GLOBALNAME_LENGTH 16
#define REMOTE_MAX 256

struct remote_name {
	char name[GLOBALNAME_LENGTH];
	uint32_t handle;
};

struct remote_message {
	struct remote_name destination;
	const void * message;
	size_t sz;
	int type;
};

void snownet_harbor_send(struct remote_message *rmsg, uint32_t source, int session);
int snownet_harbor_message_isremote(uint32_t handle);
void snownet_harbor_init(int harbor);
void snownet_harbor_start(void * ctx);
void snownet_harbor_exit();

#endif
