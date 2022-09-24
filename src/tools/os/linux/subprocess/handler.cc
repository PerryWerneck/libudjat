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

 using namespace std;

 namespace Udjat {

	void SubProcess::Handler::parse() {

		char *from = buffer;
		char *to = strchr(from,'\n');
		while(to) {

			*to = 0;
			if(to > from && *(to-1) == '\r') {
				*(to-1) = 0;
			}

			on_input(from);

			from = to+1;
			to = strchr(from,'\n');
		}

		if(from && from != buffer) {
			length = strlen(from);
			char *to = buffer;
			while(*from) {
				*(to++) = *(from++);
			}
			*to = 0;
		}

		length = strlen(buffer);

	}

	void SubProcess::Handler::handle_event(const Event event) {

		if(event & oninput) {

			ssize_t szRead = read(buffer+length, sizeof(buffer)-(length+1));

			if(szRead < 0) {

				on_error((string{"Error '"} + strerror(errno) + "' reading from pipe").c_str());
				close();

			} else if(!szRead) {

				on_error("Unexpected 'EOF' reading from pipe");

			} else {

				buffer[length+szRead] = 0;
				parse();

			}

		}

		if(event & onerror) {
			on_error("I/O error");
			close();
		}

		if(event & onhangup) {
			on_error("Pipe closed");
			close();
		}

	}

 }
