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

	Alert::Alert(const pugi::xml_node &node) : Alert(Attribute(node,"name",false).as_string("alert")) {

		// Get configuration file section.
		string section = getConfigSection(node);

#ifdef DEBUG
		cout << "Loading configuration from '" << section << "'" << endl;
#endif // DEBUG

		// max-retries
		retry.limit =
			Attribute(node,"max-retries")
				.as_uint(
					Config::Value<uint32_t>(section.c_str(),"max-retries",retry.limit)
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

		// delay-when-failed
		retry.restart =
			Attribute(node,"delay-when-failed")
				.as_uint(
					Config::Value<uint32_t>(section.c_str(),"delay-when-failed",retry.restart)
				);

		// disable-when-failed
		disable_when_failed =
			Attribute(node,"disable-when-failed")
				.as_bool(
					Config::Value<bool>(section.c_str(),"disable-when-failed",disable_when_failed)
				);

		// reset-when-activated
		reset_when_activated =
			Attribute(node,"reset-when-activated")
				.as_bool(
					Config::Value<bool>(section.c_str(),"reset-when-activated",reset_when_activated)
				);

	}

	Alert::~Alert() {
	}

	std::string Alert::getConfigSection(const pugi::xml_node &node) {

		// First check for configuration attribute
		auto attribute = Attribute(node,"configuration-from");
		if(attribute) {
			return attribute.as_string("alert-default");
		}

		// Then for the node name in the format alert-${type}
		const char *name = node.name();
		if(!strncasecmp(name,"alert-",6) && name[7]) {
			return name;
		}

		// And, last, for the alert type.
		return string{"alert-"} + Controller::getType(node);

	}

	void Alert::set(std::shared_ptr<Alert> alert, const Abstract::Agent &agent, bool level_has_changed) {
		if(alert->activate_on_value_change || level_has_changed) {
			set(alert,agent,*agent.getState(),true);
		}
	}

	void Alert::set(std::shared_ptr<Alert> alert, const Abstract::Agent &agent, const Abstract::State &state, bool active) {

		if(active) {

			Controller::getInstance().activate(alert,agent,state);

		} else {

			Controller::getInstance().deactivate(alert);

		}
	}

 }
