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

/**
 * @file
 *
 * @brief Implement windows main loop.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include "private.h"
 #include <private/misc.h>
 #include <private/win32/mainloop.h>
 #include <iostream>
 #include <udjat/tools/logger.h>
 #include <udjat/win32/exception.h>

 using namespace std;

 int Udjat::Win32::MainLoop::run() {

	int rc = -1;

	if(!PostMessage(hwnd,WM_START,0,0)) {
		cerr << "MainLoop\tError posting WM_START message to " << hex << hwnd << dec << " : " << Win32::Exception::format() << endl;
		return -1;
	}

	{
		MSG msg;
		memset(&msg,0,sizeof(msg));

		Logger::String("Running message loop").write((Logger::Level) (Logger::Debug+1),"win32");

		running = true;
		while( (rc = GetMessage(&msg, NULL, 0, 0)) > 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		running = false;

		Logger::String("Message loop ends with rc=",rc).write((Logger::Level) (Logger::Debug+1),"win32");
	}

	return rc;

 }

