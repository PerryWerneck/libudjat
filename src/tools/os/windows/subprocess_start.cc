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
 #include <udjat/win32/event.h>
 #include <udjat/tools/mainloop.h>
 #include <system_error>
 #include <iostream>
 #include <memory>

 using namespace std;

 namespace Udjat {

	class UDJAT_PRIVATE SubProcess::Watcher : public Win32::Event {
	private:
		SubProcess * process;
		int id;

	public:
		Watcher(SubProcess *p, int i) : Win32::Event(p->pipes[i].hRead), process(p), id(i) {
#ifdef DEBUG
			process->info() << " *** Watcher " << id << " is starting" << endl;
#endif // DEBUG
			start();
		}

		virtual ~Watcher() {
#ifdef DEBUG
			process->info() << " *** Watcher " << id << " was destroyed" << endl;
#endif // DEBUG
		}

		bool handle(bool UDJAT_UNUSED(abandoned)) override {
			process->info() << " *** Activity on watcher " << id << endl;
			return process->read(id);
		}

	};

	/*
	class UDJAT_PRIVATE SubProcess::Proxy {
	private:
		SubProcess *process;

	public:

		constexpr Proxy(SubProcess *p) : process(p) {
		}

		~Proxy() {

#ifdef DEBUG
			process->warning() << "*** Proxy was deleted, cleaning" << endl;
#endif // DEBUG

			{
				DWORD rc = 0;
				if(GetExitCodeProcess(process->piProcInfo.hProcess,&rc) != STILL_ACTIVE) {
					process->onExit(rc);
				} else {
					process->warning() << "Still active, no more pipes" << endl;
				}
			}

			delete process;
		}

		bool read(int id) {
			process->read(id);
			return process->pipes[id].hRead != 0;
		}
	};
	*/

	void SubProcess::start() {

		if(!MainLoop::getInstance()) {
			delete this;
			throw runtime_error("Cant start win32 async subprocess without an active main loop");
		}

		init();

		new Watcher(this,0);
		new Watcher(this,1);

		/*
		MainLoop::insert(pipes[0].hRead, [this](HANDLE handle,bool abandoned) {
			return read(0);
		});

		MainLoop::insert(pipes[1].hRead, [this](HANDLE handle,bool abandoned) {
			return read(1);
		});
		*/

	}

 }
