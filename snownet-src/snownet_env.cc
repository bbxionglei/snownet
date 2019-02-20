#include "snownet.h"
#include "snownet_env.h"
#include "spinlock.h"
#include <lua.hpp>
#include <stdlib.h>
#include <assert.h>

struct snownet_env {
	struct spinlock lock;
	lua_State *L;
};

static struct snownet_env *E = NULL;

const char * 
snownet_getenv(const char *key) {
	SPIN_LOCK(E)

	lua_State *L = E->L;
	
	lua_getglobal(L, key);
	const char * result = lua_tostring(L, -1);
	lua_pop(L, 1);

	SPIN_UNLOCK(E)

	return result;
}

void 
snownet_setenv(const char *key, const char *value) {
	SPIN_LOCK(E)
	
	lua_State *L = E->L;
	lua_getglobal(L, key);
	assert(lua_isnil(L, -1));
	lua_pop(L,1);
	lua_pushstring(L,value);
	lua_setglobal(L,key);

	SPIN_UNLOCK(E)
}

void
snownet_env_init() {
	E = (struct snownet_env *)snownet_malloc(sizeof(*E));
	SPIN_INIT(E)
	E->L = luaL_newstate();
}
