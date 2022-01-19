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
 #include <udjat.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/url.h>

 namespace Udjat {

	void Alert::activate(const char *name, const char *url, const char *action, const char *payload) {
		Controller::getInstance().insert(name,url,action,payload);
	}

	void Alert::Controller::insert(const char *name, const char *url, const char *action, const char *payload) {

		class UrlAlert : public Alert {
		private:
			string url;
			string action;
			string payload;

		public:
			UrlAlert(const char *name, const char *u, const char *a, const char *p) : Alert(name), url(u), action(a), payload(p) {
			}

			void emit() const override {
				cout << "alerts\tEmitting '" << url << "'" << endl;
				auto response = URL(url.c_str()).call(action.c_str(),nullptr,payload.c_str());
				if(response->failed()) {
					throw runtime_error(to_string(response->getStatusCode()) + " " + response->getStatusMessage());
				}
 			}

		};

		insert(make_shared<Activation>(make_shared<UrlAlert>(name,url,action,payload)));

	}

	Alert::Activation::Activation(std::shared_ptr<Alert> alert) : alertptr(alert) {
		cout << "alerts\tActivating " << name() << endl;
		timers.next = time(0) + alert->timers.start;
	}

	Alert::Activation::~Activation() {
		cout << "alerts\tDeactivating " << name() << endl;
	}

	void Alert::Activation::checkForSleep(const char *msg) noexcept {

		time_t rst = (count.success ? alertptr->restart.success : alertptr->restart.failed);

		if(rst) {
			restarting = true;
			timers.next = time(0) + rst;
			clog
				<< "alerts\tAlert '" << alertptr->c_str() << "' cycle " << msg << ", sleeping until " << TimeStamp(timers.next)
				<< endl;

		} else {
			timers.next = 0;
			clog
				<< "alerts\tAlert '" << alertptr->c_str() << "' cycle " << msg << ", stopping"
				<< endl;
		}

	}

	void Alert::Activation::emit() const noexcept {
		alertptr->emit();
	}

	void Alert::Activation::failed() noexcept {
		count.failed++;
		next();
	}

	void Alert::Activation::success() noexcept {
		count.success++;
		next();
	}

	void Alert::Activation::next() noexcept {

		cout << "alerts\t" << alertptr->c_str() << " success=" << count.success << " failed=" << count.failed << " min=" << alertptr->retry.min << " max= " << alertptr->retry.max << endl;

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
