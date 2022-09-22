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
		signal(SIGCHLD,handle_signal);
	}

	SubProcess::Controller::~Controller() {
		signal(SIGCHLD,SIG_DFL);
	}

	void SubProcess::Controller::push_back(SubProcess::Controller::Entry &entry) {
		std::lock_guard<std::mutex> lock(guard);
		entries.push_back(entry);
	}

	SubProcess::Controller & SubProcess::Controller::getInstance() {
		static Controller instance;
		return instance;
	}

	void SubProcess::Controller::handle_signal(int UDJAT_UNUSED(sig)) noexcept {

		int status = 0;
		pid_t pid = waitpid(0,&status,WNOHANG);

		ThreadPool::getInstance().push("SubProcCleanup",[pid,status](){
			getInstance().child_ended(pid,status);
		});

	}

	void SubProcess::Controller::child_ended(pid_t pid, int status) noexcept {

		std::lock_guard<std::mutex> lock(guard);
		entries.remove_if([status,pid](const Entry &entry){

			if(entry.proc->pid != pid) {
				return false;
			}

			entry.proc->pid = -1;

			// Disable streams
			entry.out->disable();
			entry.err->disable();



			// Flush streams
			{
				MainLoop::Handler *hdl[]{entry.out.get(),entry.err.get()};

#ifdef DEBUG
				cout << __FILE__ << "(" << __LINE__ << ") Wait for stream data begin" << endl;
#endif // DEBUG
				Handler::poll(hdl,2,1000);
#ifdef DEBUG
				cout << __FILE__ << "(" << __LINE__ << ") Wait for stream data ends" << endl;
#endif // DEBUG

			}

			//entry.out->flush();
			//entry.err->flush();

			// Close streams.
			entry.out->close();
			entry.err->close();

			// Get process exit.
			if(WIFEXITED(status)) {
				entry.proc->onExit(WEXITSTATUS(status));
			}

			if(WIFSIGNALED(status)) {
				entry.proc->onSignal(WTERMSIG(status));
			}

			return true;
		});

#ifdef DEBUG
		cout << "subprocess\t*** Process count: " << entries.size() << endl;
#endif // DEBUG

	}
 }

