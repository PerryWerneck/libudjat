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

 #include <config.h>
 #include <private/alert.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/alert/activation.h>

 namespace Udjat {

	Abstract::Alert::Alert(const pugi::xml_node &node,const char *defaults) : Alert(Quark(node,"name","alert",false).c_str()) {

		// Get section from configuration file with the defaults.
		const char *section = node.attribute("settings-from").as_string(defaults);

		// options.
		options.verbose = getAttribute(node,section,"verbose",Logger::enabled(Logger::Trace));
		options.asyncronous = getAttribute(node,section,"asyncronous",options.asyncronous);

		// Seconds to wait before first activation.
		timers.start = getAttribute(node,section,"delay-before-start",timers.start);

		// Seconds to wait on every try.
		timers.interval = getAttribute(node,section,"delay-before-retry",timers.interval);

		// Seconds to wait if busy.
		timers.busy = getAttribute(node,section,"delay-when-busy",timers.busy);

		// How many success emissions after deactivation or sleep?
		retry.min = getAttribute(node,section,"min-retries",retry.min);

		// How many retries (success+fails) after deactivation or sleep?
		retry.max = getAttribute(node,section,"max-retries",retry.max);

		// How many seconds to restart when failed?
		restart.failed = getAttribute(node,section,"restart-when-failed",restart.failed);

		// How many seconds to restart when suceeded?
		restart.success = getAttribute(node,section,"restart-when-succeeded",restart.success);

	}

	Abstract::Alert::~Alert() {
	}

	std::shared_ptr<Udjat::Alert::Activation> Abstract::Alert::ActivationFactory() const {
		throw runtime_error("Cant activate an abstract alert");
	}

	void Abstract::Alert::activate(const Abstract::Object &object) {
		auto activation = ActivationFactory();
		activation->set(object);
		Udjat::start(activation);
	}

	void Abstract::Alert::deactivate() {
		Udjat::Alert::Controller::getInstance().remove(this);
	}

	Value & Abstract::Alert::getProperties(Value &value) const noexcept {

		NamedObject::getProperties(value);

		value["minretry"] = retry.min;
		value["maxretry"] = retry.max;
		value["startdelay"] = timers.start;
		value["busydelay"] = timers.busy;
		value["interval"] = timers.interval;
		value["faildelay"] = restart.failed;
		value["delay"] = restart.success;

		return value;
	}

	const char * Abstract::Alert::getPayload(const pugi::xml_node &node) {

		String child(node.child_value());

		if(child.empty()) {

			auto payload = node.attribute("payload");
			if(!payload) {
				payload = getAttribute(node,"alert-payload",false);
			}

			if(payload) {
				child = payload.as_string();
			}

		}

		if(getAttribute(node,"strip-payload",true).as_bool(true)) {
			child.strip();
		}

		return Quark(child.expand(node,"alert-defaults")).c_str();

	}

 }

