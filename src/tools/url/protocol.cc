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

 #include "private.h"
 #include <cstring>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <udjat/tools/configuration.h>

 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 static const char * method_names[] = {
	"GET",
	"HEAD",
	"POST",
	"PUT",
	"DELETE",
	"CONNECT",
	"OPTIONS",
	"TRACE",
	"PATCH",
 };

 namespace Udjat {

	HTTP::Method HTTP::MethodFactory(const char *name) {
		for(size_t ix = 0; ix < (sizeof(method_names)/sizeof(method_names[0])); ix++) {
			if(!strcasecmp(name,method_names[ix])) {
				return (Method) ix;
			}
		}
		throw system_error(EINVAL,system_category(),string{"The method '"} + name + "' is invalid");
	}

	Protocol::Protocol(const char *n, const ModuleInfo &i) : name(n), module(i) {
		Controller::getInstance().insert(this);
	}

	Protocol::~Protocol() {
		Controller::getInstance().remove(this);
	}

	std::ostream & Protocol::info() const {
		return cout << name << "\t";
	}

	std::ostream & Protocol::warning() const {
		return clog << name << "\t";
	}

	std::ostream & Protocol::error() const {
		return cerr << name << "\t";
	}

	const Protocol * Protocol::find(const URL &url) {
		string scheme = url.scheme();

		const char *ptr = strrchr(scheme.c_str(),'+');
		if(ptr) {
			scheme.resize(ptr - scheme.c_str());
		}

		return find(scheme.c_str());

	}

	const Protocol * Protocol::find(const char *name) {
		return Controller::getInstance().find(name);
	}

	std::shared_ptr<Protocol::Worker> Protocol::WorkerFactory() const {
		throw system_error(ENOTSUP,system_category(),string{"No worker support on "} + name + " protocol handler");
	}

	void Protocol::getInfo(Udjat::Response &response) noexcept {
		Controller::getInstance().getInfo(response);
	}

	String Protocol::call(const char *url, const HTTP::Method method, const char *payload) {

		const Protocol * protocol = nullptr;
		const char *hostname = strstr(url,"://");
		const char *prefix = strchr(url,'+');

		if(prefix && prefix < hostname) {
			protocol = find(string(url,prefix-url).c_str());
			url = prefix+1;
		} else {
			protocol = find(string(url,hostname-url).c_str());
		}

		if(!protocol) {
			throw system_error(ENOENT,system_category(),"No available protocol worker");
		}

		return protocol->call(URL(url),method,payload);

	}

	String Protocol::call(const URL &url, const HTTP::Method method, const char *payload) const {
		std::shared_ptr<Protocol::Worker> worker = WorkerFactory();
		worker->url(url);
		worker->method(method);
		worker->payload(payload);
		return worker->get();
	}

	String Protocol::call(const URL &url, const char *method, const char *payload) const {
		return call(url,HTTP::MethodFactory(method), payload);
	}

	bool Protocol::get(const URL &url, const char *filename, const std::function<bool(double current, double total)> &progress) const {

		//
		// Get file status.
		//
		struct stat st;

		if(stat(filename,&st) < 0) {

			if(errno != ENOENT) {
				throw system_error(errno,system_category(),Logger::Message("Can't stat '{}'",filename));
			}

			memset(&st,0,sizeof(st));
			st.st_mode = 0644;

		}

		//
		// Get and setup worker.
		//
		std::shared_ptr<Protocol::Worker> worker = WorkerFactory();
		worker->url(url);

		Config::for_each(
			"default-download-headers",
			[worker](const char *key, const char *value) {
				worker->header(key) = value;
				return true;
			}
		);

		if(st.st_mtime) {
			worker->header("If-Modified-Since") = TimeStamp(st.st_mtime);

			Header &hdr = worker->header("Cache-Control");
			if(hdr.empty()) {
				warning() << "No cache-control in the 'default-download-headers' section, using defaults" << endl;
				hdr = "Cache-Control=public, max-age=31536000";
			}
		}

		if(!worker->save(filename,progress)) {
			return false;
		}

		chmod(filename, st.st_mode);

		return true;
	}

 }

 namespace std {

	const char * to_string(const Udjat::HTTP::Method method) {
		if((size_t) method > (sizeof(method_names)/sizeof(method_names[0]))) {
			throw system_error(EINVAL,system_category(),"Invalid method id");
		}
		return method_names[method];
	}

 }

