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
  * @brief Implements the abstract action.
  */

 #define LOG_DOMAIN "action"

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/actions/abstract.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/container.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/script.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/abstract/object.h>
 #include <list>
 #include <sys/stat.h>
 #include <fstream>
 #include <stdexcept>
 #include <private/action.h>

 using namespace std;

 namespace Udjat {

	Action::Action(const XML::Node &node)
		: Activatable{node}, title{String{node,"title"}.as_quark()} {
	}
	
	Action::~Action() {
	}

	int Action::call(bool except) {
		Udjat::Request request;
		Udjat::Response response;
		return call(request,response,except);
	}

	bool Action::activate(const Udjat::Abstract::Object &object) noexcept {

		try {

			Udjat::Request request;
			object.getProperties(request);

			Udjat::Response response;
			int rc = call(request,response,true);
			if(rc) {
				Logger::String{"Action failed with code ",rc}.error(name());
				return false;
			}

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error(name());
			return false;

		} catch(...) {

			Logger::String{"Unexpected error"}.error(name());
			return false;

		}

		return true;

	}

	bool Action::activate() noexcept {

		try {

			Udjat::Request request;
			Udjat::Response response;
			int rc = call(request,response,true);
			if(rc) {
				Logger::String{"Action failed with code ",rc}.error(name());
				return false;
			}

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error(name());
			return false;

		} catch(...) {

			Logger::String{"Unexpected error"}.error(name());
			return false;

		}

		return true;

	}

	void Action::introspect(const std::function<void(const char *name, const Value::Type type, bool in)> &) const {
	}

 }
