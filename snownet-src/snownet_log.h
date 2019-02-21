#ifndef snownet_log_h
#define snownet_log_h

#include "snownet_env.h"
#include "snownet.h"

#include <stdio.h>
#include <stdint.h>

FILE * snownet_log_open(struct snownet_context * ctx, uint32_t handle);
void snownet_log_close(struct snownet_context * ctx, FILE *f, uint32_t handle);
void snownet_log_output(FILE *f, uint32_t source, int type, int session, void * buffer, size_t sz);

#endif