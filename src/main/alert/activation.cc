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
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/object.h>
 #include <udjat/alerts/abstract.h>
 #include <udjat/agent/state.h>

 namespace Udjat {

 	void start(std::shared_ptr<Abstract::Alert::Activation> activation) {
		Abstract::Alert::Controller::getInstance().push_back(activation);
 	}

	Abstract::Alert::Activation::Activation(const Alert *alert) : id(alert) {

		options.verbose = alert->verbose();

		name = alert->name();

		retry.min = alert->retry.min;
		retry.max = alert->retry.max;

		timers.interval = alert->timers.interval;
		timers.busy = alert->timers.busy;
		timers.next = time(0) + alert->timers.start;
		timers.success = alert->restart.success;
		timers.failed = alert->restart.failed;

	}

	Abstract::Alert::Activation::~Activation() {
	}

	std::ostream & Abstract::Alert::Activation::info() const {
		return cout << name << "\t";
	}

	std::ostream & Abstract::Alert::Activation::warning() const {
		return clog << name << "\t";
	}

	std::ostream & Abstract::Alert::Activation::error() const {
		return cerr << name << "\t";
	}

	void Abstract::Alert::Activation::set(const Abstract::Object UDJAT_UNUSED(&object)) {
#ifdef DEBUG
		cerr << "alert\t*** Object was set on an abstract alert" << endl;
#endif
	}

	void Abstract::Alert::Activation::emit() {
		throw runtime_error("Cant emit an abstract activation");
	}

	Value & Abstract::Alert::Activation::getProperties(Value &value) const noexcept {

		value["name"] = name;
		value["level"] = std::to_string(options.level);
		value["description"] = description;
		value["next"] = TimeStamp(timers.next);
		value["last"] = TimeStamp(timers.last);
		value["failed"] = count.failed;
		value["success"] = count.success;
		value["restarting"] = state.restarting;
		value["running"] = TimeStamp(state.running);

		return value;
	}

	bool Abstract::Alert::Activation::run() noexcept {

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

	void Abstract::Alert::Activation::checkForSleep(const char *msg) noexcept {

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
