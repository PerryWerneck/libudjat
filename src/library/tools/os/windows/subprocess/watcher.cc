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
 #include <udjat/tools/threadpool.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	SubProcess::Watcher::Watcher(shared_ptr<SubProcess> p, shared_ptr<Win32::Handler> o, shared_ptr<Win32::Handler> e) : Win32::Handler{p->piProcInfo.hProcess}, proc{p}, out{o}, err{e} {

		out->enable();
		err->enable();

		enable();

	}

	SubProcess::Watcher::~Watcher() {
	}

	void SubProcess::Watcher::handle(bool UDJAT_UNUSED(abandoned)) {

		if(GetExitCodeProcess(proc->piProcInfo.hProcess,&proc->exitcode) != STILL_ACTIVE) {

			disable();

			ThreadPool::getInstance().push("subprocess-cleanup",[this](){

				out->disable();
				err->disable();

				Sleep(100);

				// Flush streams
				{
					Win32::Handler *hdl[]{out.get(),err.get()};

#ifdef DEBUG
					cout << "Waiting for streams begin ----------------------------- " << __FILE__ << "(" << __LINE__ << ")" << endl;
#endif // DEBUG

					while(Win32::Handler::poll(hdl,2,1000));

#ifdef DEBUG
					cout << "Waiting for streams end ----------------------------- " << __FILE__ << "(" << __LINE__ << ")" << endl;
#endif // DEBUG
				}

				close();

				proc->onExit(proc->exitcode);
				delete this;

			});
		}

	}

 }
