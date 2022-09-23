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

 #include "private.h"
 #include <udjat/tools/threadpool.h>

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

			ThreadPool::getInstance().push("subprocess-cleanup",[this](){

				disable();

				out->disable();
				err->disable();

				Sleep(100);

				// Flush streams
				{
					Win32::Handler *hdl[]{out.get(),err.get()};
					while(Win32::Handler::poll(hdl,3,1000));
				}

				close();

				proc->onExit(proc->exitcode);
				delete this;

			});
		}

	}

 }
