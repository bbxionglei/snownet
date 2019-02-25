#include "snownet.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

struct logger {
	FILE * handle;
	char * filename;
	int close;
};

void *
logger_create(void) {
	struct logger * inst = (struct logger *)snownet_malloc(sizeof(*inst));
	inst->handle = NULL;
	inst->close = 0;
	inst->filename = NULL;

	return inst;
}

void
logger_release(void * inst_) {
	struct logger * inst = (struct logger *)inst_;
	if (inst->close) {
		fclose(inst->handle);
	}
	snownet_free(inst->filename);
	snownet_free(inst);
}

static int
logger_cb(struct snownet_context * context, void *ud, int type, int session, uint32_t source, const void * msg, size_t sz) {
	struct logger * inst = (struct logger *)ud;
	switch (type) {
	case PTYPE_SYSTEM:
		if (inst->filename) {
			inst->handle = freopen(inst->filename, "a", inst->handle);
		}
		break;
	case PTYPE_TEXT:
		fprintf(inst->handle, "[:%08x] ",source);
		fwrite(msg, sz , 1, inst->handle);
		fprintf(inst->handle, "\n");
		fflush(inst->handle);
		break;
	}

	return 0;
}

int
logger_init(void * inst_, struct snownet_context *ctx, const char * parm) {
	struct logger * inst = (struct logger *)inst_;
	if (parm) {
		inst->handle = fopen(parm,"w");
		if (inst->handle == NULL) {
			return 1;
		}
		inst->filename = (char*)snownet_malloc(strlen(parm) + 1);//为什么不用snownet_strdup
		strcpy(inst->filename, parm);
		inst->close = 1;
	} else {
		inst->handle = stdout;
	}
	if (inst->handle) {
		snownet_callback(ctx, inst, logger_cb);//设置回调
		return 0;
	}
	return 1;
}
