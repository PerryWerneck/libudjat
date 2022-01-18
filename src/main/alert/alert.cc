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

	Alert::Alert(const pugi::xml_node &node) : settings(node) {

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

	}

	Alert::~Alert() {

		if(running) {
			cerr << name() << "Critical error: Deleting an active alert" << endl;
		}
		deactivate();
	}

	void Alert::activate() {
		Controller::getInstance().activate(this,settings.url,settings.payload);
	}

	void Alert::activate(const string &url, const string &payload) {
		Controller::getInstance().activate(this,url.c_str(),payload.c_str());
	}

	void Alert::activate(const string &payload) {
		Controller::getInstance().activate(this,settings.url,payload.c_str());
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

		Controller::getInstance().refresh();

	}

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

 }

