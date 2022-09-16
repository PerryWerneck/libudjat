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
 #include <private/mainloop.h>
 #include <private/misc.h>

 namespace Udjat {

	std::mutex MainLoop::guard;

	void MainLoop::quit() {
		enabled = false;
		wakeup();
	}

	void MainLoop::remove(const void *id) {

		lock_guard<mutex> lock(guard);

#ifdef DEBUG
		cout << "handler\tRemoving handlers with id " << hex << id << dec << endl;
#endif // DEBUG

		/*
		timers.active.remove_if([id](auto timer){
			return timer->id == id;
		});
		*/

		handlers.remove_if([id](auto handler){
			return handler->id() == id;
		});

	}


 }

