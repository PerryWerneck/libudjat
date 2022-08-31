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
 #include <udjat/tools/logger.h>
 #include <udjat/win32/exception.h>
 #include <system_error>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	int SubProcess::run() {

		throw system_error(ENOTSUP,system_category(),"Unable to run sync processes");

		/*
		init();

		while(pipes[0].hRead && pipes[1].hRead) {

			HANDLE lpHandles[sizeof(pipes)/sizeof(pipes[0] = { pipes[0].hRead, pipes[1].hRead };

			DWORD response = WaitForMultipleObjects(sizeof(lpHandles)/sizeof(lpHandles[0]),lpHandles,FALSE,500);

			switch(response) {
			case WAIT_OBJECT_0:
				read(0);
				break;

			case WAIT_OBJECT_0+1:
				read(1);
				break;

			case WAIT_FAILED:
				error() << "Error " << GetLastError() << " waiting for subprocess data" << endl;
				return -1;

			default:
				warning() << "Unexpected return " << response << " from WaitForMultipleObjects while waiting for subprocess data" << endl;
				return -1;

			}

		}

		DWORD rc = 0;

		if(GetExitCodeProcess(piProcInfo.hProcess,&rc) != STILL_ACTIVE) {
			onExit(rc);
			return rc;
		}

		error() << "Process still active after closing pipes" << endl;
		return -1;
		*/

	}

 }
