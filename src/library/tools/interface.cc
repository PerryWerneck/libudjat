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
 #include <udjat/tools/schema.h>
 #include <udjat/tools/template.h>
 #include <udjat/tools/logger.h>
 
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
		return false;
	}

	HTTP::StatusCode Interface::process(const char *path, const Request &request, std::ostream &stream) const noexcept {

		// Default process: Call API, format response on requested mimetype.

		debug("Processing path '",path,"' at interface '",name(),"'");
		
		MimeType mimetype = request.mimetype();
		Response response{mimetype};

		try {

			if(!allow(request.role())) {

				debug("Invalid authentitcation");
				response.failed(HTTP::Forbidden);

			} else if(!process(path,request,response)) {

				debug("Request failed, returning")
				response.failed(HTTP::NotFound);

			}

		} catch(const std::exception &e) {

			response.failed(e);

		} catch(...) {

			response.failed(_("Unexpected error processing request"));

		}

		if(!request.apicall()) {

			// It's not an apicall, try to use templates.
			debug("Incomplete");

		} else {

			response.serialize(stream);

		}

		return response.status_code();
	}


 }
