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

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/linux/subprocess.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <unistd.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/handler.h>
 #include <iostream>
 #include <cstring>
 #include <csignal>
 #include <sys/wait.h>
 #include <sys/poll.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/logger.h>

 namespace Udjat {

	SubProcess::Controller::Controller() {
		signal(SIGCHLD,handle_signal);
	}

	SubProcess::Controller::~Controller() {
		signal(SIGCHLD,SIG_DFL);
	}

	void SubProcess::Controller::handle_signal(int UDJAT_UNUSED(sig)) noexcept {

		int status = 0;
		pid_t pid = waitpid(0,&status,WNOHANG);

		ThreadPool::getInstance().push("SubProcCleanup",[pid,status](){
			getInstance().child_ended(pid,status);
		});

	}

	void SubProcess::Controller::child_ended(pid_t pid, int status) noexcept {

		entries.remove_if([status,pid](const Entry &entry){

			if(entry.proc->pid != pid) {
				return false;
			}

			try {

				entry.proc->post(status);

			} catch(const std::exception &e) {

				cerr << "Post script has failed on pid " << pid << ": " << e.what() << endl;

			} catch(...) {

				cerr << "Unexpected error on post script for pid " << pid << endl;

			}

			entry.proc->pid = -1;

			// Disable streams
			entry.out->disable();
			entry.err->disable();

			usleep(100);

			// check for pending events.
			{
				MainLoop::Handler *hdl[]{entry.out.get(),entry.err.get()};
				Handler::flush(hdl,2,1000);
			}

			// Flush streams.
			entry.out->flush();
			entry.err->flush();

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

