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
 #include "private.h"
 #include <memory>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/handler.h>

 /*
 #include <udjat/defs.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/logger.h>
 #include <udjat/win32/exception.h>
 #include <udjat/win32/event.h>
 #include <system_error>
 #include <iostream>
 */

 using namespace std;

 namespace Udjat {

	void SubProcess::start() {

		class ASyncHandler : public SubProcess::Handler {
		private:
			shared_ptr<SubProcess> proc;

		protected:

			void on_error(const char *reason) override {
				proc->onStdErr(reason);
			}

			void on_input(const char *line) override {
				if(id == 0) {
					proc->onStdOut(line);
				} else {
					proc->onStdErr(line);
				}
			}

		public:
			ASyncHandler(shared_ptr<SubProcess> p, unsigned short id) : SubProcess::Handler(id), proc(p) {
			}

		};


		if(!MainLoop::getInstance()) {
			delete this;
			throw runtime_error("Cant start async subprocess without an active main loop");
		}

		auto proc = shared_ptr<SubProcess>{this};
		auto out = make_shared<ASyncHandler>(proc,0);
		auto err = make_shared<ASyncHandler>(proc,1);

		init(*out,*err);

		if(*out && *err) {
			new Watcher(proc,out,err);
		}

		/*
		init();

		new Watcher(this,0);
		new Watcher(this,1);

		MainLoop::insert(piProcInfo.hProcess, [this](HANDLE handle,bool abandoned) {
			DWORD rc;
			if(GetExitCodeProcess(piProcInfo.hProcess,&rc) != STILL_ACTIVE) {
				onExit(exitcode = rc);
				CloseHandle(piProcInfo.hProcess);
				piProcInfo.hProcess = 0;
				if(!running()) {
					delete this;
				}
				return false;
			}
			return true;
		});
		*/

	}

 }
