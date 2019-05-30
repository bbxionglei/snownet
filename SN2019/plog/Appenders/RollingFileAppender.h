#pragma once
#include <plog/Appenders/IAppender.h>
#include <plog/Converters/UTF8Converter.h>
#include <plog/Util.h>
#include <algorithm>
#include <time.h>

namespace plog
{
	template<class Formatter, class Converter = UTF8Converter>
	class RollingFileAppender : public IAppender
	{
	public:
		RollingFileAppender(const util::nchar* fileName, size_t maxFileSize = 0, int maxFiles = 0)
			: m_fileSize()
			, m_maxFileSize((std::max)(static_cast<off_t>(maxFileSize), static_cast<off_t>(1000))) // set a lower limit for the maxFileSize
			, m_maxFiles(maxFiles)
			, m_firstWrite(true)
		{
			util::splitFileName(fileName, m_fileNameNoExt, m_fileExt);
		}

		//#ifdef _WIN32
		//        RollingFileAppender(const char* fileName, size_t maxFileSize = 0, int maxFiles = 0)
		//            : m_fileSize()
		//            , m_maxFileSize((std::max)(static_cast<off_t>(maxFileSize), static_cast<off_t>(1000))) // set a lower limit for the maxFileSize
		//            , m_maxFiles(maxFiles)
		//            , m_firstWrite(true)
		//        {
		//            util::splitFileName(util::toWide(fileName).c_str(), m_fileNameNoExt, m_fileExt);
		//        }
		//#endif

		virtual void write(const Record& record)
		{
			util::MutexLock lock(m_mutex);

			if (m_firstWrite)
			{
				openLogFile();
				m_firstWrite = false;
			}
			else if (m_maxFiles > 0 && m_fileSize > m_maxFileSize && -1 != m_fileSize)
			{
				rollLogFiles();
			}

			int bytesWritten = m_file.write(Converter::convert(Formatter::format(record)));

			if (bytesWritten > 0)
			{
				m_fileSize += bytesWritten;
			}
		}

		void rollLogFiles()
		{
			m_file.close();

			util::nstring lastFileName = buildFileName(m_maxFiles - 1);
			util::File::unlink(lastFileName.c_str());

			for (int fileNumber = m_maxFiles - 2; fileNumber >= 0; --fileNumber)
			{
				util::nstring currentFileName = buildFileName(fileNumber);
				util::nstring nextFileName = buildFileName(fileNumber + 1);

				util::File::rename(currentFileName.c_str(), nextFileName.c_str());
			}

			openLogFile();
			m_firstWrite = false;
		}

	private:
		void openLogFile()
		{
			util::nstring fileName = buildFileName();
			m_fileSize = m_file.open(fileName.c_str());

			if (0 == m_fileSize)
			{
				int bytesWritten = m_file.write(Converter::header(Formatter::header()));

				if (bytesWritten > 0)
				{
					m_fileSize += bytesWritten;
				}
			}
		}

		util::nstring buildFileName(int fileNumber = 0)
		{
			util::nostringstream ss;
			ss << m_fileNameNoExt;

			if (fileNumber > 0)
			{
				ss << '.' << fileNumber;
			}

			if (!m_fileExt.empty())
			{
				ss << '.' << m_fileExt;
			}

			return ss.str();
		}

	private:
		util::Mutex     m_mutex;
		util::File      m_file;
		off_t           m_fileSize;
		const off_t     m_maxFileSize;
		const int       m_maxFiles;
		util::nstring   m_fileExt;
		util::nstring   m_fileNameNoExt;
		bool            m_firstWrite;
	};

	template<class Formatter, class Converter = UTF8Converter>
	class RollingFileAppenderDate : public IAppender
	{
	public:
		RollingFileAppenderDate(const util::nchar* fileName, lfm mode, const time_t* t_)
			: IAppender(t_), m_lfm(mode), m_TimeZero(0)
		{
			util::splitFileName(fileName, m_fileNameNoExt, m_fileExt);
		}

		virtual void write(const Record& record)
		{
			util::MutexLock lock(m_mutex);
			openLogFile();
			m_file.write(Converter::convert(Formatter::format(record)));
		}
	private:
		void openLogFile()
		{
			time_t t;
			if (m_lfm == lfm_postfix_date) {
				if (m_time == nullptr)
					t = time(0);
				else
					t = *m_time;
				if (t - m_TimeZero < 86400) {
					return;
				}
				tm ptm;
				util::localtime_s(&ptm, &t);
				ptm.tm_hour = 0;
				ptm.tm_min = 0;
				ptm.tm_sec = 0;
				m_fileNameAppend = "" + 1;
				m_TimeZero = mktime(&ptm);
			}
			else if (m_lfm == lfm_postfix_date_hour) {
				if (m_time == nullptr)
					t = time(0);
				else
					t = *m_time;
				if (t - m_TimeZero < 3600) {
					return;
				}
				tm ptm;
				util::localtime_s(&ptm, &t);
				ptm.tm_min = 0;
				ptm.tm_sec = 0;
				m_TimeZero = mktime(&ptm);
			}
			else {
				if (m_TimeZero != 0) {
					return;
				}
				m_TimeZero = 1;
			}
			if (m_TimeZero != 0) {
				m_file.close();
			}
			util::nstring fileName = buildFileName();
			off_t m_fileSize = m_file.open(fileName.c_str());

			if (0 == m_fileSize)
			{
				m_file.write(Converter::header(Formatter::header()));
			}
		}

		util::nstring buildFileName()
		{
			util::nostringstream ss;
			ss << m_fileNameNoExt;
			if (m_lfm == lfm_postfix_date) {
				struct tm * ptm = localtime(&m_TimeZero);
				ss << ptm->tm_year + 1900
					<< "_" << std::setfill(PLOG_NSTR('0')) << std::setw(2) << ptm->tm_mon + 1
					<< "_" << std::setfill(PLOG_NSTR('0')) << std::setw(2) << ptm->tm_mday;
			}
			else if (m_lfm == lfm_postfix_date_hour) {
				struct tm * ptm = localtime(&m_TimeZero);
				ss << ptm->tm_year + 1900
					<< "_" << std::setfill(PLOG_NSTR('0')) << std::setw(2) << ptm->tm_mon + 1
					<< "_" << std::setfill(PLOG_NSTR('0')) << std::setw(2) << ptm->tm_mday
					<< "_" << std::setfill(PLOG_NSTR('0')) << std::setw(2) << ptm->tm_hour;
			}
			if (!m_fileExt.empty())
			{
				ss << '.' << m_fileExt;
			}
			return ss.str();
		}

	private:
		util::Mutex     m_mutex;
		util::File      m_file;
		util::nstring   m_fileExt;
		util::nstring   m_fileNameNoExt;
		util::nstring   m_fileNameAppend;
		lfm             m_lfm;
		time_t          m_TimeZero;
	};
}
