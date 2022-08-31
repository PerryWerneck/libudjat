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
 #include <udjat/tools/mainloop.h>

 using namespace std;

 namespace Udjat {

	mutex Win32::Event::Controller::guard;

	Win32::Event::Controller & Win32::Event::Controller::getInstance() {
		lock_guard<mutex> lock(guard);
		static Win32::Event::Controller instance;
		return instance;
	}

	void Win32::Event::Controller::insert(Event *event) {
		lock_guard<mutex> lock(guard);
		for(auto worker = workers.begin(); worker != workers.end(); worker++) {

			if(worker->events.size() < MAXIMUM_WAIT_OBJECTS) {
				worker->events.push_back(event);
				return;
			}

		}

		// Not found, alloc a new worker.
		workers.emplace_back(event);

	}

	void Win32::Event::Controller::remove(Event *event) {
#ifdef DEBUG
		cout << "win32\tRemoving event " << hex << ((unsigned long long) event->hEvent) << dec << endl;
#endif // DEBUG
		lock_guard<mutex> lock(guard);
		workers.remove_if([event](Worker &worker){
			worker.events.remove_if([event](const Event *e) {
				return event == e;
			});
			return worker.events.empty();
		});
#ifdef DEBUG
		cout << "win32\tEvent " << hex << ((unsigned long long) event->hEvent) << dec << " was removed" << endl;
#endif // DEBUG
	}

	Win32::Event * Win32::Event::Controller::find(HANDLE handle) noexcept {

		lock_guard<mutex> lock(guard);
		for(Worker &worker : workers) {
			for(auto event : worker.events) {
				if(event->hEvent == handle) {
					return event;
				}
			}
		}

		return nullptr;

	}

	Win32::Event * Win32::Event::Controller::find(Worker *worker, HANDLE handle) noexcept {

		lock_guard<mutex> lock(guard);

		for(auto event : worker->events) {
			if(event->hEvent == handle) {
				return event;
			}
		}

		return nullptr;
	}

	bool Win32::Event::Controller::wait(Worker *worker) noexcept {
		DWORD nCount = 0;
		HANDLE *lpHandles = nullptr;

		{
			lock_guard<mutex> lock(guard);
			nCount = (DWORD) worker->events.size();

			if(!nCount) {
				// TODO: Enqueue worker cleanup.
				return false;
			}

			lpHandles = new HANDLE[nCount+1];
			lpHandles[nCount] = 0; // Just in case.

			DWORD ix = 0;
			for(auto event : worker->events) {
				lpHandles[ix++] = event->hEvent;
			}

		}

		// https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects
		DWORD response = WaitForMultipleObjects(nCount,lpHandles,FALSE,500);
		if(response >= WAIT_ABANDONED_0 && response < (WAIT_ABANDONED_0+nCount)) {

			// Abandoned.
			if(!MainLoop::getInstance().post(WM_EVENT_ACTION,1,(LPARAM) lpHandles[response - WAIT_ABANDONED_0])) {
				cerr << "win32\tError " << GetLastError() << " posting WM_EVENT_ACTION" << endl;
			}

		} else if(response >= WAIT_OBJECT_0 && response < WAIT_OBJECT_0+nCount) {

			// Signaled.
#ifdef DEBUG
			cout <<  __FUNCTION__ << "(" << __LINE__ << ") posting WM_EVENT_ACTION" << endl;
#endif //

			if(!MainLoop::getInstance().post(WM_EVENT_ACTION,0,(LPARAM) lpHandles[response - WAIT_OBJECT_0])) {
				cerr << "win32\tError " << GetLastError() << " posting WM_EVENT_ACTION" << endl;
			}

		} else if(response == WAIT_FAILED) {

			DWORD errcode = GetLastError();

			if(errcode != ERROR_INVALID_HANDLE) {
				cerr << "win32\tWaitForMultipleObjects has failed with error '" << Win32::Exception::format(errcode) << "' (" << errcode << ")" << endl;

				if(errcode) {
					return false;
				}
			}

		}

		delete[] lpHandles;
		return true;

	}

 }
