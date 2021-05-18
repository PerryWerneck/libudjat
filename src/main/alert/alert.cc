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

 #include "private.h"
 #include <udjat/tools/xml.h>
 #include <udjat/tools/configuration.h>
 #include <pugixml.hpp>
 #include <string>

 namespace Udjat {

	Alert::Alert(const Quark &name) : Logger(name) {
	}

	Alert::Alert(const char *name) : Alert(Quark(name)) {
	}

	Alert::Alert(const pugi::xml_node &node, const char *type) : Alert(Attribute(node,"name",false).as_string("alert")) {

		// Get configuration file section.
		string section = getConfigSection(node,type);

#ifdef DEBUG
		cout << "Loading configuration from '" << section << "'" << endl;
#endif // DEBUG

		// min-retries
		retry.min =
			Attribute(node,"min-retries")
				.as_uint(
					Config::Value<uint32_t>(section.c_str(),"min-retries",retry.min)
				);

		// max-retries
		retry.max =
			Attribute(node,"max-retries")
				.as_uint(
					Config::Value<uint32_t>(section.c_str(),"max-retries",retry.max)
				);

		// activate-on-value-change
		activate_on_value_change =
			Attribute(node,"activate-on-value-change")
				.as_bool(
					Config::Value<bool>(section.c_str(),"activate-on-value-change",activate_on_value_change)
				);

		// delay-before-start
		retry.start =
			Attribute(node,"delay-before-start")
				.as_uint(
					Config::Value<uint32_t>(section.c_str(),"delay-before-start",retry.start)
				);

		// delay-before-retry
		retry.interval =
			Attribute(node,"delay-before-retry")
				.as_uint(
					Config::Value<uint32_t>(section.c_str(),"delay-before-retry",retry.interval)
				);

		// How many seconds to restart when failed?
		retry.restart.failed =
			Attribute(node,"restart-when-failed")
				.as_uint(
					Config::Value<uint32_t>(section.c_str(),"delay-when-failed",retry.restart.failed)
				);


		// How many seconds to restart when suceeded?
		retry.restart.success =
			Attribute(node,"restart-when-succeeded")
				.as_uint(
					Config::Value<uint32_t>(section.c_str(),"restart-when-succeeded",retry.restart.success)
				);


	}

	Alert::~Alert() {
		Controller::getInstance().remove(this);
	}

	void Alert::getInfo(Response &response) {
		Controller::getInstance().getInfo(response);
	}

	std::string Alert::getConfigSection(const pugi::xml_node &node,const char *type) {

		// First check for configuration attribute
		auto attribute = Attribute(node,"configuration-from");
		if(attribute) {
			return attribute.as_string("alert-default");
		}

		// Then type (if available)
		if(type) {
			string section = string{"alert-"} + type;
			if(Config::hasGroup(section))
				return section;
		}

		// Then for the node name in the format alert-${type}
		const char *name = node.name();
		if(!strncasecmp(name,"alert-",6) && name[7]) {
			if(Config::hasGroup(name))
				return name;
		}

		// Check for type
		{
			string section = string{"alert-"} + Controller::getType(node);
			if(Config::hasGroup(section))
				return section;
		}

		return "alert-default";

	}

	void Alert::activate(std::shared_ptr<Alert::Event> event) {
		Controller::getInstance().insert(this,event);
	}

	void Alert::deactivate() const {
		Controller::getInstance().remove(this);
	}

	/*
	void Alert::insert(Event *event) {
		if(event->parent) {
			event->parent->warning("Moving event to alert '{}'",getName());
		}
		event->parent = this;
	}
	*/

	/*
	void Alert::remove(Event *event) {
		if(event->parent == this) {
			event->parent = nullptr;
			events--;
		}
	}
	*/

	void Alert::set(const Abstract::Agent &agent, bool level_has_changed) {
		if(activate_on_value_change || level_has_changed) {
			deactivate();
			activate(agent,*agent.getState());
		}
	}

	void Alert::set(const Abstract::Agent &agent, const Abstract::State &state, bool active) {

		deactivate();

		if(active) {
			activate(agent,state);
		}

	}

 }
