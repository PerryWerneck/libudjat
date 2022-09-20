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

	SubProcess::SubProcess(const char *n, const char *c) : NamedObject(n), command(c) {
		info() << "Running '" << command << "'" << endl;
		memset(pipes[0].buffer,0,sizeof(pipes[0].buffer));
		memset(pipes[1].buffer,0,sizeof(pipes[1].buffer));
		Controller::getInstance().insert(this);
	}

	SubProcess::~SubProcess() {

		Controller::getInstance().remove(this);

		for(size_t ix = 0; ix < (sizeof(pipes) /sizeof(pipes[0])); ix++) {
			if(pipes[ix].fd > 0) {
				::close(pipes[ix].fd);
			}
		}

	}

	bool SubProcess::read(int id) {

		ssize_t szRead =
			::read(
				pipes[id].fd,
				pipes[id].buffer+pipes[id].length,
				sizeof(pipes[id].buffer) - (pipes[id].length+1)
			);

		if(szRead < 0) {
			onStdErr((string{"Error '"} + strerror(errno) + "' reading from pipe").c_str());
			return true;
		}

		if(szRead == 0) {
			onStdErr("Unexpected 'EOF' reading from pipe");
			return false;
		}

		pipes[id].buffer[pipes[id].length+szRead] = 0;
		parse(id);

		return true;
	}

 }
