#pragma once
#include <plog/Record.h>

namespace plog
{
    class IAppender
    {
    public:
		IAppender(const time_t* t_) :
			m_time(t_) {}
        virtual ~IAppender()
        {
        }

        virtual void write(const Record& record) = 0;
		const time_t* m_time;
    };
}
