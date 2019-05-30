#pragma once
#include <plog/Appenders/IAppender.h>
#include <plog/WinApi.h>

namespace plog
{
    template<class Formatter>
    class DebugOutputAppender : public IAppender
    {
    public:
		DebugOutputAppender() :IAppender(0) {}
        virtual void write(const Record& record)
        {
			OutputDebugStringA(Formatter::format(record).c_str());
        }
    };
}
