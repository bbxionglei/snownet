#ifndef snownet_malloc_h
#define snownet_malloc_h

#include <stddef.h>

#define snownet_malloc malloc
#define snownet_calloc calloc
#define snownet_realloc realloc
#define snownet_free free

//void * snownet_malloc(size_t sz);
//void * snownet_calloc(size_t nmemb, size_t size);
//void * snownet_realloc(void *ptr, size_t size);
//void snownet_free(void *ptr);
char * snownet_strdup(const char *str);
void * snownet_lalloc(void *ptr, size_t osize, size_t nsize);	// use for lua

#endif
