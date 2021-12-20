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
  * @brief Implement the Windows SubProcess object.
  *
  * References:
  *
  * <https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output>
  *
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/subprocess.h>
 #include <system_error>

 using namespace std;

 #pragma GCC diagnostic ignored "-Wunused-parameter"

 namespace Udjat {

	SubProcess::SubProcess(const char *c) {
		throw system_error(ENOTSUP,system_category(),"Not available on windows");
	}

	SubProcess::~SubProcess() {
	}

	void SubProcess::start(const char *command) {
	}

	/// @brief Called on subprocess stdout.
	void SubProcess::onStdOut(const char *line) {
	}

	/// @brief Called on subprocess stderr.
	void SubProcess::onStdErr(const char *line) {
	}

	/// @brief Called on subprocess normal exit.
	void SubProcess::onExit(int rc) {
	}

	/// @brief Called on subprocess abnormal exit.
	void SubProcess::onSignal(int sig) {
	}

	void SubProcess::start() {
	}

	void SubProcess::read(int id) {
	}

 }
