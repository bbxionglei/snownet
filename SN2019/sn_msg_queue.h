#ifndef _SN_MSG_QUEUE_H_
#define _SN_MSG_QUEUE_H_
#include <queue>
#include <mutex>

#include "sn_log.h"
#include "sn_work.h"

using namespace std;

namespace sn {
	struct sn_message {
		sn_message() :source(0), session(0), data(0), sz(0) {}
		sn_message(uint32_t source, int session, char * data, size_t sz)
			:source(source), session(session), data(data), sz(sz) {
		}
		uint32_t source;
		int session;
		char* data;
		size_t sz;
	};

	class sn_msg_queue_server {
	public:
		sn_msg_queue_server();
		void push(uint32_t source, int session, char * data, size_t sz);
		sn_message* pop();
	private:
		queue<sn_message*> m_queueMsg;
		mutex m_mutex;
	};

	class sn_msg_queue :public plog::util::Singleton<sn_msg_queue> {
	public:
		sn_msg_queue();
		void set_work(sn_work* work);
		void push(sn_msg_queue_server* s);
		sn_msg_queue_server* pop();
	private:
		queue<sn_msg_queue_server*> m_queueMsg;
		mutex m_mutex;
		sn_work* m_work;
	};
	extern sn_msg_queue gs_msg_queue;

}
#endif//_SN_MSG_QUEUE_H_
