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
		lock_guard<mutex> lock(guard);
		for(auto worker = workers.begin(); worker != workers.end(); worker++) {
			worker->events.remove_if([event](const Event *e) {
				return event == e;
			});
		}
	}

	Win32::Event::Event(HANDLE h) : handle(h) {
		Controller::getInstance().insert(this);
	}

	Win32::Event::~Event() {
		Controller::getInstance().remove(this);
	}

	Win32::Event::Controller::Worker::Worker(Win32::Event *event) {

		this->hThread = new thread([this]() {
#ifdef DEBUG
			cout << "win32\tStarting event monitor thread" << endl;
#endif // DEBUG

		while(enabled && Controller::getInstance().wait(this) {
		}

#ifdef DEBUG
			cout << "win32\tStopping event monitor thread" << endl;
#endif // DEBUG
		});
	}

	Win32::Event::Controller::Worker::~Worker() {
#ifdef DEBUG
		cout << "win32\tStopping event monitor" << endl;
#endif // DEBUG
		enabled = false;
		hThread->join();
		delete hThread;

	}

	bool Win32::Event::Controller::wait(Worker *worker) {
		DWORD nCount = 0;
		const HANDLE *lpHandles = nullptr;

		{
			lock_guard<mutex> lock(guard);
			nCount = (DWORD) worker.events.size();

			if(!nCount) {
				// TODO: Enqueue worker cleanup.
				return false;
			}

			lpHandles = malloc((lpHandles+1) * sizeof(HANDLE *));

			DWORD ix = 0;
			for(event in worker.events) {
				lpHandles[ix++] = event->handle;
			}

		}

		// https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects
		DWORD response = WaitForMultipleObjects(nCount,lpHandles,FALSE,500);
		if(response >= WAIT_ABANDONED_0 && response < (WAIT_ABANDONED_0+nCount)) {

			// Abandoned.
			size_t index = response - WAIT_ABANDONED_0;

		} else if(response >= WAIT_OBJECT_0 && response < WAIT_OBJECT_0+nCount) {

			// Object was signaled.
			size_t index = response - WAIT_OBJECT_0;

		} else if(response == WAIT_FAILED) {

			cerr << "win32\t" << Win32::Exception::format() << endl;
			return false;

		}

		free(lpHandles);
		return true;

	}

 }
