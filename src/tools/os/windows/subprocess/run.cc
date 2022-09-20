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
 #include "private.h"
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

		class ProcHandler : public Win32::Handler {
		private:
			SubProcess &proc;

		public:
			ProcHandler(SubProcess *p, HANDLE h) : Win32::Handler(h), proc(*p) {
			}

			void handle(bool UDJAT_UNUSED(abandoned)) override {
				DWORD rc;
				if(GetExitCodeProcess(this->hEvent,&rc) != STILL_ACTIVE) {
					proc.onExit(proc.exitcode = rc);
					close();
					proc.piProcInfo.hProcess = 0;
				}
			}

		};

		SyncHandler handlers[]{ { this,0 }, { this,1 } };

		init(handlers[0],handlers[1]);

		ProcHandler prochandler{this,piProcInfo.hProcess};

		Win32::Handler *hdl[]{&handlers[0],&handlers[1],&prochandler};

		// Wait for child.
		while(Win32::Handler::poll(hdl,3,1000));

		return exitcode;

	}

 }
