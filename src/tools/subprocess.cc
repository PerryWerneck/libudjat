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
 #include <iostream>
 #include <udjat/tools/threadpool.h>

 using namespace std;

 namespace Udjat {

	int SubProcess::run(const char *command) {
		return SubProcess(command).run();
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

 }
