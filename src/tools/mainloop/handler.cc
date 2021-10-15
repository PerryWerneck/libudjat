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
 #include <udjat-internals.h>

 namespace Udjat {

	MainLoop::Handler::Handler(const void *i, int f, const Event e, const function<bool(const Event event)> c)
		: id(i),fd(f),events(e),running(0),call(c) { }

#ifndef _WIN32
	nfds_t MainLoop::getHandlers(struct pollfd **fds, nfds_t *length) {

		lock_guard<mutex> lock(guard);

		nfds_t nfds = 0;

		// Get waiting sockets.
		handlers.remove_if([fds,length,&nfds](Handler &handle) {

			// Are we active? If not return true *only* if theres no pending event.
			if(handle.fd <= 0)
				return handle.running == 0;

			// Am I running? If yes don't pool for me, but keep me in the list.
			if(handle.running)
				return false;

			if(nfds >= (*length-1)) {
				*length += 2;
				*fds = (struct pollfd *) realloc(*fds, sizeof(struct pollfd) * *length);
				for(size_t ix = nfds; ix < *length; ix++) {
					(*fds)[ix].fd = -1;
					(*fds)[ix].events = 0;
					(*fds)[ix].revents = 0;
				}
			}

			(*fds)[nfds].fd = handle.fd;
			(*fds)[nfds].events = handle.events;
			(*fds)[nfds].revents = 0;
			nfds++;
			return false;
		});

		return nfds;
	}
#endif // _WIN32

 }

