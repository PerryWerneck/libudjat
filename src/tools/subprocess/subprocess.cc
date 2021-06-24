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
 #include <iostream>
 #include <cstring>
 #include <csignal>
 #include <sys/wait.h>
 #include <poll.h>
 #include <udjat/tools/threadpool.h>

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

				MainLoop::getInstance().remove(process);

				// Delay reading to avoid timing problems.
				MainLoop::getInstance().insert(NULL,100,[process]() {

					// cleanup and delete process on another thread to avoid dead-locks.
					ThreadPool::getInstance().push([process](){

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

	SubProcess::SubProcess(const char *c) : command(c) {
		memset(pipes[0].buffer,0,sizeof(pipes[0].buffer));
		memset(pipes[1].buffer,0,sizeof(pipes[1].buffer));
		Controller::getInstance().insert(this);
	}

	SubProcess::~SubProcess() {

		MainLoop::getInstance().remove(this);
		Controller::getInstance().remove(this);

		for(size_t ix = 0; ix < (sizeof(pipes) /sizeof(pipes[0])); ix++) {
			if(pipes[ix].fd > 0) {
				::close(pipes[ix].fd);
			}
		}

	}

	void SubProcess::start(const char *command) {

		SubProcess *process = new SubProcess(command);

		ThreadPool::getInstance().push([process]() {

			try {

				process->start();

			} catch(const std::exception &e) {

				process->onStdErr((string{"Error '"} + e.what() + "' starting process").c_str());
				delete process;

			} catch(...) {

				process->onStdErr("Unexpected error starting process");
				delete process;

			}

		});



	}

	/// @brief Called on subprocess stdout.
	void SubProcess::onStdOut(const char *line) {
		cout << line << endl;
	}

	/// @brief Called on subprocess stderr.
	void SubProcess::onStdErr(const char *line) {
		onStdOut(line);
	}

	/// @brief Called on subprocess normal exit.
	void SubProcess::onExit(int rc) {
		string msg = string{"Process '"} + command + "' finishes with rc=" + to_string(rc);

		if(rc) {
			onStdOut(msg.c_str());
		} else {
			onStdErr(msg.c_str());
		}

	}

	/// @brief Called on subprocess abnormal exit.
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	void SubProcess::onSignal(int sig) {
		onStdErr((string{"Process '" + command + "' finishes with signal "} + to_string(sig)).c_str());
	}
	#pragma GCC diagnostic pop

	void SubProcess::start() {

		if(this->pid != -1) {
			throw system_error(EBUSY,system_category(),string{"The child process is active on pid "} + to_string(this->pid));
		}

		//
		// Start process.
		//
		int errcode;

		int out[2] = {-1, -1};
		int err[2] = {-1, -1};

		// Create sockets.
		if(socketpair(AF_UNIX, SOCK_STREAM, 0, out) < 0) {
			throw system_error(errno,system_category(),string{"Can't create stdout pipes for '"} + c_str() + "'");
		}

		if(socketpair(AF_UNIX, SOCK_STREAM, 0, err) < 0) {
			errcode = errno;
			::close(out[0]);
			::close(out[1]);
			throw system_error(errcode,system_category(),string{"Can't create stderr pipes for '"} + c_str() + "'");
		}

		switch (this->pid = vfork()) {
		case -1: // Error
			errcode = errno;
			::close(out[0]);
			::close(out[1]);
			::close(err[0]);
			::close(err[1]);
			throw system_error(errcode,system_category(),string{"Can't start child for '"} + c_str() + "'");

		case 0:	// child

			if(out[1] != STDOUT_FILENO) {
				(void)dup2(out[1], STDOUT_FILENO);
				(void)close(out[1]);
				out[1] = STDOUT_FILENO;
			}

			if(err[1] != STDERR_FILENO) {
				(void)dup2(err[1], STDERR_FILENO);
				(void)close(err[1]);
				err[1] = STDERR_FILENO;
			}

			execl("/bin/bash", "/bin/bash", "-c", command.c_str(), NULL);
			_exit(127);

		}

		// Child started, capture pipes.
		this->pipes[0].fd = out[0];
		this->pipes[1].fd = err[0];

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

	}

	void SubProcess::read(int id) {

		ssize_t szRead =
			::read(
				pipes[id].fd,
				pipes[id].buffer+pipes[id].length,
				sizeof(pipes[id].buffer) - (pipes[id].length+1)
			);

		if(szRead < 0) {
			onStdErr((string{"Error '"} + strerror(errno) + "' reading from pipe").c_str());
			return;
		}

		if(szRead == 0) {
			onStdErr("Unexpected 'EOF' reading from pipe");
			return;
		}

		pipes[id].buffer[pipes[id].length+szRead] = 0;

		char *from = pipes[id].buffer;
		char *to = strchr(from,'\n');
		while(to) {

			*to = 0;

			if(id) {
				onStdErr(from);
			} else {
				onStdOut(from);
			}

			from = to+1;
			to = strchr(from,'\n');
		}

		if(from && from != pipes[id].buffer) {
			pipes[id].length = strlen(from);
			char *to = pipes[id].buffer;
			while(*from) {
				*(to++) = *(from++);
			}
			*to = 0;
		}

		pipes[id].length = strlen(pipes[id].buffer);

	}

 }
