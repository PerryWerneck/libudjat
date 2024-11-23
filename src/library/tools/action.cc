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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/action.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/container.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/script.h>
 #include <list>

 using namespace std;

 namespace Udjat {

#ifdef _WIN32
#else
#endif

	static Container<Action::Factory> & Factories() {
		static Container<Action::Factory> instance;
		return instance;
	}

	Action::Factory::Factory(const char *n) : name{n} {
		Factories().push_back(this);
	}

	Action::Factory::~Factory() {
		Factories().remove(this);
	}

	std::shared_ptr<Action> Action::Factory::build(const XML::Node &node) {

		const char *type = node.attribute("type").as_string("shell");

		for(const auto factory : Factories()) {
			if(strcasecmp(type,factory->name)) {
				continue;
			}
			auto action = factory->ActionFactory(node);
			if(action) {
				return action;
			}
		}

		//
		// Build internal actions
		//
		if(strcasecmp(type,"shell") == 0 || strcasecmp(type,"script") == 0) {
			return make_shared<Script>(node);
		}

		//
		// Cant find factory, return empty action.
		//
		Logger::String{"Cant find a valid factory for action type '",type,"'"}.trace(node.attribute("name").as_string(PACKAGE_NAME));
		return std::shared_ptr<Action>(); // Legacy

	}

	Action::Action(const XML::Node &node)
		: NamedObject{node}, title{String{node,"title"}.as_quark()} {
	}
	
	Action::~Action() {
	}

	/// @brief Execute action.
	/// @param request The client request.
	/// @param response The response to client.
	void Action::call(Udjat::Request &, Udjat::Response &response) {
		call(response);
	}

 }
