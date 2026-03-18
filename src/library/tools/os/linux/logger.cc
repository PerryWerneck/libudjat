
#include <config.h>
#include <private/logger.h>
#include <udjat/tools/timestamp.h>
#include <udjat/tools/application.h>
#include <udjat/tools/configuration.h>
#include <mutex>
#include <unistd.h>
#include <syslog.h>
#include <udjat/tools/quark.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <fcntl.h>

using namespace std;

namespace Udjat {

	void Logger::syslog(bool enable) {
		Options::getInstance().syslog = enable;
	}

	bool Logger::syslog() {
		return Options::getInstance().syslog;
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

	void Logger::write(Level level, const char *domain, const char *text) noexcept {
		write(level,domain,text,false);
	}

	/// @brief Default file writer.
	void Logger::file_writer(Level, const char *domain, const char *text) noexcept {

		static string format;
		static unsigned int keep = 0;
		auto &options = Options::getInstance();

		try {

			// Current timestamp.
			TimeStamp timestamp;

			string filename;

			if(options.filename && *options.filename) {

				// Use custom log file name.
				filename = options.filename;

			} else {

				// Use default log file name.
				filename = Application::LogDir::getInstance().c_str();

				if(format.empty()) {

					try {

						keep = Config::Value<unsigned int>("logfile","max-age",86400).get();
						format = Config::Value<std::string>("logfile","name-format", (Application::Name() + "-%d.log").c_str()).c_str();

					} catch(...) {

						// On error assume defaults.
						keep = 86400;
						format = (Application::Name() + "-%d.log");

					}

				}

				// Get logfile path.

				filename.append(timestamp.to_string(format.c_str()));

				struct stat st;
				if(!stat(filename.c_str(),&st) && (time(nullptr) - st.st_mtime) > keep) {
					// More than one day, remove it
					remove(filename.c_str());
				}

			}

			int fd = ::open(filename.c_str(),O_WRONLY|O_APPEND|O_CREAT,0664);

			if(fd < 0) {
				// Error opening file, fallback to syslog.
				throw system_error(errno,system_category(),filename);
			}

			String line{timestamp.to_string().c_str()," ",domain," ",text,"\n"};
		
			if(::write(fd,line.c_str(),line.size()) != (ssize_t) line.size()) {
				::close(fd);
				throw system_error(errno,system_category(),"Error writing log file");
			}

			::close(fd);

			/*
			// Open file
			std::ofstream ofs;
			ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);

			ofs.open(filename, ofstream::out | ofstream::app);
			ofs << timestamp << " " << domain << " " << text << endl;
			ofs.close();
			*/

		} catch(const std::exception &e) {

			// Error writing file, fallback to syslog
#ifdef DEBUG
			printf("----> Error writing log file: %s\n",e.what());
#endif // DEBUG

			Logger::syslog(true);
			Logger::file(false);
			::syslog(LOG_ERR,"%s",e.what());
			::syslog(LOG_ERR,"%s",text);

		} catch(...) {

			// Error writing file, fallback to syslog
#ifdef DEBUG
			printf("----> Error writing log file: %s\n","Unexpected error");
#endif // DEBUG

			Logger::syslog(true);
			Logger::file(false);
			::syslog(LOG_ERR,"%s","Unexpected error writing log file");
			::syslog(LOG_ERR,"%s",text);

		}

	}

	UDJAT_PRIVATE const char * Logger::decoration(Level level) noexcept {

		static const char *decorations[] = {
			"\x1b[91m",	// Error
			"\x1b[93m",	// Warning
			"\x1b[92m",	// Info
			"\x1b[94m",	// Trace
			"\x1b[95m",	// Debug

			"\x1b[96m",	// SysInfo (Allways Debug+1)
		};

		return decorations[((size_t) level) % (sizeof(decorations)/sizeof(decorations[0]))];

	}

	bool Logger::decorated() noexcept {
		static bool flag = isatty(1) && (getenv("TERM") != NULL);
		return flag;
	}

	/// @brief Default console writer.
	void Logger::console_writer(Logger::Level level, const char *domain, const char *text) noexcept {

		// Write to console.

		if(decorated()) {
			Logger::write(1,decoration(level));
		}

		Logger::timestamp(1);

		Logger::write(1,domain);
		Logger::write(1," ");
		Logger::write(1,text);

		if(decorated()) {
			Logger::write(1,"\x1b[0m");
		}

		Logger::write(1,"\r\n");
		fsync(1);
	}


}

