/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/interface.h>
 #include <udjat/tools/container.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/response.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/properties.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/schema.h>
 #include <udjat/tools/schema.h>
 #include <udjat/tools/template.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/template.h>
 #include <udjat/tools/configuration.h>
 
 using namespace std;

 namespace Udjat {

	static Container<Interface> & Interfaces() {

		class Controller : public Container<Interface>, public Properties::ObjectBuilder {
		public:
			Controller() : Properties::ObjectBuilder{"interface"} {
			}

			~Controller() {
			}

			bool build(const Properties &props) override {
				throw runtime_error("No support for custom interfaces (yet)");
			}

		};

		static Controller instance;
		return instance;
	}

	Interface * Interface::find(const char * &path) noexcept {
		for(const auto interface : Interfaces()) {
			if(Request::pop(interface->name(),path)) {
				return interface;
			}
		}
		return nullptr;
	}

	Interface * Interface::find(Request &request) noexcept {
		for(const auto interface : Interfaces()) {
			if(request.pop(interface->name())) {
				return interface;
			}
		}
		return nullptr;
	}

	Interface::Interface(const char *name, const Authentication::Role r)
		: interface_name{name}, role{r} {
		Interfaces().push_back(this);
	}
		
	Interface::~Interface() {
		Interfaces().remove(this);
	}

	bool Interface::for_each(const std::function<bool(const Interface &interface)> &func) {
		for(const auto &interface : Interfaces()) {
			if(func(*interface)) {
				return true;
			}
		}
		return false;
	}

	bool Interface::schema(const char *, HTTPSchema &schema) const noexcept {
		schema.add({ HTTP::Get, Authentication::None });
		return true;
	}

	bool Interface::schema(const char *, InputSchema &) const noexcept {
		return false;
	}

	bool Interface::schema(const char *, OutputSchema &) const noexcept {
		return false;
	}

	bool Interface::process(const Request &, Response &response) const {
		Logger::String{"Unable to process requests, the method 'process' was not overrided by interface code"}.error(name());
		response.failed(HTTP::NotFound);
		return true;
	}

	bool Interface::allow(const char *path, const Request &request, Response &response) const noexcept {

		auto role = request.role();

		// Check the interface default role.
		if(!allow(role)) {
			request.info(name(),strerror(EPERM));
			response.failed(HTTP::Forbidden);
			return false;
		}

		// Check the HTTP actions & roles.
		HTTPSchema scm;
		if(schema(path,scm)) {
			bool rc = false;
			HTTP::Method method = request.method();
			for(const auto &item : scm) {
				if(item.method() == method && item.role() <= role) {
					rc = true;
					break;
				}
			}
			if(!rc) {
				request.info(name(),"Rejected by method rules");
				response.failed(HTTP::MethodNotAllowed);
				return false;
			}
		}

		// Allowed.
		if(Logger::enabled(Logger::Debug)) {
			request.info(name(),"Accepted");
		}


		return true;
	}

	bool Interface::process(const Request &request, HTTP::Status &status, std::ostream &stream) const noexcept {

		/// @brief Adapter.
		class Adapter : public Udjat::Response {
		public:
			Adapter(HTTP::Status &status) : Udjat::Response{status.mimetype} {
				this->status = status;
			}

			~Adapter() override {
			}

		};

		try {

			Adapter response{status};
			if(process(request,response)) {

				// copy response;
				status = (HTTP::Status) response;

				if(status == HTTP::Ok) {

					OutputSchema schema;
					if(request.apicall() || status.mimetype != MimeType::html || !this->schema(request.path(),schema)) {

						// It's an API call, dont have schema or not an html request, just serialize.
						response.serialize(stream);

					} else if(schema.template_name) {

						// Have template, use it.
						Template{schema.template_name}.apply(stream,response);
		
					} else {

						// FIX-ME: Has schema but no template, serialize using schema.
						response.serialize(stream);

					}

				}

			} else {

				// Rejected, return 'not found'
				status = HTTP::NotFound;

			}

		} catch(const std::exception &e) {
			status.assign(e);
		}


		return true;
	}

 }
