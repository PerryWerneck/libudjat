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
 #include <udjat/tools/logger.h>
 #include <private/logger.h>
 #include <thread>
 #include <mutex>
 #include <vector>
 #include <iostream>

 using namespace std;

 namespace Udjat {

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

	static thread_local std::vector<Logger::Stream::Buffer> streams;

	Logger::Stream::Buffer & Logger::Stream::Buffer::getInstance(Level level) {

		for(auto &stream : streams) {
			if(stream.level == level) {
				return stream;
			}
		}

		streams.emplace_back(level);
		return streams.back();

	}

	Logger::Stream::Buffer::Buffer(Level l) : level{l} {
	}

	Logger::Stream::Buffer::~Buffer() {
		if(!empty()) {
			sync();
		}
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

	void Logger::Stream::Buffer::sync() {

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

	Logger::Stream::~Stream() {
	}

	int Logger::Stream::sync() {
		Buffer::getInstance(level).sync();
		return 0;
	}

	int Logger::Stream::overflow(int c) {

		auto &buffer = Buffer::getInstance(level);

		if(buffer.push_back(c)) {
			sync();
		}

		return c;

	}

}

