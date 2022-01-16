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

 namespace Udjat {

	mutex Alert::Controller::guard;

	Alert::Controller & Alert::Controller::getInstance() {
		lock_guard<mutex> lock(guard);
		static Controller instance;
		return instance;
	}

	Alert::Controller::Controller() {
	}

	Alert::Controller::~Controller() {
	}

	void Alert::Controller::deactivate(Alert *alert) {
		lock_guard<mutex> lock(guard);
		cout << alert->settings.name << "\tDeactivating alert " << alert->settings.url << endl;
		alerts.remove_if([alert](Alert *active){
			return active == alert;
		});
	}

	void Alert::Controller::activate(Alert *alert) {
		lock_guard<mutex> lock(guard);
		cout << alert->settings.name << "\tActivating alert " << alert->settings.url << endl;
		alert->next = time(0) + alert->timers.start;
		alerts.push_back(alert);
	}


 }
