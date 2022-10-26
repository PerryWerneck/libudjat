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
 #include <private/logger.h>
 #include <udjat/tools/application.h>
 #include <private/logger.h>
 #include <udjat/tools/quark.h>
 #include <cstring>
 #include <list>
 #include <ostream>

 #ifdef _WIN32
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
 #else
	#include <unistd.h>
	#include <syslog.h>
 #endif // _WIN32

 using namespace std;

 namespace Udjat {

	static const char * typenames[] = {
		"info",
		"warning",
		"error"
	};

	void Logger::console(bool enable) {
		Options::getInstance().console = enable;
	}

	bool Logger::console() {
		return Options::getInstance().console;
	}

	void Logger::file(bool enable) {
		Options::getInstance().file = enable;
	}

	bool Logger::file() {
		return Options::getInstance().file;
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

	Logger::Controller::~Controller() {
		while(!buffers.empty()) {
			Buffer * buffer = buffers.back();
			buffers.pop_back();
			delete buffer;
		}
#ifndef _WIN32
		::closelog();
#endif // _WIN32
	}

	Logger::Buffer * Logger::Controller::BufferFactory(Level level) {

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
		Controller::getInstance().buffers.remove(this);
	}

	void Logger::enable(Level level) noexcept {
		Logger::Options::getInstance().enabled[level % N_ELEMENTS(Logger::Options::enabled)] = level;
	}

	/// @brief Writes characters to the associated output sequence from the put area.
	int Logger::Writer::overflow(int c) {

		// lock_guard<std::mutex> lock(guard);
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


	Logger::Level Logger::LevelFactory(const char *name) noexcept {
		for(uint8_t ix = 0; ix < (sizeof(typenames)/sizeof(typenames[0])); ix++) {
			if(!strcasecmp(typenames[ix],name)) {
				return (Logger::Level) ix;
			}
		}
		return Logger::Error;
	}

	//
	// Log messages.
	//
	Logger::Message & Logger::Message::append(const char *value) {

		size_t from = find("{");
		if(from == std::string::npos) {
			throw std::runtime_error("Invalid template string");
		}

		size_t to = find("}",from);
		if(to == std::string::npos) {
			throw std::runtime_error("Invalid template string");
		}

		replace(from,(to-from)+1,value);

		return *this;
	}

	Logger::Message & Logger::Message::append(const std::exception &e) {
		return append(e.what());
	}

	void Logger::redirect(bool console, bool file) {

		static const Level levels[] = { Info,Warning,Error };
		std::ostream *streams[] = {&std::cout, &std::clog, &std::cerr};

		Options &options{Options::getInstance()};
		options.console = console;
		options.file = file;

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


	void Logger::String::write(const Logger::Level level) const {
		Logger::write(level,c_str());
	}

	void Logger::String::write(const Logger::Level level, const char *domain) const {
		Logger::write(level,domain,c_str());
	}

	void Logger::Message::write(const Logger::Level level) const {
		Logger::write(level,c_str());
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
