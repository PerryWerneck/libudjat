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
 #include <udjat/service.h>

 namespace Udjat {

 	mutex Abstract::EventListener::mtx;

	void Service::Controller::push_back(Abstract::EventListener *listener) {
		debug("Inserting listener ",listener->name," (size=", listeners.size(),")");
		lock_guard<mutex> lock(Abstract::EventListener::mtx);
		listeners.push_back(listener);
		wakeup();
	}

	void Service::Controller::remove(Abstract::EventListener *listener) {
		debug("Removing listener ",listener->name," (size=", listeners.size(),")");
		lock_guard<mutex> lock(Abstract::EventListener::mtx);
		listeners.remove(listener);
		wakeup();
	}

	Abstract::EventListener::EventListener(const EventListener *src) {
		this->name = src->name;
		Service::Controller::getInstance().push_back(this);
	}

	Abstract::EventListener::EventListener(const EventListener &src) {
		this->name = src.name;
		Service::Controller::getInstance().push_back(this);
	}

	Abstract::EventListener::EventListener(const char *name) {
		this->name = name;
		Service::Controller::getInstance().push_back(this);
	}

	Abstract::EventListener::~EventListener() {
		Service::Controller::getInstance().remove(this);
	}

	void Abstract::EventListener::start() noexcept {
	}

	void Abstract::EventListener::stop() noexcept {
	}

	void Abstract::EventListener::reload() noexcept {
		stop();
		start();
	}


 }

