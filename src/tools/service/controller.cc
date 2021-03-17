/*
 *
 * Copyright (C) <2019> <Perry Werneck>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 *
 * @file
 *
 * @brief
 *
 * @author Perry Werneck <perry.werneck@gmail.com>
 *
 *
 */

 #include "private.h"

 namespace Udjat::Service {

	static Controller controller;

	Controller::Controller() {
		debug("Creating service controller");

	}

	Controller::~Controller() {
		debug("Deleting service controller");
	}

	Controller& Controller::getInstance() {
		return controller;
	}

	void Controller::insert(Abstract::Timer *timer) {
		lock_guard<mutex> lock(mtx);
		timers.push_back(timer);
		wakeup();
	}

	void Controller::remove(Abstract::Timer *timer) {
		lock_guard<mutex> lock(mtx);
		timers.remove(timer);
		wakeup();
	}

 }

