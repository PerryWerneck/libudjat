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
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/object.h>
 #include <udjat/alert/abstract.h>
 #include <udjat/agent/state.h>
 #include <udjat/alert/activation.h>
 #include <cstdarg>

 namespace Udjat {

 	void start(std::shared_ptr<Udjat::Alert::Activation> activation) {
		Alert::Controller::getInstance().push_back(activation);
 	}

	Alert::Activation::Activation(const Abstract::Alert *alert) : id(alert) {

		options.verbose = alert->verbose();
		options.asyncronous = alert->asyncronous();

		name = alert->name();

		retry.min = alert->retry.min;
		retry.max = alert->retry.max;

		timers.interval = alert->timers.interval;
		timers.busy = alert->timers.busy;
		timers.next = time(0) + alert->timers.start;
		timers.success = alert->restart.success;
		timers.failed = alert->restart.failed;

	}

	Alert::Activation::~Activation() {
	}

	std::ostream & Alert::Activation::info() const {
		return cout << name << "\t";
	}

	std::ostream & Alert::Activation::warning() const {
		return clog << name << "\t";
	}

	std::ostream & Alert::Activation::error() const {
		return cerr << name << "\t";
	}

	Alert::Activation & Alert::Activation::set(const Abstract::Object UDJAT_UNUSED(&object)) {
		return *this;
	}

	Alert::Activation & Alert::Activation::apply(const Abstract::Object *object, ...) {
		va_list args;
		va_start(args, object);
		while(object) {
			try {
				set(*object);
			} catch(...) {
				va_end(args);
				throw;
			}
			object = va_arg(args, const Abstract::Object *);
		}
		va_end(args);
		return *this;
	}

	Alert::Activation & Alert::Activation::expand(const std::function<bool(const char *key, std::string &value)> UDJAT_UNUSED(&expander)) {
		return *this;
	}

	void Alert::Activation::emit() {
		throw runtime_error("Cant emit an abstract activation");
	}

	Value & Alert::Activation::getProperties(Value &value) const noexcept {

		value["name"] = name;
		value["next"] = TimeStamp(timers.next);
		value["last"] = TimeStamp(timers.last);
		value["failed"] = count.failed;
		value["success"] = count.success;
		value["restarting"] = state.restarting;
		value["running"] = TimeStamp(state.running);

		return value;
	}

	bool Alert::Activation::run() noexcept {

		bool succeeded = false;
		try {
			timers.last = time(0);
			emit();
			count.success++;
			succeeded = true;
			timers.next = time(0) + timers.interval;
		} catch(const exception &e) {
			error() << "Alert emmission failed with '" << e.what() << "'" << endl;
			count.failed++;
		} catch(...) {
			error() << "Unexpected error emitting alert" << endl;
			count.failed++;
		}


		// Setup for next emission.
#ifdef DEBUG
		info() << "success=" << count.success << " failed=" << count.failed << " min=" << retry.min << " max= " << retry.max << endl;
#endif // DEBUG

		if(count.success >= retry.min) {
			checkForSleep("was sucessfull");
		} else if( (count.success + count.failed) >= retry.max ) {
			checkForSleep("reached the maximum number of emissions");
		} else if(timers.interval) {
			timers.next = time(0) + timers.interval;
			if(verbose()) {
				info() << "Next emission set to " << TimeStamp(timers.next) << endl;
			}
		} else {
			timers.next = 0;
			if(verbose()) {
				info() << "No interval, deactivating alert" << endl;
			}
		}

		return succeeded;

	}

	void Alert::Activation::checkForSleep(const char *msg) noexcept {

		time_t rst = (count.success ? timers.success : timers.failed);

		if(rst) {
			state.restarting = true;
			timers.next = time(0) + rst;
			if(options.verbose) {
				LogFactory(count.failed ? Udjat::error : Udjat::ready)
					<< name << "\t" << msg << ", sleeping until " << TimeStamp(timers.next)
					<< endl;
			}
		}
		else {
			timers.next = 0;
			if(options.verbose) {
				LogFactory(count.failed ? Udjat::error : Udjat::ready)
					<< name << "\t" << msg << ", stopping"
					<< endl;
			}
		}

	}

 }
