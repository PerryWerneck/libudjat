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
 #include "private.h"
 #include <udjat-internals.h>

 namespace Udjat {

	std::mutex MainLoop::guard;

	void MainLoop::quit() {
		enabled = false;
		wakeup();
	}

	void MainLoop::remove(const void *id) {

		lock_guard<mutex> lock(guard);

		timers.active.remove_if([id](auto timer){
			return timer->id == id;
		});

		for(auto handler = handlers.begin(); handler != handlers.end(); handler++) {
			if(handler->id == id) {
				handler->fd = -1;	// When set to '-1' the handle will be removed when possible.
			}
		}

	}

	void MainLoop::insert(const void *id, int fd, const Event event, const function<bool(const Event event)> call) {
		lock_guard<mutex> lock(guard);
		handlers.emplace_back(id,fd,event,call);
		wakeup();
	}

 }

