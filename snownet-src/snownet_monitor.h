#ifndef SNOWNET_MONITOR_H
#define SNOWNET_MONITOR_H

#include <stdint.h>

struct snownet_monitor;

struct snownet_monitor * snownet_monitor_new();
void snownet_monitor_delete(struct snownet_monitor *);
void snownet_monitor_trigger(struct snownet_monitor *, uint32_t source, uint32_t destination);
void snownet_monitor_check(struct snownet_monitor *);

#endif
