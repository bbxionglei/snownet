#include "sn_2019.h"
#include "sn_work.h"

using namespace sn;

void thread_cin(void* p, int iParameter) {
	sn_work* psn_work = static_cast<sn_work*>(p);
	sn_msg_queue_server *s = new sn_msg_queue_server;
	for (;;) {
		string str;
		cin >> str;
		if (str.compare("quit") == 0) {
			psn_work->m_bExit = true;
			char *data = new char[5];
			memcpy(data, "quit", 5);
			s->push(0, 0, data, str.size());
			gs_msg_queue.push(s);
			break;
		}
		else if (str.compare("auto") == 0) {
			char *data = new char[128];
			memcpy(data, "auto", 5);
			for (int i = 0; i < 10000; i++) {
				s->push(0, 0, data, str.size());
				gs_msg_queue.push(s);
			}
		}
		else{
			char *data = new char[str.size()+1];
			data[str.size()] = 0;
			memcpy(data, str.c_str(), str.size());
			s->push(0, 0, data, str.size());
			gs_msg_queue.push(s);
		}
		LOGV << str;
	}
	LOGV << "bye bye !";
}
int main(int argc, char* argv[]) {
	LOGN << argv[0];
	sn_work work;
	gs_msg_queue.set_work(&work);

	work.run(2);
	LOGN << "finish!";
	return 0;
}