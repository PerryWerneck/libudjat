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

 /**
  * @brief Implements the action factory.
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

	void Action::register_factories() {
		Controller::getInstance();
	}

	Action::Factory::Factory(const char *n) : name{n} {
		Controller::getInstance().push_back(this);
	}

	Action::Factory::~Factory() {
		Controller::getInstance().remove(this);
	}

	const std::list<Action::Factory *>::const_iterator Action::Factory::begin() {
		return Controller::getInstance().begin();
	}

	const std::list<Action::Factory *>::const_iterator Action::Factory::end() {
		return Controller::getInstance().end();
	}

	bool Action::Factory::for_each(const std::function<bool(Action::Factory &factory)> &func) noexcept {

		for(auto factory : Controller::getInstance()) {

			try {

				if(func(*factory)) {
					return true;
				}

			} catch(const std::exception &e) {

				Logger::String{e.what()}.error(factory->name);

			} catch(...) {

				Logger::String{"Unexpected error"}.error(factory->name);

			}


		}

		return false;
	}

	std::shared_ptr<Action> Action::Factory::build(const XML::Node &node) {		
		Abstract::Object object;
		return std::dynamic_pointer_cast<Action>(Controller::getInstance().ObjectFactory(object,node));
	}

 }
