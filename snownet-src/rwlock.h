#ifndef SNOWNET_RWLOCK_H
#define SNOWNET_RWLOCK_H
#include <atomic>

struct rwlock {
	std::atomic<char> write;
	std::atomic<char> read;
};

static inline void
rwlock_init(struct rwlock *lock) {
	lock->write.store(0);
	lock->read.store(0);
}

static inline void
rwlock_rlock(struct rwlock *lock) {
	for (;;) {
		while (lock->write)
			lock->write.load();
		lock->read++;
		if (lock->write) {
			lock->read--;
		}
		else {
			break;
		}
	}
}

static inline void
rwlock_wlock(struct rwlock *lock) {
	while (!lock->write)lock->write++;
	while (lock->read)lock->read.load();
}

static inline void
rwlock_wunlock(struct rwlock *lock) {
	lock->write--;
}

static inline void
rwlock_runlock(struct rwlock *lock) {
	lock->read--;
}

#endif//SNOWNET_RWLOCK_H
