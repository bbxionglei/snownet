#ifndef SNOWNET_CONTEXT_HANDLE_H
#define SNOWNET_CONTEXT_HANDLE_H

#include <stdint.h>

// reserve high 8 bits for remote id
#define HANDLE_MASK 0xffffff
#define HANDLE_REMOTE_SHIFT 24

struct snownet_context;

uint32_t snownet_handle_register(struct snownet_context *);
int snownet_handle_retire(uint32_t handle);
struct snownet_context * snownet_handle_grab(uint32_t handle);
void snownet_handle_retireall();

uint32_t snownet_handle_findname(const char * name);
const char * snownet_handle_namehandle(uint32_t handle, const char *name);

void snownet_handle_init(int harbor);

#endif
