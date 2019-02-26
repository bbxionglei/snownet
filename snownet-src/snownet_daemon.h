#ifndef snownet_daemon_h
#define snownet_daemon_h

int daemon_init(const char *pidfile);
int daemon_exit(const char *pidfile);

#endif
