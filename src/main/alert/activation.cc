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
 #include <udjat/alert.h>

 namespace Udjat {

	Alert::Activation::Activation(const string &u, const HTTP::Method a, const string &p) : url(u), action(a), payload(p) {
	}

	Alert::Activation::Activation(const Alert &alert, const std::function<void(std::string &str)> &expander) : Activation(alert.url,alert.action,alert.payload) {

		expander(url);
		expander(payload);

	}

	void Alert::Activation::emit() const {
		Protocol::call(url.c_str(),action,payload.c_str());
	}

	Abstract::Alert::Activation::Activation() {
	}

	Abstract::Alert::Activation::~Activation() {
	}

	Value & Abstract::Alert::Activation::getProperties(Value &value) const noexcept {

		if(alertptr) {
			alertptr->getProperties(value);
		}

		value["next"] = TimeStamp(timers.next);
		value["last"] = TimeStamp(timers.last);
		value["failed"] = count.failed;
		value["success"] = count.success;
		value["restarting"] = state.restarting;
		value["running"] = TimeStamp(state.running);

		return value;
	}

	const char * Abstract::Alert::Activation::c_str() const noexcept {
		return name.c_str();
	}

	const char * Alert::Activation::c_str() const noexcept {
		return url.c_str();
	}

	void Abstract::Alert::Activation::checkForSleep(const char *msg) noexcept {

		time_t rst = (count.success ? alertptr->restart.success : alertptr->restart.failed);

		if(rst) {
			state.restarting = true;
			timers.next = time(0) + rst;
			clog
				<< name << "\tAlert '" << alertptr->c_str() << "' " << msg << ", sleeping until " << TimeStamp(timers.next)
				<< endl;

		} else {
			timers.next = 0;
			clog
				<< name << "\tAlert '" << alertptr->c_str() << "' " << msg << ", stopping"
				<< endl;
		}

	}

	void Abstract::Alert::Activation::emit() const {
		throw system_error(ENOTSUP,system_category(),"Cant emit an abstract activation");
	}

	void Abstract::Alert::Activation::failed() noexcept {
		count.failed++;
		next();
	}

	void Abstract::Alert::Activation::success() noexcept {
		count.success++;
		next();
	}

	void Abstract::Alert::Activation::next() noexcept {

#ifdef DEBUG
		cout << name << "\t" << alertptr->c_str() << " success=" << count.success << " failed=" << count.failed << " min=" << alertptr->retry.min << " max= " << alertptr->retry.max << endl;
#endif // DEBUG

		if(count.success >= alertptr->retry.min) {
			checkForSleep("was sucessfull");
		} else if( (count.success + count.failed) >= alertptr->retry.max ) {
			checkForSleep("reached the maximum number of emissions");
		} else {
			timers.next = time(0) + alertptr->timers.interval;
		}

		Controller::getInstance().refresh();

	}


 }
