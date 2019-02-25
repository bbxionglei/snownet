#ifndef SKYNET_H
#define SKYNET_H

//TODO ≤‚ ‘
#include "snownet_malloc.h"

#include <stddef.h>
#include <stdint.h>

#define PTYPE_TEXT 0
#define PTYPE_RESPONSE 1
#define PTYPE_MULTICAST 2
#define PTYPE_CLIENT 3
#define PTYPE_SYSTEM 4
#define PTYPE_HARBOR 5
#define PTYPE_SOCKET 6
// read lualib/snownet.lua examples/simplemonitor.lua
#define PTYPE_ERROR 7	
// read lualib/snownet.lua lualib/mqueue.lua lualib/snax.lua
#define PTYPE_RESERVED_QUEUE 8
#define PTYPE_RESERVED_DEBUG 9
#define PTYPE_RESERVED_LUA 10
#define PTYPE_RESERVED_SNAX 11

#define PTYPE_TAG_DONTCOPY 0x10000
#define PTYPE_TAG_ALLOCSESSION 0x20000

struct snownet_context;
void snownet_error(struct snownet_context * context, const char *msg, ...);
const char * snownet_command(struct snownet_context * context, const char * cmd, const char * parm);
uint32_t snownet_queryname(struct snownet_context * context, const char * name);
int snownet_send(struct snownet_context * context, uint32_t source, uint32_t destination, int type, int session, void * msg, size_t sz);
int snownet_sendname(struct snownet_context * context, uint32_t source, const char * destination, int type, int session, void * msg, size_t sz);

int snownet_isremote(struct snownet_context *, uint32_t handle, int * harbor);

typedef int(*snownet_cb)(struct snownet_context * context, void *ud, int type, int session, uint32_t source, const void * msg, size_t sz);
void snownet_callback(struct snownet_context * context, void *ud, snownet_cb cb);

uint32_t snownet_current_handle(void);
uint64_t snownet_now(void);
void snownet_debug_memory(const char *info);	// for debug use, output current service memory to stderr
#endif
