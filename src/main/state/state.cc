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

/**
 * @file src/core/state/state.cc
 *
 * @brief Implements the abstract state methods
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <cstring>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/expander.h>
 #include <udjat/alert.h>
 #include <udjat/factory.h>
 #include <iostream>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/string.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Abstract::State::State(const char *name, const Level level, const char *summary, const char *body) : Object((name && *name) ? name : "unnamed") {
		if(summary && *summary) {
			Object::properties.summary = summary;
		}

		if(body && *body) {
			properties.body = body;
		}

		this->properties.level = level;
	}

	Abstract::State::State(const pugi::xml_node &node) : Object(node) {
		set(node);
	}

	void Abstract::State::set(const pugi::xml_node &node) {

		Object::set(node);

		const char *section = node.attribute("settings-from").as_string("state-defaults");

		properties.level = LevelFactory(node);
		properties.body = getAttribute(node,section,"body",properties.body);

		for(pugi::xml_node child : node) {

			if(strcasecmp(node.name(),"attribute")) {

				// Parse generic node.
				Factory::for_each(child.name(),[this,&child](const Factory & factory){

					try {

						factory.parse(*this,child);

					} catch(const std::exception &e) {
						factory.error() << "Error '" << e.what() << "' parsing <" << child.name() << ">" << endl;
					} catch(...) {
						factory.error() << "Unexpected error parsing <" << child.name() << ">" << endl;
					}

					return false;

				});

			}

		}

		if(node.attribute("alert").as_bool(false) || node.attribute("alert-type")) {

			// Insert alert using the same node.
			try {

				const char *type = getAttribute(node, section, "alert-type", "default");

				if(strcasecmp(type,"default")) {

					// Not default, use factory.
					Factory::for_each(type,[this,node](const Factory &factory){

						try {

							auto alert = factory.AlertFactory(node);
							if(alert) {
#ifdef DEBUG
								factory.info() << "Using agent from factory" << endl;
#endif // DEBUG
								append(alert);
								return true;
							}
#ifdef DEBUG
							else {
								factory.info() << "EMPTY ALERT!!" << endl;
							}
#endif // DEBUG

						} catch(const std::exception &e) {

							factory.error() << "Error '" << e.what() << "' creating alert" << endl;

						} catch(...) {

							factory.error() << "Unexpected error creating alert" << endl;

						}

						return false;

					});

				} else {

					// Create the default alert.
#ifdef DEBUG
					cout << "alerts\tCreating default alert" << endl;
#endif // DEBUG
					append(make_shared<Udjat::Alert>(node));

				}

			} catch(const std::exception &e) {

				error() << "Error '" << e.what() << "' embedding alert"  << endl;

			}

		}

	}

	Abstract::State::~State() {

	}

	Value & Abstract::State::getProperties(Value &value) const noexcept {

		Object::getProperties(value);
		value["body"] = properties.body;
		value["level"] = std::to_string(properties.level);

		return value;
	}

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	void Abstract::State::get(const Request &request, Response &response) const {
		getProperties(response);
	}
	#pragma GCC diagnostic pop

	void Abstract::State::activate(const Agent &agent) noexcept {

		agent.info() << "State '" << *this << "' was activated" << endl;

		for(auto alert : alerts) {
			Abstract::Alert::activate(alert,[agent,this](std::string &text) {
				text = expand(text.c_str());
				text = agent.expand(text.c_str());
			});
		}

	}

	void Abstract::State::deactivate(const Agent &agent) noexcept {

		agent.info() << "State '" << *this << "' was deactivated" << endl;

		for(auto alert : alerts) {
			alert->deactivate();
		}

	}

	bool Abstract::State::getProperty(const char *key, std::string &value) const noexcept {

		if(Object::getProperty(key,value)) {
			return true;
		}

		if(!strcasecmp(key,"level")) {
			value = std::to_string(properties.level);
			return true;
		}

		if(!strcasecmp(key,"body")) {
			value = properties.body;
			return true;
		}

		return false;

	}

	std::shared_ptr<Abstract::State> StateFactory(const std::exception &except, const char *summary) {

		const std::system_error *syserror = dynamic_cast<const std::system_error *>(&except);

		if(!syserror) {

			// It's not a system error

			/// @brief Exception state.
			class Error : public Abstract::State {
			private:

				// Use string to keep the contents.
				string summary;
				string body;

			public:
				Error(const char *s, const exception &e) : Abstract::State("error",Udjat::critical), summary(s), body(e.what()) {
					Abstract::State::properties.body = this->body.c_str();
					Object::properties.icon = "dialog-error";
					Object::properties.summary = this->summary.c_str();
				}

			};

			return make_shared<Error>(summary,except);

		}

		/// @brief System Error State
		class SysError : public Abstract::State {
		private:

			// Use string to keep the contents.
			string summary;
			string body;
			int code;

		public:
			SysError(const char *s, const system_error *e) : Abstract::State("error",Udjat::critical), summary(s), body(e->what()), code(e->code().value()) {
				Abstract::State::properties.body = this->body.c_str();
				Object::properties.icon = "dialog-error";
				Object::properties.summary = this->summary.c_str();
			}

			Value & getProperties(Value &value) const noexcept override {
				Abstract::State::getProperties(value);
				value["syscode"] = code;
				return value;
			}

		};

		// It's a system error, create state from it.
		// id = code.value()
		// body = code.message();
		return make_shared<SysError>(summary,syserror);

	}

}
