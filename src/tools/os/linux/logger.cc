
#include <config.h>
#include <udjat/tools/logger.h>
#include <udjat/tools/timestamp.h>
#include <udjat/tools/application.h>
#include <mutex>
#include <unistd.h>
#include <syslog.h>
#include <udjat/tools/quark.h>

using namespace std;

namespace Udjat {

	//
	// Syslog writer
	//
	class SysWriter {
	private:
		SysWriter() {
			::openlog(Quark(Application::Name()).c_str(), LOG_PID, LOG_DAEMON);
		}

	public:
		~SysWriter() {
			::closelog();
		}

		SysWriter(const SysWriter &src) = delete;
		SysWriter(const SysWriter *src) = delete;

		static SysWriter & getInstance();

		void write(uint8_t id, const char *message) {
			static const int priority[] = {LOG_INFO,LOG_WARNING,LOG_ERR};
			syslog(priority[id],"%s",message);

		}

	};

	SysWriter & SysWriter::getInstance() {
		static SysWriter instance;
		return instance;
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

		// Get '\t' position.
		auto pos = buffer.find("\t");

		// If enable write console output.
		if(console) {

			// Write current time.
			{
				time_t t = time(0);
				struct tm tm;
				localtime_r(&t,&tm);

				char buffer[80];
				memset(buffer,0,sizeof(buffer));

				size_t len = strftime(buffer, 79, "%x %X", &tm);

				if(len) {
					write(1,buffer);
				} else {
					write(1,"--/--/-- --:--:--");
				}

				write(1," ");
			}

			// Write module name & message
			char module[12];
			memset(module,' ',sizeof(module));

			if(pos == string::npos) {

				module[sizeof(module)-1] = 0;
				write(1,module);
				write(1," ");
				write(1,buffer);

			} else {

				strncpy(module,buffer.c_str(),min(sizeof(module),pos));
				for(size_t ix = 0; ix < sizeof(module); ix++) {
					if(module[ix] < ' ') {
						module[ix] = ' ';
					}
				}
				module[sizeof(module)-1] = 0;
				write(1,module);
				write(1," ");
				write(1,buffer.c_str()+pos+1);
			}

			write(1,"\n");
			fsync(1);
		}

		{
			size_t length = buffer.size()+2;
			char tmp[length+1];
			size_t ix = 0;
			for(const char *ptr = buffer.c_str(); *ptr && ix < length; ptr++) {
				if(*ptr == '\t') {
					tmp[ix++] = ':';
					tmp[ix++] = ' ';
				} else if(*ptr < ' ') {
					tmp[ix++] = '?';
				} else {
					tmp[ix++] = *ptr;
				}
			}
			tmp[ix] = 0;

			SysWriter::getInstance().write(id,tmp);
		}

		buffer.erase();

	}


}

