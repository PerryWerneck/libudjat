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
 #include "private.h"
 #include <udjat/win32/exception.h>
 #include <udjat/tools/threadpool.h>

 using namespace std;

 namespace Udjat {


	void MainLoop::insert(HANDLE handle, std::function<void(HANDLE handle,bool abandoned)> exec) {
		new Win32::Event(handle,exec);
	}

	void MainLoop::remove(HANDLE handle) {
		Win32::Event *event = Win32::Event::Controller::getInstance().find(handle);
		if(event) {
			delete event;
		}
	}

	Win32::Event::Event(HANDLE h, std::function<void(HANDLE,bool)> e) : handle(h),exec(e) {
		Controller::getInstance().insert(this);
	}

	Win32::Event::~Event() {
		Controller::getInstance().remove(this);
	}

	/*
	void Win32::Event::abandon() noexcept {

		try {

			exec(handle,true);

		} catch(const exception &e) {

			cerr << "win32\tError '" << e.what() << "' abandoning event" << endl;

		} catch(...) {

			cerr << "win32\tUnexpected error abandoning event" << endl;

		}
	}

	void Win32::Event::call() noexcept {
		HANDLE h = this->handle;
		std::function<void(HANDLE,bool)> e = this->exec;

		ThreadPool::getInstance().push([h,e]() {
			try {

				e(h,false);

			} catch(const exception &e) {

				cerr << "win32\tError '" << e.what() << "' running event processor" << endl;

			} catch(...) {

				cerr << "win32\tUnexpected error running event processor" << endl;

			}

		});
	}
	*/

 }
