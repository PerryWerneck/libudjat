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
 #include <ctype.h>

 using namespace std;

 namespace Udjat {

	SubProcess::SubProcess(const char *c, Logger::Level out, Logger::Level err) : SubProcess("subprocess",c,out,err) {
	}

	void SubProcess::pre() {
	}

	void SubProcess::post(int) {
	}

	int SubProcess::run(const char *command, Logger::Level out, Logger::Level err) {
		return SubProcess{command,out,err}.run();
	}

	int SubProcess::run(const char *name, const char *command, Logger::Level out, Logger::Level err) {
		return SubProcess{name,command,out,err}.run();
	}

	int SubProcess::run(const NamedObject *obj, const char *command, Logger::Level out, Logger::Level err) {
		return SubProcess(obj,command,out,err).run();
	}

	void SubProcess::start(const char *command) {
		(new SubProcess(command))->start();
	}

	void SubProcess::start(const NamedObject *obj, const char *command) {
		(new SubProcess(obj, command))->start();
	}

	/// @brief Called on subprocess stdout.
	void SubProcess::onStdOut(const char *line) {
		if(Logger::enabled(loglevels.out)) {
			Logger::String{line}.write(loglevels.out,name());
		}
	}

	/// @brief Called on subprocess stderr.
	void SubProcess::onStdErr(const char *line) {
		if(Logger::enabled(loglevels.err)) {
			Logger::String{line}.write(loglevels.err,name());
		}
	}

	/// @brief Called on subprocess normal exit.
	void SubProcess::onExit(int rc) {
		if(rc) {
			Logger::String{"'",command,"' failed with rc=",rc}.error(name());
		} else {
			Logger::String{"'",command,"' complete with rc=",rc}.info(name());
		}
	}

	/// @brief Called on subprocess abnormal exit.
	void SubProcess::onSignal(int sig) {
#ifdef _WIN32
		error() << "'" << command << "' finishes with signal '" << sig << "'" << endl;
#else
		error() << "'" << command << "' finishes with signal '" << strsignal(sig) << "' (" << sig << ")" << endl;
#endif // _WIN32
	}

	static char *extract_delimiter(char *argument) {

		char marker = *(argument++);

		char *ptr = strrchr(argument,marker);
		if(!ptr) {
			throw runtime_error("Delimiter mismatch");
		}

		*ptr = 0;

		return argument;
	}

	char * SubProcess::get_next_argument(char **txtptr) {

		char *argument = chug(*txtptr);

		char *ptr = argument;
		while(*ptr && !isspace(*ptr)) {
			ptr++;
		}
		if(*ptr) {
			*(ptr++) = 0;
		}
		*txtptr = ptr;

		argument = strip(argument);

		debug(argument);
		if(*argument == '\'' || *argument == '"') {
			argument = extract_delimiter(argument);
		}
		debug(argument);

		return argument;
	}

 }

