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

 #include "subprocess.h"
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

	int SubProcess::run() {

		int rc = -1;

		/*
		init();

		while(running()) {

			#error Refactor using Handler::poll

			struct pollfd fds[sizeof(pipes)/sizeof(pipes[0])];

			size_t nfds = 0;
			for(size_t ix = 0; ix < (sizeof(pipes) /sizeof(pipes[0])); ix++) {
				if(pipes[ix].fd > 0) {
					fds[nfds].fd = pipes[ix].fd;
					fds[nfds].events = POLLIN|POLLERR|POLLHUP;
					fds[nfds].revents = 0;
					nfds++;
				}
			}

			int nEvents = poll(fds,nfds,1000);

			if(nEvents < 0) {

				throw system_error(errno,system_category(),Logger::Message{"Error reading data from pid {}",this->pid});

			} else if(nEvents > 0) {

				for(size_t ix = 0; nEvents > 0 && ix < (sizeof(pipes) /sizeof(pipes[0])); ix++) {

					if(fds[ix].revents) {

						if(fds[ix].revents & MainLoop::Handler::oninput) {
							read(ix);
						}

						if(fds[ix].revents & MainLoop::Handler::onerror) {
							onStdErr("Error reading pipe");
							close(this->pipes[ix].fd);
							this->pipes[ix].fd = -1;
						}

						if(fds[ix].revents & MainLoop::Handler::onhangup) {
							onStdErr("Pipe was closed");
							close(this->pipes[ix].fd);
							this->pipes[ix].fd = -1;
						}

						nEvents--;
					}

				}

			}
			int status = 0;
			waitpid(this->pid,&status,WNOHANG);

			if(WIFEXITED(status)) {
				rc = this->status.exit = WEXITSTATUS(status);
				onExit(rc);
				break;
			}

			if(WIFSIGNALED(status)) {
				rc = -1;
				this->status.failed = true;
				this->status.termsig = WTERMSIG(status);
				onSignal(this->status.termsig);
				break;
			}

		}

		*/
		return rc;

	}


 }
