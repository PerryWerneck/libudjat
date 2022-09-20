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

	SubProcess::Controller::Controller() {
		signal(SIGCHLD,handle_signal);
	}

	SubProcess::Controller::~Controller() {
		signal(SIGCHLD,SIG_DFL);
	}

	void SubProcess::Controller::insert(SubProcess *subprocess) {
		processes.push_back(subprocess);
	}

	void SubProcess::Controller::remove(SubProcess *subprocess) {
		processes.remove_if([subprocess](SubProcess *p) {
			return p == subprocess;
		});
	}

	SubProcess::Controller & SubProcess::Controller::getInstance() {
		static Controller instance;
		return instance;
	}

	void SubProcess::Controller::handle_signal(int sig) noexcept {

		int status = 0;
		pid_t pid = waitpid(0,&status,WNOHANG);

		for(auto process : Controller::getInstance().processes) {

			if(process->pid == pid) {

				if(WIFEXITED(status)) {
					process->status.exit = WEXITSTATUS(status);
				}

				if(WIFSIGNALED(status)) {
					process->status.failed = true;
					process->status.termsig = WTERMSIG(status);
				}

				// Delay reading to avoid timing problems.
				MainLoop::getInstance().TimerFactory(100,[process]() {

					// cleanup and delete process on another thread to avoid dead-locks.
					ThreadPool::getInstance().push("ProcessCleanup",[process](){

						// Read pending data.
						int nfds = 0;
						do {

							struct pollfd pfd[2];

							for(size_t ix = 0; ix < 2; ix++) {
								pfd[ix].fd = process->pipes[ix].fd;
								pfd[ix].events = POLLIN;
							}

							nfds = poll(pfd,2,10);

							for(size_t ix = 0; ix < 2; ix++) {
								if(pfd[ix].revents & POLLIN) {
									process->read(ix);
								}
							}

						} while(nfds > 0);

						if(process->status.failed) {
							process->onSignal(process->status.termsig);
						} else {
							process->onExit(process->status.exit);
						}

						delete process;
					});

					return false;

				});

				/*
				// Remove FD from mainloop.
				lock_guard<mutex> lock(Controller::getInstance().guard);


				// Process exit codes.


				delete process;
				*/
				break;

			}

		}

	}
 }

