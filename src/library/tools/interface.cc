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

	bool Interface::input_schema(const char *, Schema &) const noexcept {
		return false;
	}

	bool Interface::output_schema(const char *, Schema &) const noexcept {
		return false;
	}

	bool Interface::process(const char *, const Request &request, Response &) const {
		if(!allow(request.role())) {
			Response::Exception error{EPERM,_("You dont have access to this resource")};
			error.title = strerror(EPERM);
			throw error;
		}
		return true;
	}

	bool Interface::process(const char *path, const Request &request, std::ostream &stream) const {

		// Default process: Call API, format response on request mimetype.

		MimeType mimetype = request.mimetype();
		Response response{mimetype};

		if(!process(path,request,response)) {
			debug("Request failed, returning")
			return false;
		}

		if(!request.apicall()) {

			// It's not an api call, can we use a template?
			debug("Request isnt an API call, trying template");

			try {

				Schema schema;
				if(output_schema(schema) && schema.template_name && *schema.template_name) {

					// We have a template name, do we have a template file?
					Template tmplt(schema.template_name,mimetype);
					if(tmplt) {
						tmplt.apply(stream, response);
						return true;
					}
					
				}

			} catch(const std::exception &e) {
				Logger::String{e.what()}.error(name());
			}

		}

		// Format the response.
		debug("Serializing the response using API format");
		response.serialize(stream);
		return true;

	}


 }
