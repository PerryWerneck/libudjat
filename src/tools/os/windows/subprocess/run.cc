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

		class SyncHandler : public SubProcess::Handler {
		private:
			SubProcess &proc;

		protected:

			void on_error(const char *reason) override {
				proc.onStdErr(reason);
			}

			void on_input(const char *line) override {
				if(id == 0) {
					proc.onStdOut(line);
				} else {
					proc.onStdErr(line);
				}
			}

		public:
			SyncHandler(SubProcess *p, unsigned short id) : SubProcess::Handler(id), proc(*p) {
			}

		};

		SyncHandler handlers[]{ { this,0 }, { this,1}  };

		/*
		init();

		while(running()) {

			HANDLE lpHandles[ (sizeof(pipes)/sizeof(pipes[0]))+1 ];
			DWORD nCount = 0;

			for(size_t pipe = 0; pipe < (sizeof(pipes)/sizeof(pipes[0])); pipe++) {
				if(pipes[pipe].hRead) {
					lpHandles[nCount++] = pipes[pipe].hRead;
				}
			}

			if(piProcInfo.hProcess) {
				lpHandles[nCount++] = piProcInfo.hProcess;
			}

			DWORD response = WaitForMultipleObjects(nCount,lpHandles,FALSE,1000);
			HANDLE handle = 0;

			if(response >= WAIT_OBJECT_0 && response < WAIT_OBJECT_0+nCount) {

				handle = lpHandles[response - WAIT_OBJECT_0];

			} else if(response >= WAIT_ABANDONED_0 && response < (WAIT_ABANDONED_0+nCount)) {

				handle = lpHandles[response - WAIT_ABANDONED_0];

			} else if(response == WAIT_FAILED) {

				throw Win32::Exception();

			} else if(response == WAIT_TIMEOUT) {

				continue;

			}

			if(handle == piProcInfo.hProcess) {

				DWORD rc;
				if(GetExitCodeProcess(piProcInfo.hProcess,&rc) != STILL_ACTIVE) {
					onExit(exitcode = rc);
					CloseHandle(piProcInfo.hProcess);
					piProcInfo.hProcess = 0;
				}

			} else {

				for(size_t pipe = 0; pipe < (sizeof(pipes)/sizeof(pipes[0])); pipe++) {
					if(handle == pipes[pipe].hRead) {
						read(pipe);
					}
				}

			}

		}
		*/

		return exitcode;

	}

 }
