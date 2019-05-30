#pragma once

namespace plog
{
    enum Severity
    {
        none = 0,
        fatal = 1,
        error = 2,
        warning = 3,
        info = 4,
        debug = 5,
        verbose = 6
    };
	enum lfm {
		lfm_filename = 0,			//ֱ��ָ���ļ���
		lfm_postfix_date = 1,		//��׺��ʽ:����	dir/file-title_YYYY-mm-dd.txt
		lfm_postfix_date_hour = 2,	//��׺��ʽ:Сʱ	dir/file-title_YYYY-mm-dd-HH.txt
	};
    inline const char* severityToString(Severity severity)
    {
        switch (severity)
        {
        case fatal:
            return "FATAL";
        case error:
            return "ERROR";
        case warning:
            return "WARN";
        case info:
            return "INFO";
        case debug:
            return "DEBUG";
        case verbose:
            return "VERB";
        default:
            return "NONE";
        }
    }

    inline Severity severityFromString(const char* str)
    {
        for (Severity severity = fatal; severity <= verbose; severity = static_cast<Severity>(severity + 1))
        {
            if (severityToString(severity)[0] == str[0])
            {
                return severity;
            }
        }

        return none;
    }
}
