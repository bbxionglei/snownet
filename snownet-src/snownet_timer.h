#ifndef SNOWNET_TIMER_H
#define SNOWNET_TIMER_H

#include <stdint.h>

int snownet_timeout(uint32_t handle, int time, int session);
void snownet_updatetime(void);
uint32_t snownet_starttime(void);
uint64_t snownet_thread_time(void);	// for profile, in micro second

void snownet_timer_init(void);

#endif
