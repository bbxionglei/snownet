#include <stdio.h>
#include "snownet_test.h"
#include <iostream>             // std::cout
#include <future>               // std::async, std::future
#include <chrono>               // std::chrono::milliseconds
#include <sstream>                // std::stringstream
#include <string.h>				// strlen

#include <thread>
#include <chrono>         // std::chrono::seconds

#include "socket_poll.h"
//TODO 测试
#include "atomic.h"//PASS
#include "rwlock.h"//PASS
#include "spinlock.h"//PASS
#include "snownet_malloc.h"//PASS
#include "snownet_timer.h"//bug
#include "socket_poll.h"//PASS

int
is_prime(int x)
{
	int i = 2;
	for (; i < x; ++i) {
		if (x % i == 0){
			return i;
		}
	}
	return i;
}


std::atomic_flag lock_stream = ATOMIC_FLAG_INIT;
std::stringstream stream;

void append_number(int x)
{
	//while (lock_stream.test_and_set(std::memory_order::memory_order_relaxed)) {
	//}
	stream << "thread #" << x << '\n';
	//lock_stream.clear();
}


std::atomic<bool> _3_ready(false);
std::atomic<bool> _3_winner(false);

void count1m(int id)
{
	while (!_3_ready) {}                  // wait for the ready signal
	for (int i = 0; i < 1000000; ++i) {}   // go!, count to 1 million
	if (!_3_winner.exchange(true)) { std::cout << "thread #" << id << " won!\n"; }
};

class test_class_a {
public:
	test_class_a() {
		a = 1;
		b = 1;

	}
	~test_class_a() {
		a = 2;
		b = 2;
	}
	int a;
	int b;
};

void
test_function_atomic() {

	std::atomic<int> a;
	a = 100;
	int b = ATOM_INC(&a);
	b = ATOM_FINC(&a);
	b = ATOM_DEC(&a);
	b = ATOM_FDEC(&a);
	b = ATOM_FADD(&a,10);
	b = ATOM_FSUB(&a,10);
	b = ATOM_FAND(&a,0xff);
	int c = 10;
	ATOM_CAS(&a, c, 11111);
	ATOM_CAS(&a, c, 22222);
//#define ATOM_CAS(ptr, oval, nval) (ptr)->compare_exchange_strong(oval, nval)
//#define ATOM_CAS_POINTER(ptr, oval, nval) (ptr)->compare_exchange_strong(oval, nval)
//#define ATOM_INC(ptr) ++(*ptr)//__sync_add_and_fetch(ptr, 1)
//#define ATOM_FINC(ptr) (*ptr)++//__sync_fetch_and_add(ptr, 1)
//#define ATOM_DEC(ptr) --(*ptr)//__sync_sub_and_fetch(ptr, 1)
//#define ATOM_FDEC(ptr) (*ptr)--//__sync_fetch_and_sub(ptr, 1)
//
//#define ATOM_FADD(ptr,n) (ptr)->fetch_add(n)//__sync_fetch_and_add(ptr, n)
//#define ATOM_FSUB(ptr,n) (ptr)->fetch_sub(n)//__sync_fetch_and_sub(ptr, n)
//#define ATOM_FAND(ptr,n) (ptr)->fetch_and(n)//__sync_fetch_and_and(ptr, n)
}
rwlock g_test_rw_lock;
int g_test_rw_lock_val = 0;
const int g_test_rw_lock_val_count = 1;
void foo()
{
	for (int i = 0;i< g_test_rw_lock_val_count;i++) {
		rwlock_wlock(&g_test_rw_lock);
		g_test_rw_lock_val++;
		printf("write g_test_rw_lock_val=%d\n", g_test_rw_lock_val);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		rwlock_wunlock(&g_test_rw_lock);
		std::this_thread::yield();
	}
}

void bar()
{
	for (int i = 0; i < g_test_rw_lock_val_count; i++) {
		std::this_thread::yield();
		rwlock_wlock(&g_test_rw_lock);
		g_test_rw_lock_val++;
		printf("read  g_test_rw_lock_val=%d\n", g_test_rw_lock_val);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		rwlock_wunlock(&g_test_rw_lock);
	}
}

void
test_function_rwlock() {
	rwlock_init(&g_test_rw_lock);
	std::thread first(foo);     // spawn new thread that calls foo()
	std::thread second(bar);  // spawn new thread that calls bar(0)

	first.join();                // pauses until first finishes
	second.join();               // pauses until second finishes
}
struct spinlock_test {
	struct spinlock lock;
};
spinlock_test slock;
void spinlock_foo()
{
	SPIN_LOCK(&slock);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	SPIN_UNLOCK(&slock);
}

void spinlock_bar()
{
	auto iret = SPIN_TRY_LOCK(&slock);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	iret = SPIN_TRY_LOCK(&slock);
	iret = 0;
}
void
test_function_spinlock() {
	SPIN_INIT(&slock);
	std::thread first(spinlock_foo);     // spawn new thread that calls foo()
	std::thread second(spinlock_bar);  // spawn new thread that calls bar(0)

	first.join();                // pauses until first finishes
	second.join();               // pauses until second finishes
}
void
test_function_malloc() {
	char * ret0 = (char*)snownet_malloc(10);
	char * ret1 = (char*)snownet_malloc(10);
	char* ret2 = snownet_strdup("hello snownet_strdup");
	char* ret3 = (char*)snownet_lalloc(ret2, strlen(ret2), strlen(ret2) + 1);
	snownet_free(ret0);
	snownet_free(ret1);
	//snownet_free(ret2);
	snownet_free(ret3);
}

void
test_function_timer() {
	//int snownet_timeout(uint32_t handle, int time, int session);
	//void snownet_updatetime(void);
	//uint32_t snownet_starttime(void);
	//uint64_t snownet_thread_time(void);	// for profile, in micro second

	//void snownet_timer_init(void);
	snownet_timer_init();
	auto st = snownet_starttime();
	auto tt = snownet_thread_time();
	snownet_updatetime();
	//snownet_timeout(1, 0, 0);
	//snownet_timeout(1, 1, 0);
}

int64_t itotalread = 0;
int64_t itotalsend = 0;
void select_foo()
{
#ifdef _WIN32
	int iSts = 0;

	int iret = -1;
	int iErno;
	int iSockfd;
	int iSocklen;
	int nPortNo = 7777;
	long lTemp;
	int fd[2] = { -1 };
	struct sockaddr_in srcAddr;
	struct sockaddr_in dstAddr;
	WSADATA data;
	// 变量初始化
	iSockfd = -1;
	iSocklen = sizeof(struct sockaddr_in);
	lTemp = 0;
	iErno = 0;
	memset(&srcAddr, 0, sizeof(srcAddr));
	memset(&dstAddr, 0, sizeof(dstAddr));
	memset(&data, 0, sizeof(data));
	do 
	{
		// 创建socket
		iSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (iSockfd == -1) {
			printf(" CreateSocketLikePipe  create socket\n");
			break;
		}

		// 设置sockaddr_in 结构体
		memset(&srcAddr, 0, sizeof(srcAddr));
		srcAddr.sin_port = htons((u_short)nPortNo);
		srcAddr.sin_family = AF_INET;
		srcAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		// 绑定
		iSts = bind(iSockfd, (struct sockaddr*)&srcAddr, sizeof(srcAddr));
		if (iSts != 0) {
			printf(" CreateSocketLikePipe bind\n");
			break;
		}

		// 监听
		iSts = listen(iSockfd, 1);
		if (iSts != 0) {
			printf(" CreateSocketLikePipe listen\n");
			break;
		}
	} while (0);
	int spfd = sp_create();

	sp_nonblocking((int)iSockfd);
	sp_add(spfd, iSockfd, (void*)iSockfd);

	printf("iSockfd=%d\n", iSockfd);
	struct event ev[64];
	int i, n,isendbytes;
	char recvBuff[1024] = { 0 };
	int client_remove_count = 0;
	for (;;) {
		n = sp_wait(spfd, ev, 64);
		for (i = 0; i < n; i++) {
			void* ud = ev[i].s;
			if (ev[i].read) {
				if ((int)ud == iSockfd) {
					fd[0] = (int)accept(iSockfd, (struct sockaddr *)&srcAddr, &iSocklen);
					sp_add(spfd, fd[0], (void*)fd[0]);
					printf("sp_wait read accept fd=%d\n", fd[0]);
				}
				else {
					int m = recv((int)ud, recvBuff, 1024, 0);
					if (m) {
						recvBuff[m] = 0;
						printf("sp_wait read from %d,data=%s\n", ud, recvBuff);
						//send((int)ud, recvBuff, strlen(recvBuff), 0);
						//不停的写，写到缓冲区满为止
						sp_nonblocking((int)ud);
						for (;;) {
							isendbytes = send((int)ud, "hello from serverffffffffffffffffffff\n", strlen("hello from serverffffffffffffffffffff\n"), 0);
							if (isendbytes <= 0) {
								printf("sp_write %d\n", ud);
								sp_write(spfd, (int)ud, ud, true);
								break;
							}
							else {
								itotalsend += isendbytes;
							}
						}
					}
					else {
						printf("sp_wait sp_del ud=%d m=%d\n", ud, m);
						sp_del(spfd, (int)ud);
						client_remove_count++;
					}
					printf("sp_wait read ud=%d\n", ud);
				}
			}
			if (ev[i].write) {
				isendbytes = send((int)ud, "hello from serverooooooooo\n", strlen("hello from serverooooooooo\n"), 0);
				if (isendbytes <= 0) {
					printf("sp_write %d\n", ud);
					break;
				}
				else {
					itotalsend += isendbytes;
				}
				printf("end   send %d / %ld %ld\n", ud, itotalsend, itotalsend - itotalread);
			}
			if (ev[i].error) {
				printf("sp_wait error ud=%d\n", ud);
				sp_del(spfd, (int)ud);
				client_remove_count++;
				break;
			}
		}
		if (n)
			printf("sp_wait n=%d\n", n);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		if (client_remove_count)
			break;
	}
#endif
}

void
test_function_socket_select() {
	std::thread first(select_foo);     // spawn new thread that calls foo()

#ifdef _WIN32

	int sockfd = 0, n = 0;
	char recvBuff[1024] = { 0 };
	struct sockaddr_in serv_addr;

	memset(recvBuff, '0', sizeof(recvBuff));
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Error : Could not create socket \n");
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(7777);
	serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	//sp_nonblocking(sockfd);
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\n Error : Connect Failed \n");
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	send(sockfd, "hello", strlen("hello"), 0);
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	for(;;){
		n = recv(sockfd, recvBuff, 1023, 0);
		if (n) {
			itotalread += n;
			recvBuff[n] = 0;
			printf("recvBuff=%s itotalread=%d \n", recvBuff, itotalread);
			if (itotalread == itotalsend) {
				break;
			}
		}
		else {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	printf("\n Read end \n");
	closesocket(sockfd);

#endif
	first.join();                // pauses until first finishes
}
int
test_function() {
	test_function_socket_select();
	test_function_timer();
	test_function_malloc();
	test_function_spinlock();
	test_function_rwlock();
	test_function_atomic();
	//new
	test_class_a a1;
	test_class_a* a2 = new test_class_a();
	char *a3ptr = new char[128]{ 0 };
	test_class_a* a3 = new (a3ptr) test_class_a();
	//test_class_a* a4 = new (a2) test_class_a();
	delete a2;
	delete a3;
	sp_invalid(0);

	printf("hello test_function\n");
	{
		// call function asynchronously:
		std::future < int > fut = std::async(is_prime, 444444443);

		// do something while waiting for function to set future:
		std::cout << "checking, please wait";
		std::chrono::milliseconds span(100);
		//while (fut.wait_for(span) == std::future_status::timeout)
		//	std::cout << '.';

		std::shared_future<int> shared_fut = fut.share();

		int x = shared_fut.get();         // retrieve return value
		//int y = shared_fut.get();         // retrieve return value

		std::cout << "\n444444443 " << (x ? "is" : "is not") << " prime.\n";

	}
	/*{
		std::vector < std::thread > threads;
		threads.push_back(std::thread(append_number, 88));
		for (int i = 1; i <= 10; ++i)
			threads.push_back(std::thread(append_number, i));
		for (auto & th : threads)
			th.join();

		std::cout << stream.str() << std::endl;;
	}
	{
		std::vector<std::thread> threads;
		std::cout << "spawning 10 threads that count to 1 million...\n";
		for (int i = 1; i <= 10; ++i) threads.push_back(std::thread(count1m, i));
		_3_ready = true;
		for (auto& th : threads) th.join();
	}
*/
	return 0;
}
	