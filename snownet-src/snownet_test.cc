#include <stdio.h>
#include "snownet_test.h"
#include <iostream>             // std::cout
#include <future>               // std::async, std::future
#include <chrono>               // std::chrono::milliseconds
#include <sstream>                // std::stringstream


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

int
test_function() {
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
	