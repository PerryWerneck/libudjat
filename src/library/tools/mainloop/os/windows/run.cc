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
 #include <udjat/tools/timer.h>

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

		while( (rc = GetMessage(&msg, NULL, 0, 0)) > 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		Logger::String("Message loop ends with rc=",rc).write((Logger::Level) (Logger::Debug+1),"win32");
	}

	return rc;

 }

 unsigned long Udjat::Win32::MainLoop::compute_poll_timeout() noexcept {

	unsigned long now = MainLoop::Timer::getCurrentTime();
	unsigned long next = now + timers.maxwait;

	// Get expired timers.
	std::list<Timer *> expired;
	for_each([&expired,&next,now](Timer &timer){
		if(timer.activation_time() <= now) {
			expired.push_back(&timer);
		} else {
			next = std::min(next,timer.activation_time());
		}
		return false;
	});

	// Run expired timers.
	for(auto timer : expired) {
		unsigned long n = timer->activate();
		if(n) {
			next = std::min(next,n);
		}
	}

	if(next > now) {
		debug("Time interval ",(next-now)," ms (",TimeStamp{time(0) + ((time_t) ((next-now)/1000))}.to_string(),")");
		return (next - now);
	}

	Logger::String{"Unexpected interval '",((int) (next - now)),"' on timer processing, using default"}.error();

	return timers.maxwait;
 }
