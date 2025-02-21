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
 #include <private/misc.h>
 #include <iostream>
 #include <udjat/tools/logger.h>

 #ifdef _WIN32
	#include <udjat/win32/exception.h>
	#include <private/win32/mainloop.h>
 #else
	#include <private/linux/mainloop.h>
 #endif // _WIN32

 #ifdef HAVE_SYSTEMD
	#include <systemd/sd-daemon.h>
 #endif // HAVE_SYSTEMD

 using namespace std;

 namespace Udjat {

	MainLoop::MainLoop(MainLoop::Type type) : mtype{type} {
		if(instance) {
			throw system_error(EBUSY,system_category(),"Mainloop was already set");
		}
		instance = this;
	}

	MainLoop::~MainLoop() {
		if(instance == this) {
			instance = nullptr;
		}
	}

#ifdef _WIN32
	void Win32::MainLoop::quit() {
		if(!PostMessage(hwnd,WM_STOP,0,0)) {
			cerr << "win32\tError posting WM_STOP message to " << hex << hwnd << dec << " : " << Win32::Exception::format() << endl;
		}
	}

	void Win32::MainLoop::quit(const char *message) {
		if(!PostMessage(hwnd,WM_STOP_WITH_MESSAGE,(WPARAM) (Logger::Debug+1),(LPARAM)(LPCTSTR)message)) {
			cerr << "win32\tError posting WM_STOP message('" << message << "') to " << hex << hwnd << dec << " : " << Win32::Exception::format() << endl;
		}
	}
#else
	void Linux::MainLoop::quit() {
		running = false;
		wakeup();
	}

#endif // _WIN32

	void MainLoop::quit(const char *message) {
#ifdef HAVE_SYSTEMD
		sd_notifyf(0,"STOPPING=1\nSTATUS=%s",message);
#endif // HAVE_SYSTEMD
		Logger::String{message}.write((Logger::Level) (Logger::Debug+1),"MainLoop");
		quit();
	}

 }

