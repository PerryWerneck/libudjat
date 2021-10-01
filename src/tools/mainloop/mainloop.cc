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
 #include <udjat.h>
 #include "private.h"
 #include <sys/eventfd.h>
 #include <udjat/agent.h>

 namespace Udjat {

	std::mutex MainLoop::guard;

	MainLoop::MainLoop() {
		cout << "MainLoop\tStarting service loop" << endl;
		efd = eventfd(0,0);
		if(efd < 0)
			throw system_error(errno,system_category(),"eventfd() has failed");

	}

	MainLoop::~MainLoop() {

		cout << "MainLoop\tStopping service loop" << endl;

		enabled = false;
		wakeup();

		{
			lock_guard<mutex> lock(guard);
			::close(efd);
		}

	}

	void MainLoop::quit() {
		enabled = false;
		wakeup();
	}

	void MainLoop::remove(const void *id) {

		lock_guard<mutex> lock(guard);

		//
		// We can't simple remove the handlers; they can be waiting for a slot to run.
		//
		for(auto timer = timers.active.begin(); timer != timers.active.end(); timer++) {
			if(timer->id == id) {
				timer->interval = 0;	// When set to '0' the timer will be removed when possible.
			}
		}

		for(auto handler = handlers.begin(); handler != handlers.end(); handler++) {
			if(handler->id == id) {
				handler->fd = -1;	// When set to '-1' the handle will be removed when possible.
			}
		}

		wakeup();
	}

	void MainLoop::insert(const void *id, int fd, const Event event, const function<bool(const Event event)> call) {
		lock_guard<mutex> lock(guard);
		handlers.emplace_back(id,fd,event,call);
		wakeup();
	}

 }

