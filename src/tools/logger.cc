/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #include <config.h>
 #include <udjat/tools/logger.h>
 #include <private/logger.h>
 #include <udjat/tools/application.h>
 #include <private/logger.h>
 #include <udjat/tools/quark.h>
 #include <cstring>
 #include <list>
 #include <ostream>
 #include <vector>

 #ifdef _WIN32
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
 #else
	#include <unistd.h>
	#include <syslog.h>
 #endif // _WIN32

 using namespace std;

 static const char * typenames[] = {
	"error",
	"warning",
 	"info",
	"trace",
	"debug",
 };

 namespace Udjat {

	UDJAT_API unsigned short Logger::verbosity() noexcept {
		for(size_t level = 0; level < N_ELEMENTS(typenames);level++) {
			if(!enabled((Level) level)) {
				return level;
			}
		}
		return N_ELEMENTS(typenames);
	}

	UDJAT_API void Logger::verbosity(unsigned short lvl) noexcept {

		for(size_t level = 0; level < N_ELEMENTS(typenames);level++) {
			if(lvl > 0) {
				enable((Level) level,true);
				lvl--;
			} else {
				enable((Level) level,false);
			}
		}

#ifndef _WIN32
		if(Options::getInstance().syslog) {
			::syslog(LOG_INFO,"Log verbosity was changed to %s %s %s %s %s",
				(enabled(Info) ? "info" : "no-info"),
				(enabled(Warning) ? "warning" : "no-warning"),
				(enabled(Error) ? "error" : "no-error"),
				(enabled(Trace) ? "trace" : "no-trace"),
				(enabled(Debug) ? "debug" : "no-debug")
			);

		}
#endif // _WIN32

#ifdef DEBUG
		printf("%s\n",String{"info=",enabled(Info)," warning=",enabled(Warning)," error=",enabled(Error)," trace=",enabled(Trace)}.c_str());
#endif // DEBUG

	}

	void Logger::setup(const XML::Node &node) noexcept {

		static const struct {
			const char * name;
			const char * message;
		} attributes[] = {
			{ "log-info",		"Info messages are "	},	// Informational message.
			{ "log-warning",	"Warning messages are "	},	// Warning conditions.
			{ "log-error",		"Error messages are "	},	// Error conditions.
			{ "log-debug",		"Debug messages are "	},	// Debug message.
			{ "log-trace",		"Trace messages are "	},	// Trace message.
		};

		Logger::Options &options{Logger::Options::getInstance()};

		for(size_t ix = 0; ix < N_ELEMENTS(attributes); ix++) {

			auto attribute = node.attribute(attributes[ix].name);
			if(!attribute) {
				continue;
			}

			size_t option = ix % N_ELEMENTS(options.enabled);
			bool enabled = attribute.as_bool(options.enabled[option]);

			if(enabled == options.enabled[option]) {
				continue;
			}

			options.enabled[option] = enabled;
			const char * text = (options.enabled[option] ? "enabled" : "disabled");

			write(
				Trace,
				"logger",
				String(attributes[ix].message,text).c_str(),
				true
			);

		}

	}

	bool Logger::console() {
		return (bool) Options::getInstance().console;
	}

	void Logger::dummy_writer(Level, const char *, const char *) noexcept {
	}

	void Logger::console(bool enable) {
		if(enable) {
			Options::getInstance().console = console_writer;
		} else {
			Options::getInstance().console = dummy_writer;
		}
	}

	void Logger::file(bool enable) {
		if(enable) {
			Application::LogDir::getInstance();	// Get log path, mkdir if necessary.
			Options::getInstance().file = file_writer;
		} else {
			Options::getInstance().file = dummy_writer;
		}
	}

	bool Logger::file() {
		return Options::getInstance().file;
	}

	UDJAT_API void Logger::console(void (*writer)(Level, const char *, const char *)) {
		Options::getInstance().console = writer;
	}

	UDJAT_API void Logger::file(void (*writer)(Level, const char *, const char *)) {
		Options::getInstance().file = writer;
	}

	Logger::Options & Logger::Options::getInstance() {
		static Options instance;
		return instance;
	}

	Logger::Controller & Logger::Controller::getInstance() {
		static Controller instance;
		return instance;
	}

	Logger::Controller::Controller() {
#ifndef _WIN32
		::openlog(Quark(Application::Name()).c_str(), LOG_PID, LOG_DAEMON);
#endif // _WIN32
	}

	void Logger::Controller::remove(Buffer *buffer) noexcept {
		lock_guard<std::mutex> lock(guard);
		buffers.remove(buffer);
	}

	Logger::Controller::~Controller() {

		std::vector<Buffer *> buf;

		{
			lock_guard<std::mutex> lock(guard);
			for(auto buffer : this->buffers) {
				buf.push_back(buffer);
			}
		}

		for(auto buffer : buf) {
			delete buffer;
		}

#ifndef _WIN32
		::closelog();
#endif // _WIN32
	}

	Logger::Buffer * Logger::Controller::BufferFactory(Level level) {

		lock_guard<std::mutex> lock(guard);
		for(auto buffer : buffers) {
			if(buffer->level == level && buffer->thread == pthread_self()) {
				return buffer;
			}
		}

		Buffer * bf = new Buffer(pthread_self(),level);
		buffers.push_back(bf);

		return bf;

	}

	Logger::Buffer::~Buffer() {
		Controller::getInstance().remove(this);
	}

	void Logger::enable(Level level, bool enabled) noexcept {
		Logger::Options::getInstance().enabled[level % N_ELEMENTS(Logger::Options::enabled)] = enabled;
	}

	bool Logger::enabled(Level level) noexcept {
		return Logger::Options::getInstance().enabled[level % N_ELEMENTS(Logger::Options::enabled)];
	}

	/// @brief Writes characters to the associated output sequence from the put area.
	int Logger::Writer::overflow(int c) {

		Buffer * buffer = Controller::getInstance().BufferFactory(id);

		if(buffer->push_back(c)) {
			write(*buffer);
			delete buffer;
		}

		return c;

	}

	/// @brief Writes characters to the associated file from the put area
	int Logger::Writer::sync() {
		return 0;
	}

	bool Logger::Buffer::push_back(int c) {

		if(c == EOF || c == '\n' || c == '\r') {
			return true;
		}

		if( ((unsigned char) c) >= ((unsigned char) ' ') || c == '\t') {
			std::string::push_back(c);
		}

		return false;
	}

	Logger::Level Logger::LevelFactory(const XML::Node &node, const char *attr, const char *def) {
		const char *name = node.attribute(attr).as_string(def);
		for(uint8_t ix = 0; ix < (sizeof(typenames)/sizeof(typenames[0])); ix++) {
			if(!strcasecmp(typenames[ix],name)) {
				return (Logger::Level) ix;
			}
		}
		throw system_error(EINVAL,system_category(),"Invalid log level");
	}

	Logger::Level Logger::LevelFactory(const char *name) noexcept {
		for(uint8_t ix = 0; ix < (sizeof(typenames)/sizeof(typenames[0])); ix++) {
			if(!strcasecmp(typenames[ix],name)) {
				return (Logger::Level) ix;
			}
		}
		return Logger::Error;
	}

	void Logger::redirect() {

		static const Level levels[] = { Info,Warning,Error };
		std::ostream *streams[] = {&std::cout, &std::clog, &std::cerr};

		if(Logger::file()) {
			Application::LogDir::getInstance();	// Get log path, mkdir if necessary.
		}

		for(size_t ix = 0; ix < (sizeof(streams)/sizeof(streams[0])); ix++) {

			Writer * writer = dynamic_cast<Writer *>(streams[ix]->rdbuf());
			if(!writer) {
				streams[ix]->rdbuf(new Writer(levels[ix]));
			}

		}

	}

	void Logger::write(const Level level, const char *message) noexcept {

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

		write(level, domain, text);

	}

	void Logger::write(Level level, const char *d, const char *text, bool force) noexcept {

		char domain[15];
		memset(domain,' ',15);
		memcpy(domain,d,std::min(sizeof(domain),strlen(d)));
		domain[14] = 0;

		// Log options.
		Logger::Options &options{Options::getInstance()};

		// Serialize
		static mutex mtx;
		lock_guard<mutex> lock(mtx);

		// Write
		if((options.enabled[level % N_ELEMENTS(options.enabled)] || force)) {

			// Write to console
			options.console(level,domain,text);

#ifndef _WIN32
			if(options.syslog) {
				//
				// Write to syslog.
				//
				static const int priority[] = {
					LOG_ERR,		// Error
					LOG_WARNING,	// Warning
					LOG_INFO,		// Info
					LOG_DEBUG,		// Trace
					LOG_DEBUG,		// Debug
					LOG_NOTICE		// Debug+1
				};

				::syslog(priority[ ((size_t) level) % (sizeof(priority)/sizeof(priority[0])) ],"%s %s",domain,text);
			}
#endif // _WIN32

			if(options.file) {

				try {

					// Write to file
					options.file(level,domain,text);

				} catch(...) {

					// Ignore errors.

				}
			}

		}

	}

	void Logger::write(const Level level, const std::string &message) noexcept {
		write(level,message.c_str());
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

		Logger::write(id,buffer.c_str());

		buffer.erase();

	}

	void Logger::String::write(const Logger::Level level, const char *domain) const {
		Logger::write(level,domain,c_str());
	}

	void Logger::Message::write(const Logger::Level level, const char *domain) const {
		Logger::write(level,domain,c_str());
	}

	UDJAT_API std::ostream & Logger::info() {
		return cout;
	}

	UDJAT_API std::ostream & Logger::warning() {
		return clog;
	}

	UDJAT_API std::ostream & Logger::error() {
		return cerr;
	}

	UDJAT_API std::ostream & Logger::trace() {
		static std::ostream ctrace{new Writer(Logger::Trace)};
		return ctrace;
	}

 }

 namespace std {

	UDJAT_API const char * to_string(const Udjat::Logger::Level level) {
		return typenames[((size_t) level) % N_ELEMENTS(typenames)];
	}

 }
