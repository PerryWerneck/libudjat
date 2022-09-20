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
 #include <udjat/tools/handler.h>
 #include <poll.h>
 #include <system_error>

 using namespace std;

 namespace Udjat {

	size_t MainLoop::Handler::poll(MainLoop::Handler **handlers, size_t nfds, int timeout) {

		size_t valid_handlers = 0;
		struct pollfd fds[nfds];
		Handler *index[nfds];

		for(size_t ix = 0; ix < nfds; ix++) {

			if(handlers[ix]->fd != -1) {
				index[valid_handlers] = handlers[ix];
				fds[valid_handlers].fd = handlers[ix]->fd;
				fds[valid_handlers].events = handlers[ix]->events;
				fds[valid_handlers].revents = 0;
				valid_handlers++;
			}

		}

		int nEvents = ::poll(fds,valid_handlers,timeout);

		if(nEvents < 0) {

			throw system_error(errno,system_category());

		} else if(nEvents > 0) {

			for(size_t ix = 0; ix < valid_handlers && nEvents > 0; ix++) {

				if(fds[ix].revents) {
					index[ix]->handle_event((Event) fds[ix].revents);
					nEvents--;
				}

			}

		}

		return valid_handlers;
	}

 }

