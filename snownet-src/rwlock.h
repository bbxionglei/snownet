#ifndef SNOWNET_RWLOCK_H
#define SNOWNET_RWLOCK_H
#include <atomic>

struct rwlock {
	std::atomic<char> write;
	std::atomic<char> read;
};

static inline void
rwlock_init(struct rwlock *lock) {
	atomic_init(lock->write, 0);
	atomic_init(lock->read, 0);
	
}

static inline void
rwlock_rlock(struct rwlock *lock) {
	//for (;;) {
	//	while(lock->write) {
	//		__sync_synchronize();
	//	}
	//	__sync_add_and_fetch(&lock->read,1);
	//	if (lock->write) {
	//		__sync_sub_and_fetch(&lock->read,1);
	//	} else {
	//		break;
	//	}
	//}
}

static inline void
rwlock_wlock(struct rwlock *lock) {
	//while (__sync_lock_test_and_set(&lock->write,1)) {}
	//while(lock->read) {
	//	__sync_synchronize();
	//}
}

static inline void
rwlock_wunlock(struct rwlock *lock) {
	//__sync_lock_release(&lock->write);
}

static inline void
rwlock_runlock(struct rwlock *lock) {
	//__sync_sub_and_fetch(&lock->read,1);
}

#endif//SNOWNET_RWLOCK_H
