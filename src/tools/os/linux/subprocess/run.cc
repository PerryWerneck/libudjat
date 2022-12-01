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

	int SubProcess::run() {

		class SyncHandler : public SubProcess::Handler {
		private:
			SubProcess &proc;

		protected:

			void on_error(const char *reason) override {
				proc.onStdErr(reason);
			}

			void on_input(const char *line) override {
				if(id == 0) {
					proc.onStdOut(line);
				} else {
					proc.onStdErr(line);
				}
			}

		public:
			SyncHandler(SubProcess *p, unsigned short id) : SubProcess::Handler(id), proc(*p) {
			}

		};

		int rc = -1;
		SyncHandler handlers[]{ { this,0 }, { this,1}  };

		Controller::init(*this,handlers[0],handlers[1]);

		MainLoop::Handler *hdl[]{&handlers[0],&handlers[1]};
		while(running()) {

			int status = 0;

			if(Handler::poll(hdl,2,1000)) {
				// Have active handlers, keep loop running.
				waitpid(this->pid,&status,WNOHANG);
			} else {
				// No more active handlers, wait.
				waitpid(this->pid,&status,0);
			}

			if(WIFEXITED(status)) {
				rc = WEXITSTATUS(status);
				onExit(rc);
				break;
			}

			if(WIFSIGNALED(status)) {
				rc = -1;
				onSignal(WTERMSIG(status));
				break;
			}

		}

		return rc;

	}


 }
