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
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/logger.h>

 using namespace std;

 #pragma GCC diagnostic ignored "-Wunused-parameter"

 namespace Udjat {

	SubProcess::SubProcess(const char *n, const char *c, Logger::Level out, Logger::Level err) : NamedObject{n}, command{c}, loglevels{out,err} {
		info() << "Running '" << command << "'" << endl;
		ZeroMemory(&piProcInfo,sizeof(piProcInfo));
	}

	SubProcess::~SubProcess() {

		if(piProcInfo.hProcess) {
			CloseHandle(piProcInfo.hProcess);
		}

		if(piProcInfo.hThread) {
			CloseHandle(piProcInfo.hThread);
		}

		debug("Subprocess was destroyed");

	}

 }
