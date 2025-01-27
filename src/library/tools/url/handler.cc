/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/url.h>
 #include <list>
 #include <udjat/tools/container.h>
 #include <sstream>
 #include <udjat/tools/file/handler.h>
 
 using namespace std;

 namespace Udjat {

	Container<URL::Handler::Factory> & factories() {
		static Container<URL::Handler::Factory> factories;
		return factories;
	}

	URL::Handler::Factory::Factory(const char *n) : name{n} {
		factories().push_back(this);
	}

	URL::Handler::Factory::~Factory() {
		factories().remove(this);
	}

	std::shared_ptr<URL::Handler> URL::handler() const {
		auto scheme = this->scheme();
		for(const auto factory : factories()) {
			if(*factory == scheme.c_str()) {
				return factory->HandlerFactory(*this);
			}
		}

		/*
		if(!strcasecmp(scheme.c_str(),"file")) {
		}

		if(!strcasecmp(scheme.c_str(),"script")) {
		}
		*/

		throw invalid_argument(String{"Cant handle ",c_str()});
	}

	URL::Handler::Handler(const URL &url) : url{url} {
	}

	URL::Handler::~Handler() {
	}

	URL::Handler & URL::Handler::set(const MimeType) {
		return *this;
	}

	String URL::Handler::response(const char *name) const {
		return "";
	}

	int URL::Handler::test(const HTTP::Method method, const char *payload) {
		return ENOTSUP;
	}

	bool URL::Handler::get(Udjat::Value &value) {
		return false;
	}

	String URL::Handler::get(const std::function<bool(uint64_t current, uint64_t total)> &progress) {
		stringstream str;
		call(
			HTTP::Get, 
			"", 
			[&str,&progress](uint64_t current, uint64_t total, const char *data, size_t){
				str << data;
				return progress(current,total);
			}
		);
		return String{str.str()};
	}

	bool URL::Handler::get(const char *filename, const std::function<bool(uint64_t current, uint64_t total)> &progress) {
		File::Handler file{filename,true};
		file.truncate();
		call(
			HTTP::Get, 
			"", 
			[&file,&progress](uint64_t current, uint64_t total, const char *data, size_t len){
				file.write(current,data,len);
				return progress(current,total);
			}
		);
		return true;

	}

	String URL::Handler::get() {
		return get([](uint64_t,uint64_t){ return false; });
	}

	bool URL::Handler::get(const char *filename) {
		return get(filename,[](uint64_t,uint64_t){ return false; });
	}

 }

