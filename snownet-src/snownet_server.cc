#include "snownet.h"

#include "snownet_server.h"
#include "snownet_module.h"
#include "snownet_handle.h"
#include "snownet_mq.h"
#include "snownet_timer.h"
//#include "snownet_harbor.h"
#include "snownet_env.h"
//#include "snownet_monitor.h"
#include "snownet_imp.h"
#include "snownet_log.h"
#include "spinlock.h"
#include "atomic.h"

//#include <pthread.h>

#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef CALLING_CHECK

#define CHECKCALLING_BEGIN(ctx) if (!(spinlock_trylock(&ctx->calling))) { assert(0); }
#define CHECKCALLING_END(ctx) spinlock_unlock(&ctx->calling);
#define CHECKCALLING_INIT(ctx) spinlock_init(&ctx->calling);
#define CHECKCALLING_DESTROY(ctx) spinlock_destroy(&ctx->calling);
#define CHECKCALLING_DECL struct spinlock calling;

#else

#define CHECKCALLING_BEGIN(ctx)
#define CHECKCALLING_END(ctx)
#define CHECKCALLING_INIT(ctx)
#define CHECKCALLING_DESTROY(ctx)
#define CHECKCALLING_DECL

#endif

struct snownet_context {
	void * instance;
	struct snownet_module * mod;
	void * cb_ud;
	snownet_cb cb;
	struct message_queue *queue;
	//FILE * logfile; //TODO 为什么要设计成原子操作
	std::atomic<int64_t> logfile;
	uint64_t cpu_cost;	// in microsec
	uint64_t cpu_start;	// in microsec
	char result[32];
	uint32_t handle;
	int session_id;
	std::atomic<int> ref;
	int message_count;
	bool init;
	bool endless;
	bool profile;

	CHECKCALLING_DECL
};

struct snownet_node {
	std::atomic<int> total;//总的服务个数
	int init;
	uint32_t monitor_exit;
	//TODO
	//pthread_key_t handle_key;
	bool profile;	// default is off
};

static struct snownet_node G_NODE;//服务节点

int 
snownet_context_total() {
	return G_NODE.total;
}

static void
context_inc() {
	ATOM_INC(&G_NODE.total);
}

static void
context_dec() {
	ATOM_DEC(&G_NODE.total);
}

uint32_t 
snownet_current_handle(void) {
	if (G_NODE.init) {
		void * handle = 0;// pthread_getspecific(G_NODE.handle_key);
		return (uint32_t)(uintptr_t)handle;
	} else {
		uint32_t v = (uint32_t)(-THREAD_MAIN);
		return v;
	}
}

static void
id_to_hex(char * str, uint32_t id) {
	int i;
	static char hex[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
	str[0] = ':';
	for (i=0;i<8;i++) {
		str[i+1] = hex[(id >> ((7-i) * 4))&0xf];
	}
	str[9] = '\0';
}

struct drop_t {
	uint32_t handle;
};

static void
drop_message(struct snownet_message *msg, void *ud) {
	struct drop_t *d = (struct drop_t*)ud;
	snownet_free(msg->data);
	uint32_t source = d->handle;
	assert(source);
	// report error to the message source
	snownet_send(NULL, source, msg->source, PTYPE_ERROR, 0, NULL, 0);
}

struct snownet_context * 
snownet_context_new(const char * name, const char *param) {
	struct snownet_module * mod = snownet_module_query(name);

	if (mod == NULL)
		return NULL;

	void *inst = snownet_module_instance_create(mod);//调用模块的 _create 如snlua_create
	if (inst == NULL)
		return NULL;
	struct snownet_context * ctx = (struct snownet_context *)snownet_malloc(sizeof(*ctx));
	CHECKCALLING_INIT(ctx)

	ctx->mod = mod;
	ctx->instance = inst;
	ctx->ref = 2;//ref为什么是2 下面的假装调用一下snownet_context_release会释放1，如果再snownet_context_release一次就真的释放了
	ctx->cb = NULL;
	ctx->cb_ud = NULL;
	ctx->session_id = 0;
	ctx->logfile = NULL;

	ctx->init = false;
	ctx->endless = false;

	ctx->cpu_cost = 0;
	ctx->cpu_start = 0;
	ctx->message_count = 0;
	ctx->profile = G_NODE.profile;
	// Should set to 0 first to avoid snownet_handle_retireall get an uninitialized handle
	ctx->handle = 0;	
	ctx->handle = snownet_handle_register(ctx);
	struct message_queue * queue = ctx->queue = snownet_mq_create(ctx->handle);
	// init function maybe use ctx->handle, so it must init at last
	context_inc();//增加正在运行的服务个数

	CHECKCALLING_BEGIN(ctx)
	int r = snownet_module_instance_init(mod, inst, ctx, param);//调用模块的 _init 如snlua_init
	CHECKCALLING_END(ctx)
	if (r == 0) {
		//调用成功了 假装调用一下release
		struct snownet_context * ret = snownet_context_release(ctx);//调用模块的 _release 如snlua_release
		if (ret) {
			ctx->init = true;
		}
		snownet_globalmq_push(queue);
		if (ret) {
			snownet_error(ret, "LAUNCH %s %s", name, param ? param : "");
		}
		return ret;
	} else {
		//失败了
		snownet_error(ctx, "FAILED launch %s", name);
		uint32_t handle = ctx->handle;
		snownet_context_release(ctx);
		snownet_handle_retire(handle);
		struct drop_t d = { handle };
		snownet_mq_release(queue, drop_message, &d);
		return NULL;
	}
	return NULL;
}

int
snownet_context_newsession(struct snownet_context *ctx) {
	// session always be a positive number
	int session = ++ctx->session_id;
	if (session <= 0) {
		ctx->session_id = 1;
		return 1;
	}
	return session;
}

void 
snownet_context_grab(struct snownet_context *ctx) {
	ATOM_INC(&ctx->ref);
}

void
snownet_context_reserve(struct snownet_context *ctx) {
	snownet_context_grab(ctx);
	// don't count the context reserved, because snownet abort (the worker threads terminate) only when the total context is 0 .
	// the reserved context will be release at last.
	context_dec();//减少正在运行的服务个数
}

static void 
delete_context(struct snownet_context *ctx) {
	if (ctx->logfile) {
		fclose((FILE*)(int64_t)(ctx->logfile));
	}
	snownet_module_instance_release(ctx->mod, ctx->instance);
	snownet_mq_mark_release(ctx->queue);
	CHECKCALLING_DESTROY(ctx)
		snownet_free(ctx);
	context_dec();
}

struct snownet_context * 
snownet_context_release(struct snownet_context *ctx) {
	if (ATOM_DEC(&ctx->ref) == 0) {
		delete_context(ctx);
		return NULL;
	}
	return ctx;
}

int
snownet_context_push(uint32_t handle, struct snownet_message *message) {
	struct snownet_context * ctx = snownet_handle_grab(handle);//如果成功 则 ctx->ref++
	if (ctx == NULL) {
		return -1;
	}
	snownet_mq_push(ctx->queue, message);
	snownet_context_release(ctx);//如果成功 则 ctx->ref--

	return 0;
}

void 
snownet_context_endless(uint32_t handle) {
	struct snownet_context * ctx = snownet_handle_grab(handle);
	if (ctx == NULL) {
		return;
	}
	ctx->endless = true;
	snownet_context_release(ctx);
}

int 
snownet_isremote(struct snownet_context * ctx, uint32_t handle, int * harbor) {
	//TODO
	//int ret = snownet_harbor_message_isremote(handle);
	//if (harbor) {
	//	*harbor = (int)(handle >> HANDLE_REMOTE_SHIFT);
	//}
	//return ret;
	return 0;
}

static void
dispatch_message(struct snownet_context *ctx, struct snownet_message *msg) {
	assert(ctx->init);
	CHECKCALLING_BEGIN(ctx)
	//TODO
	//pthread_setspecific(G_NODE.handle_key, (void *)(uintptr_t)(ctx->handle));
	int type = msg->sz >> MESSAGE_TYPE_SHIFT;
	size_t sz = msg->sz & MESSAGE_TYPE_MASK;
	if (ctx->logfile) {
		snownet_log_output((FILE*)(int64_t)(ctx->logfile), msg->source, type, msg->session, msg->data, sz);
	}
	++ctx->message_count;
	int reserve_msg;
	// ctx->cb 执行回调函数
	if (ctx->profile) {
		ctx->cpu_start = snownet_thread_time();
		reserve_msg = ctx->cb(ctx, ctx->cb_ud, type, msg->session, msg->source, msg->data, sz);
		uint64_t cost_time = snownet_thread_time() - ctx->cpu_start;
		ctx->cpu_cost += cost_time;
	} else {
		reserve_msg = ctx->cb(ctx, ctx->cb_ud, type, msg->session, msg->source, msg->data, sz);
	}
	if (!reserve_msg) {
		snownet_free(msg->data);
	}
	CHECKCALLING_END(ctx)
}

void 
snownet_context_dispatchall(struct snownet_context * ctx) {
	// for snownet_error
	struct snownet_message msg;
	struct message_queue *q = ctx->queue;
	while (!snownet_mq_pop(q,&msg)) {
		dispatch_message(ctx, &msg);
	}
}

struct message_queue * 
snownet_context_message_dispatch(struct snownet_monitor *sm, struct message_queue *q, int weight) {
	if (q == NULL) {
		q = snownet_globalmq_pop();
		if (q==NULL)
			return NULL;
	}

	uint32_t handle = snownet_mq_handle(q);

	struct snownet_context * ctx = snownet_handle_grab(handle);
	if (ctx == NULL) {
		struct drop_t d = { handle };
		snownet_mq_release(q, drop_message, &d);
		return snownet_globalmq_pop();
	}

	int i,n=1;
	struct snownet_message msg;

	for (i=0;i<n;i++) {
		if (snownet_mq_pop(q,&msg)) {
			snownet_context_release(ctx);
			return snownet_globalmq_pop();
		} else if (i==0 && weight >= 0) {
			n = snownet_mq_length(q);
			n >>= weight;
		}
		int overload = snownet_mq_overload(q);
		if (overload) {
			snownet_error(ctx, "May overload, message queue length = %d", overload);
		}
		//TODO
		//snownet_monitor_trigger(sm, msg.source , handle);

		if (ctx->cb == NULL) {
			snownet_free(msg.data);
		} else {
			dispatch_message(ctx, &msg); // 处理回调函数
		}
		//TODO
		//snownet_monitor_trigger(sm, 0,0);
	}

	assert(q == ctx->queue);//逻辑保证，这个消息处理的时候，ctx->queue没有被修改过
	/*
	* 对于 全局消息队列, 处理了一个之后处理下一个, 循环处理
	* 对于 某个message_queue, 排队处理所有消息
	*/
	struct message_queue *nq = snownet_globalmq_pop();
	if (nq) {
		// If global mq is not empty , push q back, and return next queue (nq)
		// Else (global mq is empty or block, don't push q back, and return q again (for next dispatch)
		snownet_globalmq_push(q);
		q = nq;
	} 
	snownet_context_release(ctx);
	return q;
}

//TODO
//static void
//copy_name(char name[GLOBALNAME_LENGTH], const char * addr) {
//	int i;
//	for (i = 0; i < GLOBALNAME_LENGTH && addr[i]; i++) {
//		name[i] = addr[i];
//	}
//	for (; i < GLOBALNAME_LENGTH; i++) {
//		name[i] = '\0';
//	}
//}

uint32_t 
snownet_queryname(struct snownet_context * context, const char * name) {
	switch(name[0]) {
	case ':':
		return strtoul(name+1,NULL,16);
	case '.':
		return snownet_handle_findname(name + 1);
	}
	snownet_error(context, "Don't support query global name %s",name);
	return 0;
}

static void
handle_exit(struct snownet_context * context, uint32_t handle) {
	if (handle == 0) {
		handle = context->handle;
		snownet_error(context, "KILL self");
	} else {
		snownet_error(context, "KILL :%0x", handle);
	}
	if (G_NODE.monitor_exit) {
		snownet_send(context,  handle, G_NODE.monitor_exit, PTYPE_CLIENT, 0, NULL, 0);
	}
	snownet_handle_retire(handle);
}

// snownet command

struct command_func {
	const char *name;
	const char * (*func)(struct snownet_context * context, const char * param);
};

static const char *
cmd_timeout(struct snownet_context * context, const char * param) {
	char * session_ptr = NULL;
	int ti = strtol(param, &session_ptr, 10);
	int session = snownet_context_newsession(context);
	snownet_timeout(context->handle, ti, session);
	sprintf(context->result, "%d", session);
	return context->result;
}

static const char *
cmd_reg(struct snownet_context * context, const char * param) {
	if (param == NULL || param[0] == '\0') {
		sprintf(context->result, ":%x", context->handle);
		return context->result;
	} else if (param[0] == '.') {
		return snownet_handle_namehandle(context->handle, param + 1);
	} else {
		snownet_error(context, "Can't register global name %s in C", param);
		return NULL;
	}
}

static const char *
cmd_query(struct snownet_context * context, const char * param) {
	if (param[0] == '.') {
		uint32_t handle = snownet_handle_findname(param+1);
		if (handle) {
			sprintf(context->result, ":%x", handle);
			return context->result;
		}
	}
	return NULL;
}

static const char *
cmd_name(struct snownet_context * context, const char * param) {
	int size = (int)strlen(param);
	char *name = (char*)snownet_malloc(size + 1);
	char *handle = (char*)snownet_malloc(size + 1);
	const char *ret = NULL;
	do 
	{
		sscanf(param, "%s %s", name, handle);
		if (handle[0] != ':') {
			break;
		}
		uint32_t handle_id = strtoul(handle + 1, NULL, 16);
		if (handle_id == 0) {
			break;
		}
		if (name[0] == '.') {
			ret = snownet_handle_namehandle(handle_id, name + 1);
		}
		else {
			snownet_error(context, "Can't set global name %s in C", name);
		}
	} while (0);
	snownet_free(name);
	snownet_free(handle);
	return ret;
}

static const char *
cmd_exit(struct snownet_context * context, const char * param) {
	handle_exit(context, 0);
	return NULL;
}

static uint32_t
tohandle(struct snownet_context * context, const char * param) {
	uint32_t handle = 0;
	if (param[0] == ':') {
		handle = strtoul(param+1, NULL, 16);
	} else if (param[0] == '.') {
		handle = snownet_handle_findname(param+1);
	} else {
		snownet_error(context, "Can't convert %s to handle",param);
	}

	return handle;
}

static const char *
cmd_kill(struct snownet_context * context, const char * param) {
	uint32_t handle = tohandle(context, param);
	if (handle) {
		handle_exit(context, handle);
	}
	return NULL;
}

char *strsep_(char **stringp, const char *delim)
{
	char *s;
	const char *spanp;
	int c, sc;
	char *tok;
	if ((s = *stringp) == NULL)
		return (NULL);
	for (tok = s;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*stringp = s;
				return (tok);
			}
		} while (sc != 0);
	}
}

static const char *
cmd_launch(struct snownet_context * context, const char * param) {
	size_t sz = strlen(param);
	char *tmp = (char*)snownet_malloc(sz + 1);
	strcpy(tmp, param);
	char * args = tmp;
	char * mod = strsep_(&args, " \t\r\n");
	args = strsep_(&args, "\r\n");
	struct snownet_context * inst = snownet_context_new(mod, args);
	snownet_free(tmp);
	if (inst == NULL) {
		return NULL;
	}
	else {
		id_to_hex(context->result, inst->handle);
		return context->result;
	}
	return NULL;
}

static const char *
cmd_getenv(struct snownet_context * context, const char * param) {
	return snownet_getenv(param);
}

static const char *
cmd_setenv(struct snownet_context * context, const char * param) {
	size_t sz = strlen(param);
	char *key = (char*)snownet_malloc(sz + 1);
	int i;
	for (i = 0; param[i] != ' ' && param[i]; i++) {
		key[i] = param[i];
	}
	if (param[i] != '\0') {
		key[i] = '\0';
		param += i + 1;
		snownet_setenv(key, param);
	}
	snownet_free(key);
	return NULL;
}

static const char *
cmd_starttime(struct snownet_context * context, const char * param) {
	uint32_t sec = snownet_starttime();
	sprintf(context->result, "%u", sec);
	return context->result;
}

static const char *
cmd_abort(struct snownet_context * context, const char * param) {
	snownet_handle_retireall();
	return NULL;
}

static const char *
cmd_monitor(struct snownet_context * context, const char * param) {
	uint32_t handle=0;
	if (param == NULL || param[0] == '\0') {
		if (G_NODE.monitor_exit) {
			// return current monitor serivce
			sprintf(context->result, ":%x", G_NODE.monitor_exit);
			return context->result;
		}
		return NULL;
	} else {
		handle = tohandle(context, param);
	}
	G_NODE.monitor_exit = handle;
	return NULL;
}

static const char *
cmd_stat(struct snownet_context * context, const char * param) {
	if (strcmp(param, "mqlen") == 0) {
		int len = snownet_mq_length(context->queue);
		sprintf(context->result, "%d", len);
	} else if (strcmp(param, "endless") == 0) {
		if (context->endless) {
			strcpy(context->result, "1");
			context->endless = false;
		} else {
			strcpy(context->result, "0");
		}
	} else if (strcmp(param, "cpu") == 0) {
		double t = (double)context->cpu_cost / 1000000.0;	// microsec
		sprintf(context->result, "%lf", t);
	} else if (strcmp(param, "time") == 0) {
		if (context->profile) {
			uint64_t ti = snownet_thread_time() - context->cpu_start;
			double t = (double)ti / 1000000.0;	// microsec
			sprintf(context->result, "%lf", t);
		} else {
			strcpy(context->result, "0");
		}
	} else if (strcmp(param, "message") == 0) {
		sprintf(context->result, "%d", context->message_count);
	} else {
		context->result[0] = '\0';
	}
	return context->result;
}

static const char *
cmd_logon(struct snownet_context * context, const char * param) {
	uint32_t handle = tohandle(context, param);
	if (handle == 0)
		return NULL;
	struct snownet_context * ctx = snownet_handle_grab(handle);
	if (ctx == NULL)
		return NULL;
	FILE *f = NULL;
	FILE * lastf = (FILE*)(int64_t)(ctx->logfile);
	if (lastf == NULL) {
		f = snownet_log_open(context, handle);
		if (f) {
			int64_t intf = (int64_t)f;
			int64_t int0 = NULL;
			if (!ATOM_CAS(&ctx->logfile, int0, intf)) {
				// logfile opens in other thread, close this one.
				fclose(f);
			}
		}
	}
	snownet_context_release(ctx);
	return NULL;
}

static const char *
cmd_logoff(struct snownet_context * context, const char * param) {
	uint32_t handle = tohandle(context, param);
	if (handle == 0)
		return NULL;
	struct snownet_context * ctx = snownet_handle_grab(handle);
	if (ctx == NULL)
		return NULL;
	FILE * f = (FILE*)(int64_t)(ctx->logfile);
	if (f) {
		// logfile may close in other thread
		int64_t intf = (int64_t)f;
		if (ATOM_CAS(&ctx->logfile, intf, NULL)) {
			snownet_log_close(context, f, handle);
		}
	}
	snownet_context_release(ctx);
	return NULL;
}

static const char *
cmd_signal(struct snownet_context * context, const char * param) {
	uint32_t handle = tohandle(context, param);
	if (handle == 0)
		return NULL;
	struct snownet_context * ctx = snownet_handle_grab(handle);
	if (ctx == NULL)
		return NULL;
	param = strchr(param, ' ');
	int sig = 0;
	if (param) {
		sig = strtol(param, NULL, 0);
	}
	// NOTICE: the signal function should be thread safe.
	snownet_module_instance_signal(ctx->mod, ctx->instance, sig);

	snownet_context_release(ctx);
	return NULL;
}

static struct command_func cmd_funcs[] = {
	{ "TIMEOUT", cmd_timeout },
	{ "REG", cmd_reg },
	{ "QUERY", cmd_query },
	{ "NAME", cmd_name },
	{ "EXIT", cmd_exit },
	{ "KILL", cmd_kill },
	{ "LAUNCH", cmd_launch },
	{ "GETENV", cmd_getenv },
	{ "SETENV", cmd_setenv },
	{ "STARTTIME", cmd_starttime },
	{ "ABORT", cmd_abort },
	{ "MONITOR", cmd_monitor },
	{ "STAT", cmd_stat },
	{ "LOGON", cmd_logon },
	{ "LOGOFF", cmd_logoff },
	{ "SIGNAL", cmd_signal },
	{ NULL, NULL },
};

const char * 
snownet_command(struct snownet_context * context, const char * cmd , const char * param) {
	struct command_func * method = &cmd_funcs[0];
	while(method->name) {
		if (strcmp(cmd, method->name) == 0) {
			return method->func(context, param);
		}
		++method;
	}

	return NULL;
}

static void
_filter_args(struct snownet_context * context, int type, int *session, void ** data, size_t * sz) {
	int needcopy = !(type & PTYPE_TAG_DONTCOPY);
	int allocsession = type & PTYPE_TAG_ALLOCSESSION;
	type &= 0xff;

	if (allocsession) {
		assert(*session == 0);
		*session = snownet_context_newsession(context);
	}

	if (needcopy && *data) {
		char * msg = (char*)snownet_malloc(*sz+1);
		memcpy(msg, *data, *sz);
		msg[*sz] = '\0';
		*data = msg;
	}

	*sz |= (size_t)type << MESSAGE_TYPE_SHIFT;
}

int
snownet_send(struct snownet_context * context, uint32_t source, uint32_t destination , int type, int session, void * data, size_t sz) {
	if ((sz & MESSAGE_TYPE_MASK) != sz) {
		snownet_error(context, "The message to %x is too large", destination);
		if (type & PTYPE_TAG_DONTCOPY) {
			snownet_free(data);
		}
		return -2;
	}
	_filter_args(context, type, &session, (void **)&data, &sz);

	if (source == 0) {
		source = context->handle;
	}

	if (destination == 0) {
		if (data) {
			snownet_error(context, "Destination address can't be 0");
			snownet_free(data);
			return -1;
		}

		return session;
	}
	//TODO
	//if (snownet_harbor_message_isremote(destination)) {
	//	struct remote_message * rmsg = snownet_malloc(sizeof(*rmsg));
	//	rmsg->destination.handle = destination;
	//	rmsg->message = data;
	//	rmsg->sz = sz & MESSAGE_TYPE_MASK;
	//	rmsg->type = sz >> MESSAGE_TYPE_SHIFT;
	//	snownet_harbor_send(rmsg, source, session);
	//}
	//else {
		struct snownet_message smsg;
		smsg.source = source;
		smsg.session = session;
		smsg.data = data;
		smsg.sz = sz;

		if (snownet_context_push(destination, &smsg)) {
			snownet_free(data);
			return -1;
		}
	//}
	return session;
}

int
snownet_sendname(struct snownet_context * context, uint32_t source, const char * addr , int type, int session, void * data, size_t sz) {
	if (source == 0) {
		source = context->handle;
	}
	uint32_t des = 0;
	if (addr[0] == ':') {
		des = strtoul(addr + 1, NULL, 16);
	}
	else if (addr[0] == '.') {
		des = snownet_handle_findname(addr + 1);
		if (des == 0) {
			if (type & PTYPE_TAG_DONTCOPY) {
				snownet_free(data);
			}
			return -1;
		}
	}
	else {
		if ((sz & MESSAGE_TYPE_MASK) != sz) {
			snownet_error(context, "The message to %s is too large", addr);
			if (type & PTYPE_TAG_DONTCOPY) {
				snownet_free(data);
			}
			return -2;
		}
		_filter_args(context, type, &session, (void **)&data, &sz);

		//TODO
		//struct remote_message * rmsg = (struct remote_message *)snownet_malloc(sizeof(*rmsg));
		//copy_name(rmsg->destination.name, addr);
		//rmsg->destination.handle = 0;
		//rmsg->message = data;
		//rmsg->sz = sz & MESSAGE_TYPE_MASK;
		//rmsg->type = sz >> MESSAGE_TYPE_SHIFT;

		//snownet_harbor_send(rmsg, source, session);
		return session;
	}

	return snownet_send(context, source, des, type, session, data, sz);
	return session;
}

uint32_t 
snownet_context_handle(struct snownet_context *ctx) {
	return ctx->handle;
}

void 
snownet_callback(struct snownet_context * context, void *ud, snownet_cb cb) {
	context->cb = cb;
	context->cb_ud = ud;
}

void
snownet_context_send(struct snownet_context * ctx, void * msg, size_t sz, uint32_t source, int type, int session) {
	struct snownet_message smsg;
	smsg.source = source;
	smsg.session = session;
	smsg.data = msg;
	smsg.sz = sz | (size_t)type << MESSAGE_TYPE_SHIFT;

	snownet_mq_push(ctx->queue, &smsg);
}

void 
snownet_globalinit(void) {
	G_NODE.total = 0;
	G_NODE.monitor_exit = 0;
	G_NODE.init = 1;
	//TODO
	//if (pthread_key_create(&G_NODE.handle_key, NULL)) {
	//	fprintf(stderr, "pthread_key_create failed");
	//	exit(1);
	//}
	// set mainthread's key
	snownet_initthread(THREAD_MAIN);
}

void 
snownet_globalexit(void) {
	//TODO
	//pthread_key_delete(G_NODE.handle_key);
}

void
snownet_initthread(int m) {
	//TODO
	//uintptr_t v = (uint32_t)(-m);
	//pthread_setspecific(G_NODE.handle_key, (void *)v);
}

void
snownet_profile_enable(int enable) {
	G_NODE.profile = (bool)enable;
}
