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
 #include <udjat/tools/application.h>
 #include <private/logger.h>
 #include <udjat/tools/quark.h>
 #include <cstring>
 #include <list>

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

	std::mutex Logger::guard;
	Logger::Level Logger::level = Logger::Error;

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

	/// @brief Writes characters to the associated output sequence from the put area.
	int Logger::Writer::overflow(int c) {

		lock_guard<std::mutex> lock(guard);
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

	void Logger::set(const pugi::xml_node &node) {
		auto attribute = node.attribute("name");
		if(attribute)
			properties.name = Quark(attribute.as_string(properties.name)).c_str();
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

	void Logger::redirect(bool console) {

		static const Level levels[] = { Info,Warning,Error };
		std::ostream *streams[] = {&std::cout, &std::clog, &std::cerr};

		for(size_t ix = 0; ix < (sizeof(streams)/sizeof(streams[0])); ix++) {

			Writer * writer = dynamic_cast<Writer *>(streams[ix]->rdbuf());
			if(writer) {
				trace("Stream(",ix,") was already redirected");
				writer->set_console(console);
			} else {
				streams[ix]->rdbuf(new Writer(levels[ix],console));
			}

		}

		/*
		std::cout.rdbuf(new Writer(Logger::Info,console));
		std::clog.rdbuf(new Writer(Logger::Warning,console));
		std::cerr.rdbuf(new Writer(Logger::Error,console));
		*/

	}

	void Logger::console(bool enable) {

		std::ostream *streams[] = {&std::cout, &std::clog, &std::cerr};

		for(size_t ix = 0; ix < (sizeof(streams)/sizeof(streams[0])); ix++) {

			Writer * writer = dynamic_cast<Writer *>(streams[ix]->rdbuf());
			if(writer) {
				writer->console = enable;
			}

		}

	}

	void Logger::write(const Logger::Level level, const char *message) noexcept {
		Controller::getInstance().write(level,true,message);
	}

	void Logger::write(const Logger::Level level, const std::string &message) noexcept {
		write(level,message.c_str());
	}

 }
