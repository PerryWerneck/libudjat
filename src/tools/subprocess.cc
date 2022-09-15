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
  * @brief Implement the SubProcess common methods.
  *
  */

 #include <config.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <iostream>
 #include <udjat/tools/threadpool.h>

 using namespace std;

 namespace Udjat {

	SubProcess::SubProcess(const char *c) : SubProcess("subprocess",c) {
	}

	int SubProcess::run(const char *command) {
		return SubProcess(command).run();
	}

	int SubProcess::run(const NamedObject *obj, const char *command) {
		return SubProcess(obj,command).run();
	}

	void SubProcess::start(const char *command) {
		(new SubProcess(command))->start();
	}

	void SubProcess::start(const NamedObject *obj, const char *command) {
		(new SubProcess(obj, command))->start();
	}

	/// @brief Called on subprocess stdout.
	void SubProcess::onStdOut(const char *line) {
		info() << line << endl;
	}

	/// @brief Called on subprocess stderr.
	void SubProcess::onStdErr(const char *line) {
		error() << line << endl;
	}

	/// @brief Called on subprocess normal exit.
	void SubProcess::onExit(int rc) {

		Logger::Message msg("Process '{}' finishes with rc={}",command,rc);

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
		onStdErr(Logger::Message{"Process '{}' finishes with signal {}",command,std::to_string(sig)}.c_str());
	}
	#pragma GCC diagnostic pop

	void SubProcess::parse(int id) {

		char *from = pipes[id].buffer;
		char *to = strchr(from,'\n');
		while(to) {

			*to = 0;
			if(to > from && *(to-1) == '\r') {
				*(to-1) = 0;
			}

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
