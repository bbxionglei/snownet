#ifndef _SN_LOG_H_
#define _SN_LOG_H_
#include <plog/Log.h>

#if defined(_WIN32)
#include <plog/Appenders/DebugOutputAppender.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#endif

using namespace std;

namespace sn {
	class sn_log : public plog::util::Singleton<sn_log>{
	public:
		sn_log();
	};
}
#endif//_SN_LOG_H_
