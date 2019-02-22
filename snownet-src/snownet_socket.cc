#include "snownet.h"

#include "snownet_socket.h"
#include "socket_server.h"
#include "snownet_server.h"
#include "snownet_mq.h"
#include "snownet_harbor.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static struct socket_server * SOCKET_SERVER = NULL;

void 
snownet_socket_init() {
	SOCKET_SERVER = socket_server_create(snownet_now());
}

void
snownet_socket_exit() {
	socket_server_exit(SOCKET_SERVER);
}

void
snownet_socket_free() {
	socket_server_release(SOCKET_SERVER);
	SOCKET_SERVER = NULL;
}

void
snownet_socket_updatetime() {
	socket_server_updatetime(SOCKET_SERVER, snownet_now());
}

// mainloop thread
static void
forward_message(int type, bool padding, struct socket_message * result) {
	struct snownet_socket_message *sm;
	size_t sz = sizeof(*sm);
	if (padding) {
		if (result->data) {
			size_t msg_sz = strlen(result->data);
			if (msg_sz > 128) {
				msg_sz = 128;
			}
			sz += msg_sz;
		} else {
			result->data = "";
		}
	}
	sm = (struct snownet_socket_message *)snownet_malloc(sz);
	sm->type = type;
	sm->id = result->id;
	sm->ud = result->ud;
	if (padding) {
		sm->buffer = NULL;
		memcpy(sm+1, result->data, sz - sizeof(*sm));
	} else {
		sm->buffer = result->data;
	}

	struct snownet_message message;
	message.source = 0;
	message.session = 0;
	message.data = sm;
	message.sz = sz | ((size_t)PTYPE_SOCKET << MESSAGE_TYPE_SHIFT);
	
	if (snownet_context_push((uint32_t)result->opaque, &message)) {
		// todo: report somewhere to close socket
		// don't call snownet_socket_close here (It will block mainloop)
		snownet_free(sm->buffer);
		snownet_free(sm);
	}
}

int 
snownet_socket_poll() {
	struct socket_server *ss = SOCKET_SERVER;
	assert(ss);
	struct socket_message result;
	int more = 1;
	int type = socket_server_poll(ss, &result, &more);
	switch (type) {
	case SOCKET_EXIT:
		return 0;
	case SOCKET_DATA:
		forward_message(SNOWNET_SOCKET_TYPE_DATA, false, &result);
		break;
	case SOCKET_CLOSE:
		forward_message(SNOWNET_SOCKET_TYPE_CLOSE, false, &result);
		break;
	case SOCKET_OPEN:
		forward_message(SNOWNET_SOCKET_TYPE_CONNECT, true, &result);
		break;
	case SOCKET_ERR:
		forward_message(SNOWNET_SOCKET_TYPE_ERROR, true, &result);
		break;
	case SOCKET_ACCEPT:
		forward_message(SNOWNET_SOCKET_TYPE_ACCEPT, true, &result);
		break;
	case SOCKET_UDP:
		forward_message(SNOWNET_SOCKET_TYPE_UDP, false, &result);
		break;
	case SOCKET_WARNING:
		forward_message(SNOWNET_SOCKET_TYPE_WARNING, false, &result);
		break;
	default:
		snownet_error(NULL, "Unknown socket message type %d.",type);
		return -1;
	}
	if (more) {
		return -1;
	}
	return 1;
}

int
snownet_socket_send(struct snownet_context *ctx, int id, void *buffer, int sz) {
	return socket_server_send(SOCKET_SERVER, id, buffer, sz);
}

int
snownet_socket_send_lowpriority(struct snownet_context *ctx, int id, void *buffer, int sz) {
	return socket_server_send_lowpriority(SOCKET_SERVER, id, buffer, sz);
}

int 
snownet_socket_listen(struct snownet_context *ctx, const char *host, int port, int backlog) {
	uint32_t source = snownet_context_handle(ctx);
	return socket_server_listen(SOCKET_SERVER, source, host, port, backlog);
}

int 
snownet_socket_connect(struct snownet_context *ctx, const char *host, int port) {
	uint32_t source = snownet_context_handle(ctx);
	return socket_server_connect(SOCKET_SERVER, source, host, port);
}

int 
snownet_socket_bind(struct snownet_context *ctx, int fd) {
	uint32_t source = snownet_context_handle(ctx);
	return socket_server_bind(SOCKET_SERVER, source, fd);
}

void 
snownet_socket_close(struct snownet_context *ctx, int id) {
	uint32_t source = snownet_context_handle(ctx);
	socket_server_close(SOCKET_SERVER, source, id);
}

void 
snownet_socket_shutdown(struct snownet_context *ctx, int id) {
	uint32_t source = snownet_context_handle(ctx);
	socket_server_shutdown(SOCKET_SERVER, source, id);
}

void 
snownet_socket_start(struct snownet_context *ctx, int id) {
	uint32_t source = snownet_context_handle(ctx);
	socket_server_start(SOCKET_SERVER, source, id);
}

void
snownet_socket_nodelay(struct snownet_context *ctx, int id) {
	socket_server_nodelay(SOCKET_SERVER, id);
}

int 
snownet_socket_udp(struct snownet_context *ctx, const char * addr, int port) {
	uint32_t source = snownet_context_handle(ctx);
	return socket_server_udp(SOCKET_SERVER, source, addr, port);
}

int 
snownet_socket_udp_connect(struct snownet_context *ctx, int id, const char * addr, int port) {
	return socket_server_udp_connect(SOCKET_SERVER, id, addr, port);
}

int 
snownet_socket_udp_send(struct snownet_context *ctx, int id, const char * address, const void *buffer, int sz) {
	return socket_server_udp_send(SOCKET_SERVER, id, (const struct socket_udp_address *)address, buffer, sz);
}

const char *
snownet_socket_udp_address(struct snownet_socket_message *msg, int *addrsz) {
	if (msg->type != SNOWNET_SOCKET_TYPE_UDP) {
		return NULL;
	}
	struct socket_message sm;
	sm.id = msg->id;
	sm.opaque = 0;
	sm.ud = msg->ud;
	sm.data = msg->buffer;
	return (const char *)socket_server_udp_address(SOCKET_SERVER, &sm, addrsz);
}

struct socket_info *
snownet_socket_info() {
	return socket_server_info(SOCKET_SERVER);
}
