#ifndef SNOWNET_SERVER_H
#define SNOWNET_SERVER_H

//TODO 测试
#include <stdint.h>
#include <stdlib.h>

struct snownet_context;
struct snownet_message;
struct snownet_monitor;

struct snownet_context * snownet_context_new(const char * name, const char * parm);//创建服务，注册服务，初始化消息队列服务之前根据服务的handle通信
void snownet_context_grab(struct snownet_context *);
void snownet_context_reserve(struct snownet_context *ctx);
struct snownet_context * snownet_context_release(struct snownet_context *);
uint32_t snownet_context_handle(struct snownet_context *);
int snownet_context_push(uint32_t handle, struct snownet_message *message);
void snownet_context_send(struct snownet_context * context, void * msg, size_t sz, uint32_t source, int type, int session);
int snownet_context_newsession(struct snownet_context *);
struct message_queue * snownet_context_message_dispatch(struct snownet_monitor *, struct message_queue *, int weight);	// return next queue
int snownet_context_total();
void snownet_context_dispatchall(struct snownet_context * context);	// for snownet_error output before exit

void snownet_context_endless(uint32_t handle);	// for monitor

void snownet_globalinit(void);
void snownet_globalexit(void);
void snownet_initthread(int m);

void snownet_profile_enable(int enable);

#endif
