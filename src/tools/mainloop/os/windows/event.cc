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


	void MainLoop::insert(HANDLE handle, std::function<bool(HANDLE handle,bool abandoned)> exec) {

		class Event : public Win32::Event {
		private:
			std::function<bool(HANDLE,bool)> exec;

		public:
			Event(HANDLE handle, std::function<bool(HANDLE,bool)> e) : Win32::Event(handle), exec(e) {
				start();
			}

			bool handle(bool abandoned) override {
				return exec(hEvent,abandoned);
			}

		};

		new Event(handle,exec);
	}

	void MainLoop::remove(HANDLE handle) {
		Win32::Event *event = Win32::Event::Controller::getInstance().find(handle);
		if(event) {
			delete event;
		}
	}

	void Win32::Event::start() {
		Controller::getInstance().insert(this);
	}

	Win32::Event::~Event() {
		Controller::getInstance().remove(this);
	}

 }
