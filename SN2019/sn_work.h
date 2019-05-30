#ifndef _SN_WORK_H_
#define _SN_WORK_H_
#include <thread>
#include <mutex>
#include <condition_variable>

#include "sn_log.h"

using namespace std;
void thread_cin(void* p, int iParameter);

namespace sn {
	class sn_work :plog::util::Singleton<sn_work> {
	public:
		sn_work();
		void run(int workers_number);
		bool m_bExit;
		condition_variable cv;
		mutex m;
	private:
	};
}
#endif//_SN_WORK_H_
