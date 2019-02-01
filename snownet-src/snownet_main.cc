#include <stdio.h>
#include "spinlock.h"
#include "snownet_test.h"

struct skynet_env {
	struct spinlock lock;
};
int
main(int argc, char *argv[]) {
#ifdef _WIN32
	printf("hello _WIN32\n");
#else
	printf("hello not _WIN32\n");
#endif
	printf("hello snownet 2\n");
	struct skynet_env e;
	SPIN_INIT(&e);
	SPIN_LOCK(&e);
	SPIN_UNLOCK(&e);
	{
		SPIN_AUTO(&e);
		int i = 0;
		i++;
	}
	test_function();


	return 0;
}
	