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

 namespace Udjat {

	Alert::Alert(const pugi::xml_node &node) : settings(node) {

		const char *section = node.attribute("settings-from").as_string("alert-defaults");

		// Seconds to wait before first activation.
		timers.start =
			Attribute(node,"delay-before-start")
				.as_uint(
					Config::Value<uint32_t>(section,"delay-before-start",timers.start)
				);

		// Seconds to wait on every try.
		timers.interval =
			Attribute(node,"delay-before-retry")
				.as_uint(
					Config::Value<uint32_t>(section,"delay-before-retry",timers.interval)
				);

		// How many success emissions after deactivation or sleep?
		limits.min =
			Attribute(node,"min-retries")
				.as_uint(
					Config::Value<uint32_t>(section,"min-retries",limits.min)
				);

		// How many retries (success+fails) after deactivation or sleep?
		limits.max =
			Attribute(node,"max-retries")
				.as_uint(
					Config::Value<uint32_t>(section,"max-retries",limits.max)
				);

		// How many seconds to restart when failed?
		restart.failed =
			Attribute(node,"restart-when-failed")
				.as_uint(
					Config::Value<uint32_t>(section,"delay-when-failed",restart.failed)
				);

		// How many seconds to restart when suceeded?
		restart.success =
			Attribute(node,"restart-when-succeeded")
				.as_uint(
					Config::Value<uint32_t>(section,"restart-when-succeeded",restart.success)
				);

	}

	Alert::~Alert() {
		deactivate();
	}

	void Alert::activate() {
#ifdef DEBUG
		cout << settings.name << "\tActivating alert" << endl << settings.url << endl << settings.payload << endl;
#endif // DEBUG
		Controller::getInstance().activate(this);
	}

	void Alert::deactivate() {
		Controller::getInstance().deactivate(this);
	}

 }

