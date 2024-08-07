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

 using namespace std;

 namespace Udjat {

	mutex MainLoop::Service::guard;

	static const ModuleInfo moduleinfo{"service"};

	MainLoop::Service::Service(const ModuleInfo *i) : info(i) {
		lock_guard<mutex> lock(guard);
		MainLoop::getInstance().services.push_back(this);
	}

	MainLoop::Service::Service() : Service(&moduleinfo) {
	}

	MainLoop::Service::~Service() {
		lock_guard<mutex> lock(guard);
		MainLoop::getInstance().services.remove_if([this](Service *s) {
			return s == this;
		});
	}

	void MainLoop::Service::start() {
	}

	void MainLoop::Service::stop() {
	}

 }
