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

	void Alert::initialize() {
		Controller::getInstance();
	}

	Alert::Alert(const pugi::xml_node &node) : settings(node) {

		initialize();

		const char *section = node.attribute("settings-from").as_string("alert-defaults");

		// Get alert worker
		string worker_name = Config::Value<string>(section,"engine","default");
		worker =
			Controller::getInstance().getWorker(
				node.attribute("engine")
					.as_string(
						Attribute(node,"alert-engine")
							.as_string(
								worker_name.c_str()
							)
					)
				);

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
		cout << settings.name << "\tAlert created" << endl;
#endif // DEBUG

	}

	Alert::~Alert() {
	}

	void Alert::activate(std::shared_ptr<Alert> alert) {
		Controller::getInstance().insert(make_shared<Alert::Activation>(alert));
	}

	void Alert::deactivate(std::shared_ptr<Alert> alert) {
		Controller::getInstance().remove(alert);
	}

	void Alert::activate(std::shared_ptr<Alert> alert, const Abstract::Agent &agent) {
		throw runtime_error("Not implemented");
	}

	void Alert::activate(std::shared_ptr<Alert> alert, const Abstract::Agent &agent, const Abstract::State &state) {
		throw runtime_error("Not implemented");
	}

	/*
	void Alert::emit(const Alert::PrivateData &data) noexcept {

		if(running) {
			clog << name() << "\tIs active since " << TimeStamp(running) << endl;
			activations.next = time(0) + timers.busy;
		}

		if(restarting) {
			restarting = false;
            cout << name() << "\tRestarting alert cycle" << endl;
			activations.success = activations.failed = 0;
		}

		if(!worker) {
			worker = &Controller::getInstance();
			clog << settings.name << "\tNo worker, using the default one" << endl;
		}

		running = time(0);
        activations.next = running + timers.busy;

		// Get a copy of the formatted private data, just in case of it changes.
		Alert::PrivateData *dyndata = new Alert::PrivateData(data);

        ThreadPool::getInstance().push([this,dyndata]{

			try {

				worker->send(*this, dyndata->url,dyndata->payload);
				activations.success++;

			} catch(const std::exception &e) {
				activations.failed++;
				cerr << name() << "\tActivation " << (activations.failed + activations.success) << " failed: " << e.what() << endl;
			} catch(...) {
				activations.failed++;
				cerr << name() << "\tActivation " << (activations.failed + activations.success) << " failed: Unexpected error"  << endl;
			}

			delete dyndata;

			running = 0;
			next();
		});

	}
	*/

 }

