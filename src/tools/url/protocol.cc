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

 #include <private/protocol.h>
 #include <cstring>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/logger.h>
 #include <udjat/module/info.h>

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

	HTTP::Method HTTP::MethodFactory(const pugi::xml_node &node, const char *def) {
		return MethodFactory(node.attribute("http-method").as_string(def));
	}

	Protocol::Protocol(const char *n, const ModuleInfo &i) : name(n), module(i) {
		Controller::getInstance().insert(this);
	}

	Protocol::~Protocol() {
		Controller::getInstance().remove(this);
	}

	bool Protocol::for_each(const std::function<bool(const Protocol &protocol)> &method) {
		return Controller::getInstance().for_each(method);
	}

	void Protocol::setDefault() noexcept {
		Controller::getInstance().setDefault(this);
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

	std::ostream & Protocol::trace() const {
		return Logger::trace() << name << "\t";
	}

	const Protocol * Protocol::find(const URL &url, bool allow_default, bool autoload) {
		string scheme = url.scheme();

		const char *ptr = strrchr(scheme.c_str(),'+');
		if(ptr) {
			scheme.resize(ptr - scheme.c_str());
		}

		return find(scheme.c_str(),allow_default,autoload);

	}

	const Protocol * Protocol::find(const char *name, bool allow_default, bool autoload) {
		return Controller::getInstance().find(name,allow_default,autoload);
	}

	const Protocol * Protocol::verify(const void *protocol) {
		return Controller::getInstance().verify(protocol);
	}

	std::shared_ptr<Protocol::Worker> Protocol::WorkerFactory(const char *url) {

		string name{url};

		auto pos = name.find(":");
		if(pos != string::npos) {
			name.resize(pos);
		}

		const Protocol * protocol = Protocol::find(name.c_str());
		if(!protocol) {
			throw runtime_error(string{"Cant find a protocol handler for "} + url);
		}

		auto worker = protocol->WorkerFactory();
		if(!worker) {
			throw runtime_error(string{"Cant create protocol worker for "} + url);
		}

		worker->url(url);

		return worker;
	}

	/// @brief Create a worker for this protocol.
	/// @return Worker for this protocol or empty shared pointer if the protocol cant factory workers.
	std::shared_ptr<Protocol::Worker> Protocol::WorkerFactory() const {
		return std::shared_ptr<Protocol::Worker>();
	}

	Value & Protocol::getProperties(Value &properties) const {
		properties["name"] = name;
		return module.getProperties(properties);
	}

	String Protocol::call(const char *u, const HTTP::Method method, const char *payload) {

		URL url{u};
		const Protocol * protocol = find(url);

		if(!protocol) {
			throw runtime_error(string{"Can't handle '"} + url + "' - no protocol handler");
		}

		return protocol->call(url,method,payload);

	}

	String Protocol::call(const URL &url, const HTTP::Method method, const char *payload) const {
		std::shared_ptr<Protocol::Worker> worker = WorkerFactory();
		if(!worker) {
			throw runtime_error(string{"Cant handle '"} + name + "' protocol");
		}
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
		if(!worker) {
			throw runtime_error(string{"Cant handle '"} + name + "' protocol");
		}

		worker->url(url);

		string settings = string{name} + "-file-headers";

		Config::for_each(
			settings.c_str(),
			[worker](const char *key, const char *value) {
				worker->request(key) = value;
				return true;
			}
		);

		if(st.st_mtime) {
			worker->request("If-Modified-Since") = TimeStamp(st.st_mtime);
			Header &hdr = worker->request("Cache-Control");
			if(hdr.empty()) {
				warning() << "No cache-control in the '" << settings << "' section, using defaults" << endl;
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

