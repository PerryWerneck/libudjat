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

	Alert::Controller::Controller() {
	}

	Alert::Controller::~Controller() {
	}

	Alert::Controller & Alert::Controller::getInstance() {

		lock_guard<mutex> lock(guard);

		static Controller instance;
		return instance;
	}

	/// @brief Agent value has changed.
	void Alert::Controller::deactivate(std::shared_ptr<Alert> alert) {

		lock_guard<mutex> lock(guard);

	}

	/// @brief Activate alert.
	void Alert::Controller::activate(std::shared_ptr<Alert> alert, const Abstract::Agent &agent, const Abstract::State &state) {

		lock_guard<mutex> lock(guard);

	}

 }
