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

 #include "../private.h"
 #include <udjat/service.h>

 namespace Udjat {

	Abstract::Timer::Timer(int seconds, bool start) {

		interval = seconds;
		next = time(nullptr)+interval;

		if(start) {
			this->start(false);
		}

	}

	Abstract::Timer::~Timer() {
		stop();
	}

	void Abstract::Timer::start(bool immediately) {

		next = (interval ? (time(nullptr)+interval) : 0);

		if(immediately) {
			onTimer();
		}

		Service::Controller::getInstance().insert(this);

	}

	void Abstract::Timer::stop() {
		Service::Controller::getInstance().remove(this);
	}

 }

