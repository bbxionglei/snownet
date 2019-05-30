#include "sn_msg_queue.h"
namespace sn {

	sn::sn_msg_queue_server::sn_msg_queue_server()
	{
	}
	void sn_msg_queue_server::push(uint32_t source, int session, char * data, size_t sz)
	{
		lock_guard<mutex> lg(m_mutex);
		m_queueMsg.push(new sn_message(source, session, data, sz));
	}
	sn_message * sn_msg_queue_server::pop()
	{
		lock_guard<mutex> lg(m_mutex);
		sn_message * ret = nullptr;
		if (m_queueMsg.size()){
			ret = m_queueMsg.front();
			m_queueMsg.pop();
		}
		return ret;
	}
	sn::sn_msg_queue::sn_msg_queue()
	{
	}
	void sn_msg_queue::set_work(sn_work * work)
	{
		m_work = work;
	}
	void sn_msg_queue::push(sn_msg_queue_server * s)
	{
		lock_guard<mutex> lg(m_mutex);
		m_queueMsg.push(s);
		m_work->cv.notify_all();
	}
	sn_msg_queue_server * sn_msg_queue::pop()
	{
		lock_guard<mutex> lg(m_mutex);
		sn_msg_queue_server * ret = nullptr;
		if (m_queueMsg.size()) {
			ret = m_queueMsg.front();
			m_queueMsg.pop();
		}
		return ret;
	}
	sn_msg_queue gs_msg_queue;
}
