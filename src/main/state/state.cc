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

 #include <config.h>
 #include <private/state.h>
 #include <cstring>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/expander.h>
 #include <udjat/alert/abstract.h>
 #include <udjat/alert/activation.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
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

	std::string Abstract::State::value() const {
		return "";
	}

	void Abstract::State::refresh() {
	}

	void Abstract::State::set(const pugi::xml_node &node) {

		Object::set(node);

		const char *section = node.attribute("settings-from").as_string("state-defaults");

		properties.level = LevelFactory(node);
		properties.body = getAttribute(node,section,"body",properties.body);
		options.forward = getAttribute(node,section,"forward-to-children",options.forward);

		for(pugi::xml_node child : node) {

			if(strcasecmp(child.name(),"attribute")) {
				push_back(child);
			}

		}

		debug(
			"name=",node.attribute("name").as_string()," ",
			"Attribute('alert')=",(node.attribute("alert").as_bool(false) ? "Yes" : "No"),
			" Attibute('alert-type')=",getAttribute(node,"alert-type",false).as_string("")
		);

		{
			auto type = getAttribute(node,"alert-type",false);
			auto enabled = getAttribute(node,"alert",true);

			if(enabled.as_bool(type)) {
				auto alert = Udjat::AlertFactory(*this, node, type.as_string(""));
				if(alert) {
					alerts.push_back(alert);
				}
			}

		}

		//	" Attribute('alert-type')=",(node.attribute("alert-type") ? "Yes" : "No"), " ", node.attribute("alert-type").as_string("default")
		// Check for alert.

		/*
		if(node.attribute("alert").as_bool(node.attribute("alert-type"))) {
			auto alert = Udjat::AlertFactory(*this, node, node.attribute("alert-type").as_string(""));
			if(alert) {
				alerts.push_back(alert);
			}
		} else if(node.attribute("alert").as_bool(node.parent().attribute("alert-type"))) {
			auto alert = Udjat::AlertFactory(*this, node, node.parent().attribute("alert-type").as_string(""));
			if(alert) {
				alerts.push_back(alert);
			}
		}
		*/

	}

	Abstract::State::~State() {

	}

	const char * Abstract::State::icon() const noexcept {
		const char *icon = Object::icon();
		if(!(icon && *icon)) {

			// Set icon based on level
			static const char * names[] = {
				"dialog-information",	// undefined,
				"dialog-information",	// unimportant,
				"dialog-information",	// ready,
				"dialog-warning",		// warning,
				"dialog-error",			// error,
				"dialog-error",			// critical
			};

			if( ((size_t) properties.level) < (sizeof(names)/sizeof(names[0]))) {
				icon = names[properties.level];
			}
		}
		return icon;
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

	void Abstract::State::activate() noexcept {

		for(auto alert : alerts) {

			auto activation = alert->ActivationFactory();
			activation->set(*this);
			Udjat::start(activation);

		}

	}

	void Abstract::State::deactivate() noexcept {
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

	std::string State<bool>::value() const {
		return (this->state_value ? _("on") : _("off"));
	}

}
