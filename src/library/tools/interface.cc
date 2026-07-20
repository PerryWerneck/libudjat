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

	bool Interface::process(const Request &request, Response &response) const {
		debug(__FUNCTION__);
		return process(request.path(),request,response);
	}

	bool Interface::process(const Request &request, std::ostream &stream) const {
		debug(__FUNCTION__);
		return process(request.path(),request,stream);
	}

	bool Interface::process(const char *, const Request &, Response &response) const {
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

	HTTP::StatusCode Interface::process(const char *path, const Request &request, std::ostream &stream) const noexcept {

		// Default process: Call API, format response on requested mimetype.

		debug("Processing path '",path,"' at interface '",name(),"'");
		
		MimeType mimetype = request.mimetype();
		Response response{mimetype};

		try {

			if(allow(path,request,response)) {

				// Request was allowed.
				if(!process(path,request,response)) {
					// Request was not processed.
					response.failed(HTTP::NotFound);
				}

			}

		} catch(const std::exception &e) {

			response.failed(e);

		} catch(...) {

			response.failed(_("Unexpected error processing request"));

		}

		if(!request.apicall()) {

			// It's not an apicall, try to use templates.
			OutputSchema schema;
			this->schema(schema);

			return main_page(schema, response, stream);

		} else {

			response.serialize(stream);

		}

		return response.status_code();
	}

	HTTP::StatusCode Interface::main_page(const OutputSchema &schema, Response &response, std::ostream &stream) const noexcept {
	
		Template main_page{"main",(MimeType) response};

		if(!main_page) {
			Logger::String{"Main page template is not available."}.error();
			response.failed(
				_("A required file is unavailable within the selected theme. Please contact the system administrator for assistance.")
			);
			response.serialize(stream);
			return response.status_code();
		}

		try {

			main_page.apply(stream,[&schema,&response](const char *key, std::ostream &stream){

				if(!strcasecmp(key,"page-summary")) {

					// Page-summary is unsupported (for now).
					return true;

				}

				MimeType mimetype = (MimeType) response;

				if(!strcasecmp(key,"page-contents")) {

					// Parse page contents.
					if(response.status_code() != HTTP::Ok) {

						// Use error template.
						Template inner_page{"dialog-error",mimetype};
						if(!inner_page) {
							Logger::String{"Template 'dialog-error is missing'"}.warning();
							response.serialize(stream);
							return true;
						}

						inner_page.apply(stream, (HTTP::Status) response);

					} else if(schema.template_name && *schema.template_name) {

						// Use template from schema
						Template inner_page{schema.template_name,mimetype};
						if(!inner_page) {
							Logger::String{"Template '",schema.template_name, "' is missing"}.warning();
							response.serialize(stream);
							return true;
						}

						inner_page.apply(stream, response);

					} else {

						// No template, just serialize.
						response.serialize(stream);
					}

					return true;

				}

				return false;
			});

		} catch(const std::exception &e) {

			Logger::String{e.what()};
			return HTTP::SystemError;

		}


		return response.status_code();
	}

 }
