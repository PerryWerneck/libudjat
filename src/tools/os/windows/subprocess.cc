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

	SubProcess::SubProcess(const char *c) : command(c) {
		ZeroMemory(&piProcInfo,sizeof(piProcInfo));
	}

	SubProcess::~SubProcess() {

		if(piProcInfo.hProcess) {
			CloseHandle(piProcInfo.hProcess);
		}

		if(piProcInfo.hThread) {
			CloseHandle(piProcInfo.hThread);
		}
	}

	void SubProcess::init() {

		STARTUPINFO siStartInfo;

		ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
		siStartInfo.cb = sizeof(STARTUPINFO);
		siStartInfo.hStdOutput = pipes[0].hWrite;
		siStartInfo.hStdError = pipes[1].hWrite;
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
			throw runtime_error("Cant create process");
		}

		// Close handles to the stdin and stdout pipes no longer needed by the child process.
		// If they are not explicitly closed, there is no way to recognize that the child process has ended.
		CloseHandle(pipes[0].hWrite);
		pipes[0].hWrite = 0;

		CloseHandle(pipes[1].hWrite);
		pipes[1].hWrite = 0;


	}

	void SubProcess::start() {

		throw system_error(ENOTSUP,system_category(),"Cant start Win32 subprocess (yet)");

	}

	int SubProcess::run() {

		throw system_error(ENOTSUP,system_category(),"Cant run Win32 subprocess (yet)");

	}

 }
