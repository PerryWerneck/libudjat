
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

	static void fdwrite(int fd, const char *text) {

		size_t bytes = strlen(text);
		while(bytes) {
			ssize_t sz = ::write(fd,text,bytes);
			if(sz < 0)
				return;
			bytes -= sz;
			text += sz;
		}

	}

	void Logger::Controller::write(const Level level, bool console, const char *message) noexcept {

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
			// Write current time.
			{
				time_t t = time(0);
				struct tm tm;
				localtime_r(&t,&tm);

				char timestr[80];
				memset(timestr,0,sizeof(timestr));

				size_t len = strftime(timestr, 79, "%x %X", &tm);

				if(len) {
					fdwrite(1,timestr);
				} else {
					fdwrite(1,"--/--/-- --:--:--");
				}

				fdwrite(1," ");
			}

			fdwrite(1,domain);
			fdwrite(1," ");
			fdwrite(1,text);
			fdwrite(1,"\n");
			fsync(1);

		}

		// Write to syslog.
		static const int priority[] = {LOG_INFO,LOG_WARNING,LOG_ERR,LOG_INFO};

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

		Controller::getInstance().write((Level) id, console, buffer.c_str());

		buffer.erase();

	}


}

