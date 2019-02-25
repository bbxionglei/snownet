#include "snownet.h"

#include <lua.hpp>

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MEMORY_WARNING_REPORT (1024 * 1024 * 32)

struct snlua {
	lua_State * L;
	struct snownet_context * ctx;
	size_t mem;
	size_t mem_report;
	size_t mem_limit;
};

// LUA_CACHELIB may defined in patched lua for shared proto
#ifdef LUA_CACHELIB

#define codecache luaopen_cache

#else

static int
cleardummy(lua_State *L) {
  return 0;
}

static int 
codecache(lua_State *L) {
	luaL_Reg l[] = {
		{ "clear", cleardummy },
		{ "mode", cleardummy },
		{ NULL, NULL },
	};
	luaL_newlib(L,l);
	lua_getglobal(L, "loadfile");
	lua_setfield(L, -2, "loadfile");
	return 1;
}

#endif

static int 
traceback (lua_State *L) {
	const char *msg = lua_tostring(L, 1);
	if (msg)
		luaL_traceback(L, L, msg, 1);
	else {
		lua_pushliteral(L, "(no error message)");
	}
	return 1;
}

static void
report_launcher_error(struct snownet_context *ctx) {
	// sizeof "ERROR" == 5
	char szError[] = "ERROR";
	snownet_sendname(ctx, 0, ".launcher", PTYPE_TEXT, 0, szError, 5);
}

static const char *
optstring(struct snownet_context *ctx, const char *key, const char * str) {
	const char * ret = snownet_command(ctx, "GETENV", key);
	if (ret == NULL) {
		return str;
	}
	return ret;
}

static int
init_cb(struct snlua *l, struct snownet_context *ctx, const char * args, size_t sz) {
	lua_State *L = l->L;
	l->ctx = ctx;
	lua_gc(L, LUA_GCSTOP, 0);
	lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. *///#stack == 1
	lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");////#stack == 0
	luaL_openlibs(L);
	lua_pushlightuserdata(L, ctx);////#stack == 1
	lua_setfield(L, LUA_REGISTRYINDEX, "snownet_context");//#stack == 0
	luaL_requiref(L, "snownet.codecache", codecache , 0);//#stack == 1
	lua_pop(L,1);//#stack == 0

	const char *path = optstring(ctx, "lua_path","./lualib/?.lua;./lualib/?/init.lua");
	lua_pushstring(L, path);//#stack == 1
	lua_setglobal(L, "LUA_PATH");//#stack == 0
	const char *cpath = optstring(ctx, "lua_cpath","./luaclib/?.so");
	lua_pushstring(L, cpath);//#stack == 1
	lua_setglobal(L, "LUA_CPATH");//#stack == 0
	const char *service = optstring(ctx, "luaservice", "./service/?.lua");
	lua_pushstring(L, service);//#stack == 1
	lua_setglobal(L, "LUA_SERVICE");//#stack == 0
	const char *preload = snownet_command(ctx, "GETENV", "preload");
	lua_pushstring(L, preload);//#stack == 1
	lua_setglobal(L, "LUA_PRELOAD");//#stack == 0

	lua_pushcfunction(L, traceback);//#stack == 1
	assert(lua_gettop(L) == 1);//������У��luaջ��û������

	const char * loader = optstring(ctx, "lualoader", "./lualib/loader.lua");

	int r = luaL_loadfile(L,loader); // ����loaderģ�����
	if (r != LUA_OK) {
		snownet_error(ctx, "Can't load %s : %s", loader, lua_tostring(L, -1));
		report_launcher_error(ctx);
		return 1;
	}
	lua_pushlstring(L, args, sz);//args ��һ�ε�������ص�Ĭ�ϲ�����bootstrap,��Ϣ��snlua_init��ʱ���͵�ҲӦ��������ϵͳ�ĵ�һ����Ϣ
	// �ѷ������Ȳ������룬ִ��loaderģ����룬ʵ������ͨ��loader���غ�ִ�з������
	r = lua_pcall(L,1,0,1);
	if (r != LUA_OK) {
		snownet_error(ctx, "lua loader error : %s", lua_tostring(L, -1));
		report_launcher_error(ctx);
		return 1;
	}
	lua_settop(L,0);
	if (lua_getfield(L, LUA_REGISTRYINDEX, "memlimit") == LUA_TNUMBER) {
		size_t limit = lua_tointeger(L, -1);
		l->mem_limit = limit;
		snownet_error(ctx, "Set memory limit to %.2f M", (float)limit / (1024 * 1024));
		lua_pushnil(L);
		lua_setfield(L, LUA_REGISTRYINDEX, "memlimit");
	}
	lua_pop(L, 1);

	lua_gc(L, LUA_GCRESTART, 0);

	return 0;
}

static int
launch_cb(struct snownet_context * context, void *ud, int type, int session, uint32_t source , const void * msg, size_t sz) {
	assert(type == 0 && session == 0);
	struct snlua *l = (struct snlua *)ud;
	snownet_callback(context, NULL, NULL);//bootstrap����ֻ��Ҫ����һ�ξ͹��ˣ���������رջص�
	int err = init_cb(l, context, (const char*)msg, sz);// msg ��Ȼ��bootstrap
	if (err) {
		snownet_command(context, "EXIT", NULL);
	}

	return 0;
}

int
snlua_init(void* l_, struct snownet_context *ctx, const char * args) {
	struct snlua* l = (struct snlua *)l_;
	int sz = strlen(args);//�˴�Ĭ���� bootstrap ��Ϊ����Ĭ���� bootstrap = "snlua bootstrap"	-- The service for bootstrap
	char * tmp = (char *)snownet_malloc(sz);
	memcpy(tmp, args, sz);
	snownet_callback(ctx, l , launch_cb);
	const char * self = snownet_command(ctx, "REG", NULL);
	uint32_t handle_id = strtoul(self+1, NULL, 16);
	/* it must be first message
	 * �Ѳ���������Ϣ���ݷ���������񣬾��� snownet.newservice(name, ...) ����� ...
	 * Ŀ��������������ɳ�ʼ��������ὲ����snownet��������Ϣ������
	 * �������������ʱ�򣬹����̻߳�û�й�������Ϣ�����ᴦ��,
	 * ��������snownet_send����ϢҪ��ϵͳ��ʼ������Ϣ��ʱ��Ż����launch_cb
	 */
	snownet_send(ctx, 0, handle_id, PTYPE_TAG_DONTCOPY,0, tmp, sz);
	return 0;
}

static void *
lalloc(void * ud, void *ptr, size_t osize, size_t nsize) {
	struct snlua *l = (struct snlua *)ud;
	size_t mem = l->mem;
	l->mem += nsize;
	if (ptr)
		l->mem -= osize;
	if (l->mem_limit != 0 && l->mem > l->mem_limit) {
		if (ptr == NULL || nsize > osize) {
			l->mem = mem;
			return NULL;
		}
	}
	if (l->mem > l->mem_report) {
		l->mem_report *= 2;
		snownet_error(l->ctx, "Memory warning %.2f M", (float)l->mem / (1024 * 1024));
	}
	return snownet_lalloc(ptr, osize, nsize);
}

void *
snlua_create(void) {
	struct snlua * l = (struct snlua *)snownet_malloc(sizeof(*l));
	memset(l,0,sizeof(*l));
	l->mem_report = MEMORY_WARNING_REPORT;
	l->mem_limit = 0;
	l->L = lua_newstate(lalloc, l);
	return l;
}

void
snlua_release(void *l_) {
	struct snlua* l = (struct snlua *)l_;
	lua_close(l->L);
	snownet_free(l);
}

void
snlua_signal(void *l_, int signal) {
	struct snlua* l = (struct snlua *)l_;
	snownet_error(l->ctx, "recv a signal %d", signal);
	if (signal == 0) {
#ifdef lua_checksig
	// If our lua support signal (modified lua version by snownet), trigger it.
	snownet_sig_L = l->L;
#endif
	} else if (signal == 1) {
		snownet_error(l->ctx, "Current Memory %.3fK", (float)l->mem / 1024);
	}
}
