#include "sn_log.h"

static sn::sn_log gs_sn_log;

sn::sn_log::sn_log()
{
#if defined(_WIN32)
	static plog::ColorConsoleAppender<plog::TxtFormatter> colorConsoleAppender;
	static plog::DebugOutputAppender<plog::TxtFormatter> debugOutputAppender;
	plog::init(plog::verbose, &colorConsoleAppender);
	plog::init(plog::verbose, &debugOutputAppender);
#endif
	plog::init(plog::verbose, plog::lfm_filename, "d:/logfile.txt", 0, 10485760, 10);
	LOGD << "sn_log is construction";
}
