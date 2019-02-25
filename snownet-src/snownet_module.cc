#include "snownet.h"

#include "snownet_module.h"
#include "spinlock.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>


#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

extern void *logger_create(void);
extern int logger_init(void * inst, struct snownet_context *ctx, const char * parm);
extern void logger_release(void * inst);

extern void * snlua_create(void);
extern int snlua_init(void *l, struct snownet_context *ctx, const char * args);
extern void snlua_release(void *l);
extern void snlua_signal(void *l, int signal);

extern void * harbor_create(void);
extern int harbor_init(void *h, struct snownet_context *ctx, const char * args);
extern void harbor_release(void *l);

extern void * gate_create(void);
extern int gate_init(void *g, struct snownet_context *ctx, const char * parm);
extern void gate_release(void *g);

struct model_st {
	char name[64];
	snownet_dl_create create;
	snownet_dl_init init;
	snownet_dl_release release;
	snownet_dl_signal signal;
};

#define USE_MODEL_IN
#define MAX_MODULE_NUMBER 4
model_st g_st_model[MAX_MODULE_NUMBER] = {
	model_st{"logger",logger_create,logger_init,logger_release,0},
	model_st{"snlua",snlua_create,snlua_init,snlua_release,snlua_signal},
	model_st{"harbor",harbor_create,harbor_init,harbor_release,0},
	model_st{"gate",gate_create,gate_init,gate_release,0},
};


#define MAX_MODULE_TYPE 32

struct modules {
	int count;
	struct spinlock lock;
	const char * path;
	struct snownet_module m[MAX_MODULE_TYPE];
};

static struct modules * M = NULL;

static void *
_try_open(struct modules *m, const char * name) {
	const char *l;
	const char * path = m->path;
	size_t path_size = strlen(path);
	size_t name_size = strlen(name);

	int sz = path_size + name_size;
	//search path
#ifdef _WIN32
	HMODULE dl = NULL;
#else
	void * dl = NULL;
#endif
	char *tmp = (char*)snownet_malloc(sz);
	do
	{
		memset(tmp, 0, sz);
		while (*path == ';') path++;
		if (*path == '\0') break;
		l = strchr(path, ';');
		if (l == NULL) l = path + strlen(path);
		int len = l - path;
		int i;
		for (i = 0; path[i] != '?' && i < len; i++) {
			tmp[i] = path[i];
		}
		memcpy(tmp + i, name, name_size);
		if (path[i] == '?') {
			strncpy(tmp + i + name_size, path + i + 1, len - i - 1);
		}
		else {
			fprintf(stderr, "Invalid C service path\n");
			exit(1);
		}
#ifdef _WIN32
		dl = LoadLibraryA(tmp);
#else
		//dl = dlopen(tmp, RTLD_NOW | RTLD_GLOBAL);
		dl = dlopen(tmp, RTLD_NOW | RTLD_GLOBAL);
#endif
		path = l;
	} while (dl == NULL);
	snownet_free(tmp);
	if (dl == NULL) {
#ifdef _WIN32
		fprintf(stderr, "LoadLibraryA try open %s failed\n", name);
#else
		fprintf(stderr, "dlopen try open %s failed : %s\n", name, dlerror());
#endif
	}
	return dl;
}

static void *
_try_open_in(struct modules *m, const char * name) {
	int i;
	for (i = 0; i < MAX_MODULE_NUMBER; i++) {
		if (strcmp(g_st_model[i].name, name) == 0) {
			return &g_st_model[i];
		}
	}
	return NULL;
}

static struct snownet_module * 
_query(const char * name) {
	int i;
	for (i=0;i<M->count;i++) {
		if (strcmp(M->m[i].name,name)==0) {
			return &M->m[i];
		}
	}
	return NULL;
}

static void *
get_api(struct snownet_module *mod, const char *api_name) {
	size_t name_size = strlen(mod->name);
	size_t api_size = strlen(api_name);
	char *tmp = (char*)snownet_malloc(name_size + api_size + 1);
	memcpy(tmp, mod->name, name_size);
	memcpy(tmp + name_size, api_name, api_size + 1);
	char *ptr = strrchr(tmp, '.');
	if (ptr == NULL) {
		ptr = tmp;
	}
	else {
		ptr = ptr + 1;
	}
	void * ret = NULL;
#ifdef _WIN32
	ret = GetProcAddress((HMODULE)mod->module, ptr);
#else
	//dlsym 通过拼接的字符串找到对应的函数 snlua_create
	ret = dlsym(mod->module, ptr);
#endif
	snownet_free(tmp);
	return ret;
}

static int
open_sym(struct snownet_module *mod) {
#ifdef USE_MODEL_IN
	if (mod != 0 && mod->module != 0) {
		model_st* st = (model_st*)mod->module;
		mod->create = st->create;
		mod->init = st->init;
		mod->release = st->release;
		mod->signal = st->signal;
	}
#else
	mod->create = (snownet_dl_create)get_api(mod, "_create");
	mod->init = (snownet_dl_init)get_api(mod, "_init");
	mod->release = (snownet_dl_release)get_api(mod, "_release");
	mod->signal = (snownet_dl_signal)get_api(mod, "_signal");
#endif

	return mod->init == NULL;
}

struct snownet_module * 
snownet_module_query(const char * name) {
	struct snownet_module * result = _query(name);
	if (result)
		return result;

	SPIN_LOCK(M)

	result = _query(name); // double check

	if (result == NULL && M->count < MAX_MODULE_TYPE) {
		int index = M->count;
#ifdef USE_MODEL_IN
		void * dl = _try_open_in(M, name);
#else
		void * dl = _try_open(M, name);
#endif
		if (dl) {
			M->m[index].name = name;
			M->m[index].module = dl;

			//open_sym 关联 _create _init _release _signal 到 snownet_module 的 create init release signal
			//如：snlua_create logger_create harbor_create gate_create
			if (open_sym(&M->m[index]) == 0) {
				M->m[index].name = snownet_strdup(name);
				M->count ++;
				result = &M->m[index];
			}
		}
	}

	SPIN_UNLOCK(M)

	return result;
}

void 
snownet_module_insert(struct snownet_module *mod) {
	SPIN_LOCK(M)

	struct snownet_module * m = _query(mod->name);
	assert(m == NULL && M->count < MAX_MODULE_TYPE);
	int index = M->count;
	M->m[index] = *mod;
	++M->count;

	SPIN_UNLOCK(M)
}

void * 
snownet_module_instance_create(struct snownet_module *m) {
	if (m->create) {
		return m->create();//调用模块的 _create 如snlua_create
	} else {
		return (void *)(intptr_t)(~0);
	}
}

int
snownet_module_instance_init(struct snownet_module *m, void * inst, struct snownet_context *ctx, const char * parm) {
	return m->init(inst, ctx, parm);//调用模块的 _init 如snlua_init
}

void 
snownet_module_instance_release(struct snownet_module *m, void *inst) {
	if (m->release) {
		m->release(inst);//调用模块的 _release 如snlua_release
	}
}

void
snownet_module_instance_signal(struct snownet_module *m, void *inst, int signal) {
	if (m->signal) {
		m->signal(inst, signal);//调用模块的 _signal 如snlua_signal
	}
}

void 
snownet_module_init(const char *path) {
	struct modules *m = (struct modules *)snownet_malloc(sizeof(*m));
	m->count = 0;
	m->path = snownet_strdup(path);

	SPIN_INIT(m)

	M = m;
}
