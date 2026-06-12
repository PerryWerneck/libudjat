/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/defs.h>
 #include <private/logger.h>
 #include <thread>
 #include <mutex>
 #include <list>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	mutex Logger::Stream::guard;
	std::list<Logger::Stream::Buffer> Logger::Stream::buffers;

	UDJAT_API std::ostream & Logger::trace() {
		static thread_local std::ostream stream{new Logger::Stream(Logger::Trace)};
		return stream;
	}

	UDJAT_API std::ostream & Logger::error() {
		static thread_local std::ostream stream{new Logger::Stream(Logger::Error)};
		return stream;
	}

	UDJAT_API std::ostream & Logger::warning() {
		static thread_local std::ostream stream{new Logger::Stream(Logger::Warning)};
		return stream;
	}

	UDJAT_API std::ostream & Logger::info() {
		static thread_local std::ostream stream{new Logger::Stream(Logger::Info)};
		return stream;
	}

	Logger::Stream::Buffer::Buffer(Level l) : level{l}, thread{pthread_self()} {
	}

	Logger::Stream::Buffer::~Buffer() {
	}

	bool Logger::Stream::Buffer::push_back(int c) {

		if(c == EOF || c == '\n' || c == '\r') {
			return true;
		}

		if( ((unsigned char) c) >= ((unsigned char) ' ') || c == '\t') {
			std::string::push_back(c);
		}

		return false;

	}

	void Logger::Stream::Buffer::send() {

		strip();
		if(empty()) {
			return;
		}

		const char *domain = c_str();
		const char *text = strchr(domain,'\t');

		if(text) {
			* ((char *) text) = 0;
			text++;
		} else {
			domain = "";
			text = c_str();
		}

		Controller::getInstance().write(level,domain,text);
		clear();

	}

	Logger::Stream::Buffer & Logger::Stream::getBuffer(Level level) {

		lock_guard<mutex> lock{guard};

		pthread_t thread = pthread_self();

		for(auto &buffer : buffers) {
			if(buffer.level == level && buffer.thread == thread) {
				return buffer;
			}
		}

		buffers.emplace_back(level);
		return buffers.back();

	}

	Logger::Stream::~Stream() {
	}

	void Logger::Stream::send() {

		lock_guard<mutex> lock{guard};

		pthread_t thread = pthread_self();

		for(auto it = buffers.begin(); it != buffers.end(); it++) {
			if(it->level == level && it->thread == thread) {
				it->send();
				buffers.erase(it);
				break;
			}
		}
	
	}

	int Logger::Stream::sync() {
		return 0;
	}

	int Logger::Stream::overflow(int c) {
		if(getBuffer(level).push_back(c)) {
			send();
		}
		return c;
	}

	void Logger::redirect() {

		static const Level levels[] = { Info, Warning, Error };		
		std::ostream *streams[] = { &std::cout, &std::clog, &std::cerr };

		for(size_t ix = 0; ix < (sizeof(streams)/sizeof(streams[0])); ix++) {

			auto *stream = dynamic_cast<Stream *>(streams[ix]->rdbuf());
			if(!stream) {
				streams[ix]->rdbuf(new Stream(levels[ix]));
			}

		}

	}

}

