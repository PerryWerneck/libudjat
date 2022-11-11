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

 #pragma GCC diagnostic ignored "-Wunused-parameter"

 namespace Udjat {

	/*
	SubProcess::Pipe::Pipe() {
		SECURITY_ATTRIBUTES saAttr;

		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		if(!CreatePipe(&hRead, &hWrite, &saAttr, 0)) {
			throw runtime_error("Error creating pipe");
		}

		// Ensure the read handle to the pipe is not inherited.
		if(!SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0)) {
			CloseHandle(hRead);
			CloseHandle(hWrite);
			throw runtime_error("Error in SetHandleInformation");
		}

	}

	SubProcess::Pipe::~Pipe() {
		if(hRead)
			CloseHandle(hRead);

		if(hWrite)
			CloseHandle(hWrite);

	}
	*/

	SubProcess::SubProcess(const char *n, const char *c) : NamedObject(n),command(c) {
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

#ifdef DEBUG
		info() << "Subprocess was destroyed" << endl;
#endif // DEBUG
	}

	/*
	bool SubProcess::read(int id) {

		DWORD dwRead = 0;

		BOOL bSuccess =
				ReadFile(
					pipes[id].hRead,
					pipes[id].buffer+pipes[id].length,
					sizeof(pipes[id].buffer) - (pipes[id].length+1),
					&dwRead,
					NULL
				);

		if(!bSuccess) {

			DWORD errcode = GetLastError();

			if(errcode != ERROR_BROKEN_PIPE) {
				error() << "Error '" << Win32::Exception::format(errcode) << "' reading from subprocess" << endl;
			}

#ifdef DEBUG
			info() << "Pipe " << id << " was closed" << endl;
#endif // DEBUG

			CloseHandle(pipes[id].hRead);
			pipes[id].hRead = 0;
			return false;

		} else if(dwRead) {

			pipes[id].buffer[pipes[id].length+dwRead] = 0;
			parse(id);

		} else {
			warning() << "Empty data from subprocess" << endl;
		}

		return true;
	}
	*/


 }
