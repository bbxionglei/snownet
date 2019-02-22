#include "snownet.h"
#include "snownet_server.h"
#include "snownet_imp.h"
#include "snownet_mq.h"
#include "snownet_handle.h"
#include "snownet_module.h"
#include "snownet_timer.h"
#include "snownet_monitor.h"
#include "snownet_socket.h"
//#include "snownet_daemon.h"
#include "snownet_harbor.h"

	//TODO
//#include <pthread.h>
//#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

struct monitor {
	int count;
	struct snownet_monitor ** m;
	//TODO
	//pthread_cond_t cond;
	//pthread_mutex_t mutex;
	int sleep;
	int quit;
};

struct worker_parm {
	struct monitor *m;
	int id;
	int weight;
};

static int SIG = 0;

static void
handle_hup(int signal) {
	//TODO
	//if (signal == SIGHUP) {
	//	SIG = 1;
	//}
}

#define CHECK_ABORT if (snownet_context_total()==0) break;//判断是不是所有的服务都运行完了。

//TODO
//static void
//create_thread(pthread_t *thread, void *(*start_routine) (void *), void *arg) {
//	if (pthread_create(thread,NULL, start_routine, arg)) {
//		fprintf(stderr, "Create thread failed");
//		exit(1);
//	}
//}

static void
wakeup(struct monitor *m, int busy) {
	if (m->sleep >= m->count - busy) {
		// signal sleep worker, "spurious wakeup" is harmless
	//TODO
		//pthread_cond_signal(&m->cond);
	}
}

static void *
thread_socket(void *p) {
	struct monitor * m = (struct monitor *)p;
	snownet_initthread(THREAD_SOCKET);
	for (;;) {
		int r = snownet_socket_poll();
		if (r==0)
			break;
		if (r<0) {
			CHECK_ABORT
			continue;
		}
		wakeup(m,0);
	}
	return NULL;
}

static void
free_monitor(struct monitor *m) {
	int i;
	int n = m->count;
	for (i=0;i<n;i++) {
		snownet_monitor_delete(m->m[i]);
	}
	//TODO
	//pthread_mutex_destroy(&m->mutex);
	//pthread_cond_destroy(&m->cond);
	snownet_free(m->m);
	snownet_free(m);
}

static void *
thread_monitor(void *p) {
	struct monitor * m = (struct monitor *)p;
	int i;
	int n = m->count;
	snownet_initthread(THREAD_MONITOR);
	for (;;) {
		CHECK_ABORT
		for (i=0;i<n;i++) {
			snownet_monitor_check(m->m[i]);
		}
		//为什么设置成5次循环sleep
		for (i=0;i<5;i++) {
			CHECK_ABORT
				//TODO
			//sleep(1);
		}
	}

	return NULL;
}

static void
signal_hup() {
	// make log file reopen

	struct snownet_message smsg;
	smsg.source = 0;
	smsg.session = 0;
	smsg.data = NULL;
	smsg.sz = (size_t)PTYPE_SYSTEM << MESSAGE_TYPE_SHIFT;
	uint32_t logger = snownet_handle_findname("logger");
	if (logger) {
		snownet_context_push(logger, &smsg);
	}
}

static void *
thread_timer(void *p) {
	struct monitor * m = (struct monitor *)p;
	snownet_initthread(THREAD_TIMER);
	for (;;) {
		snownet_updatetime();
		snownet_socket_updatetime();
		CHECK_ABORT
			wakeup(m, m->count - 1);
		//TODO
		//usleep(2500);
		if (SIG) {
			signal_hup();
			SIG = 0;
		}
	}
	// wakeup socket thread
	snownet_socket_exit();
	// wakeup all worker thread
	//TODO
	//pthread_mutex_lock(&m->mutex);
	//m->quit = 1;
	//pthread_cond_broadcast(&m->cond);
	//pthread_mutex_unlock(&m->mutex);
	return NULL;
}

// 调度线程的工作函数
static void *
thread_worker(void *p) {
	struct worker_parm *wp = (struct worker_parm *)p;
	int id = wp->id;
	int weight = wp->weight;
	struct monitor *m = wp->m;
	struct snownet_monitor *sm = m->m[id];
	snownet_initthread(THREAD_WORKER);
	struct message_queue * q = NULL;
	while (!m->quit) {
		q = snownet_context_message_dispatch(sm, q, weight); // 消息队列的派发和处理
		if (q == NULL) {
			//TODO
			//if (pthread_mutex_lock(&m->mutex) == 0) {
			//	++ m->sleep;
			//	// "spurious wakeup" is harmless,
			//	// because snownet_context_message_dispatch() can be call at any time.
			//	if (!m->quit)
			//		pthread_cond_wait(&m->cond, &m->mutex);
			//	-- m->sleep;
			//	if (pthread_mutex_unlock(&m->mutex)) {
			//		fprintf(stderr, "unlock mutex error");
			//		exit(1);
			//	}
			//}
		}
	}
	return NULL;
}

static void
start(int thread) {
	//TODO
	//pthread_t pid[thread+3];

	//struct monitor *m = snownet_malloc(sizeof(*m));
	//memset(m, 0, sizeof(*m));
	//m->count = thread;//监控几个线程，判断有没有死循环
	//m->sleep = 0;

	//m->m = snownet_malloc(thread * sizeof(struct snownet_monitor *));
	//int i;
	//for (i=0;i<thread;i++) {
	//	m->m[i] = snownet_monitor_new();
	//}
	//if (pthread_mutex_init(&m->mutex, NULL)) {
	//	fprintf(stderr, "Init mutex error");
	//	exit(1);
	//}
	//if (pthread_cond_init(&m->cond, NULL)) {
	//	fprintf(stderr, "Init cond error");
	//	exit(1);
	//}

	//create_thread(&pid[0], thread_monitor, m);
	//create_thread(&pid[1], thread_timer, m);
	//create_thread(&pid[2], thread_socket, m);

	//static int weight[] = { 
	//	-1, -1, -1, -1, 0, 0, 0, 0,
	//	1, 1, 1, 1, 1, 1, 1, 1, 
	//	2, 2, 2, 2, 2, 2, 2, 2, 
	//	3, 3, 3, 3, 3, 3, 3, 3, };
	//struct worker_parm wp[thread];
	//for (i=0;i<thread;i++) {
	//	wp[i].m = m;
	//	wp[i].id = i;
	//	if (i < sizeof(weight)/sizeof(weight[0])) {
	//		wp[i].weight= weight[i];
	//	} else {
	//		wp[i].weight = 0;
	//	}
	//	create_thread(&pid[i+3], thread_worker, &wp[i]);
	//}

	//for (i=0;i<thread+3;i++) {
	//	pthread_join(pid[i], NULL); 
	//}

	//free_monitor(m);
}

static void
bootstrap(struct snownet_context * logger, const char * cmdline) {
	//TODO
	//int sz = strlen(cmdline);
	//char name[sz+1];
	//char args[sz+1];
	//sscanf(cmdline, "%s %s", name, args);
	//struct snownet_context *ctx = snownet_context_new(name, args);
	//if (ctx == NULL) {
	//	snownet_error(NULL, "Bootstrap error : %s\n", cmdline);
	//	snownet_context_dispatchall(logger);
	//	exit(1);
	//}
}

void 
snownet_start(struct snownet_config * config) {
	//TODO
	//// register SIGHUP for log file reopen
	//struct sigaction sa;
	//sa.sa_handler = &handle_hup;
	//sa.sa_flags = SA_RESTART;
	//sigfillset(&sa.sa_mask);
	//sigaction(SIGHUP, &sa, NULL);

	//if (config->daemon) {
	//	if (daemon_init(config->daemon)) {
	//		exit(1);
	//	}
	//}
	snownet_harbor_init(config->harbor);
	snownet_handle_init(config->harbor);
	snownet_mq_init();
	snownet_module_init(config->module_path);
	snownet_timer_init();
	snownet_socket_init();
	snownet_profile_enable(config->profile);

	struct snownet_context *ctx = snownet_context_new(config->logservice, config->logger);
	if (ctx == NULL) {
		fprintf(stderr, "Can't launch %s service\n", config->logservice);
		exit(1);
	}

	snownet_handle_namehandle(snownet_context_handle(ctx), "logger");//关联名字到这个handle

	//以 snlua bootstrap 为例 加载 snlua 并传入参数 bootstrap
	bootstrap(ctx, config->bootstrap);

	//看了半天代码，终于到了开启工作线程的时候，累死我了,可是程序执行到这时1毫秒都用不了
	start(config->thread);

	// harbor_exit may call socket send, so it should exit before socket_free
	snownet_harbor_exit();
	snownet_socket_free();
	if (config->daemon) {
		//TODO
		//daemon_exit(config->daemon);
	}
}
