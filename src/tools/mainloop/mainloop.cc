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
 #include <private/mainloop.h>
 #include <private/misc.h>

 #ifdef _WIN32
	#include <udjat/win32/exception.h>
 #endif // _WIN32

 namespace Udjat {

	std::mutex MainLoop::guard;

	void MainLoop::quit() {
#ifdef _WIN32
		if(!PostMessage(hwnd,WM_STOP,0,0)) {
			cerr << "MainLoop\tError posting WM_STOP message to " << hex << hwnd << dec << " : " << Win32::Exception::format() << endl;
		}
#else
		enabled = false;
		wakeup();
#endif // _WIN32
	}


 }

