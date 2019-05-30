#pragma once
#include <plog/Logger.h>
#include <plog/Formatters/CsvFormatter.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <cstring>

namespace plog
{
    namespace
    {
        bool isCsv(const util::nchar* fileName)
        {
            const util::nchar* dot = util::findExtensionDot(fileName);
//#ifdef _WIN32
//            return dot && 0 == std::wcscmp(dot, L".csv");
//#else
            return dot && 0 == std::strcmp(dot, ".csv");
//#endif
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // Empty initializer / one appender

    template<int instance>
    inline Logger<instance>& init(Severity maxSeverity = none, IAppender* appender = NULL, const time_t* t_ = 0)
    {
        static Logger<instance> logger(maxSeverity,t_);
        return appender ? logger.addAppender(appender) : logger;
    }

    inline Logger<PLOG_DEFAULT_INSTANCE>& init(Severity maxSeverity = none, IAppender* appender = NULL, const time_t* t_ = 0)
    {
        return init<PLOG_DEFAULT_INSTANCE>(maxSeverity,appender,t_);
    }

    ////////////////////////////////////////////////////////////////////////////
    //// RollingFileAppender with any Formatter

    template<class Formatter, int instance>
    inline Logger<instance>& init(Severity maxSeverity, lfm mode , const util::nchar* fileName, const time_t* t_, size_t maxFileSize = 0, int maxFiles = 0)
    {
		//static RollingFileAppender<Formatter> rollingFileAppender(fileName, maxFileSize, maxFiles);
		//return init<instance>(maxSeverity, mode, &rollingFileAppender);
		static RollingFileAppenderDate<Formatter> rollingFileAppenderDate(fileName, mode, t_);
		return init<instance>(maxSeverity, &rollingFileAppenderDate,t_);
    }

    template<class Formatter>
    inline Logger<PLOG_DEFAULT_INSTANCE>& init(Severity maxSeverity,lfm mode, const util::nchar* fileName, const time_t* t_, size_t maxFileSize = 0, int maxFiles = 0)
    {
        return init<Formatter, PLOG_DEFAULT_INSTANCE>(maxSeverity,mode, fileName,t_, maxFileSize, maxFiles);
    }

    //////////////////////////////////////////////////////////////////////////
    // RollingFileAppender with TXT/CSV chosen by file extension

    template<int instance>
    inline Logger<instance>& init(Severity maxSeverity, lfm mode, const util::nchar* fileName, const time_t* t_, size_t maxFileSize = 0, int maxFiles = 0)
    {
        return isCsv(fileName) ? init<CsvFormatter, instance>(maxSeverity,mode, fileName, t_,maxFileSize, maxFiles) 
			: init<TxtFormatter, instance>(maxSeverity,mode, fileName, t_,maxFileSize, maxFiles);
    }
    inline Logger<PLOG_DEFAULT_INSTANCE>& init(Severity maxSeverity, lfm mode, const util::nchar* fileName, const time_t* t_, size_t maxFileSize = 0, int maxFiles = 0)
    {
        return init<PLOG_DEFAULT_INSTANCE>(maxSeverity,mode, fileName, t_,maxFileSize, maxFiles);
    }
}
