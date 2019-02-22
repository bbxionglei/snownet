#ifndef snownet_socket_h
#define snownet_socket_h

#include "socket_info.h"

struct snownet_context;

#define SNOWNET_SOCKET_TYPE_DATA 1
#define SNOWNET_SOCKET_TYPE_CONNECT 2
#define SNOWNET_SOCKET_TYPE_CLOSE 3
#define SNOWNET_SOCKET_TYPE_ACCEPT 4
#define SNOWNET_SOCKET_TYPE_ERROR 5
#define SNOWNET_SOCKET_TYPE_UDP 6
#define SNOWNET_SOCKET_TYPE_WARNING 7

struct snownet_socket_message {
	int type;
	int id;
	int ud;
	char * buffer;
};

void snownet_socket_init();
void snownet_socket_exit();
void snownet_socket_free();
int snownet_socket_poll();
void snownet_socket_updatetime();

int snownet_socket_send(struct snownet_context *ctx, int id, void *buffer, int sz);
int snownet_socket_send_lowpriority(struct snownet_context *ctx, int id, void *buffer, int sz);
int snownet_socket_listen(struct snownet_context *ctx, const char *host, int port, int backlog);
int snownet_socket_connect(struct snownet_context *ctx, const char *host, int port);
int snownet_socket_bind(struct snownet_context *ctx, int fd);
void snownet_socket_close(struct snownet_context *ctx, int id);
void snownet_socket_shutdown(struct snownet_context *ctx, int id);
void snownet_socket_start(struct snownet_context *ctx, int id);
void snownet_socket_nodelay(struct snownet_context *ctx, int id);

int snownet_socket_udp(struct snownet_context *ctx, const char * addr, int port);
int snownet_socket_udp_connect(struct snownet_context *ctx, int id, const char * addr, int port);
int snownet_socket_udp_send(struct snownet_context *ctx, int id, const char * address, const void *buffer, int sz);
const char * snownet_socket_udp_address(struct snownet_socket_message *, int *addrsz);

struct socket_info * snownet_socket_info();

#endif
