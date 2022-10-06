
#include <config.h>
#include <private/logger.h>
#include <udjat/tools/timestamp.h>
#include <udjat/tools/application.h>
#include <mutex>
#include <unistd.h>
#include <syslog.h>
#include <udjat/tools/quark.h>
#include <algorithm>

using namespace std;

namespace Udjat {

	void Logger::syslog(bool enable) {
		Options::getInstance().syslog = enable;
	}

	bool Logger::write(int fd, const char *text) {

		size_t bytes = strlen(text);
		while(bytes) {
			ssize_t sz = ::write(fd,text,bytes);
			if(sz < 0)
				return false;
			bytes -= sz;
			text += sz;
		}
		return true;

	}

	void Logger::timestamp(int fd) {

		time_t t = time(0);
		struct tm tm;
		localtime_r(&t,&tm);

		char timestr[80];
		memset(timestr,0,sizeof(timestr));

		size_t len = strftime(timestr, 79, "%x %X", &tm);
		if(len) {
			Logger::write(fd,timestr);
		} else {
			Logger::write(fd,"--/--/-- --:--:--");
		}

		Logger::write(fd," ");
	}

	void Logger::write(Level level, const char *d, const char *text) noexcept {

		char domain[15];
		memset(domain,' ',15);
		memcpy(domain,d,std::min(sizeof(domain),strlen(d)));
		domain[14] = 0;

		// Serialize
		static mutex mtx;
		lock_guard<mutex> lock(mtx);

		// Write
		if(Options::getInstance().console) {

			// Write to console.
			static bool decorated = (getenv("TERM") != NULL);

			static const char *decorations[] = {
				"\x1b[92m",	// Info
				"\x1b[93m",	// Warning
				"\x1b[91m",	// Error

				"\x1b[94m",	// Trace
				"\x1b[96m",	// SysInfo (Allways Trace+1)
			};

			if(decorated) {
				Logger::write(1,decorations[((size_t) level) % (sizeof(decorations)/sizeof(decorations[0]))]);
			}

			timestamp(1);

			Logger::write(1,domain);
			Logger::write(1," ");
			Logger::write(1,text);

			if(decorated) {
				Logger::write(1,"\x1b[0m");
			}

			Logger::write(1,"\n");
			fsync(1);

		}

		if(Options::getInstance().syslog) {
			//
			// Write to syslog.
			//
			static const int priority[] = {
				LOG_INFO,		// Info
				LOG_WARNING,	// Warning
				LOG_ERR,		// Error
				LOG_DEBUG,		// Trace
				LOG_NOTICE		// Trace+1
			};

			::syslog(priority[ ((size_t) level) % (sizeof(priority)/sizeof(priority[0])) ],"%s %s",domain,text);
		}

		// TODO: Optional write to file.

	}

}

