#ifndef SNOWNET_ENV_H
#define SNOWNET_ENV_H

const char * snownet_getenv(const char *key);
void snownet_setenv(const char *key, const char *value);

void snownet_env_init();

#endif
