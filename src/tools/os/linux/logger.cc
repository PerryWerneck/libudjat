
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

	void Logger::Writer::write(const char *message) const noexcept {
		Logger::write(level,console,file,message);
	}

	void Logger::write(Level level, bool console, bool file, const char *message) noexcept {

		// Serialize
		static mutex mtx;
		lock_guard<mutex> lock(mtx);

		// Split message.
		char domain[15];
		memset(domain,' ',15);

		const char *text = strchr(message,'\t');
		if(text) {
			memcpy(domain,message,std::min( (int) (text-message), (int) sizeof(domain) ));
		} else {
			text = message;
		}
		domain[14] = 0;

		while(*text && isspace(*text)) {
			text++;
		}

		if(console) {

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

		if(file) {
			//
			// Write to syslog.
			//
			static const int priority[] = {LOG_INFO,LOG_WARNING,LOG_ERR,LOG_DEBUG};

			{
				char *ptr = strchr(domain,' ');
				if(ptr) {
					*ptr = 0;
				}
			}

			if(*domain) {
				syslog(priority[ ((size_t) level) % (sizeof(priority)/sizeof(priority[0])) ],"%s: %s",domain,text);
			} else {
				syslog(priority[ ((size_t) level) % (sizeof(priority)/sizeof(priority[0])) ],"%s",text);
			}
		}
	}

	void Logger::Writer::write(Buffer &buffer) {

		// Remove spaces
		size_t len = buffer.size();
		while(len--) {

			if(isspace(buffer[len])) {
				buffer[len] = 0;
			} else {
				break;
			}
		}

		buffer.resize(strlen(buffer.c_str()));

		if(buffer.empty()) {
			return;
		}

		write(buffer.c_str());

		buffer.erase();

	}


}

