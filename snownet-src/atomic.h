#ifndef SNOWNET_ATOMIC_H
#define SNOWNET_ATOMIC_H

//#ifndef _WIN32
//#define ATOM_CAS(ptr, oval, nval) __sync_bool_compare_and_swap(ptr, oval, nval)
//#define ATOM_CAS_POINTER(ptr, oval, nval) __sync_bool_compare_and_swap(ptr, oval, nval)
//#define ATOM_INC(ptr) __sync_add_and_fetch(ptr, 1)
//#define ATOM_FINC(ptr) __sync_fetch_and_add(ptr, 1)
//#define ATOM_DEC(ptr) __sync_sub_and_fetch(ptr, 1)
//#define ATOM_FDEC(ptr) __sync_fetch_and_sub(ptr, 1)
//#define ATOM_ADD(ptr,n) __sync_add_and_fetch(ptr, n)
//#define ATOM_SUB(ptr,n) __sync_sub_and_fetch(ptr, n)
//#define ATOM_AND(ptr,n) __sync_and_and_fetch(ptr, n)
//#else
#include <atomic>

#define ATOM_CAS(atom, oval, n) (atom)->compare_exchange_strong(oval, n)
#define ATOM_ADD(atom,n) std::atomic_fetch_add(atom, n)
#define ATOM_INC(atom) std::atomic_fetch_add(atom, 1)

#define ATOM_SUB(atom,n) std::atomic_fetch_sub(atom, n)
#define ATOM_DEC(atom) std::atomic_fetch_sub(atom, 1)

#define ATOM_AND(atom,n) std::atomic_fetch_and(atom, n)

//#endif

#endif
