#ifndef SNOWNET_MESSAGE_QUEUE_H
#define SNOWNET_MESSAGE_QUEUE_H

//TODO ≤‚ ‘
#include <stdlib.h>
#include <stdint.h>

struct snownet_message {
	uint32_t source;
	int session;
	void * data;
	size_t sz;
};

// type is encoding in snownet_message.sz high 8bit
#define MESSAGE_TYPE_MASK (SIZE_MAX >> 8)
#define MESSAGE_TYPE_SHIFT ((sizeof(size_t)-1) * 8)

struct message_queue;

void snownet_globalmq_push(struct message_queue * queue);
struct message_queue * snownet_globalmq_pop(void);

struct message_queue * snownet_mq_create(uint32_t handle);
void snownet_mq_mark_release(struct message_queue *q);

typedef void (*message_drop)(struct snownet_message *, void *);

void snownet_mq_release(struct message_queue *q, message_drop drop_func, void *ud);
uint32_t snownet_mq_handle(struct message_queue *);

// 0 for success
int snownet_mq_pop(struct message_queue *q, struct snownet_message *message);
void snownet_mq_push(struct message_queue *q, struct snownet_message *message);

// return the length of message queue, for debug
int snownet_mq_length(struct message_queue *q);
int snownet_mq_overload(struct message_queue *q);

void snownet_mq_init();

#endif
