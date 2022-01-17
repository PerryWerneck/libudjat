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

	}

	Alert::~Alert() {
		deactivate();
	}

	void Alert::activate() {
		Controller::getInstance().activate(this);
	}

	void Alert::deactivate() {
		Controller::getInstance().deactivate(this);
	}

	void Alert::checkForSleep(const char *msg) noexcept {

		time_t rst = (activations.success ? restart.success : restart.failed);

		if(rst) {
			restarting = true;
			activations.next = time(0) + rst;
			clog
				<< name() << "\t"
				<< Logger::Message(
						"Alert cycle {}, sleeping until {}",
							msg,
							TimeStamp(activations.next).to_string()
					)
				<< endl;

		} else {
			activations.next = 0;
			clog
				<< name()
				<< Logger::Message(
					"\tAlert cycle {}, stopping",
							msg
					)
				<< endl;
		}

	}

	void Alert::next() noexcept {

		if(activations.success >= retry.min) {
			checkForSleep("was sucessfull");
		} else if( (activations.success + activations.failed) >= retry.max ) {
			checkForSleep("reached the maximum number of emissions");
		} else {
			activations.next = time(0) + timers.interval;
		}

	}

	bool Alert::emit() noexcept {

		if(running) {
			clog << name() << "\tIs active since " << TimeStamp(running) << endl;
			activations.next = time(0) + timers.interval;
			return true;
		}

		if(restarting) {
			restarting = false;
            cout << name() << "\tRestarting alert cycle" << endl;
			activations.success = activations.failed = 0;
		}

		running = time(0);
        activations.next = running + timers.interval;

        try {

			// Create and activate worker.

#ifdef DEBUG
			activations.success++;
			running = 0;
			next();
#else
			#error Still incomplete.
#endif // DEBUG

        } catch(const std::exception &e) {
			cerr << name() << "\tActivation failed: " << e.what() << endl;
			activations.failed++;
			running = 0;
			next();
        }

		return true;
	}

 }

