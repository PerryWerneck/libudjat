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

	mutex SubProcess::Controller::guard;

	SubProcess::Controller::Controller() {
	}

	SubProcess::Controller::~Controller() {
		Udjat::Event::remove(this);
	}

	void SubProcess::Controller::push_back(SubProcess::Controller::Entry &entry) {

		std::lock_guard<std::mutex> lock(guard);
		if(entries.empty()) {

			// Subscribe to signal.
			Udjat::Event::SignalHandler(this,SIGCHLD,[this](){

				int status = 0;
				pid_t pid = waitpid(0,&status,WNOHANG);

				std::lock_guard<std::mutex> lock(guard);
				entries.remove_if([status,pid](const Entry &entry){

					if(entry.proc->pid != pid) {
						return false;
					}

					if(WIFEXITED(status)) {
						entry.proc->status.exit = WEXITSTATUS(status);
						// process->onExit(process->status.exit);
					}

					if(WIFSIGNALED(status)) {
						entry.proc->status.failed = true;
						entry.proc->status.termsig = WTERMSIG(status);
						// proc->onSignal(process->status.termsig);
					}

					entry.proc->pid = -1;

					return true;
				});

				return !entries.empty();
			});

		}

		entries.push_back(entry);

	}

	SubProcess::Controller & SubProcess::Controller::getInstance() {
		static Controller instance;
		return instance;
	}

	/*
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

				break;

			}

		}

	}
	*/
 }

