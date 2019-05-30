#include "sn_work.h"
#include "sn_2019.h"
namespace sn {

	void thread_socket(void* p, int iParameter) {
		LOGV << "bye bye !";
	}
	void thread_monitor(void* p, int iParameter) {
		LOGV << "bye bye !";
	}
	void thread_timer(void* p, int iParameter) {
		LOGV << "0x" << uppercase << setw(8) << hex << p << "," << dec << iParameter;
		sn_work* psn_work = static_cast<sn_work*>(p);
		this_thread::sleep_for(2s);
		LOGV << "bye bye !";
	}
	void thread_worker(void* p, int iParameter) {
		LOGV << "0x" << uppercase << setw(8) << hex << p << "," << dec << iParameter;
		sn_work* psn_work = static_cast<sn_work*>(p);
		int iMsgDoneCount = 0;
		while (!psn_work->m_bExit) {
			sn_msg_queue_server* s = gs_msg_queue.pop();
			if (s != nullptr) {
				for(;;){
					sn_message *msg = s->pop();
					if (msg == nullptr)
						break;
					LOGV << iParameter << " has " << ++iMsgDoneCount << " msg:" << msg->data;
					delete msg;
				}
			}
			else {
				LOGV << iParameter << " will waiting for msg";
				unique_lock<mutex> lk(psn_work->m);
				psn_work->cv.wait(lk);
			}
		}
		LOGV << "bye bye !";
	}
	sn_work::sn_work()
		:m_bExit(false)
	{
	}
	void sn_work::run(int workers_number)
	{
		thread* pWorkers = new thread[workers_number+4];
		for (int i = 0; i < workers_number; i++) {
			pWorkers[i] = thread(thread_worker, this, i);
		}
		pWorkers[workers_number] = thread(thread_socket, this, workers_number);
		pWorkers[workers_number + 1] = thread(thread_monitor, this, workers_number);
		pWorkers[workers_number + 2] = thread(thread_timer, this, workers_number);
#if defined(_DEBUG)
		pWorkers[workers_number + 3] = thread(thread_cin, this, workers_number);
#endif
		for (int i = 0; i < workers_number; i++) {
			pWorkers[i].join();
		}
	}
}