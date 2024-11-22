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

 /**
  * @brief Implements the abstract interface for API Calls.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/intl.h>
 #include <list>

 using namespace std;

 namespace Udjat {

	class Interface::Controller {
	private:
		std::list<Interface *> interfaces;
		static std::mutex guard;

		Controller() {
		}

	public:
		static Controller & getInstance();

		inline Interface & find(const char *name) const {
			std::lock_guard<std::mutex> lock(guard);
			for(Interface *intf : interfaces) {
				if(!strcasecmp(intf->_name,name)) {
					return *intf;
				}
			}
			throw system_error(
					ENOENT,
					system_category(),
					Logger::Message{_("Cant find interface '{}'"),name}
				);		
		}

		inline void push_back(Interface * intf) noexcept {
			std::lock_guard<std::mutex> lock(guard);
			interfaces.push_back(intf);
		}

		inline void remove(Interface * intf) noexcept {
			std::lock_guard<std::mutex> lock(guard);
			interfaces.remove(intf);
		}

		inline const auto begin() {
			return interfaces.begin();
		} 

		inline const auto end() {
			return interfaces.end();
		} 

	};

	std::mutex Interface::Controller::guard;
	Interface::Controller & Interface::Controller::getInstance() {
		std::lock_guard<std::mutex> lock(guard);
		static Controller instance;
		return instance;
	}

	Interface & Interface::find(const char *name) {
		return Controller::getInstance().find(name);		
	}

	Interface::Interface(const char *n) : _name{n} {
		Controller::getInstance().push_back(this);
	}

	Interface::Interface(const XML::Node &node) : Interface{String{node,"name",true}.as_quark()} {
	}

	Interface::~Interface() {
		Controller::getInstance().remove(this);
	}

	void Interface::call(const char *name, const char *path, Udjat::Value &values) {
		find(name).call(path,values);
	}

	void Interface::call(const char *name, Request &request, Response &response) {
		find(name).call(request,response);
	}

	bool Interface::for_each(const std::function<bool(const size_t index, bool input, const char *name, const Value::Type type)> &) const {
		return false;		
	}

	void Interface::call(const char *, Udjat::Value &) {
		throw system_error(ENOTSUP,system_category(),_( "Unable to handle request, no backend"));		
	}

	bool Interface::for_each(const std::function<bool(const Interface &intf)> &call) {
		for(const Interface *intf : Controller::getInstance()) {
			if(call(*intf)) {
				return true;
			}
		}
		return false;
	}

	void Interface::get_inputs(const Abstract::Object &from, Udjat::Value &to) const {

		for_each([&from,&to](const size_t, bool in, const char *name, const Value::Type){
			if(in) {

				// Is an output value, get it.
				if(!from.getProperty(name,to[name])) {
					throw runtime_error(Logger::Message(_("Unable to get value for '{}"),name));
				}
			}
			return false;
		});
	}

	void Interface::call(Request &request, Response &response) {

		try {

			call(request.path(),response);

		} catch(const std::exception &e) {

			response.failed(e);

		}

	}

 }
