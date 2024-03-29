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

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/win32/subprocess.h>

 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/logger.h>
 #include <udjat/win32/exception.h>

 using namespace std;

 namespace Udjat {

	static void PipeFactory(HANDLE &hRead, HANDLE &hWrite) {

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

	void SubProcess::init(Handler &outpipe, Handler &errpipe) {

		HANDLE hRead[2];
		HANDLE hWrite[2];

		PipeFactory(hRead[0],hWrite[0]);
		PipeFactory(hRead[1],hWrite[1]);

		try {

			STARTUPINFO siStartInfo;

			ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
			siStartInfo.cb = sizeof(STARTUPINFO);
			siStartInfo.hStdOutput = hWrite[0];
			siStartInfo.hStdError = hWrite[1];
			siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

			BOOL bSuccess = CreateProcess(	NULL,
											(LPSTR) command.c_str(),	// command line
											NULL,				// process security attributes
											NULL,				// primary thread security attributes
											TRUE,				// handles are inherited
											0,					// creation flags
											NULL,				// use parent's environment
											NULL,				// use parent's current directory
											&siStartInfo,		// STARTUPINFO pointer
											&piProcInfo);		// receives PROCESS_INFORMATION

			if(!bSuccess) {
				throw Win32::Exception(Logger::Message("Cant create process '{}'",command));
			}


		} catch(...) {

			CloseHandle(hRead[0]);
			CloseHandle(hWrite[0]);

			CloseHandle(hRead[1]);
			CloseHandle(hWrite[1]);

			throw;
		}

		// Close handles to the stdin and stdout pipes no longer needed by the child process.
		// If they are not explicitly closed, there is no way to recognize that the child process has ended.
		CloseHandle(hWrite[0]);
		CloseHandle(hWrite[1]);

		// Setup pipe readers.
		outpipe.set(hRead[0]);
		errpipe.set(hRead[1]);

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
