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

 /***
  * @brief Implement the SubProcess object.
  *
  * References:
  *
  * <https://opensource.apple.com/source/Libc/Libc-167/gen.subproj/popen.c.auto.html>
  *
  */

 #include "private.h"
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <unistd.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/handler.h>
 #include <iostream>
 #include <cstring>
 #include <csignal>
 #include <sys/wait.h>
 #include <poll.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/logger.h>

 namespace Udjat {

	void SubProcess::start() {

#ifndef DEBUG
		init();

		#error Refactor.

		MainLoop::getInstance().insert(
			this,
			this->pipes[0].fd,
			(MainLoop::Event) (MainLoop::oninput|MainLoop::onerror|MainLoop::onhangup),
			[this](const MainLoop::Event event) {

				if(event & MainLoop::oninput) {
					read(0);
				}

				if(event & MainLoop::onerror) {
					onStdErr("Error on stdout pipe");
					close(this->pipes[0].fd);
					return false;
				}

				if(event & MainLoop::onhangup) {
					onStdErr("stdout pipe was closed");
					close(this->pipes[0].fd);
					return false;
				}

				return true;
			});

		MainLoop::getInstance().insert(
			this,
			this->pipes[1].fd,
			(MainLoop::Event) (MainLoop::oninput|MainLoop::onerror|MainLoop::onhangup),
			[this](const MainLoop::Event event) {

				if(event & MainLoop::oninput) {
					read(1);
				}

				if(event & MainLoop::onerror) {
					onStdErr("Error on stderr pipe");
					close(this->pipes[1].fd);
					return false;
				}

				if(event & MainLoop::onhangup) {
					onStdErr("stderr pipe was closed");
					close(this->pipes[1].fd);
					return false;
				}

				return true;
			}
		);

#endif // DEBUG

	}

 }
