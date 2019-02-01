#ifndef SNOWNET_MALLOC_HOOK_H
#define SNOWNET_MALLOC_HOOK_H

#include <stdlib.h>
#include <lua.hpp>

size_t malloc_used_memory(void);
size_t malloc_memory_block(void);
void   memory_info_dump(void);
size_t mallctl_int64(const char* name, size_t* newval);
int    mallctl_opt(const char* name, int* newval);
void   dump_c_mem(void);
int    dump_mem_lua(lua_State *L);
size_t malloc_current_memory(void);

#endif /* SNOWNET_MALLOC_HOOK_H */

