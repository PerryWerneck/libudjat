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
 #include <udjat/win32/handler.h>
 #include <udjat/win32/exception.h>
 #include <udjat/tools/threadpool.h>

 using namespace std;

 namespace Udjat {

	Win32::Handler::~Handler() {
		close();
	}

	void Win32::Handler::enable() {
		Controller::getInstance().insert(this);
	}

	void Win32::Handler::disable() {
		Controller::getInstance().remove(this);
	}

	void Win32::Handler::close() {
		disable();
		if(hEvent) {
			CloseHandle(hEvent);
			hEvent = 0;
		}
	}

	void Win32::Handler::set(HANDLE handle) {

		if(!handle) {
			throw system_error(EINVAL,system_category(),"Invalid handle");
		}

		if(hEvent) {
			throw system_error(EBUSY,system_category(),"Handle is already set");
		}

		hEvent = handle;

	}

	ssize_t Win32::Handler::read(void *buf, size_t count) {

		DWORD dwRead = 0;

		if(!ReadFile(hEvent,buf,count,&dwRead,NULL)) {
			if(GetLastError() == ERROR_BROKEN_PIPE) {
				close();
				errno = EPIPE;
				return 0;
			}
			return -1;
		}

		return dwRead;

	}

	size_t Win32::Handler::poll(Win32::Handler **handlers, size_t nfds, int timeout) {

		size_t valid_handlers = 0;
//		DWORD nCount = 0;
		HANDLE lpHandles[nfds];
		Handler *index[nfds];

		for(size_t ix = 0; ix < nfds; ix++) {

			if(handlers[ix]->hEvent != 0) {
				index[valid_handlers] = handlers[ix];
				lpHandles[valid_handlers] = handlers[ix]->hEvent;
				valid_handlers++;
			}

		}

		if(valid_handlers) {

			DWORD response = WaitForMultipleObjects(valid_handlers,lpHandles,FALSE,timeout);

			if(response >= WAIT_ABANDONED_0 && response < (WAIT_ABANDONED_0+valid_handlers)) {

				// Abandoned.
				index[response - WAIT_ABANDONED_0]->handle(true);

			} else if(response >= WAIT_OBJECT_0 && response < WAIT_OBJECT_0+valid_handlers) {

				// Signaled.
				index[response - WAIT_OBJECT_0]->handle(false);

			} else if(response == WAIT_FAILED) {

				throw Win32::Exception();

			}

		}

		return valid_handlers;
	}

 }
