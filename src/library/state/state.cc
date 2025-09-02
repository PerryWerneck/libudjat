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
 #include <udjat/agent/state.h>
 #include <cstring>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/expander.h>
 #include <udjat/alert.h>
 #include <udjat/agent/abstract.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/actions/abstract.h>
 #include <udjat/tools/activatable.h>
 #include <iostream>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/string.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	/// @brief Get Icon name from state level.
	static const char * IconNameFactory(const Level level) {

		static const char * names[] = {
			STRINGIZE_VALUE_OF(PRODUCT_NAME) "-undefined",
			STRINGIZE_VALUE_OF(PRODUCT_NAME) "-unimportant",
			STRINGIZE_VALUE_OF(PRODUCT_NAME) "-ready",
			STRINGIZE_VALUE_OF(PRODUCT_NAME) "-warning",
			STRINGIZE_VALUE_OF(PRODUCT_NAME) "-error",
			STRINGIZE_VALUE_OF(PRODUCT_NAME) "-critical"
		};

		if(((size_t) level) > N_ELEMENTS(names)) {
			return "image-missing";
		}

		return names[level];
	}

	Abstract::State::State(const char *name, const Level level, const char *summary, const char *body) : Object((name && *name) ? name : "unnamed") {

		if(summary && *summary) {
			Object::properties.summary = summary;
		}

		if(body && *body) {
			properties.body = body;
		}

		this->properties.level = level;

		if(!(Object::properties.icon && *Object::properties.icon)) {
			Object::properties.icon = IconNameFactory(properties.level);
		}

	}

	Abstract::State::State(const XML::Node &node) : Object{node} {

		if(!(Object::properties.icon && *Object::properties.icon)) {
			Object::properties.icon = IconNameFactory(properties.level);
		}

		properties.level = LevelFactory(node);
		properties.body = String{node,"body",properties.body}.as_quark();
		options.forward = node.attribute("forward-to-children").as_bool(options.forward);

		if(node.attribute("alert").as_bool(false) || node.attribute("alert-type")) {
			listeners.push_back(Alert::Factory::build(*this, node));
		} else if(node.attribute("action-type")) {
			listeners.push_back(Action::Factory::build(node));
		}

	}

	bool Abstract::State::setup(const XML::Node &node) {

		if(Udjat::Object::setup(node)) {
			return true; // Handled by object.
		}

		if(strcasecmp(node.name(),"alert") == 0) {
			listeners.push_back(Alert::Factory::build(*this,node));
			return true;
		}

		for(const char *nodename : { "action", "script"} ) {
			if(strcasecmp(node.name(),nodename) == 0) {
				listeners.push_back(Action::Factory::build(node));
				return true;
			}	
		}

		return false;
	}

	std::string Abstract::State::value() const {
		return "";
	}

	void Abstract::State::refresh() {
	}

	Abstract::State::~State() {

	}

	const char * Abstract::State::icon() const noexcept {
		const char *icon = Object::icon();
		if((icon && *icon)) {
			return "";
		}

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
			return names[properties.level];
		}

		return "";
	}

	Value & Abstract::State::getProperties(Value &value) const {
		Object::getProperties(value);
		value["body"] = properties.body;
		value["level"] = std::to_string(properties.level);
		return value;
	}

	void Abstract::State::activate(const Abstract::Object &object) noexcept {
		for(auto listener : listeners) {
			Logger::String{"Activating listener '",listener->name(),"' for object '",object.name(),"'"}.trace(name());
			listener->activate(object);
		}
	}

	void Abstract::State::deactivate() noexcept {
		for(auto listener : listeners) {
			Logger::String{"Deactivating listener '",listener->name(),"'"}.trace(name());
			listener->deactivate();
		}
	}

	bool Abstract::State::getProperty(const char *key, std::string &value) const {

		if(Object::getProperty(key,value)) {
			return true;
		}

		if(!strcasecmp(key,"level")) {
			value = std::to_string(properties.level);
			return true;
		}

		if(!strcasecmp(key,"levelnumber")) {
			value = std::to_string((unsigned int) properties.level);
			return true;
		}

		if(!strcasecmp(key,"body")) {
			value = properties.body;
			return true;
		}

		for(const auto &agent : agents) {
			if(agent->getProperty(key,value)) {
				return true;
			}
		}

		return false;

	}

	std::shared_ptr<Abstract::State> Abstract::State::Factory(const char *name, const Udjat::Level level, int code, const char *summary, const char *body) {

		class State : public Abstract::State {
		private:
			int code;
			string summary;
			string body;

		public:
			State(const char *name, const Udjat::Level level, int c, const char *s, const char *b) 
			 : Abstract::State{name,level}, code{c}, summary{s}, body{b} {
				Abstract::State::properties.body = this->body.c_str();
				Object::properties.summary = this->summary.c_str();
			}
		};

		return make_shared<State>(name,level,code,summary,body);

	}
		

	std::shared_ptr<Abstract::State> Abstract::State::Factory(const std::exception &except, const char *summary) {

		const std::system_error *syserror = dynamic_cast<const std::system_error *>(&except);

		if(syserror) {
			return Factory(
				"error",
				Udjat::critical,
				syserror->code().value(),
				summary,
				syserror->what()
			);
		}

		// It's not a system error
		return Factory(
			"error",
			Udjat::critical,
			-1,
			summary,
			except.what()
		);

	}

	std::string State<bool>::value() const {
		return (this->state_value ? _("on") : _("off"));
	}

}
