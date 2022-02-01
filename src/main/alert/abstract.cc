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
 #include <udjat/tools/threadpool.h>

 namespace Udjat {

	void Abstract::Alert::activate(std::shared_ptr<Alert> alert,const std::function<void(std::string &str)> &expander) {
		Controller::getInstance().activate(alert,expander);
	}

	void Abstract::Alert::activate(std::shared_ptr<Alert> alert) {
		activate(alert,[](std::string UDJAT_UNUSED(&text)){});
	}

	Abstract::Alert::Alert(const pugi::xml_node &node,const char *defaults) : Alert(Quark(node,"name","alert",false).c_str()) {

		// initialize();

		const char *section = node.attribute("settings-from").as_string(defaults);

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

		// Seconds to wait if busy.
		timers.busy =
			Attribute(node,"delay-when-busy")
				.as_uint(
					Config::Value<uint32_t>(section,"delay-when-busy",timers.busy)
				);

		// How many success emissions after deactivation or sleep?
		retry.min =
			Attribute(node,"min-retries")
				.as_uint(
					Config::Value<uint32_t>(section,"min-retries",retry.min)
				);

		// How many retries (success+fails) after deactivation or sleep?
		retry.max =
			Attribute(node,"max-retries")
				.as_uint(
					Config::Value<uint32_t>(section,"max-retries",retry.max)
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

#ifdef DEBUG
		cout << c_str() << "\tAlert created" << endl;
#endif // DEBUG

	}

	Abstract::Alert::~Alert() {
	}

	void Abstract::Alert::deactivate() {
		Controller::getInstance().remove(this);
	}

	void Abstract::Alert::get(Udjat::Value &response) const noexcept {

		response["name"] = name;
		response["minretry"] = retry.min;
		response["maxretry"] = retry.max;
		response["startdelay"] = timers.start;
		response["busydelay"] = timers.busy;
		response["interval"] = timers.interval;
		response["faildelay"] = restart.failed;
		response["delay"] = restart.success;

	}

 }

