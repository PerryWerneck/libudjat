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
		trace() << line << endl;
	}

	/// @brief Called on subprocess stderr.
	void SubProcess::onStdErr(const char *line) {
		error() << line << endl;
	}

	/// @brief Called on subprocess normal exit.
	void SubProcess::onExit(int rc) {
		if(rc) {
			error() <<  "'" << command << "' fails with rc=" << rc << endl;
		} else {
			info() <<  "'" << command << "' ends" << endl;
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

	SubProcess::Arguments & SubProcess::Arguments::push_back(const char *arg) noexcept {
		if(args.values) {
			args.values = (char **) realloc(args.values,(args.count+2) * sizeof(const char *));
		} else {
			args.values = (char **) malloc(sizeof(const char *) * 2);
			args.count = 0;
		}
		args.values[args.count++] = strdup(arg);
		args.values[args.count] = NULL;
		return *this;
	}

	SubProcess::Arguments & SubProcess::Arguments::push_back(const std::string &value) noexcept {
		push_back(value.c_str());
		return *this;
	}

	SubProcess::Arguments::~Arguments() {
		for(size_t ix = 0; ix < args.count; ix++) {
			free(args.values[ix]);
		}
		free(args.values);
	}

 }

 namespace std {

 	string to_string(const Udjat::SubProcess::Arguments &opt, const char *sep) {
		const char **arg = opt.argv();
		string result{*(arg++)};

		while(*arg) {
			result += sep;
			result += *arg;
			arg++;
		}
		return result;
	}

 }
