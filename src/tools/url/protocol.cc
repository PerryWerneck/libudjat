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
 #include <udjat/tools/worker.h>
 #include <udjat/module/info.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/response/value.h>
 #include <udjat/tools/response/table.h>
 #include <udjat/tools/response/object.h>

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

	HTTP::Method HTTP::MethodFactory(const XML::Node &node, const char *attrname, const char *def) {
		return MethodFactory(String{node,attrname,def}.c_str());
	}

	HTTP::Method HTTP::MethodFactory(const XML::Node &node, const char *def) {
		return MethodFactory(node,"http-method",def);
	}

	HTTP::Method HTTP::MethodFactory(const XML::Node &node) {
		return MethodFactory(node,"http-method","get");
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

	/*
	const Protocol * Protocol::find(const URL &url, bool allow_default, bool autoload) {
		string scheme = url.scheme();

		const char *ptr = strrchr(scheme.c_str(),'+');
		if(ptr) {
			scheme.resize(ptr - scheme.c_str());
		}

		return find(scheme.c_str(),allow_default,autoload);

	}
	*/

	/*
	const Protocol * Protocol::find(const char *name, bool allow_default, bool autoload) {
		return Controller::getInstance().find(name,allow_default,autoload);
	}
	*/

	const Protocol * Protocol::verify(const void *protocol) {
		return Controller::getInstance().verify(protocol);
	}

	std::shared_ptr<Protocol::Worker> Protocol::WorkerFactory(const char *url, bool allow_default, bool autoload) {

		string name{url};

		auto pos = name.find(":");
		if(pos != string::npos) {
			name.resize(pos);
		}

		debug("Searching for protocol '",name.c_str(),"'");

		// 1 - check for registered protocol.
		{
			const Protocol * protocol = nullptr;

			Protocol::for_each([&protocol,name](const Protocol &p){
				if(p == name.c_str()) {
					protocol = &p;
					return true;
				}
				return false;
			});

			if(protocol) {

				auto worker = protocol->WorkerFactory();
				if(!worker) {
					throw runtime_error(String{"Cant create protocol worker for ",url});
				}

				worker->url(url);
				return worker;
			}
		}

		// 2 - Check for internal protocols.
		if(!strcasecmp(name.c_str(),"file")) {

			auto worker = Protocol::FileHandlerFactory().WorkerFactory();
			if(!worker) {
				throw runtime_error(String{"Cant create file worker for ",url});
			}

			worker->url(url);
			return worker;

		}

		if(!strcasecmp(name.c_str(),"script")) {

			auto worker = Protocol::ScriptHandlerFactory().WorkerFactory();
			if(!worker) {
				throw runtime_error(String{"Cant create script worker for ",url});
			}

			worker->url(url);
			return worker;

		}

		// 3 - check for worker.
		{
			const Udjat::Worker *worker = nullptr;

			Udjat::Worker::for_each([&worker,name](const Udjat::Worker &w){

				cout << "---> " << w.c_str() << " - " <<name.c_str() << endl;

				if(w == name.c_str()) {
					worker = &w;
					return true;
				}
				return false;

			});

			if(worker) {

				// Got worker for url, check if it can be used.

				/// @brief Proxy forwarding URL requests to worker.
				class Proxy : public Protocol::Worker {
				private:
					const Udjat::Worker *worker;
					Udjat::Request request;
					Udjat::Worker::ResponseType type;
					MimeType mime = MimeType::json;

				public:
					Proxy(const Udjat::Worker *w, const char *url, const char *path)
						: worker{w}, request{path ? path+3 : ""}, type{worker->probe(request)} {
						this->url(url);
						if(type == Udjat::Worker::None) {
							throw runtime_error(Logger::String{"Cant handle ",url});
						}
					}

					int mimetype(const MimeType type) override {
						mime = type;
						return 0;
					}

					String get(const std::function<bool(double current, double total)> &progress) override {

						progress(0,0);

						String str;
						if(type == Udjat::Worker::Table) {

							throw system_error(ENOTSUP,system_category(),"Cant handle worker response type (yet)");

						} else {

							Response::Object response;
							if(worker->get(request,response)) {
								str = response.to_string();
							}

						}

						progress(str.size(),str.size());
						return str;
					}


				};

				debug("--------------------------------[",url,"]");
				auto proxy = make_shared<Proxy>(worker,url,strstr(url,":///"));
				return proxy;

			}

		}

		// 4 - Check for available module
		// TODO: Check if there's a module with protocol name, load it if necessary.

		// 5 - Use the default protocol handler
		if(allow_default) {

			const Protocol *def = Controller::getInstance().getDefault();

			if(def) {
				auto worker = def->WorkerFactory();
				if(!worker) {
					throw runtime_error(String{"Cant get default worker for ",url});
				}

				worker->url(url);
				return worker;
			}

		}

		throw runtime_error(String{"Cant find a protocol handler for ",url});

	}

	/// @brief Create a worker for this protocol.
	std::shared_ptr<Protocol::Worker> Protocol::WorkerFactory() const {

		//
		// This protocol is unable to create a worker, use a proxy to the 'old' API.
		//
		warning() << "No worker factory (old version?) using proxy worker" << endl;

		class Proxy : public Protocol::Worker {
		private:
			const Protocol *protocol;

		public:
			Proxy(const Protocol *p) : protocol{p} {
			}

			virtual ~Proxy() {
			}

			String get(const std::function<bool(double current, double total)> UDJAT_UNUSED(&progress)) override {
				if(!verify(protocol)) {
					throw runtime_error("Protocol is no longer available");
				}
				return protocol->call(url().c_str(),method(),out.payload.c_str());
			}

			bool save(const char UDJAT_UNUSED(*filename), const std::function<bool(double current, double total)> UDJAT_UNUSED(&progress), bool UDJAT_UNUSED(replace)) override {
				throw runtime_error("The selected protocol is unable to save files");
			}

		};

		return make_shared<Proxy>(this);

	}

	Value & Protocol::getProperties(Value &properties) const {
		properties["name"] = name;
		return module.getProperties(properties);
	}

	String Protocol::call(const char *url, const HTTP::Method method, const char *payload) {

		auto worker = Protocol::WorkerFactory(url);

		worker->url(url);
		worker->method(method);
		worker->payload(payload);
		return worker->get();

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

	bool Protocol::call(const URL &, Udjat::Value &, const HTTP::Method, const char *) const {
		return false;
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

