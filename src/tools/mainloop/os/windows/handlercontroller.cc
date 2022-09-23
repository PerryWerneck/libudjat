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

	mutex Win32::Handler::Controller::guard;

	Win32::Handler::Controller & Win32::Handler::Controller::getInstance() {
		lock_guard<mutex> lock(guard);
		static Win32::Handler::Controller instance;
		return instance;
	}

	void Win32::Handler::Controller::insert(Win32::Handler *handler) {

		if(!handler->hEvent) {
			throw system_error(EINVAL,system_category(),"Cant watch 'NULLHANDLE'");
		}

		lock_guard<mutex> lock(guard);
		for(Worker *worker : workers) {

			if(worker->handlers.size() < MAXIMUM_WAIT_OBJECTS) {
				worker->handlers.push_back(handler);
				return;
			}

		}

		// Not found, alloc a new worker.
		workers.push_back(new Worker(handler));

	}

	void Win32::Handler::Controller::remove(Handler *handler) {
#ifdef DEBUG
		cout << "win32\tRemoving handler " << hex << ((unsigned long long) handler->hEvent) << dec << endl;
#endif // DEBUG
		lock_guard<mutex> lock(guard);
		workers.remove_if([handler](Worker *worker){
			worker->handlers.remove_if([handler](const Handler *h) {
				return handler == h;
			});
			return worker->handlers.empty();
		});
#ifdef DEBUG
		cout << "win32\tHandçer " << hex << ((unsigned long long) handler->hEvent) << dec << " was removed" << endl;
#endif // DEBUG
	}

	Win32::Handler * Win32::Handler::Controller::find(HANDLE handle) noexcept {

		lock_guard<mutex> lock(guard);
		for(Worker *worker : workers) {
			for(Handler *handler : worker->handlers) {
				if(handler->hEvent == handle) {
					return handler;
				}
			}
		}

		return nullptr;

	}

	Win32::Handler * Win32::Handler::Controller::find(Worker *worker, HANDLE handle) noexcept {

		lock_guard<mutex> lock(guard);

		for(auto handler : worker->handlers) {
			if(handler->hEvent == handle) {
				return handler;
			}
		}

		return nullptr;
	}

	void Win32::Handler::Controller::call(HANDLE handle, bool abandoned) noexcept {

		Win32::Handler * handler = find(handle);
		if(handler) {

			try {

				handler->handle(abandoned);

			} catch(const std::exception &e) {
				cerr << "Win32\tError '" << e.what() << "' processing event handler" << endl;
			} catch(...) {
				cerr << "Win32\tUnexpected error processing event handler" << endl;
			}

		}

	}

	bool Win32::Handler::Controller::wait(Worker *worker) noexcept {

		DWORD nCount = 0;
		HANDLE *lpHandles = nullptr;

		{
			lock_guard<mutex> lock(guard);
			nCount = (DWORD) worker->handlers.size();

			if(!nCount) {
				return false;
			}

			lpHandles = new HANDLE[nCount+1];
			lpHandles[nCount] = 0; // Just in case.

			DWORD ix = 0;
			for(auto handler : worker->handlers) {
				lpHandles[ix++] = handler->hEvent;
			}
			nCount = ix;

		}

#ifdef DEBUG
		cout << "Waiting for " << nCount << " handlers" << endl;
#endif // DEBUG

		// https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects
		DWORD response = WaitForMultipleObjects(nCount,lpHandles,FALSE,2000);

		if(response >= WAIT_ABANDONED_0 && response < (WAIT_ABANDONED_0+nCount)) {

			// Abandoned.
			call(lpHandles[response - WAIT_ABANDONED_0],true);

		} else if(response >= WAIT_OBJECT_0 && response < WAIT_OBJECT_0+nCount) {

			// Signaled.
			call(lpHandles[response - WAIT_OBJECT_0],false);

		} else if(response == WAIT_FAILED) {

			DWORD errcode = GetLastError();
			if(errcode) {
				cerr << "win32\tWaitForMultipleObjects has failed with error '" << Win32::Exception::format(errcode) << "' (" << errcode << ")" << endl;
			}

		}

		delete[] lpHandles;
		return true;

	}

 }
