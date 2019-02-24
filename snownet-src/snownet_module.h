#ifndef SNOWNET_MODULE_H
#define SNOWNET_MODULE_H

//TODO ≤‚ ‘
struct snownet_context;

typedef void * (*snownet_dl_create)(void);
typedef int (*snownet_dl_init)(void * inst, struct snownet_context *, const char * parm);
typedef void (*snownet_dl_release)(void * inst);
typedef void (*snownet_dl_signal)(void * inst, int signal);

struct snownet_module {
	const char * name;
	void * module;
	snownet_dl_create create;
	snownet_dl_init init;
	snownet_dl_release release;
	snownet_dl_signal signal;
};

void snownet_module_insert(struct snownet_module *mod);
struct snownet_module * snownet_module_query(const char * name);
void * snownet_module_instance_create(struct snownet_module *);
int snownet_module_instance_init(struct snownet_module *, void * inst, struct snownet_context *ctx, const char * parm);
void snownet_module_instance_release(struct snownet_module *, void *inst);
void snownet_module_instance_signal(struct snownet_module *, void *inst, int signal);

void snownet_module_init(const char *path);

#endif
